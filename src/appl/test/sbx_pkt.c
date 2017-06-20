/*
 * $Id: sbx_pkt.c,v 1.38 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx_pkt.c
 * Purpose:     FE2K Packet Class functions
 * Requires:
 */

#ifdef BCM_SBX_SUPPORT

#include <soc/defs.h>
#include "c3sw_test.h"
#include <appl/test/sbx_pkt.h>
#include <soc/sbx/g3p1/g3p1_int.h>

int pkt_verbose = 0;

int sbxpkt_cmic_port(int unit) {
    return CMIC_PORT(unit);
}

static uint32 string_to_int (char *value) {
  unsigned long int lvalue;
  lvalue = strtoul(value,0,0);
  return ((uint32)(lvalue));
}
  
static int string_to_mac (char *value, uint8 *mac) {
  int i;
  int j=0;
  int k=0;
  char data[6];
  
  for (i=0; i<32; i++) {
    if (value[i] == '\0') {
       sal_memcpy (data, &value[j], i-j);
       data[2] = 0; /* terminate string */
       mac[k] = strtol(data,0,16);
       break;
    }
    if (value[i] == ':') {
       sal_memcpy (data, &value[j], i-j);
       data[2] = 0;  /* terminate string */
       mac[k] = strtol(data,0,16);
       j = i+1;
       k++;
    }
  }
  return 0;
}

static int string_to_ipv6 (char *value, uint16 *ipv6) {
  int i;
  int j=0;
  int k=0;
  char data[6];
  
  for (i=0; i<40; i++) {
    if (value[i] == '\0') {
       sal_memcpy (data, &value[j], i-j);
       data[4] = 0; /* terminate string */
       ipv6[k] = strtol(data,0,16);
       break;
    }
    if (value[i] == ':') {
       sal_memcpy (data, &value[j], i-j);
       data[4] = 0;  /* terminate string */
       ipv6[k] = strtol(data,0,16);
       j = i+1;
       k++;
    }
  }
  return 0;
}


static int string_to_hex (char *value, uint8 *hex) {
  char data[3];

  sal_memcpy (data, value, 2);
  data[2] = 0; /* terminate string */
  *hex = strtol(data,0,16);

  return 0;
}

static int string_to_maid (char *value, uint8 *maid) {
  int i;
  int j=0;
  int k=0;
  char data[48];
  
  for (i=0; i<150; i++) {
    if (value[i] == '\0') {
       sal_memcpy (data, &value[j], i-j);
       data[2] = 0; /* terminate string */
       maid[k] = strtol(data,0,16);
       break;
    }
    if (value[i] == ':') {
       sal_memcpy (data, &value[j], i-j);
       data[2] = 0;  /* terminate string */
       maid[k] = strtol(data,0,16);
       j = i+1;
       k++;
    }
  }
  return 0;
}

static int stuff_data (raw_data_t *raw_data) {
  int i;

  if (raw_data->mode == 0) {
    for (i=0; i < (MAXLEN/4); i++) {
      raw_data->raw_data[(i*4)+0] = ((raw_data->value + i) >> 24) & 0xff;
      raw_data->raw_data[(i*4)+1] = ((raw_data->value + i) >> 16) & 0xff;
      raw_data->raw_data[(i*4)+2] = ((raw_data->value + i) >> 8) & 0xff;
      raw_data->raw_data[(i*4)+3] = ((raw_data->value + i) >> 0) & 0xff;
    }
  } else {
    for (i=0; i < (MAXLEN/4); i++) {
      raw_data->raw_data[(i*4)+0] = ((raw_data->value) >> 24) & 0xff;
      raw_data->raw_data[(i*4)+1] = ((raw_data->value) >> 16) & 0xff;
      raw_data->raw_data[(i*4)+2] = ((raw_data->value) >> 8) & 0xff;
      raw_data->raw_data[(i*4)+3] = ((raw_data->value) >> 0) & 0xff;
    }
  }
  return 0;
}

static unsigned int ipv4_checksum(void *buffer, unsigned int size)
{
  unsigned int check_sum = 0;
  unsigned int index = 0;
  unsigned char *p = (unsigned char *)buffer;
  
  if ((NULL == buffer) || (0 == size))
  {
    return 0;
  }

  while (size > 1)
  {
    check_sum += ((unsigned short)p[index] << 8 & 0xFF00) | ((unsigned short)p[index + 1] & 0x00FF);
    size  -= 2;
    index += 2;
  }

  if (size > 0)
  {
    check_sum += ((unsigned short)p[index] << 8) & 0xFF00;
    index += 1;
  }

  while (check_sum >> 16)
  {
    check_sum = (check_sum & 0xFFFF) + (check_sum >> 16);
  }

  check_sum = (unsigned short)(~check_sum);

  return check_sum;  
}


/*
Checksum information (to be implemented)

UDP:
Computed as the 16-bit one's complement of the one's complement sum of a pseudo header of information from the IP header, the UDP header, and the data, padded as needed with zero bytes at the end to make a multiple of two bytes. If the checksum is cleared to zero, then checksuming is disabled. If the computed checksum is zero, then this field must be set to 0xFFFF.

TCP:
This is computed as the 16-bit one's complement of the one's complement sum of a pseudo header of information from the IP header, the TCP header, and the data, padded as needed with zero bytes at the end to make a multiple of two bytes. The pseudo header contains the following fields:
  Source IP address
  Destination IP address
  0 + IP Protocol + Total length

*/

static int is_higig (void) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    uint32 v = 0;
    int unit = 0;
    soc_sbx_g3p1_interlaken_offset_get(unit, &v);
    return  v == 0;
#endif /*BCM_CALADAN3_G3P1_SUPPORT*/
    return 0;
}

  /* ******************************
 * Build headers from the arg vector list
 * ******************************/

int erh_qe_build (erh_qe_t *erh_qe, int arg_start, int arg_end, char **argv) {
  int i;
  erh_qe->entry.type = ERH_QE;
  erh_qe->entry.length = 12;

  erh_qe->ttl       = 0;
  erh_qe->s         = 0;
  erh_qe->rdp       = 0;
  erh_qe->rcos      = 0;
  erh_qe->sid       = 0;
  erh_qe->mc        = 0;
  erh_qe->out_union = 0;
  erh_qe->len_adj   = 0;
  erh_qe->frm_len   = 0;
  erh_qe->test      = 0;
  erh_qe->e         = 0;
  erh_qe->fdp       = 0;
  erh_qe->qid       = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_qe->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_qe->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_qe->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_qe->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_qe->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_qe->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_qe->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_qe->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-frm_len"))) {
      erh_qe->frm_len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_qe->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-e"))) {
      erh_qe->e = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_qe->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_qe->qid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}


int erh_isir_build (erh_isir_t *erh_isir, int arg_start, int arg_end, char **argv) {
  int i;
  erh_isir->entry.type = ERH_ISIR;
  erh_isir->entry.length = 12;

  erh_isir->ksop       = 0;
  erh_isir->et         = 0;
  erh_isir->en         = 0;
  erh_isir->t          = 0;
  erh_isir->mc         = 0;
  erh_isir->lenadj     = 0;
  erh_isir->res        = 0;
  erh_isir->qid        = 0;
  erh_isir->out_union  = 0;
  erh_isir->sid        = 0;
  erh_isir->fdp        = 0;
  erh_isir->fcos       = 0;
  erh_isir->hdrcompr   = 0;
  erh_isir->oam        = 0;
  erh_isir->rcos       = 0;
  erh_isir->rdp        = 0;
  erh_isir->s          = 0;
  erh_isir->ttl_excidx = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ksop"))) {
      erh_isir->ksop = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-et"))) {
      erh_isir->et = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-en"))) {
      erh_isir->en = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-t"))) {
      erh_isir->t = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_isir->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lenadj"))) {
      erh_isir->lenadj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-res"))) {
      erh_isir->res = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_isir->qid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_isir->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_isir->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_isir->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos"))) {
      erh_isir->fcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrcompr"))) {
      erh_isir->hdrcompr = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-oam"))) {
      erh_isir->oam = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_isir->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_isir->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_isir->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl_excidx"))) {
      erh_isir->ttl_excidx = string_to_int(argv[i+1]);
    }
  }

  return 0;
}

int erh_esir_build (erh_esir_t *erh_esir, int arg_start, int arg_end, char **argv) {
  int i;
  erh_esir->entry.type = ERH_ESIR;
  erh_esir->entry.length = 12;

  erh_esir->ksop       = 0;
  erh_esir->et         = 0;
  erh_esir->en         = 0;
  erh_esir->t          = 0;
  erh_esir->mc         = 0;
  erh_esir->lenadj     = 0;
  erh_esir->out_union  = 0;
  erh_esir->dest_port  = 0;
  erh_esir->sid        = 0;
  erh_esir->fdp        = 0;
  erh_esir->fcos       = 0;
  erh_esir->hdrcompr   = 0;
  erh_esir->oam        = 0;
  erh_esir->rcos       = 0;
  erh_esir->rdp        = 0;
  erh_esir->s          = 0;
  erh_esir->ttl_excidx = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ksop"))) {
      erh_esir->ksop = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-et"))) {
      erh_esir->et = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-en"))) {
      erh_esir->en = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-t"))) {
      erh_esir->t = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_esir->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lenadj"))) {
      erh_esir->lenadj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_esir->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dest_port"))) {
      erh_esir->dest_port = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_esir->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_esir->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos"))) {
      erh_esir->fcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrcompr"))) {
      erh_esir->hdrcompr = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-oam"))) {
      erh_esir->oam = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_esir->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_esir->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_esir->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl_excidx"))) {
      erh_esir->ttl_excidx = string_to_int(argv[i+1]);
    }
  }

  return 0;
}



int erh_iarad_build (erh_iarad_t *erh_iarad, int arg_start, int arg_end, char **argv) {
  int i;

  sal_memset(erh_iarad, 0, sizeof(erh_iarad_t));

  erh_iarad->entry.type = ERH_IARAD;
  erh_iarad->entry.length = 16;


  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ksop"))) {
        erh_iarad->ksop = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-format"))) {
        erh_iarad->format = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-attr"))) {
        erh_iarad->attr = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-reserved"))) {
        erh_iarad->reserved = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-sys_src_port"))) {
        erh_iarad->sys_src_port = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-type"))) {
        erh_iarad->type = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-m"))) {
        erh_iarad->m = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-snoop_cmd"))) {
        erh_iarad->snoop_cmd = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-cos"))) {
        erh_iarad->cos = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-dp"))) {
        erh_iarad->dp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-dest_data"))) {
        erh_iarad->dest_data = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-pad"))) {
        erh_iarad->pad = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-out_union"))) {
        erh_iarad->out_union = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-sid"))) {
        erh_iarad->sid = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-fdp"))) {
        erh_iarad->fdp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-fcos"))) {
        erh_iarad->fcos = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-hdrcompr"))) {
        erh_iarad->hdrcompr = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-oam"))) {
        erh_iarad->oam = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-rcos"))) {
        erh_iarad->rcos = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-rdp"))) {
        erh_iarad->rdp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-s"))) {
        erh_iarad->s = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-ttl_excidx"))) {
        erh_iarad->ttl_excidx = string_to_int(argv[i + 1]);
    } else {
      printf("Unknow arad ingress parameter: %s = %s\n", argv[i], argv[i + 1]);
    }
  }

  return 0;
}

int erh_earad_build (erh_earad_t *erh_earad, int arg_start, int arg_end, char **argv) {
  int i;

  sal_memset(erh_earad, 0, sizeof(erh_earad_t));

  erh_earad->entry.type = ERH_EARAD;
  erh_earad->entry.length = 16;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ksop"))) {
        erh_earad->ksop = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-act"))) {
        erh_earad->act = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-dp"))) {
        erh_earad->dp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-m"))) {
        erh_earad->m = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-class"))) {
        erh_earad->class = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-dest_port"))) {
        erh_earad->dest_port = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-sys_src_port"))) {
        erh_earad->sys_src_port = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-cud_reserved"))) {
        erh_earad->cud_reserved = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-cud"))) {
        erh_earad->cud = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-pad"))) {
        erh_earad->pad = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-out_union"))) {
        erh_earad->out_union = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-sid"))) {
        erh_earad->sid = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-fdp"))) {
        erh_earad->fdp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-fcos"))) {
        erh_earad->fcos = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-hdrcompr"))) {
        erh_earad->hdrcompr = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-oam"))) {
        erh_earad->oam = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-rcos"))) {
        erh_earad->rcos = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-rdp"))) {
        erh_earad->rdp = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-s"))) {
        erh_earad->s = string_to_int(argv[i + 1]);
    } else if (!(sal_strcmp(argv[i], "-ttl_excidx"))) {
        erh_earad->ttl_excidx = string_to_int(argv[i + 1]);
    } else {
      printf("Unknow arad egress parameter: %s = %s\n", argv[i], argv[i + 1]);
    }
  }

  return 0;
}



int erh_ss_build (erh_ss_t *erh_ss, int arg_start, int arg_end, char **argv) {
  int i;
  erh_ss->entry.type = ERH_SS;
  erh_ss->entry.length = 12;

  erh_ss->ttl       = 0;
  erh_ss->s         = 0;
  erh_ss->rdp       = 0;
  erh_ss->rcos      = 0;
  erh_ss->lbid      = 0;
  erh_ss->fcos2     = 0;
  erh_ss->fdp       = 0;
  erh_ss->sid       = 0;
  erh_ss->out_union = 0;
  erh_ss->qid       = 0;
  erh_ss->len_adj   = 0;
  erh_ss->mc        = 0;
  erh_ss->test      = 0;
  erh_ss->ecn       = 0;
  erh_ss->ect       = 0;
  erh_ss->ksop      = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_ss->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_ss->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_ss->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_ss->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lbid"))) {
      erh_ss->lbid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos2"))) {
      erh_ss->fcos2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_ss->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_ss->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_ss->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_ss->qid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_ss->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_ss->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_ss->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      erh_ss->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ect"))) {
      erh_ss->ect = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ksop"))) {
      erh_ss->ksop = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int erh_qess_build (erh_qess_t *erh_qess, int arg_start, int arg_end, char **argv) {
  int i;
  erh_qess->entry.type = ERH_QESS;
  erh_qess->entry.length = 14;

  erh_qess->ttl       = 0;
  erh_qess->s         = 0; 
  erh_qess->rdp       = 0;
  erh_qess->rcos      = 0;
  erh_qess->lbid      = 0;
  erh_qess->fcos2     = 0;
  erh_qess->sid       = 0;
  erh_qess->out_union = 0;
  erh_qess->zero      = 0;
  erh_qess->ect       = 0;
  erh_qess->mc        = 0;
  erh_qess->len_adj   = 0;
  erh_qess->frm_len   = 0;
  erh_qess->test      = 0;
  erh_qess->ecn       = 0;
  erh_qess->fdp       = 0;
  erh_qess->qid       = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_qess->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_qess->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_qess->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_qess->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lbid"))) {
      erh_qess->lbid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos2"))) {
      erh_qess->fcos2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_qess->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_qess->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-zero"))) {
      erh_qess->zero = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_qess->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_qess->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-frm_len"))) {
      erh_qess->frm_len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_qess->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      erh_qess->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_qess->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_qess->qid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int higig_build (higig_t *higig, int arg_start, int arg_end, char **argv) {
  int i;
  higig->entry.type     = HIGIG;
  higig->entry.length   = 12;

  higig->start          = 0;
  higig->hgi            = 0; 
  higig->opcode         = 0;
  higig->cos            = 0;
  higig->cng            = 0;
  higig->ingresstagged  = 0;
  higig->l3             = 0;
  higig->dstport        = 0;
  higig->dstmod         = 0;
  higig->srcport        = 0;
  higig->srcmod         = 0;
  higig->vlan           = 0;
  higig->cfi            = 0;
  higig->pri            = 0;
  higig->hdrformat      = 0;
  higig->hdrextlen      = 0;
  higig->pfm            = 0;
  higig->mirror         = 0;
  higig->mirrordone     = 0;
  higig->mirroronly     = 0;
  higig->dsttgid        = 0; 
  higig->dstt           = 0;
  higig->labelpresent   = 0;
  higig->vclabel        = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-start"))) {
      higig->start = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hgi"))) {
      higig->hgi = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      higig->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cos"))) {
      higig->cos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cng"))) {
      higig->cng = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ingresstagged"))) {
      higig->ingresstagged = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-l3"))) {
      higig->l3 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstport"))) {
      higig->dstport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstmod"))) {
      higig->dstmod = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-srcport"))) {
      higig->srcport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-srcmod"))) {
      higig->srcmod = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vlan"))) {
      higig->vlan = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cfi"))) {
      higig->cfi = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pri"))) {
      higig->pri = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrformat"))) {
      higig->hdrformat = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrextlen"))) {
      higig->hdrextlen = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pfm"))) {
      higig->pfm = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirror"))) {
      higig->mirror = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirrordone"))) {
      higig->mirrordone = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirroronly"))) {
      higig->mirroronly = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dsttgid"))) {
      higig->dsttgid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstt"))) {
      higig->dstt = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-labelpresent"))) {
      higig->labelpresent = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vclabel"))) {
      higig->vclabel = string_to_int(argv[i+1]);
    }

  }
  return 0;
}

int higig2_build (higig2_t *higig2, int arg_start, int arg_end, char **argv) {
  int i;
  higig2->entry.type    = HIGIG2;
  higig2->entry.length  = 8;

  higig2->start         = 0;
  higig2->mcst          = 0; 
  higig2->tc            = 0;
  higig2->dstmod        = 0;
  higig2->dstport       = 0;
  higig2->srcmod        = 0;
  higig2->srcport       = 0;
  higig2->lbid          = 0;
  higig2->dp            = 0;
  higig2->hdrextvalid   = 0;
  higig2->ppdtype       = 0;
  higig2->dstt          = 0;
  higig2->dsttgid       = 0;
  higig2->ingresstagged = 0;
  higig2->mirroronly    = 0;
  higig2->mirrordone    = 0;
  higig2->mirror        = 0;
  higig2->lblovltype    = 0;
  higig2->l3            = 0;
  higig2->labelpresent  = 0;
  higig2->vclabel       = 0; 
  higig2->vlan          = 0;
  higig2->pfm           = 0;
  higig2->srct          = 0;
  higig2->preservedscp  = 0;
  higig2->preservedot1p = 0;
  higig2->opcode        = 0;
  higig2->hdrextlen     = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-start"))) {
      higig2->start = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mcst"))) {
      higig2->mcst = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tc"))) {
      higig2->tc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstmod"))) {
      higig2->dstmod = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstport"))) {
      higig2->dstport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-srcmod"))) {
      higig2->srcmod = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-srcport"))) {
      higig2->srcport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lbid"))) {
      higig2->lbid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dp"))) {
      higig2->dp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrextvalid"))) {
      higig2->hdrextvalid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ppdtype"))) {
      higig2->ppdtype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dstt"))) {
      higig2->dstt = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dsttgid"))) {
      higig2->dsttgid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ingresstagged"))) {
      higig2->ingresstagged = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirroronly"))) {
      higig2->mirroronly = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirrordone"))) {
      higig2->mirrordone = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mirror"))) {
      higig2->mirror = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lblovltype"))) {
      higig2->lblovltype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-l3"))) {
      higig2->l3 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-labelpresent"))) {
      higig2->labelpresent = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vclabel"))) {
      higig2->vclabel = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vlan"))) {
      higig2->vlan = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pfm"))) {
      higig2->pfm = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-srct"))) {
      higig2->srct = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-preservedscp"))) {
      higig2->preservedscp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-preservedot1p"))) {
      higig2->preservedot1p = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      higig2->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hdrextlen"))) {
      higig2->hdrextlen = string_to_int(argv[i+1]);
    }
  }

  if (higig2->ppdtype == 0) {
    higig2->entry.length   = 16;
  }
  return 0;
}


int mac_build (mac_t *mac, int arg_start, int arg_end, char **argv) {
  int i;
  mac->entry.type = MAC;
  mac->entry.length = 12;

  mac->dmac[0]=0; mac->dmac[1]=0x11; mac->dmac[2]=0x22; mac->dmac[3]=0x33; mac->dmac[4]=0x44; mac->dmac[5]=0x55;
  mac->smac[0]=0; mac->smac[1]=0x11; mac->smac[2]=0x22; mac->smac[3]=0x33; mac->smac[4]=0x44; mac->smac[5]=0x66;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-dmac"))) {
      string_to_mac(argv[i+1], mac->dmac);
    }
    if (!(sal_strcmp(argv[i], "-smac"))) {
      string_to_mac(argv[i+1], mac->smac);
    }
  }
  return 0;
}

int vlan_build (vlan_t *vlan, int arg_start, int arg_end, char **argv) {
  int i;
  vlan->entry.type = VLAN;
  vlan->entry.length = 4;

  vlan->tpid = 0x8100;
  vlan->vid  = 0xabc;
  vlan->cfi  = 0;
  vlan->pri  = 3;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-tpid"))) {
      vlan->tpid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vid"))) {
      vlan->vid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cfi"))) {
      vlan->cfi = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pri"))) {
      vlan->pri = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int stag_build (stag_t *stag, int arg_start, int arg_end, char **argv) {
  int i;
  stag->entry.type = STAG;
  stag->entry.length = 4;

  stag->tpid = 0x88a8;
  stag->vid  = 0xabc;
  stag->dei  = 1;
  stag->pcp  = 3;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-tpid"))) {
      stag->tpid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vid"))) {
      stag->vid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dei"))) {
      stag->dei = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pcp"))) {
      stag->pcp = string_to_int(argv[i+1]);
    }
  } 
  return 0;
}

int etype_build (etype_t *etype, int arg_start, int arg_end, char **argv) {
  int i;
  etype->entry.type = ETYPE;
  etype->entry.length = 2;

  etype->etype = 0x0801;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-etype"))) {
      etype->etype = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int llc_build (llc_t *llc, int arg_start, int arg_end, char **argv) {
  int i;
  llc->entry.type = LLC;
  llc->entry.length = 7;

  llc->len  = 64;
  llc->ssap = 0xaa;
  llc->dsap = 0xaa;
  llc->ctrl = 3;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-len"))) {
      llc->len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ssap"))) {
      llc->ssap = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dsap"))) {
      llc->dsap = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ctrl"))) {
      llc->ctrl = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int snap_build (snap_t *snap, int arg_start, int arg_end, char **argv) {
  int i;
  snap->entry.type = SNAP;
  snap->entry.length = 3;

  snap->oui = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-oui"))) {
      snap->oui = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int oam_ccm_build(oam_ccm_t *oam_ccm, int arg_start, int arg_end, char **argv) {
  int i;

  oam_ccm->entry.type   = OAM_CCM;
  oam_ccm->entry.length = 75;

  oam_ccm->lvl          = 0;
  oam_ccm->version      = 0;
  oam_ccm->opcode       = 0;
  oam_ccm->flags_rdi    = 0;
  oam_ccm->flags_rsvd   = 0;
  oam_ccm->flags_period = 0;
  oam_ccm->tlv_offset   = 0;
  oam_ccm->seq_number   = 0;
  oam_ccm->mep_id       = 0;
  oam_ccm->tx_fcf       = 0;
  oam_ccm->rx_fcb       = 0;
  oam_ccm->tx_fcb       = 0;
  oam_ccm->reserved     = 0;
  oam_ccm->end_tlv      = 0;

  sal_memset(oam_ccm->maid, 0, 48);

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_ccm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_ccm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_ccm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags_rdi"))) {
      oam_ccm->flags_rdi = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags_rsvd"))) {
      oam_ccm->flags_rsvd = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags_period"))) {
      oam_ccm->flags_period = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_ccm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seq_number"))) {
      oam_ccm->seq_number = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mep_id"))) {
      oam_ccm->mep_id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_fcf"))) {
      oam_ccm->tx_fcf = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rx_fcb"))) {
      oam_ccm->rx_fcb = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-reserved"))) {
      oam_ccm->reserved = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-end_tlv"))) {
      oam_ccm->end_tlv = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-maid"))) {
      string_to_maid(argv[i+1], oam_ccm->maid);
    }
  }
  return 0;


}

int oam_lbm_build(oam_lbm_t *oam_lbm, int arg_start, int arg_end, char **argv) {
  int i;

  oam_lbm->entry.type   = OAM_LBM;
  oam_lbm->entry.length = 9;

  oam_lbm->lvl          = 0;
  oam_lbm->version      = 0;
  oam_lbm->opcode       = 0;
  oam_lbm->flags        = 0;
  oam_lbm->tlv_offset   = 0;
  oam_lbm->seq_number   = 0;
  oam_lbm->end_tlv      = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_lbm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_lbm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_lbm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      oam_lbm->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_lbm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seq_number"))) {
      oam_lbm->seq_number = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-end_tlv"))) {
      oam_lbm->end_tlv = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int oam_ltm_build(oam_ltm_t *oam_ltm, int arg_start, int arg_end, char **argv) {
  int i;

  oam_ltm->entry.type   = OAM_LTM;
  oam_ltm->lvl          = 0;
  oam_ltm->version      = 0;
  oam_ltm->opcode       = 0;
  oam_ltm->flags        = 0;
  oam_ltm->tlv_offset   = 0;
  oam_ltm->seq_number   = 0;
  oam_ltm->ttl          = 0;
  oam_ltm->relay_action = 0;
  oam_ltm->end_tlv      = 0;
  oam_ltm->origin_mac[0]=0x00; oam_ltm->origin_mac[1]=0x11; 
  oam_ltm->origin_mac[2]=0x22; oam_ltm->origin_mac[3]=0x33; 
  oam_ltm->origin_mac[4]=0x44; oam_ltm->origin_mac[5]=0x55;
  oam_ltm->target_mac[0]=0x00; oam_ltm->target_mac[1]=0x11; 
  oam_ltm->target_mac[2]=0x22; oam_ltm->target_mac[3]=0x33; 
  oam_ltm->target_mac[4]=0x44; oam_ltm->target_mac[5]=0x66;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_ltm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_ltm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_ltm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      oam_ltm->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_ltm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seq_number"))) {
      oam_ltm->seq_number = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      oam_ltm->ttl = string_to_int(argv[i+1]);
    }
    if (OPCODE_LTM == oam_ltm->opcode) {
      oam_ltm->entry.length = 22;
      if (!(sal_strcmp(argv[i], "-origin_mac"))) {
        string_to_mac(argv[i+1], oam_ltm->origin_mac);
      }
      if (!(sal_strcmp(argv[i], "-target_mac"))) {
        string_to_mac(argv[i+1], oam_ltm->target_mac);
      }
      if (!(sal_strcmp(argv[i], "-end_tlv"))) {
       oam_ltm->end_tlv = string_to_int(argv[i+1]);
      }
    } else {
      oam_ltm->entry.length = 10;
      if (!(sal_strcmp(argv[i], "-relay_action"))) {
       oam_ltm->relay_action = string_to_int(argv[i+1]);
      }
    }
  }
  return 0;
}

int oam_lmm_build(oam_lmm_t *oam_lmm, int arg_start, int arg_end, char **argv) {
  int i;
  oam_lmm->entry.type   = OAM_LMM;
  oam_lmm->entry.length = 17;  
  oam_lmm->lvl          = 0;
  oam_lmm->version      = 0;
  oam_lmm->opcode       = 0;
  oam_lmm->flags        = 0;
  oam_lmm->tlv_offset   = 0;
  oam_lmm->tx_fcf       = 0;
  oam_lmm->rx_fcf       = 0;
  oam_lmm->tx_fcb       = 0;
  oam_lmm->end_tlv      = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_lmm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_lmm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_lmm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      oam_lmm->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_lmm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_fcf"))) {
      oam_lmm->tx_fcf = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rx_fcf"))) {
      oam_lmm->rx_fcf = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_fcb"))) {
      oam_lmm->tx_fcb = string_to_int(argv[i+1]);
    }    
    if (!(sal_strcmp(argv[i], "-end_tlv"))) {
      oam_lmm->end_tlv = string_to_int(argv[i+1]);
    }

  }
  return 0;
}

int oam_1dmm_build(oam_1dmm_t *oam_1dmm, int arg_start, int arg_end, char **argv) {
  int i;
  oam_1dmm->entry.type        = OAM_1DMM;
  oam_1dmm->entry.length      = 21;  
  oam_1dmm->lvl               = 0;
  oam_1dmm->version           = 0;
  oam_1dmm->opcode            = 0;
  oam_1dmm->flags             = 0;
  oam_1dmm->tlv_offset        = 0;
  oam_1dmm->tx_timestamp_sec  = 0;
  oam_1dmm->tx_timestamp_nano = 0;
  oam_1dmm->rx_timestamp_sec  = 0;
  oam_1dmm->rx_timestamp_nano = 0;
  oam_1dmm->end_tlv      = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_1dmm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_1dmm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_1dmm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      oam_1dmm->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_1dmm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_sec"))) {
      oam_1dmm->tx_timestamp_sec = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_nano"))) {
      oam_1dmm->tx_timestamp_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rx_timestamp_sec"))) {
      oam_1dmm->rx_timestamp_sec = string_to_int(argv[i+1]);
    }    
    if (!(sal_strcmp(argv[i], "-rx_timestamp_nano"))) {
      oam_1dmm->rx_timestamp_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-end_tlv"))) {
      oam_1dmm->end_tlv = string_to_int(argv[i+1]);
    }

  }
  return 0;
}

int oam_2dmm_build(oam_2dmm_t *oam_2dmm, int arg_start, int arg_end, char **argv) {
  int i;
  oam_2dmm->entry.type          = OAM_2DMM;
  oam_2dmm->entry.length        = 37;  
  oam_2dmm->lvl                 = 0;
  oam_2dmm->version             = 0;
  oam_2dmm->opcode              = 0;
  oam_2dmm->flags               = 0;
  oam_2dmm->tlv_offset          = 0;
  oam_2dmm->tx_timestamp_f_sec  = 0;
  oam_2dmm->tx_timestamp_f_nano = 0;
  oam_2dmm->rx_timestamp_f_sec  = 0;
  oam_2dmm->rx_timestamp_f_nano = 0;
  oam_2dmm->tx_timestamp_b_sec  = 0;
  oam_2dmm->tx_timestamp_b_nano = 0;
  oam_2dmm->rx_timestamp_b_sec  = 0;
  oam_2dmm->rx_timestamp_b_nano = 0;
  oam_2dmm->end_tlv      = 0;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-lvl"))) {
      oam_2dmm->lvl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      oam_2dmm->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opcode"))) {
      oam_2dmm->opcode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      oam_2dmm->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tlv_offset"))) {
      oam_2dmm->tlv_offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_f_sec"))) {
      oam_2dmm->tx_timestamp_f_sec = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_f_nano"))) {
      oam_2dmm->tx_timestamp_f_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rx_timestamp_f_sec"))) {
      oam_2dmm->rx_timestamp_f_sec = string_to_int(argv[i+1]);
    }    
    if (!(sal_strcmp(argv[i], "-rx_timestamp_f_nano"))) {
      oam_2dmm->rx_timestamp_f_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_b_sec"))) {
      oam_2dmm->tx_timestamp_b_sec = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tx_timestamp_b_nano"))) {
      oam_2dmm->tx_timestamp_b_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rx_timestamp_b_sec"))) {
      oam_2dmm->rx_timestamp_b_sec = string_to_int(argv[i+1]);
    }    
    if (!(sal_strcmp(argv[i], "-rx_timestamp_b_nano"))) {
      oam_2dmm->rx_timestamp_b_nano = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-end_tlv"))) {
      oam_2dmm->end_tlv = string_to_int(argv[i+1]);
    }

  }
  return 0;
}


int ipv4_build (ipv4_t *ipv4, int arg_start, int arg_end, char **argv) {
  int i;
  ipv4->entry.type = IPV4;
  ipv4->entry.length = 20;

  ipv4->ver      = 4;
  ipv4->ihl      = 5;
  ipv4->tos      = 0;
  ipv4->length   = 0;
  ipv4->id       = 0;
  ipv4->df       = 0;
  ipv4->mf       = 0;
  ipv4->offset   = 0;
  ipv4->ip_opt   = 0;
  ipv4->proto    = 6;
  ipv4->ttl      = 0x40;
  ipv4->checksum = 0;
  ipv4->sa       = 0x01010101;
  ipv4->da       = 0x02020202;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      ipv4->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ihl"))) {
      ipv4->ihl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tos"))) {
      ipv4->tos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      ipv4->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-id"))) {
      ipv4->id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-df"))) {
      ipv4->df = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mf"))) {
      ipv4->mf = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-offset"))) {
      ipv4->offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ip_opt"))) {
      ipv4->ip_opt = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-proto"))) {
      ipv4->proto = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      ipv4->ttl = string_to_int(argv[i+1]);
    }
    
    if (!(sal_strcmp(argv[i], "-sa"))) {
      ipv4->sa = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-da"))) {
      ipv4->da = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int ipv6_build (ipv6_t *ipv6, int arg_start, int arg_end, char **argv) {
  int i;
  ipv6->entry.type   = IPV6;
  ipv6->entry.length = 40;

  ipv6->ver          = 6;
  ipv6->tos          = 0;
  ipv6->length       = 0;
  ipv6->flow_label   = 0;
  ipv6->ttl          = 0xff;
  ipv6->next_header  = 0;
  ipv6->sa[0]=0x1100; ipv6->sa[1]=0x2211; ipv6->sa[2]=0x3322; ipv6->sa[3]=0x4433; 
  ipv6->sa[4]=0x5544; ipv6->sa[5]=0x6655; ipv6->sa[6]=0x7766; ipv6->sa[7]=0x8877; 
  ipv6->da[0]=0x0123; ipv6->da[1]=0x4567; ipv6->da[2]=0x89ab; ipv6->da[3]=0xcdef; 
  ipv6->da[4]=0x0123; ipv6->da[5]=0x4567; ipv6->da[6]=0x89ab; ipv6->da[7]=0xcdef; 

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      ipv6->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tos"))) {
      ipv6->tos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      ipv6->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flow_label"))) {
      ipv6->flow_label = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      ipv6->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-next_header"))) {
      ipv6->next_header = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sa"))) {
      string_to_ipv6(argv[i+1], ipv6->sa);
    }
    if (!(sal_strcmp(argv[i], "-da"))) {
      string_to_ipv6(argv[i+1], ipv6->da);
    }

  }
  return 0;
}

int bfd_build (bfd_t *bfd, int arg_start, int arg_end, char **argv) {
  int i;
  bfd->entry.type   = BFD;
  bfd->entry.length = 24;

  bfd->version      = 1;
  bfd->diag         = 0;
  bfd->flags        = 0x1111;
  bfd->state        = 3;
  bfd->detect_mult  = 0;
  bfd->length       = 0;
  bfd->my_discrim   = 0;
  bfd->your_discrim = 0;
  bfd->min_tx       = 0x2000;
  bfd->min_rx       = 0x3000;
  bfd->echo         = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-version"))) {
      bfd->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-diag"))) {
      bfd->diag = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      bfd->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-state"))) {
      bfd->state = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-detect_mult"))) {
      bfd->detect_mult = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      bfd->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-my_discrim"))) {
      bfd->my_discrim = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-your_discrim"))) {
      bfd->your_discrim = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-min_tx"))) {
      bfd->min_tx = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-min_rx"))) {
      bfd->min_rx = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-echo"))) {
      bfd->echo = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int bpdu_build (bpdu_t *bpdu, int arg_start, int arg_end, char **argv) {
  int i;
  bpdu->entry.type     = BPDU;
  bpdu->entry.length   = 35;

  bpdu->protocol_id    = 0;
  bpdu->version        = 0;
  bpdu->message_type   = 0x80;
  bpdu->flags          = 0x1111;
  bpdu->root_pri       = 0x1122;
  bpdu->root_path_cost = 0;
  bpdu->bridge_pri     = 0x1122;
  bpdu->port_id        = 0;
  bpdu->message_age    = 7;
  bpdu->max_age        = 20;
  bpdu->hello_time     = 2;
  bpdu->forward_delay  = 15;
  bpdu->root_mac[0]=0; bpdu->root_mac[1]=0x11; 
  bpdu->root_mac[2]=0x22; bpdu->root_mac[3]=0x33; 
  bpdu->root_mac[4]=0x44; bpdu->root_mac[5]=0x55;
  bpdu->bridge_mac[0]=0; bpdu->bridge_mac[1]=0x11; 
  bpdu->bridge_mac[2]=0x22; bpdu->bridge_mac[3]=0x33; 
  bpdu->bridge_mac[4]=0x44; bpdu->bridge_mac[5]=0x66;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-protocol_id"))) {
      bpdu->protocol_id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version"))) {
      bpdu->version = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-message_type"))) {
      bpdu->message_type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      bpdu->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-root_pri"))) {
      bpdu->root_pri = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-root_path_cost"))) {
      bpdu->root_path_cost = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-bridge_pri"))) {
      bpdu->bridge_pri = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-port_id"))) {
      bpdu->port_id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-message_age"))) {
      bpdu->message_age = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-max_age"))) {
      bpdu->max_age = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hello_time"))) {
      bpdu->hello_time = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-forward_delay"))) {
      bpdu->forward_delay = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-root_mac"))) {
      string_to_mac(argv[i+1], bpdu->root_mac);
    }
    if (!(sal_strcmp(argv[i], "-bridge_mac"))) {
      string_to_mac(argv[i+1], bpdu->bridge_mac);
    }

  }
  return 0;
}

int slow_build (slow_t *slow, int arg_start, int arg_end, char **argv) {
  int i;
  slow->entry.type     = SLOW;
  slow->entry.length   = 1;

  slow->sub_type       = 3;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-sub_type"))) {
      slow->sub_type = string_to_int(argv[i+1]);
    }
  }
  return 0;
}


int itag_build (itag_t *itag, int arg_start, int arg_end, char **argv) {
  int i;
  itag->entry.type = ITAG;
  itag->entry.length = 4;

  itag->ipcp = 3;
  itag->idei = 0;
  itag->nca  = 0;
  itag->isid = 0xa5a5a;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ipcp"))) {
      itag->ipcp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-idei"))) {
      itag->idei = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-nca"))) {
      itag->nca = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-isid"))) {
      itag->isid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int mpls_build (mpls_t *mpls, int arg_start, int arg_end, char **argv) {
  int i;
  mpls->entry.type = MPLS;
  mpls->entry.length = 4;

  mpls->label = 0x1234;
  mpls->exp   = 0;
  mpls->s     = 1;
  mpls->ttl   = 0x40;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-label"))) {
      mpls->label = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-exp"))) {
      mpls->exp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      mpls->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      mpls->ttl = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int udp_build (udp_t *udp, int arg_start, int arg_end, char **argv) {
  int i;
  udp->entry.type = UDP;
  udp->entry.length = 8;

  udp->sport = 1;
  udp->dport = 1;
  udp->len   = 0;  
  udp->checksum = 0; /* 0 means checksum is disabled */

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-sport"))) {
      udp->sport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dport"))) {
      udp->dport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len"))) {
      udp->len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      udp->checksum = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int tcp_build (tcp_t *tcp, int arg_start, int arg_end, char **argv) {
  int i;
  tcp->entry.type = TCP;
  tcp->entry.length = 20;

  tcp->sport    = 0;
  tcp->dport    = 0;
  tcp->seqn     = 0;
  tcp->ackn     = 0;
  tcp->dofs     = 0;
  tcp->ecn      = 0;
  tcp->ctrl     = 0;
  tcp->wind     = 0;
  tcp->checksum = 0;
  tcp->urgp     = 0;
  tcp->option   = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-sport"))) {
      tcp->sport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dport"))) {
      tcp->dport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seqn"))) {
      tcp->seqn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ackn"))) {
      tcp->ackn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dofs"))) {
      tcp->dofs = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      tcp->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ctrl"))) {
      tcp->ctrl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-wind"))) {
      tcp->wind = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      tcp->checksum = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-urgp"))) {
      tcp->urgp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-option"))) {
      tcp->option = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int igmp_build (igmp_t *igmp, int arg_start, int arg_end, char **argv) {
  int i;
  igmp->entry.type = IGMP;

  igmp->ver      = 0;
  igmp->type     = 0;
  igmp->checksum = 0;
  igmp->group    = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      igmp->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-type"))) {
      igmp->type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      igmp->checksum = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-group"))) {
      igmp->group = string_to_int(argv[i+1]);
    }
  }
  return 0;
}


int pwe_data_ach_build (pwe_data_ach_t *pwe_data_ach, int arg_start, int arg_end, char **argv) {
  int i;
  pwe_data_ach->entry.type = PWE_DATA_ACH;
  pwe_data_ach->entry.length = 4;

  pwe_data_ach->flag       = 0;
  pwe_data_ach->frg        = 0;
  pwe_data_ach->length     = 0;
  pwe_data_ach->seqno      = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-flag"))) {
      pwe_data_ach->flag = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-frg"))) {
      pwe_data_ach->frg = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      pwe_data_ach->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seqno"))) {
      pwe_data_ach->seqno = string_to_int(argv[i+1]);
    }
  }
  return 0;
} 

int pwe_ctrl_ach_build (pwe_ctrl_ach_t *pwe_ctrl_ach, int arg_start, int arg_end, char **argv) {
  int i;
  pwe_ctrl_ach->entry.type = PWE_CTRL_ACH;
  pwe_ctrl_ach->entry.length = 4;

  pwe_ctrl_ach->ctrl        = 0;
  pwe_ctrl_ach->ver        = 0;
  pwe_ctrl_ach->rsvd       = 0;
  pwe_ctrl_ach->channel    = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ctrl"))) {
      pwe_ctrl_ach->ctrl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ver"))) {
      pwe_ctrl_ach->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsvd"))) {
      pwe_ctrl_ach->rsvd = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-channel"))) {
      pwe_ctrl_ach->channel = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int gach_build (gach_t *gach, int arg_start, int arg_end, char **argv) {
  int i;
  gach->entry.type = GACH;
  gach->entry.length = 4;

  gach->ver        = 0;
  gach->rsvd       = 0;
  gach->channel    = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      gach->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsvd"))) {
      gach->rsvd = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-channel"))) {
      gach->channel = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int tlvhdr_build (tlvhdr_t *tlvhdr, int arg_start, int arg_end, char **argv) {
  int i;
  tlvhdr->entry.type = TLVHDR;
  tlvhdr->entry.length = 4;

  tlvhdr->length        = 0;
  tlvhdr->rsvd       = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-length"))) {
      tlvhdr->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsvd"))) {
      tlvhdr->rsvd = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int psc_build (psc_t *psc, int arg_start, int arg_end, char **argv) {
  int i;
  psc->entry.type = PSC;
  psc->entry.length = 4;

  psc->ver        = 0;
  psc->request    = 0;
  psc->pt         = 0;
  psc->r          = 0;
  psc->rsvd       = 0;
  psc->fpath      = 0;
  psc->path       = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      psc->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-request"))) {
      psc->request = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pt"))) {
      psc->pt = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-r"))) {
      psc->r = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsvd"))) {
      psc->rsvd = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fpath"))) {
      psc->fpath = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-path"))) {
      psc->path = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int oam_mpls_build (oam_mpls_t *oam_mpls, int arg_start, int arg_end, char **argv) {
  int i, j;
  unsigned char item = 0;
  char *buf = NULL;

  oam_mpls->entry.type   = OAM_MPLS;
  oam_mpls->entry.length = 44;

  oam_mpls->func_type    = 0;
  memset(oam_mpls->ttsi, 0, TTSI_LENGTH);
  oam_mpls->frequency    = 0;
  oam_mpls->bip16        = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-func_type"))) {
      oam_mpls->func_type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttsi"))) {
      buf = argv[i+1];
      for (j = 0; j < TTSI_LENGTH; j++) {
        string_to_hex(buf+j*2, &item);
        oam_mpls->ttsi[j] = item;
      }
    }
    if (!(sal_strcmp(argv[i], "-frequency"))) {
      oam_mpls->frequency = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-bip16"))) {
      oam_mpls->bip16 = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int ip_option_build (ip_option_t *ip_option, int arg_start, int arg_end, char **argv) {
  int i, j;
  unsigned char item = 0;
  char *buf = NULL;

  ip_option->entry.type   = IP_OPTION;
  ip_option->option_code  = 0;
  ip_option->length       = 0;
  memset(ip_option->data, 0, IP_OPTION_LENGTH);


  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-option_code"))) {
      ip_option->option_code = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      ip_option->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-data"))) {
      buf = argv[i+1];
      for (j = 0; j < ip_option->length - 1; j++) {
        string_to_hex(buf+j*2, &item);
        ip_option->data[j] = item;
      }
    }
  }

  ip_option->entry.length = ip_option->length + 1;

  return 0;
}

int hop_build (hop_t *hop, int arg_start, int arg_end, char **argv) {
  int i;
  
  hop->entry.type   = HOP;
  hop->entry.length = 8;
  hop->next_header  = 0;
  hop->length       = 0;
  hop->opt_type     = 0;
  hop->opt_len      = 0;
  hop->data         = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-next_header"))) {
      hop->next_header = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      hop->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opt_type"))) {
      hop->opt_type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-opt_len"))) {
      hop->opt_len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-data"))) {
      hop->data = string_to_int(argv[i+1]);
    }
  }

  return 0;
}

int ptp_build (ptp_t *ptp, int arg_start, int arg_end, char **argv) {
  int i;
  
  ptp->entry.type           = PTP;
  ptp->entry.length         = 34;
  ptp->message_type         = 0;
  ptp->version_ptp          = 0;
  ptp->message_length       = 0;
  ptp->domain_number        = 0;
  ptp->flag_field           = 0;
  ptp->correction_field1    = 0;
  ptp->correction_field2    = 0;
  ptp->sourceport_identity1 = 0;
  ptp->sourceport_identity2 = 0;
  ptp->sourceport_identity3 = 0;
  ptp->sequence_id          = 0;
  ptp->control_field        = 0;
  ptp->logmessage_interval  = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-message_type"))) {
      ptp->message_type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-version_ptp"))) {
      ptp->version_ptp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-message_length"))) {
      ptp->message_length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-domain_number"))) {
      ptp->domain_number = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flag_field"))) {
      ptp->flag_field = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-correction_field1"))) {
      ptp->correction_field1 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-correction_field2"))) {
      ptp->correction_field2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sourceport_identity1"))) {
      ptp->sourceport_identity1 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sourceport_identity2"))) {
      ptp->sourceport_identity2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sourceport_identity3"))) {
      ptp->sourceport_identity3 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sequence_id"))) {
      ptp->sequence_id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-control_field"))) {
      ptp->control_field = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-logmessage_interval"))) {
      ptp->logmessage_interval = string_to_int(argv[i+1]);
    }

  }

  return 0;
}

int itmh_build (itmh_t *itmh, int arg_start, int arg_end, char **argv) 
{
    int i;

    itmh->type = 0;
    itmh->p_t  = 0;
    itmh->pkt_head_len = 0;
    itmh->dp = 0;
    itmh->uc_flow_id = 0;
    itmh->mc_t_c = 0;
    itmh->multicast_id = 0;

    itmh->entry.type   = ITMH;
    itmh->entry.length = 4;
		
		
		
    
    for (i=arg_start; i < arg_end; i += 2) {
        if (!(sal_strcmp(argv[i], "-type"))) {
            itmh->type = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-p_t"))) {
            itmh->p_t = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-pkt_head_len"))) {
            itmh->pkt_head_len = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-dp"))) {
            itmh->dp = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-uc_flow_id"))) {
            itmh->uc_flow_id = string_to_int(argv[i+1]);
        }

        if (!(sal_strcmp(argv[i], "-mc_t_c"))) {
            itmh->mc_t_c = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-multicast_id"))) {
            itmh->multicast_id = string_to_int(argv[i+1]);
        }
    }

    return 0;
}



int nph_build (nph_t *nph, int arg_start, int arg_end, char **argv) 
{
    int i;

    nph->m_u_flag = 0;
    nph->main_type = 0;
    nph->sub_type = 0;
    nph->ptp_1588flag = 0;
    nph->cos_dp = 0;
    nph->slot_in = 0;
    nph->trunk_flag = 0;
    nph->port_or_trunk_id = 0;
    nph->vlan_status = 0;
    nph->del_length = 0;
    nph->default_pri_cfi = 0;
    nph->protect_status_section = 0;

    nph->entry.type   = NPH;
    nph->entry.length = 16;

    
    for (i=arg_start; i<arg_end; i+=2) 
    {
        if (!(sal_strcmp(argv[i], "-m_u_flag"))) 
        {
            nph->m_u_flag = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-main_type"))) 
        {
            nph->main_type = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-sub_type"))) 
        {
            nph->sub_type = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-ptp_1588flag"))) 
        {
            nph->ptp_1588flag = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-cos_dp"))) 
        {
            nph->cos_dp = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-slot_in"))) 
        {
            nph->slot_in = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-trunk_flag"))) 
        {
            nph->trunk_flag = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-port_or_trunk_id"))) 
        {
            nph->port_or_trunk_id = string_to_int(argv[i+1]);
        }

        if (!(sal_strcmp(argv[i], "-vlan_status"))) 
        {
            nph->vlan_status = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-del_length"))) 
        {
            nph->del_length = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-default_pri_cfi"))) 
        {
            nph->default_pri_cfi = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-protect_status_section"))) 
        {
            nph->protect_status_section = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-protect_status_lsp"))) 
        {
            nph->protect_status_lsp = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-protect_status_pw_uni"))) 
        {
            nph->protect_status_pw_uni = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-output_fp"))) 
        {
            nph->output_fp = string_to_int(argv[i+1]);
        }

        if (!(sal_strcmp(argv[i], "-root_leaf"))) 
        {
            nph->root_leaf = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-hub_spoke"))) 
        {
            nph->hub_spoke = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-tag_num"))) 
        {
            nph->tag_num = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-b_f"))) 
        {
            nph->b_f = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-port_out"))) 
        {
            nph->port_out = string_to_int(argv[i+1]);
        }

        if (!(sal_strcmp(argv[i], "-lrn_on"))) 
        {
            nph->lrn_on = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-mstp_Lrn"))) 
        {
            nph->mstp_Lrn = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-msp_opo"))) 
        {
            nph->msp_opo = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-vpn_id"))) 
        {
            nph->vpn_id = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-nph_word3"))) 
        {
            nph->nph_word3 = string_to_int(argv[i+1]);
        }
    }

    return 0;
}



int nfh_build (nfh_t *nfh, int arg_start, int arg_end, char **argv) {
  int i;
  
  nfh->entry.type    = NFH;
  nfh->entry.length  = 20;
  nfh->destpoint     = 0;
  nfh->linktype      = 0;
  nfh->packettype_to = 0;
  nfh->packetlen     = 0;
  nfh->packettype    = 0;
  nfh->vlan_status   = 0;
  nfh->port          = 0;
  nfh->fp            = 0;
  nfh->desttype      = 0;
  nfh->vpnid         = 0;
  nfh->flowtype      = 0;
  nfh->greencap      = 0;
  nfh->slot          = 0;
  nfh->to_fpga       = 0;
  nfh->cookielen     = 0;
  nfh->cookie        = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-destpoint"))) {
      nfh->destpoint = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-linktype"))) {
      nfh->linktype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-packettype_to"))) {
      nfh->packettype_to = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-packetlen"))) {
      nfh->packetlen = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-packettype"))) {
      nfh->packettype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vlan_status"))) {
      nfh->vlan_status = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-port"))) {
      nfh->port = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fp"))) {
      nfh->fp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-desttype"))) {
      nfh->desttype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vpnid"))) {
      nfh->vpnid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flowtype"))) {
      nfh->flowtype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-greencap"))) {
      nfh->greencap = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-slot"))) {
      nfh->slot = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-to_fpga"))) {
      nfh->to_fpga = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cookielen"))) {
      nfh->cookielen = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cookie"))) {
      nfh->cookie = string_to_int(argv[i+1]);
    }

  }

  return 0;
}

int fnh_full_fwd_build (fnh_full_fwd_t *fnh_full_fwd, int arg_start, int arg_end, char **argv) {
  int i;
  
  fnh_full_fwd->entry.type   = FNH_FULL_FWD;
  fnh_full_fwd->entry.length = 10;
  fnh_full_fwd->cmheader     = 0;
  fnh_full_fwd->rsv0         = 0;
  fnh_full_fwd->ftype        = 0;
  fnh_full_fwd->rsv1         = 0;
  fnh_full_fwd->ptype        = 0;
  fnh_full_fwd->rsv2         = 0;
  fnh_full_fwd->port         = 0;
  fnh_full_fwd->rsv3         = 0;
  fnh_full_fwd->slot         = 0;
  fnh_full_fwd->rsv4         = 0;
  fnh_full_fwd->cos          = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-cmheader"))) {
      fnh_full_fwd->cmheader = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv0"))) {
      fnh_full_fwd->rsv0 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ftype"))) {
      fnh_full_fwd->ftype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv1"))) {
      fnh_full_fwd->rsv1 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ptype"))) {
      fnh_full_fwd->ptype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv2"))) {
      fnh_full_fwd->rsv2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-port"))) {
      fnh_full_fwd->port = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv3"))) {
      fnh_full_fwd->rsv3 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-slot"))) {
      fnh_full_fwd->slot = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv4"))) {
      fnh_full_fwd->rsv4 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cos"))) {
      fnh_full_fwd->cos = string_to_int(argv[i+1]);
    }

  }

  return 0;
}

int fnh_half_fwd_build (fnh_half_fwd_t *fnh_half_fwd, int arg_start, int arg_end, char **argv) {
  int i;
  
  fnh_half_fwd->entry.type   = FNH_HALF_FWD;
  fnh_half_fwd->entry.length = 12;
  fnh_half_fwd->cmheader     = 0;
  fnh_half_fwd->rsv0         = 0;
  fnh_half_fwd->ftype        = 0;
  fnh_half_fwd->rsv1         = 0;
  fnh_half_fwd->ptype        = 0;
  fnh_half_fwd->rsv2         = 0;
  fnh_half_fwd->protect_status = 0;
  fnh_half_fwd->dp           = 0;
  fnh_half_fwd->rsv3         = 0;
  fnh_half_fwd->cos          = 0;
  fnh_half_fwd->oampduoffset = 0;
  fnh_half_fwd->rsv4         = 0;
  fnh_half_fwd->outfp        = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-cmheader"))) {
      fnh_half_fwd->cmheader = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv0"))) {
      fnh_half_fwd->rsv0 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ftype"))) {
      fnh_half_fwd->ftype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv1"))) {
      fnh_half_fwd->rsv1 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ptype"))) {
      fnh_half_fwd->ptype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv2"))) {
      fnh_half_fwd->rsv2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-protect_status"))) {
      fnh_half_fwd->protect_status = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dp"))) {
      fnh_half_fwd->dp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv3"))) {
      fnh_half_fwd->rsv3 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cos"))) {
      fnh_half_fwd->cos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-oampduoffset"))) {
      fnh_half_fwd->oampduoffset = string_to_int(argv[i+1]);
    }

    if (!(sal_strcmp(argv[i], "-rsv4"))) {
      fnh_half_fwd->rsv4 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-outfp"))) {
      fnh_half_fwd->outfp = string_to_int(argv[i+1]);
    }

  }

  return 0;
}

int fnh_non_fwd_build (fnh_non_fwd_t *fnh_non_fwd, int arg_start, int arg_end, char **argv) 
{
    int i;

    sal_memset(fnh_non_fwd, 0, sizeof(fnh_non_fwd_t));
    fnh_non_fwd->entry.type   = FNH_NON_FWD;
    fnh_non_fwd->entry.length = 10;

    
    for (i=arg_start; i < arg_end; i += 2) {
        if (!(sal_strcmp(argv[i], "-cmheader"))) {
            fnh_non_fwd->cmheader = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-ftype"))) {
            fnh_non_fwd->ftype = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-ptype"))) {
            fnh_non_fwd->ptype = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-slot"))) {
            fnh_non_fwd->slot = string_to_int(argv[i+1]);
        }
        if (!(sal_strcmp(argv[i], "-packets"))) {
            fnh_non_fwd->packets = string_to_int(argv[i+1]);
        }
    }

    return 0;
}

int fnh_np_term_build (fnh_np_term_t *fnh_np_term, int arg_start, int arg_end, char **argv) {
  int i;
  
  fnh_np_term->entry.type   = FNH_NP_TERM;
  fnh_np_term->entry.length = 8;
  fnh_np_term->cmheader     = 0;
  fnh_np_term->rsv0         = 0;
  fnh_np_term->ftype        = 0;
  fnh_np_term->reserved     = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-cmheader"))) {
      fnh_np_term->cmheader = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rsv0"))) {
      fnh_np_term->rsv0 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ftype"))) {
      fnh_np_term->ftype = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-reserved"))) {
      fnh_np_term->reserved = string_to_int(argv[i+1]);
    }
  }

  return 0;
}


int raw_data_build (raw_data_t *raw_data, int arg_start, int arg_end, char **argv) {
  int i;
  raw_data->entry.type = RAW_DATA;
  raw_data->entry.length = 40;

  raw_data->value = 0xba53ba11;
  raw_data->mode  = 0;  /* INCREMENT */
  raw_data->flags = 0;

  
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-value"))) {
      raw_data->value = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mode"))) {
      raw_data->mode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      raw_data->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      raw_data->entry.length = string_to_int(argv[i+1]);
    }
  }

  stuff_data (raw_data);
  return 0;
}

int hex_data_build (hex_data_t *hex_data, int arg_start, int arg_end, char **argv) {
  int i, j;
  unsigned char item = 0;
  char *buf = NULL;
  hex_data->entry.type = HEX_DATA;
  hex_data->entry.length = 64;

  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-length"))) {
      hex_data->entry.length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-hex"))) {
      buf = argv[i+1];
      for (j = 0; j < hex_data->entry.length; j++) {
        string_to_hex(buf+j*2, &item);
        hex_data->hex_data[j] = item;
      }
    }
  }

  return 0;
}



/* ******************************
 * print header functions
 * ******************************/
void print_mac_header (mac_t *mac) {
  printf ("--mac     -dmac %02x:%02x:%02x:%02x:%02x:%02x -smac %02x:%02x:%02x:%02x:%02x:%02x\n", 
    mac->dmac[0], mac->dmac[1], mac->dmac[2], mac->dmac[3], mac->dmac[4], mac->dmac[5], 
    mac->smac[0], mac->smac[1], mac->smac[2], mac->smac[3], mac->smac[4], mac->smac[5]);
}
void print_vlan_header (vlan_t *vlan) {
  printf ("--vlan    -tpid 0x%04x pri 0x%01x -cfi 0x%01x -vid 0x%03x\n", vlan->tpid, vlan->pri, vlan->cfi, vlan->vid);
}
void print_stag_header (stag_t *stag) {
  printf ("--stag    -tpid 0x%04x -pcp 0x%01x -dei 0x%01x -vid 0x%03x\n", stag->tpid, stag->pcp, stag->dei, stag->vid);
}
void print_etype_header (etype_t *etype) {
  printf ("--etype   -etype 0x%04x\n", etype->etype);
}
void print_llc_header (llc_t *llc) {
  printf ("--llc     -len 0x%04x -ssap 0x%04x -dsap 0x%04x -ctrl 0x%04x\n",   
                       llc->len, llc->ssap, llc->dsap, llc->ctrl);
}
void print_snap_header (snap_t *snap) {
  printf ("--snap    -oui=0x%08x\n", snap->oui);
}
void print_oam_ccm_pdu (oam_ccm_t *oam_ccm) {
  int i;
  printf ("--oam_ccm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags_rdi 0x%01x -flags_rsvd 0x%01x -flags_period 0x%01x -tlv_offset 0x%02x\n",  
      oam_ccm->lvl, oam_ccm->version, oam_ccm->opcode, oam_ccm->flags_rdi, oam_ccm->flags_rsvd, oam_ccm->flags_period, oam_ccm->tlv_offset);
  printf ("          -seq_number 0x%08x -mep_id 0x%04x\n", 
                        oam_ccm->seq_number, oam_ccm->mep_id);
  printf ("          -maid ");
  for(i=0; i<24; i++) {
    printf("%02x:", oam_ccm->maid[i]);
  }
  printf ("\n");

  printf ("                ");
  for(i=24; i<48; i++) {
    printf("%02x:", oam_ccm->maid[i]);
  }
  printf ("\n");

  printf ("          -tx_fcf 0x%08x -rx_fcb 0x%08x -tx_fcb 0x%08x\n", 
      oam_ccm->tx_fcf, oam_ccm->rx_fcb, oam_ccm->tx_fcb);
  printf ("          -reserved 0x%08x -end_tlv 0x%02x \n", 
      oam_ccm->reserved, oam_ccm->end_tlv);
  return;
}

void print_oam_lbm_pdu (oam_lbm_t *oam_lbm) {
  printf ("--oam_lbm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags 0x%02x -tlv_offset 0x%02x\n",  
      oam_lbm->lvl, oam_lbm->version, oam_lbm->opcode, oam_lbm->flags, oam_lbm->tlv_offset);
  printf ("          -seq_number 0x%08x -end_tlv 0x%02x\n", 
    oam_lbm->seq_number, oam_lbm->end_tlv);
  return;
}

void print_oam_ltm_pdu (oam_ltm_t *oam_ltm) {
  printf ("--oam_ltm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags 0x%02x -tlv_offset 0x%02x\n",  
      oam_ltm->lvl, oam_ltm->version, oam_ltm->opcode, oam_ltm->flags, oam_ltm->tlv_offset);
  printf ("          -seq_number 0x%08x -ttl 0x%02x\n", 
    oam_ltm->seq_number, oam_ltm->ttl);

  if (OPCODE_LTM == oam_ltm->opcode) {
    printf ("          -origin_mac %02x:%02x:%02x:%02x:%02x:%02x -target_mac %02x:%02x:%02x:%02x:%02x:%02x\n", 
      oam_ltm->origin_mac[0], oam_ltm->origin_mac[1], oam_ltm->origin_mac[2], 
      oam_ltm->origin_mac[3], oam_ltm->origin_mac[4], oam_ltm->origin_mac[5], 
      oam_ltm->target_mac[0], oam_ltm->target_mac[1], oam_ltm->target_mac[2], 
      oam_ltm->target_mac[3], oam_ltm->target_mac[4], oam_ltm->target_mac[5]);
    printf ("          -end_tlv 0x%02x\n", 
      oam_ltm->end_tlv); 
  } else {
    printf ("          -relay_action 0x%08x\n", 
      oam_ltm->relay_action); 
  }
 
  return;
}

void print_oam_lmm_pdu (oam_lmm_t *oam_lmm) {
  printf ("--oam_lmm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags 0x%02x -tlv_offset 0x%02x\n",  
      oam_lmm->lvl, oam_lmm->version, oam_lmm->opcode, oam_lmm->flags, oam_lmm->tlv_offset);
  printf ("          -tx_fcf 0x%08x -rx_fcf 0x%08x -tx_fcb 0x%08x\n", 
      oam_lmm->tx_fcf, oam_lmm->rx_fcf, oam_lmm->tx_fcb);
  printf ("          -end_tlv 0x%02x\n", 
    oam_lmm->end_tlv); 

  return;
}

void print_oam_1dmm_pdu (oam_1dmm_t *oam_1dmm) {
  printf ("--oam_1dmm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags 0x%02x -tlv_offset 0x%02x\n",  
      oam_1dmm->lvl, oam_1dmm->version, oam_1dmm->opcode, oam_1dmm->flags, oam_1dmm->tlv_offset);
  printf ("          -tx_timestamp_sec 0x%08x -tx_timestamp_nano 0x%08x\n", 
    oam_1dmm->tx_timestamp_sec, oam_1dmm->tx_timestamp_nano) ;
  printf ("          -rx_timestamp_sec 0x%08x -rx_timestamp_nano 0x%08x\n", 
    oam_1dmm->rx_timestamp_sec, oam_1dmm->rx_timestamp_nano) ;
  printf ("          -end_tlv 0x%02x\n", 
    oam_1dmm->end_tlv);
  return;
}

void print_oam_2dmm_pdu (oam_2dmm_t *oam_2dmm) {
  printf ("--oam_2dmm -lvl 0x%01x -version 0x%01x -opcode 0x%02x -flags 0x%02x -tlv_offset 0x%02x\n",  
      oam_2dmm->lvl, oam_2dmm->version, oam_2dmm->opcode, oam_2dmm->flags, oam_2dmm->tlv_offset);
  printf ("          -tx_timestamp_f_sec 0x%08x -tx_timestamp_f_nano 0x%08x\n", 
    oam_2dmm->tx_timestamp_f_sec, oam_2dmm->tx_timestamp_f_nano) ;
  printf ("          -rx_timestamp_f_sec 0x%08x -rx_timestamp_f_nano 0x%08x\n", 
    oam_2dmm->rx_timestamp_f_sec, oam_2dmm->rx_timestamp_f_nano) ;
  printf ("          -tx_timestamp_b_sec 0x%08x -tx_timestamp_b_nano 0x%08x\n", 
    oam_2dmm->tx_timestamp_b_sec, oam_2dmm->tx_timestamp_b_nano) ;
  printf ("          -rx_timestamp_b_sec 0x%08x -rx_timestamp_b_nano 0x%08x\n", 
    oam_2dmm->rx_timestamp_b_sec, oam_2dmm->rx_timestamp_b_nano) ;
  printf ("          -end_tlv 0x%02x\n", 
    oam_2dmm->end_tlv);
  return;
}

void print_bfd_header (bfd_t *bfd) {
  printf ("--bfd -version 0x%02x -diag 0x%02x -flags 0x%02x -state 0x%02x\n",  
      bfd->version, bfd->diag, bfd->flags, bfd->state);
  printf ("          -detect_mult 0x%02x -length 0x%02x\n", 
    bfd->detect_mult, bfd->length) ;
  printf ("          -my_discrim 0x%08x -your_discrim 0x%08x\n", 
    bfd->my_discrim, bfd->your_discrim) ;
  printf ("          -min_tx 0x%08x -min_rx 0x%08x\n", 
    bfd->min_tx, bfd->min_rx) ;
  printf ("          -echo 0x%08x\n", 
    bfd->echo);
  return;
}

void print_bpdu_header (bpdu_t *bpdu) {
  printf ("--bpdu -protocol_id 0x%04x -version 0x%02x -message_type 0x%02x -flags 0x%02x\n",  
      bpdu->protocol_id, bpdu->version, bpdu->message_type, bpdu->flags);
  printf ("          -root_pri 0x%04x -root_path_cost 0x%08x -bridge_pri 0x%04x\n", 
    bpdu->root_pri, bpdu->root_path_cost, bpdu->bridge_pri) ;
  printf ("          -root_mac %02x:%02x:%02x:%02x:%02x:%02x -bridge_mac %02x:%02x:%02x:%02x:%02x:%02x\n", 
    bpdu->root_mac[0], bpdu->root_mac[1], bpdu->root_mac[2], 
    bpdu->root_mac[3], bpdu->root_mac[4], bpdu->root_mac[5], 
    bpdu->bridge_mac[0], bpdu->bridge_mac[1], bpdu->bridge_mac[2], 
    bpdu->bridge_mac[3], bpdu->bridge_mac[4], bpdu->bridge_mac[5]);
  printf ("          -port_id 0x%04x -message_age 0x%04x -max_age 0x%4x\n", 
    bpdu->port_id, bpdu->message_age, bpdu->max_age) ;
  printf ("          -hello_time 0x%04x -forward_delay 0x%04x\n", 
    bpdu->forward_delay, bpdu->forward_delay) ;
  return;
}

void print_slow_header (slow_t *slow) {
  printf ("--slow -sub_type 0x%02x\n",  
      slow->sub_type);
  return;
}

void print_ipv4_header (ipv4_t *ipv4) {
  printf ("--ipv4    -ver 0x%01x -ihl 0x%01x -tos 0x%02x -length 0x%04x -id 0x%04x -df 0x%01x\n",  
                        ipv4->ver, ipv4->ihl, ipv4->tos, ipv4->length, ipv4->id, ipv4->df);
  printf ("          -mf 0x%01x -offset 0x%04x -proto 0x%02x -ttl 0x%02x -checksum 0x%04x\n",  
                        ipv4->mf, ipv4->offset, ipv4->proto, ipv4->ttl, ipv4->checksum);
  printf ("          -sa 0x%08x -da 0x%08x\n", 
                        ipv4->sa, ipv4->da);
  if (ipv4->ihl > 5) {
    printf ("          -ip_opt 0x%08x\n", ipv4->ip_opt);
  }
}
void print_ipv6_header (ipv6_t *ipv6) {
  printf ("--ipv6    -ver 0x%01x -tos 0x%02x -length 0x%04x -flow_label 0x%08x\n",  
                        ipv6->ver, ipv6->tos, ipv6->length, ipv6->flow_label);
  printf ("          -ttl 0x%02x -next_header 0x%02x\n",  
                        ipv6->ttl, ipv6->next_header);
  printf ("          -sa %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", 
    ipv6->sa[0], ipv6->sa[1], ipv6->sa[2], ipv6->sa[3], 
    ipv6->sa[4], ipv6->sa[5], ipv6->sa[6], ipv6->sa[7]);
  printf ("          -da %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", 
    ipv6->da[0], ipv6->da[1], ipv6->da[2], ipv6->da[3], 
    ipv6->da[4], ipv6->da[5], ipv6->da[6], ipv6->da[7]);
}

void print_itag_header (itag_t *itag) {
  printf ("--itag    -ipcp 0x%01x -idei 0x%01x -ncs 0x%01x -isid 0x%04x\n",   
                        itag->ipcp, itag->idei, itag->nca, itag->isid);
}
void print_mpls_header (mpls_t *mpls) {
  printf ("--mpls    -label 0x%05x -exp 0x%01x -s 0x%01x -ttl 0x%02x\n",    
                        mpls->label, mpls->exp, mpls->s, mpls->ttl);
}
void print_udp_header (udp_t *udp) {
  printf ("--udp     -sport 0x%04x -dport 0x%04x -len 0x%04x -checksum 0x%04x\n",    
                       udp->sport, udp->dport, udp->len, udp->checksum);
}
void print_tcp_header (tcp_t *tcp) {
  printf ("--tcp     -sport 0x%04x -dport 0x%04x -seqn 0x%04x -ackn 0x%04x -dofs 0x%04x\n",  
                       tcp->sport, tcp->dport, tcp->seqn, tcp->ackn, tcp->dofs);
  printf ("          -ecn 0x%01x -ctrl 0x%01x -wind 0x%01x -checksum 0x%04x -urgp 0x%01x\n", 
                       tcp->ecn, tcp->ctrl, tcp->wind, tcp->checksum, tcp->urgp);
  if (tcp->dofs > 5) {
    printf ("          -option=0x%08x\n", tcp->option);
  }
}
void print_igmp_header (igmp_t *igmp) {
  printf ("--igmp    -ver 0x%04x -type 0x%04x -checksum 0x%04x -group 0x%04x\n",
                        igmp->ver, igmp->type, igmp->checksum, igmp->group);
}
void print_pwe_data_ach_header(pwe_data_ach_t *pwe_data_ach) {
  printf ("--pwe_data_ach    -flag 0x%02x -frg 0x%02x -length 0x%02x -seqno 0x%04x\n",
                        pwe_data_ach->flag, pwe_data_ach->frg, pwe_data_ach->length, pwe_data_ach->seqno);
}
void print_pwe_ctrl_ach_header(pwe_ctrl_ach_t *pwe_ctrl_ach) {
  printf ("--pwe_ctrl_ach    -ctrl 0x%02x -ver 0x%02x -rsvd 0x%02x -channel 0x%04x\n",
                        pwe_ctrl_ach->ctrl, pwe_ctrl_ach->ver, pwe_ctrl_ach->rsvd, pwe_ctrl_ach->channel);
}
void print_gach_header(gach_t *gach) {
  printf ("--gach    -ver 0x%02x -rsvd 0x%02x -channel 0x%04x\n",
                        gach->ver, gach->rsvd, gach->channel);
}
void print_tlvhdr_header(tlvhdr_t *tlvhdr) {
  printf ("--tlvhdr    -length 0x%04x -rsvd 0x%04x\n",
                        tlvhdr->length, tlvhdr->rsvd);
}
void print_psc_header(psc_t *psc) {
  printf ("--psc    -ver 0x%02x -request 0x%02x -pt 0x%02x -r 0x%02x\n",
                        psc->ver, psc->request, psc->pt, psc->r);
  printf ("         -rsvd 0x%02x -fpath 0x%02x -path 0x%02x\n",
                        psc->rsvd, psc->fpath, psc->path);
}
void print_oam_mpls_header(oam_mpls_t *oam_mpls) {
  int i;
  int line_len = 8;

  printf ("--oam_mpls    -func_type 0x%02x -frequency 0x%02x -bip16 0x%04x\n",
                        oam_mpls->func_type, oam_mpls->frequency, oam_mpls->bip16);
  printf ("              -ttsi\n");
  for (i=0; i<TTSI_LENGTH; i++) {
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x: ", i);
    }
    printf ("%02x ", oam_mpls->ttsi[i]);
  }

  printf ("\n");
}
void print_ip_option_header(ip_option_t *ip_option) {
  int i;
  int line_len = 8;

  printf ("--ip_option    -option_code 0x%02x -length 0x%02x\n",
                        ip_option->option_code, ip_option->length);
  printf ("               -data\n");
  for (i=0; i<ip_option->length-1; i++) {
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x: ", i);
    }
    printf ("%02x ", ip_option->data[i]);
  }
  printf ("\n");
}

void print_hop_header(hop_t *hop) {
  printf ("--hop    -next_header 0x%02x -length 0x%02x, -opt_type %02x, -opt_len %02x\n",
                    hop->next_header, hop->length, hop->opt_type, hop->opt_len);
  printf ("         -data 0x%08x\n", hop->data);
}

void print_ptp_header(ptp_t *ptp) {
  printf ("--ptp    -message_type 0x%02x -version_ptp 0x%02x, -message_length %04x\n",
                    ptp->message_type, ptp->version_ptp, ptp->message_length);
  printf ("         -domain_number 0x%02x -flag_field 0x%04x\n", 
                    ptp->domain_number, ptp->flag_field);
  printf ("         -correction_field1 0x%08x -correction_field2 0x%08x\n", 
                    ptp->correction_field1, ptp->correction_field2);
  printf ("         -sourceport_identity1 0x%08x -sourceport_identity2 0x%08x -sourceport_identity3 0x%04x\n", 
                    ptp->sourceport_identity1, ptp->sourceport_identity2, ptp->sourceport_identity3);
  printf ("         -sequence_id 0x%04x -control_field 0x%02x -logmessage_interval 0x%02x\n", 
                    ptp->sequence_id, ptp->control_field, ptp->logmessage_interval);
}



void print_erh_qe_header (erh_qe_t *erh_qe) {
  printf ("--erh_qe  -frm_len 0x%04x -test 0x%04x -e 0x%01x -fdp 0x%01x -qid 0x%04x\n",
                    erh_qe->frm_len, erh_qe->test, erh_qe->e, erh_qe->fdp, erh_qe->qid);
  printf ("          -sid 0x%04x -mc 0x%01x -out_union 0x%05x -len_adj 0x%01x\n", 
                    erh_qe->sid, erh_qe->mc, erh_qe->out_union, erh_qe->len_adj);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x\n",
                    erh_qe->ttl, erh_qe->s, erh_qe->rdp, erh_qe->rcos);
}
void print_erh_isir_header (erh_isir_t *erh_isir) {
  printf ("--erh_isir -ksop 0x%02x -et 0x%01x -en 0x%01x -t 0x%01x -mc 0x%01x -lenadj 0x%01x -res 0x%01x -qid 0x%04x\n",
                    erh_isir->ksop, erh_isir->et, erh_isir->en, erh_isir->t, erh_isir->mc, erh_isir->lenadj, erh_isir->res, erh_isir->qid);
  printf ("          -out_union 0x%05x -sid 0x%04x\n",
                erh_isir->out_union, erh_isir->sid); 
  printf ("          -fdp 0x%01x -fcos 0x%01x -hdrcompr 0x%01x -oam 0x%01x -rcos 0x%01x -rdp 0x%01x -s 0x%01x -ttl_excidx 0x%02x\n", 
                    erh_isir->fdp, erh_isir->fcos, erh_isir->hdrcompr, erh_isir->oam, erh_isir->rcos, erh_isir->rdp, erh_isir->s, erh_isir->ttl_excidx );
}
void print_erh_esir_header (erh_esir_t *erh_esir) {
  printf ("--erh_esir -ksop 0x%02x -et 0x%01x -en 0x%01x -t 0x%01x -mc 0x%01x -lenadj 0x%01x  -out_union 0x%04x\n",
                    erh_esir->ksop, erh_esir->et, erh_esir->en, erh_esir->t, erh_esir->mc, erh_esir->lenadj, erh_esir->out_union);
  printf ("          -dest_port 0x%02x -sid 0x%04x \n",
                erh_esir->dest_port, erh_esir->sid); 
  printf ("          -fdp 0x%01x -fcos 0x%01x -hdrcompr 0x%01x -oam 0x%01x -rcos 0x%01x -rdp 0x%01x -s 0x%01x -ttl_excidx 0x%02x\n", 
      erh_esir->fdp, erh_esir->fcos, erh_esir->hdrcompr, erh_esir->oam, erh_esir->rcos, erh_esir->rdp, erh_esir->s, erh_esir->ttl_excidx );
}


void print_erh_iarad_header (erh_iarad_t *erh_iarad) {
  printf ("--erh_iarad ");
  if (is_higig()) {
    printf ("-ksop 0x%02x\n", erh_iarad->ksop);
    printf ("          ");
  }
  printf (           "-format 0x%01x -attr 0x%01x -reserved 0x%01x -sys_src_port 0x%04x \n",
          erh_iarad->format, erh_iarad->attr, erh_iarad->reserved, erh_iarad->sys_src_port);

  printf ("          -type 0x%01x -m 0x%01x -snoop_cmd 0x%01x -cos 0x%01x -dp 0x%01x -dest_data 0x%05x\n",
          erh_iarad->type, erh_iarad->m, erh_iarad->snoop_cmd, erh_iarad->cos, erh_iarad->dp, erh_iarad->dest_data);
  if (!is_higig()) {
    printf ("          -pad 0x%02x\n",
            erh_iarad->pad);
  }
  printf ("          -out_union 0x%04x -sid 0x%04x -fdp 0x%01x -fcos 0x%01x -hdrcompr 0x%01x -oam 0x%01x -rcos 0x%01x -rdp 0x%01x -s 0x%01x -ttl_excidx 0x%02x\n",
          erh_iarad->out_union, erh_iarad->sid, erh_iarad->fdp, erh_iarad->fcos, erh_iarad->hdrcompr, erh_iarad->oam, erh_iarad->rcos, erh_iarad->rdp, erh_iarad->s, erh_iarad->ttl_excidx);
}
void print_erh_earad_header (erh_earad_t *erh_earad) {
    printf ("--erh_earad ");
    if (is_higig()) {
      printf ("-ksop 0x%02x\n", erh_earad->ksop);
      printf ("          ");
    }
    printf (             "-act 0x%01x -dp 0x%01x -m 0x%01x -class 0x%01x -dest_port 0x%02x -sys_src_port 0x%04x -cud_reserved 0x%02x -cud 0x%05x\n",
            erh_earad->act, erh_earad->dp, erh_earad->m, erh_earad->class, erh_earad->dest_port, erh_earad->sys_src_port, erh_earad->cud_reserved, erh_earad->cud);
    if (!is_higig()) {
      printf ("          -pad 0x%02x\n",
              erh_earad->pad);
    }
    printf ("          -out_union 0x%04x -sid 0x%04x -fdp 0x%01x -fcos 0x%01x -hdrcompr 0x%01x -oam 0x%01x -rcos 0x%01x -rdp 0x%01x -s 0x%01x -ttl_excidx 0x%02x\n",
            erh_earad->out_union, erh_earad->sid, erh_earad->fdp, erh_earad->fcos, erh_earad->hdrcompr, erh_earad->oam, erh_earad->rcos, erh_earad->rdp, erh_earad->s, erh_earad->ttl_excidx);
}
void print_erh_ss_header (erh_ss_t *erh_ss) {
  printf ("--erh_ss  -test 0x%04x -ecn 0x%04x -ect 0x%04x -ksop 0x%04x\n",
                    erh_ss->test, erh_ss->ecn, erh_ss->ect, erh_ss->ksop);
  printf ("          -sid 0x%04x -out_union 0x%05x -qid 0x%04x -len_adj 0x%01x -mc 0x%01x\n", 
                    erh_ss->sid, erh_ss->out_union, erh_ss->qid, erh_ss->len_adj, erh_ss->mc);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x -lbid 0x%04x -fcos2 0x%01x -fdp 0x%01x\n",
                    erh_ss->ttl, erh_ss->s, erh_ss->rdp, erh_ss->rcos, erh_ss->lbid, erh_ss->fcos2, erh_ss->fdp);
}

void print_erh_qess_header (erh_qess_t *erh_qess) {
  printf ("--erh_qess -len_adj 0x%01x -frm_len 0x%04x -test 0x%04x -ecn 0x%04x -fdp 0x%04x -qid 0x%04x\n",
                    erh_qess->len_adj, erh_qess->frm_len, erh_qess->test, erh_qess->ecn, erh_qess->fdp, erh_qess->qid);
  printf ("          -sid 0x%04x -out_union 0x%05x -zero 0x%04x -ect 0x%04x -mc 0x%01x\n", 
                    erh_qess->sid, erh_qess->out_union, erh_qess->zero, erh_qess->ect, erh_qess->mc);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x -lbid 0x%04x -fcos2 0x%01x\n", 
                    erh_qess->ttl, erh_qess->s, erh_qess->rdp, erh_qess->rcos, erh_qess->lbid, erh_qess->fcos2);
}
void print_higig_header (higig_t *higig) {
  printf ("--higig -start 0x%02x -hgi 0x%02x -opcode 0x%02x -cos 0x%02x -cng 0x%02x -ingresstagged 0x%02x -l3 0x%02x\n",
                    higig->start, higig->hgi, higig->opcode, higig->cos, higig->cng, higig->ingresstagged, higig->l3);
  printf ("        -dstport 0x%02x -dstmod 0x%02x -srcport 0x%02x -srcmod 0x%02x -vlan 0x%04x -cfi 0x%02x -pri 0x%02x\n", 
                    higig->dstport, higig->dstmod, higig->srcport, higig->srcmod, higig->vlan, higig->cfi, higig->pri);
  printf ("        -hdrformat 0x%02x -hdrextlen 0x%02x -pfm 0x%02x -mirror 0x%02x -mirrordone 0x%02x -mirroronly 0x%02x\n", 
                    higig->hdrformat, higig->hdrextlen, higig->pfm, higig->mirror, higig->mirrordone, higig->mirroronly);
  printf ("        -dsttgid 0x%02x -dstt 0x%02x -labelpresent 0x%02x -vclabel 0x%05x\n", 
                    higig->dsttgid, higig->dstt, higig->labelpresent, higig->vclabel);
}
void print_higig2_header (higig2_t *higig2) {
  printf ("--higig2 -start 0x%02x -mcst 0x%02x -tc 0x%02x -dstmod 0x%02x -dstport 0x%02x -srcmod 0x%02x -srcport 0x%02x\n",
                    higig2->start, higig2->mcst, higig2->tc, higig2->dstmod, higig2->dstport, higig2->srcmod, higig2->srcport);
  printf ("         -lbid 0x%02x -dp 0x%02x -hdrextvalid 0x%02x -ppdtype 0x%02x\n", 
                    higig2->lbid, higig2->dp, higig2->hdrextvalid, higig2->ppdtype);
  if (higig2->ppdtype != 0) {
    return;
  }
  printf ("         -dstt 0x%02x -dsttgid 0x%02x -ingresstagged 0x%02x -mirroronly 0x%02x -mirrordone 0x%02x -mirror 0x%02x\n", 
                    higig2->dstt, higig2->dsttgid, higig2->ingresstagged, higig2->mirroronly, higig2->mirrordone, higig2->mirror);
  printf ("         -lblovltype 0x%02x -l3 0x%02x -labelpresent 0x%02x -vclabel 0x%05x -vlan 0x%04x\n", 
                    higig2->lblovltype, higig2->l3, higig2->labelpresent, higig2->vclabel, higig2->vlan);
  printf ("         -pfm 0x%02x -srct 0x%02x -preservedscp 0x%02x -preservedot1p 0x%02x -opcode 0x%02x -hdrextlen 0x%02x\n", 
                    higig2->pfm, higig2->srct, higig2->preservedscp, higig2->preservedot1p, higig2->opcode, higig2->hdrextlen);
}

void print_itmh_header (itmh_t *itmh) 
{
    printf ("--itmh    -type 0x%02x -p_t 0x%02x -pkt_head_len 0x%02x -dp 0x%02x\n",
                    itmh->type, itmh->p_t, itmh->pkt_head_len, itmh->dp);
    if (itmh->type == SBX_PKT_ITMH_TYPE_UC)
    {
        printf("          ucast_itmh: -uc_flow_id 0x%04x\n\r", itmh->uc_flow_id);
    }
    else
    {
        printf("          mcast_itmh: -mc_t_c 0x%04x,     multicast_id 0x%04x\n\r",
            itmh->mc_t_c, itmh->multicast_id);
    }
}



void print_nph_header (nph_t *nph) 
{
    printf ("--nph     -m_u_flag 0x%01x,     -main_type 0x%01x,    -sub_type 0x%01x\n",
                    nph->m_u_flag, nph->main_type, nph->sub_type);
    printf ("          -ptp_1588flag 0x%01x, -cos_dp 0x%02x,      -slot_in 0x%02x\n",
                    nph->ptp_1588flag, nph->cos_dp, nph->slot_in);
    printf ("          -trunk_flag 0x%01x,                -port_or_trunk_id 0x%02x\n",
                    nph->trunk_flag, nph->port_or_trunk_id);
    printf ("          -vlan_status 0x%01x,  -del_length 0x%02x,  -default_pri_cfi 0x%01x\n",
                      nph->vlan_status, nph->del_length, nph->default_pri_cfi);
    printf ("          -protect_status_section 0x%01x,    -protect_status_lsp 0x%01x\n",
                    nph->protect_status_section, nph->protect_status_lsp);
    printf ("          -protect_status_pw_uni 0x%01x,     -output_fp 0x%05x\n",
                    nph->protect_status_pw_uni, nph->output_fp);

    
    printf ("          -root_leaf 0x%01x,    -hub_spoke 0x%01x,    -tag_num 0x%01x\n",
                      nph->root_leaf, nph->hub_spoke, nph->tag_num);
    printf ("          -b_f 0x%01x,          -port_out 0x%03x,   -lrn_on 0x%01x\n",
                      nph->b_f, nph->port_out, nph->lrn_on);
    printf ("          -mstp_Lrn 0x%01x,     -msp_opo 0x%01x,      -vpn_id 0x%04x\n",
                      nph->mstp_Lrn, nph->msp_opo, nph->vpn_id);
    printf ("          -nph_word3 0x%08x\n", nph->nph_word3);
}

void print_nfh_header (nfh_t *nfh) {
  printf ("--nfh -destpoint 0x%02x -linktype 0x%02x -packettype_to 0x%02x -packetlen 0x%04x\n",
                    nfh->destpoint, nfh->linktype, nfh->packettype_to, nfh->packetlen);
  printf ("      -packettype 0x%02x -vlan_status 0x%02x -port 0x%04x -fp 0x%08x\n",
                    nfh->packettype, nfh->vlan_status, nfh->port, nfh->fp);
  printf ("      -desttype 0x%02x -vpnid 0x%04x -flowtype 0x%02x -greencap 0x%02x\n",
                    nfh->desttype, nfh->vpnid, nfh->flowtype, nfh->greencap);
  printf ("      -slot 0x%02x -to_fpga 0x%08x -cookielen 0x%02x -cookie 0x%08x\n",
                    nfh->slot, nfh->to_fpga, nfh->cookielen, nfh->cookie);

}

void print_fnh_full_fwd_header (fnh_full_fwd_t *fnh_full_fwd) {
  printf ("--fnh_full_fwd -cmheader 0x%08x -rsv0 0x%02x -ftype 0x%02x -rsv1 0x%02x\n",
                    fnh_full_fwd->cmheader, fnh_full_fwd->rsv0, fnh_full_fwd->ftype, fnh_full_fwd->rsv1);
  printf ("      -ptype 0x%02x -rsv2 0x%02x -port 0x%04x -rsv3 0x%02x\n",
                    fnh_full_fwd->ptype, fnh_full_fwd->rsv2, fnh_full_fwd->port, fnh_full_fwd->rsv3);
  printf ("      -slot 0x%02x -rsv4 0x%02x -cos 0x%02x\n",
                    fnh_full_fwd->slot, fnh_full_fwd->rsv4, fnh_full_fwd->cos);

}

void print_fnh_half_fwd_header (fnh_half_fwd_t *fnh_half_fwd) {
  printf ("--fnh_half_fwd -cmheader 0x%08x -rsv0 0x%02x -ftype 0x%02x -rsv1 0x%02x\n",
                    fnh_half_fwd->cmheader, fnh_half_fwd->rsv0, fnh_half_fwd->ftype, fnh_half_fwd->rsv1);
  printf ("      -ptype 0x%02x -rsv2 0x%02x -protect_status 0x%02x -dp 0x%02x\n",
                    fnh_half_fwd->ptype, fnh_half_fwd->rsv2, fnh_half_fwd->protect_status, fnh_half_fwd->dp);
  printf ("      -rsv3 0x%02x -cos 0x%02x -oampduoffset 0x%02x -rsv4 0x%04x\n",
                    fnh_half_fwd->rsv3, fnh_half_fwd->cos, fnh_half_fwd->oampduoffset, fnh_half_fwd->rsv4);
  printf ("      -outfp 0x%08x\n",
                    fnh_half_fwd->outfp);

}

void print_fnh_non_fwd_header (fnh_non_fwd_t *fnh_non_fwd) {
    printf ("--fnh_non_fwd -cmheader 0x%08x,   -ftype 0x%02x,   -ptype 0x%02x\n",
                    fnh_non_fwd->cmheader, fnh_non_fwd->ftype, fnh_non_fwd->ptype);
    printf ("              -slot 0x%02x,      -packets 0x%04x\n",
                     fnh_non_fwd->slot, fnh_non_fwd->packets);

}

void print_fnh_np_term_header (fnh_np_term_t *fnh_np_term) {
  printf ("--fnh_np_term -cmheader 0x%08x -rsv0 0x%02x -ftype 0x%02x -reserved 0x%08x\n",
                    fnh_np_term->cmheader, fnh_np_term->rsv0, fnh_np_term->ftype, fnh_np_term->reserved);

}

void print_raw_data_header (raw_data_t *raw_data) {
  int i;
  int line_len = 16;

  /* 
   * printf ("--raw_data  -length %d  -mode %d -value 0x%08x\n", 
   *             raw_data->entry.length, raw_data->mode, raw_data->value); 
   */

  printf ("--raw_data    -length %d\n", raw_data->entry.length);

  for (i=0; i<raw_data->entry.length; i++) {
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x  ", i);
    }
    printf ("%02x ", raw_data->raw_data[i]);
  }

  printf ("\n");

} 

void print_hex_data_header (hex_data_t *hex_data) {
  int i;
  int line_len = 16;

  printf ("--hex_data -length %d\n", hex_data->entry.length);

  for (i=0; i<hex_data->entry.length; i++) {
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x: ", i);
    }
    printf ("%02x ", hex_data->hex_data[i]);
  }

  printf ("\n");
}


/* ******************************
 * test for htype
 * ******************************/
int is_htype(char *arg) { 
  if (*arg != '-') return 0;
  arg++;
  if (*arg != '-') return 0;
  return 1;
}

/* ******************************
 * Main Build header
 * - options:
 *       create new packet
 *       modify fields (not implemented)
 *       prepend fields
 *       append fields
 * ******************************/
int header_build (op_e op, sbxpkt_t* pkt, int argc, char **argv) {
  int i,j;
  int num_fields;
  entry_desc_t *cur_hdr = NULL;
  entry_desc_t *next_hdr = NULL;

  erh_qe_t   *erh_qe;
  erh_isir_t *erh_isir;
  erh_esir_t *erh_esir;
  erh_iarad_t *erh_iarad;
  erh_earad_t *erh_earad;
  erh_ss_t   *erh_ss;
  erh_qess_t *erh_qess;
  higig_t *higig;
  higig2_t *higig2;
  mac_t   *mac;
  vlan_t  *vlan;
  stag_t  *stag;
  llc_t   *llc;
  snap_t  *snap;
  etype_t *etype;
  mpls_t  *mpls;
  itag_t  *itag;
  ipv4_t  *ipv4;
  ipv6_t  *ipv6;
  oam_ccm_t  *oam_ccm;
  oam_lbm_t  *oam_lbm;
  oam_ltm_t  *oam_ltm;
  oam_lmm_t  *oam_lmm;
  oam_1dmm_t  *oam_1dmm;
  oam_2dmm_t  *oam_2dmm;
  bfd_t   *bfd;
  bpdu_t  *bpdu;
  slow_t  *slow;
  tcp_t   *tcp;
  udp_t   *udp;
  igmp_t  *igmp;
  pwe_data_ach_t *pwe_data_ach;
  pwe_ctrl_ach_t *pwe_ctrl_ach;
  gach_t *gach;
  tlvhdr_t *tlvhdr;
  psc_t *psc;
  oam_mpls_t *oam_mpls;
  ip_option_t *ip_option;
  hop_t *hop;
  ptp_t *ptp;
  itmh_t *itmh;
  nph_t *nph;
  nfh_t *nfh;
  fnh_full_fwd_t *fnh_full_fwd;
  fnh_half_fwd_t *fnh_half_fwd;
  fnh_non_fwd_t *fnh_non_fwd;
  fnh_np_term_t *fnh_np_term;

  raw_data_t *raw_data;
  hex_data_t *hex_data;

  if (op == CREATE) {
    cur_hdr = (entry_desc_t *)pkt;
    cur_hdr->type = PACKET;
    cur_hdr->length = 0;
  } else if (op == PREPEND) {
    cur_hdr = (entry_desc_t *)pkt;
    next_hdr = cur_hdr->next;

    /*printf("header_builder: cur_hdr=%#X, next_hdr=%#x\n\r", (int)cur_hdr, (int)next_hdr);*/
  } else if (op == APPEND) {
    cur_hdr = (entry_desc_t *)pkt;
    if ((cur_hdr == NULL) || (cur_hdr->next == NULL)) {
      printf("no header found to append\n");
      return -1;
    }
    next_hdr = cur_hdr->next;
    while ((cur_hdr != NULL) && (next_hdr->type != RAW_DATA)) {
      if (cur_hdr->next == NULL) {
        printf("no header found to append\n");
        return -1;
      }
      if (((entry_desc_t*)cur_hdr->next)->next != NULL) {
        next_hdr = ((entry_desc_t*)cur_hdr->next)->next;
      }
      cur_hdr = cur_hdr->next;
    }
    if (cur_hdr == NULL) {
      /* fatal error */
      printf("expected raw header at end of packet\n");
      return -1;
    }
    /* adjust packet length to remove old data and free old data header  */
    pkt->entry.length -= next_hdr->length;
    sal_free (next_hdr);
  } else {
    printf("unexpected operation %d\n", op);
    return -1;
  }

  for (i=0; i <= argc; ) {
    /* first arg should be a htype */
    if (!(is_htype (argv[i]))) {
      printf("expected htype at %d, got %s\n", i, argv[i]);
      return -1;
    }

    /* find next htype */
    for (j=i+1; j <= argc; j++) {
      if (is_htype (argv[j])) {
        break;
      }
    }

    num_fields = j - i - 1;

    if (!sal_strcmp(argv[i], "--erh_qe")) {
      erh_qe = sal_alloc(sizeof(erh_qe_t), "erh_qe");
      erh_qe_build (erh_qe, i+1, i+num_fields, argv);
      /* print_erh_qe_header (erh_qe); */
      cur_hdr->next = (void *)erh_qe;
      cur_hdr = (entry_desc_t *)erh_qe;
    } else if (!sal_strcmp(argv[i], "--erh_isir")) {
      erh_isir = sal_alloc(sizeof(erh_isir_t),"erh_isir");
      erh_isir_build (erh_isir, i+1, i+num_fields, argv);
      /* print_erh_isir_header (erh_isir); */
      cur_hdr->next = (void *)erh_isir;
      cur_hdr = (entry_desc_t *)erh_isir;
    } else if (!sal_strcmp(argv[i], "--erh_esir")) {
      erh_esir = sal_alloc(sizeof(erh_esir_t),"erh_esir");
      erh_esir_build (erh_esir, i+1, i+num_fields, argv);
      /* print_erh_esir_header (erh_esir); */
      cur_hdr->next = (void *)erh_esir;
      cur_hdr = (entry_desc_t *)erh_esir;
    } else if (!sal_strcmp(argv[i], "--erh_iarad")) {
      erh_iarad = sal_alloc(sizeof(erh_iarad_t),"erh_iarad");
      erh_iarad_build (erh_iarad, i+1, i+num_fields, argv);
      /* print_erh_iarad_header (erh_iarad); */
      cur_hdr->next = (void *)erh_iarad;
      cur_hdr = (entry_desc_t *)erh_iarad;
    } else if (!sal_strcmp(argv[i], "--erh_earad")) {
      erh_earad = sal_alloc(sizeof(erh_earad_t),"erh_earad");
      erh_earad_build (erh_earad, i+1, i+num_fields, argv);
      /* print_erh_earad_header (erh_earad); */
      cur_hdr->next = (void *)erh_earad;
      cur_hdr = (entry_desc_t *)erh_earad;
    } else if (!sal_strcmp(argv[i], "--erh_ss")) {
      erh_ss = sal_alloc(sizeof(erh_ss_t),"erh_ss");
      erh_ss_build (erh_ss, i+1, i+num_fields, argv);
      /* print_erh_ss_header (erh_ss); */
      cur_hdr->next = (void *)erh_ss;
      cur_hdr = (entry_desc_t *)erh_ss;
    } else if (!sal_strcmp(argv[i], "--erh_qess")) {
      erh_qess = sal_alloc(sizeof(erh_qess_t),"erh_qess");
      erh_qess_build (erh_qess, i+1, i+num_fields, argv);
      /* print_erh_qess_header (erh_qess); */
      cur_hdr->next = (void *)erh_qess;
      cur_hdr = (entry_desc_t *)erh_qess;
    } else if (!sal_strcmp(argv[i], "--higig")) {
      higig = sal_alloc(sizeof(higig_t),"higig");
      higig_build (higig, i+1, i+num_fields, argv);
      /* print_higig_header (higig); */
      cur_hdr->next = (void *)higig;
      cur_hdr = (entry_desc_t *)higig;
    } else if (!sal_strcmp(argv[i], "--higig2")) {
      higig2 = sal_alloc(sizeof(higig2_t),"higig2");
      higig2_build (higig2, i+1, i+num_fields, argv);
      /* print_higig2_header (higig2); */
      cur_hdr->next = (void *)higig2;
      cur_hdr = (entry_desc_t *)higig2;
    } else if (!sal_strcmp(argv[i], "--mac")) {
      mac = sal_alloc(sizeof(mac_t),"mac");
      mac_build (mac, i+1, i+num_fields, argv);
      /* print_mac_header (mac); */
      cur_hdr->next = (void *)mac;
      cur_hdr = (entry_desc_t *)mac;
    } else if (!sal_strcmp(argv[i], "--vlan")) {
      vlan = sal_alloc(sizeof(vlan_t),"vlan");
      vlan_build (vlan, i+1, i+num_fields, argv);
      /* print_vlan_header (vlan); */
      cur_hdr->next = (void *)vlan;
      cur_hdr = (entry_desc_t *)vlan;
    } else if (!sal_strcmp(argv[i], "--stag")) {
      stag = sal_alloc(sizeof(stag_t),"stag");
      stag_build (stag, i+1, i+num_fields, argv);
      /* print_stag_header (stag); */
      cur_hdr->next = (void *)stag;
      cur_hdr = (entry_desc_t *)stag;
    } else if (!sal_strcmp(argv[i], "--etype")) {
      etype = sal_alloc(sizeof(etype_t),"etype");
      etype_build (etype, i+1, i+num_fields, argv);
      /* print_etype_header (etype); */
      cur_hdr->next = (void *)etype;
      cur_hdr = (entry_desc_t *)etype;
    } else if (!sal_strcmp(argv[i], "--llc")) {
      llc = sal_alloc(sizeof(llc_t),"llc");
      llc_build (llc, i+1, i+num_fields, argv);
      /* print_llc_header (llc); */
      cur_hdr->next = (void *)llc;
      cur_hdr = (entry_desc_t *)llc;
    } else if (!sal_strcmp(argv[i], "--snap")) {
      snap = sal_alloc(sizeof(snap_t),"snap");
      snap_build (snap, i+1, i+num_fields, argv);
      /* print_snap_header (snap); */
      cur_hdr->next = (void *)snap;
      cur_hdr = (entry_desc_t *)snap;
    } else if (!sal_strcmp(argv[i], "--ipv4")) {
      ipv4 = sal_alloc(sizeof(ipv4_t),"ipv4");
      ipv4_build (ipv4, i+1, i+num_fields, argv);
      /* print_ipv4_header (ipv4); */
      cur_hdr->next = (void *)ipv4;
      cur_hdr = (entry_desc_t *)ipv4;
    } else if (!sal_strcmp(argv[i], "--ipv6")) {
      ipv6 = sal_alloc(sizeof(ipv4_t),"ipv6");
      ipv6_build (ipv6, i+1, i+num_fields, argv);
      /* print_ipv6_header (ipv6); */
      cur_hdr->next = (void *)ipv6;
      cur_hdr = (entry_desc_t *)ipv6;
    } else if (!sal_strcmp(argv[i], "--oam_ccm")) {
      oam_ccm = sal_alloc(sizeof(oam_ccm_t),"oam_ccm");
      oam_ccm_build (oam_ccm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_ccm;
      cur_hdr = (entry_desc_t *)oam_ccm;
    } else if (!sal_strcmp(argv[i], "--oam_lbm")) {
      oam_lbm = sal_alloc(sizeof(oam_lbm_t),"oam_lbm");
      oam_lbm_build (oam_lbm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_lbm;
      cur_hdr = (entry_desc_t *)oam_lbm;
    } else if (!sal_strcmp(argv[i], "--oam_ltm")) {
      oam_ltm = sal_alloc(sizeof(oam_ltm_t),"oam_ltm");
      oam_ltm_build (oam_ltm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_ltm;
      cur_hdr = (entry_desc_t *)oam_ltm;
    } else if (!sal_strcmp(argv[i], "--oam_lmm")) {
      oam_lmm = sal_alloc(sizeof(oam_lmm_t),"oam_lmm");
      oam_lmm_build (oam_lmm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_lmm;
      cur_hdr = (entry_desc_t *)oam_lmm;
    } else if (!sal_strcmp(argv[i], "--oam_1dmm")) {
      oam_1dmm = sal_alloc(sizeof(oam_1dmm_t),"oam_1dmm");
      oam_1dmm_build (oam_1dmm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_1dmm;
      cur_hdr = (entry_desc_t *)oam_1dmm;
    } else if (!sal_strcmp(argv[i], "--oam_2dmm")) {
      oam_2dmm = sal_alloc(sizeof(oam_2dmm_t),"oam_2dmm");
      oam_2dmm_build (oam_2dmm, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)oam_2dmm;
      cur_hdr = (entry_desc_t *)oam_2dmm;
    } else if (!sal_strcmp(argv[i], "--bfd")) {
      bfd = sal_alloc(sizeof(bfd_t),"bfd");
      bfd_build (bfd, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)bfd;
      cur_hdr = (entry_desc_t *)bfd;
    } else if (!sal_strcmp(argv[i], "--bpdu")) {
      bpdu = sal_alloc(sizeof(bpdu_t),"bpdu");
      bpdu_build (bpdu, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)bpdu;
      cur_hdr = (entry_desc_t *)bpdu;
    } else if (!sal_strcmp(argv[i], "--slow")) {
      slow = sal_alloc(sizeof(slow_t),"slow");
      slow_build (slow, i+1, i+num_fields, argv);
      cur_hdr->next = (void *)slow;
      cur_hdr = (entry_desc_t *)slow;
    } else if (!sal_strcmp(argv[i], "--itag")) {
      itag = sal_alloc(sizeof(itag_t),"itag");
      itag_build (itag, i+1, i+num_fields, argv);
      /* print_itag_header (itag); */
      cur_hdr->next = (void *)itag;
      cur_hdr = (entry_desc_t *)itag;
    } else if (!sal_strcmp(argv[i], "--mpls")) {
      mpls = sal_alloc(sizeof(mpls_t),"mpls");
      mpls_build (mpls, i+1, i+num_fields, argv);
      /* print_mpls_header (mpls); */
      cur_hdr->next = (void *)mpls;
      cur_hdr = (entry_desc_t *)mpls;
    } else if (!sal_strcmp(argv[i], "--udp")) {
      udp = sal_alloc(sizeof(udp_t),"udp");
      udp_build (udp, i+1, i+num_fields, argv);
      /* print_udp_header (udp); */
      cur_hdr->next = (void *)udp;
      cur_hdr = (entry_desc_t *)udp;
    } else if (!sal_strcmp(argv[i], "--tcp")) {
      tcp = sal_alloc(sizeof(tcp_t),"tcp");
      tcp_build (tcp, i+1, i+num_fields, argv);
      /* print_tcp_header (tcp); */
      cur_hdr->next = (void *)tcp;
      cur_hdr = (entry_desc_t *)tcp;
    } else if (!sal_strcmp(argv[i], "--igmp")) {
      igmp = sal_alloc(sizeof(igmp_t),"igmp");
      igmp_build (igmp, i+1, i+num_fields, argv);
      /* print_igmp_header (igmp); */
      cur_hdr->next = (void *)igmp;
      cur_hdr = (entry_desc_t *)igmp;
    } else if (!sal_strcmp(argv[i], "--pwe_data_ach")) {
      pwe_data_ach = sal_alloc(sizeof(pwe_data_ach_t),"pwe_data_ach");
      pwe_data_ach_build (pwe_data_ach, i+1, i+num_fields, argv);
      /* print_pwe_data_ach_header (pwe_data_ach); */
      cur_hdr->next = (void *)pwe_data_ach;
      cur_hdr = (entry_desc_t *)pwe_data_ach;
    } else if (!sal_strcmp(argv[i], "--pwe_ctrl_ach")) {
      pwe_ctrl_ach = sal_alloc(sizeof(pwe_data_ach_t),"pwe_ctrl_ach");
      pwe_ctrl_ach_build (pwe_ctrl_ach, i+1, i+num_fields, argv);
      /* print_pwe_ctrl_ach_header (pwe_ctrl_ach); */
      cur_hdr->next = (void *)pwe_ctrl_ach;
      cur_hdr = (entry_desc_t *)pwe_ctrl_ach;
    } else if (!sal_strcmp(argv[i], "--gach")) {
      gach = sal_alloc(sizeof(gach_t),"gach");
      gach_build (gach, i+1, i+num_fields, argv);
      /* print_gach_header (gach); */
      cur_hdr->next = (void *)gach;
      cur_hdr = (entry_desc_t *)gach;
    } else if (!sal_strcmp(argv[i], "--tlvhdr")) {
      tlvhdr = sal_alloc(sizeof(gach_t),"tlvhdr");
      tlvhdr_build (tlvhdr, i+1, i+num_fields, argv);
      /* print_tlvhdr_header (tlvhdr); */
      cur_hdr->next = (void *)tlvhdr;
      cur_hdr = (entry_desc_t *)tlvhdr;
    } else if (!sal_strcmp(argv[i], "--psc")) {
      psc = sal_alloc(sizeof(psc_t),"psc");
      psc_build (psc, i+1, i+num_fields, argv);
      /* print_psc_header (psc); */
      cur_hdr->next = (void *)psc;
      cur_hdr = (entry_desc_t *)psc;
    } else if (!sal_strcmp(argv[i], "--oam_mpls")) {
      oam_mpls = sal_alloc(sizeof(oam_mpls_t),"oam_mpls");
      oam_mpls_build (oam_mpls, i+1, i+num_fields, argv);
      /* print_oam_mpls_header (oam_mpls); */
      cur_hdr->next = (void *)oam_mpls;
      cur_hdr = (entry_desc_t *)oam_mpls;
    } else if (!sal_strcmp(argv[i], "--ip_option")) {
      ip_option = sal_alloc(sizeof(ip_option_t),"ip_option");
      ip_option_build (ip_option, i+1, i+num_fields, argv);
      /* print_ip_option_header (ip_option); */
      cur_hdr->next = (void *)ip_option;
      cur_hdr = (entry_desc_t *)ip_option;
    } else if (!sal_strcmp(argv[i], "--hop")) {
      hop = sal_alloc(sizeof(hop_t),"hop");
      hop_build (hop, i+1, i+num_fields, argv);
      /* print_hop_header (hop); */
      cur_hdr->next = (void *)hop;
      cur_hdr = (entry_desc_t *)hop;
    } else if (!sal_strcmp(argv[i], "--ptp")) {
      ptp = sal_alloc(sizeof(ptp_t),"ptp");
      ptp_build (ptp, i+1, i+num_fields, argv);
      /* print_ptp_header (ptp); */
      cur_hdr->next = (void *)ptp;
      cur_hdr = (entry_desc_t *)ptp;
    } else if (!sal_strcmp(argv[i], "--itmh")) {
      itmh = sal_alloc(sizeof(itmh_t),"itmh");
      itmh_build (itmh, i+1, i+num_fields, argv);
      /* print_itmh_header (itmh); */
      cur_hdr->next = (void *)itmh;
      cur_hdr = (entry_desc_t *)itmh;
    } else if (!sal_strcmp(argv[i], "--nph")) {   
      nph = sal_alloc(sizeof(nph_t),"nph");
      nph_build (nph, i+1, i+num_fields, argv);
      /* print_nph_header (nph); */
      cur_hdr->next = (void *)nph;
      cur_hdr = (entry_desc_t *)nph;
    } else if (!sal_strcmp(argv[i], "--nfh")) {
      nfh = sal_alloc(sizeof(nfh_t),"nfh");
      nfh_build (nfh, i+1, i+num_fields, argv);
      /* print_nfh_header (nfh); */
      cur_hdr->next = (void *)nfh;
      cur_hdr = (entry_desc_t *)nfh;
    } else if (!sal_strcmp(argv[i], "--fnh_full_fwd")) {
      fnh_full_fwd = sal_alloc(sizeof(fnh_full_fwd_t),"fnh_full_fwd");
      fnh_full_fwd_build (fnh_full_fwd, i+1, i+num_fields, argv);
      /* print_fnh_full_fwd_header (fnh_full_fwd); */
      cur_hdr->next = (void *)fnh_full_fwd;
      cur_hdr = (entry_desc_t *)fnh_full_fwd;
    } else if (!sal_strcmp(argv[i], "--fnh_half_fwd")) {
      fnh_half_fwd = sal_alloc(sizeof(fnh_half_fwd_t),"fnh_half_fwd");
      fnh_half_fwd_build (fnh_half_fwd, i+1, i+num_fields, argv);
      /* print_fnh_half_fwd_header (fnh_half_fwd); */
      cur_hdr->next = (void *)fnh_half_fwd;
      cur_hdr = (entry_desc_t *)fnh_half_fwd;
    } else if (!sal_strcmp(argv[i], "--fnh_non_fwd")) {
      fnh_non_fwd = sal_alloc(sizeof(fnh_non_fwd_t),"fnh_non_fwd");
      fnh_non_fwd_build (fnh_non_fwd, i+1, i+num_fields, argv);
      /* print_fnh_non_fwd_header (fnh_non_fwd); */
      cur_hdr->next = (void *)fnh_non_fwd;
      cur_hdr = (entry_desc_t *)fnh_non_fwd;
    } else if (!sal_strcmp(argv[i], "--fnh_np_term")) {
      fnh_np_term = sal_alloc(sizeof(fnh_np_term_t),"fnh_np_term");
      fnh_np_term_build (fnh_np_term, i+1, i+num_fields, argv);
      /* print_fnh_np_term_header (fnh_np_term); */
      cur_hdr->next = (void *)fnh_np_term;
      cur_hdr = (entry_desc_t *)fnh_np_term;
    } else if (!sal_strcmp(argv[i], "--raw_data")) {
      raw_data = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data_build (raw_data, i+1, i+num_fields, argv);
      /* print_raw_data_header (raw_data); */
      cur_hdr->next = (void *)raw_data;
      cur_hdr = (entry_desc_t *)raw_data;
      /* Special case - last header */
      cur_hdr->next = NULL;
    } else if (!sal_strcmp(argv[i], "--hex_data")) {
      hex_data = sal_alloc(sizeof(hex_data_t),"hex_data");
      hex_data_build (hex_data, i+1, i+num_fields, argv);
      /* print_hex_data_header (hex_data); */
      cur_hdr->next = (void *)hex_data;
      cur_hdr = (entry_desc_t *)hex_data;
    } else {
      printf("header build: unexpected htype %s\n", argv[i]);
    }

    /* printf ("found htype %s and %d fields at %d\n", argv[i], num_fields, i); */
    pkt->entry.length += cur_hdr->length;

    i = j;
  }

  if ((op == CREATE) || (op == APPEND)) {
    /* add RAW_DATA if not specified */
    if (cur_hdr->type != RAW_DATA) {
      raw_data = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data->entry.type = RAW_DATA;
      if (pkt->entry.length < 64) {
        raw_data->entry.length = 64 - pkt->entry.length;
        pkt->entry.length = 64;
      } else {
        raw_data->entry.length = 0;
      }
      raw_data->value = 0xba53ba11;
      raw_data->mode = 0;
      stuff_data (raw_data);
      cur_hdr->next = (void *)raw_data;
      cur_hdr = (entry_desc_t *)raw_data;
      cur_hdr->next = NULL;
    }
  }
  if (op == PREPEND) {
    cur_hdr->next = next_hdr;

    /*printf("header_build11: cur_hdr=%#x, next_hdr=%#x\n\r", (int)cur_hdr, (int)next_hdr);*/

  }

  return 0;
}

/* ******************************
 * Convert_string
 * ******************************/
int convert_string(char *char_str, int *argc_return, char **argv) {

  char *next_str;
  char *curr_str;
  int  argc;
  int  i;

  char copy_str[MAXUSRSTR];
  for (i=0; i< MAXUSRSTR; i++) {
    copy_str[i] = '\0';
  }

  if (sal_strlen(char_str) > MAXUSRSTR) return -1;
  /* coverity[secure_coding] */
  sal_strcpy (copy_str, char_str);

  curr_str = copy_str;

  /* printf ("convert string \n"); */

  /* initialize */
  for(argc=0; argc < MAXARGS; argc++) {
    argv[argc][0] = '\0';
  }

  /* printf ("String = %s \n", curr_str); */

  /* skip leading white space */
  for (argc=0; argc < MAXARGS && (*curr_str) != '\0';) {
    while(*curr_str == ' ') {
      curr_str++;
    }
    /* find space or end */
    for (next_str = curr_str; *next_str; next_str++) {
      if (*next_str == ' ') {
        break;
      }
    }

    *next_str = '\0';
    /* printf ("argc = %d and string = %s \n", argc, curr_str); */

    /* coverity[secure_coding] */
    sal_strcpy (argv[argc++], curr_str);

    next_str++;
    curr_str = next_str;
  }

  /* empty command line */
  if (argc < 1) {
    argc = 1;
    argv[0][0] = '\0';
  }

  *argc_return = argc - 1;
  return 0;
}

/* ************************
 * Decodes and Encodes
 * ************************/
int decode_erh_qe (erh_qe_t *erh_qe, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_qe->qid  = (value >> 18) & 0x3fff;  /* includes fcos */
  erh_qe->fdp  = (value >> 16) & 0x3;
  erh_qe->e       = (value >> 15) & 0x1;
  erh_qe->test    = (value >> 14) & 0x1;
  erh_qe->frm_len = (value >> 0)  & 0x3ff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_qe->len_adj   = (value >> 28) & 0xf; 
  erh_qe->out_union = (value >>  8) & 0x3ffff;
  erh_qe->mc        = (value >>  7) & 0x1;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_qe->sid  = (value >> 18) & 0x3fff;
  
  /* erh_qe->swop = (value >> 16) & 0x3; */
  erh_qe->rcos = (value >> 11) & 0x7;
  erh_qe->rdp  = (value >> 9) & 0x3;
  erh_qe->s    = (value >> 8) & 0x1;
  erh_qe->ttl  = (value >> 0) & 0xff;

  erh_qe->entry.length = 12;
  return 0;
}
int encode_erh_qe (erh_qe_t *erh_qe, uint8 *data) {
  uint32 value;
  uint16 value16;
  uint8  value8;
 /* word 0 */
  value16 = (((erh_qe->qid & 0x3fff) >> 2) |
             ((erh_qe->fdp & 0x3)    >> 0));
  value16 = soc_htons (value16);
  sal_memcpy (data, &value16, 2);

  value16 = (((erh_qe->e       & 0x1)   << 15) |
             ((erh_qe->test    & 0x1)   << 14) |
             ((erh_qe->frm_len & 0x3ff) << 0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 2, &value16, 2);

 /* word 1 */
  value = (((erh_qe->len_adj   & 0xf)     << 28) |
           ((erh_qe->out_union & 0x3ffff) <<  8) |
           ((erh_qe->mc        & 0x1)     <<  7) |
           ((0                 & 0x7f)    <<  0));
  value = soc_htonl (value);
  sal_memcpy (data + 4, &value, 4);

 /* word 2 */
  value16 = (((erh_qe->sid & 0x3fff) << 2) |
             ((0           & 0x3)    << 0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 8, &value16, 2);

  value8 = (((erh_qe->rcos & 0x7) << 3) |
            ((erh_qe->rdp  & 0x3) << 1) |
            ((erh_qe->s    & 0x1) << 0));
  sal_memcpy (data + 10, &value8, 1);

  value8 = (erh_qe->ttl * 0xff);
  sal_memcpy (data + 11, &value8, 1);
  return 0;
}

/*
 * Ingress Sirius ERH
 *
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |      ksop             |et|en| t|mc|  lenadj   | res |        qid                              | 
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |     out_union (OI/VLAN/Multicast_Id)          |         sid                             |     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | fdp |  fcos  |               -                   |hc| rcos   | rdp | s|     ttl_excidx        |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * hc - hdrcompr
 */
int decode_erh_isir (erh_isir_t *erh_isir, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);

  erh_isir->ksop       = (value >> 24) & 0xff;
  erh_isir->et         = (value >> 23) & 0x1;
  erh_isir->en         = (value >> 22) & 0x1;
  erh_isir->t          = (value >> 21) & 0x1;
  erh_isir->mc         = (value >> 20) & 0x1;
  erh_isir->lenadj     = (value >> 16) & 0xf;
  erh_isir->res        = (value >> 14) & 0x3;
  erh_isir->qid        = (value >> 0)  & 0x3fff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_isir->out_union  = (value >> 16) & 0xffff;
  erh_isir->sid        = (value >> 2)  & 0x3fff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_isir->fdp        = (value >> 30) & 0x3;
  erh_isir->fcos       = (value >> 27) & 0x7;
  erh_isir->hdrcompr   = (value >> 26) & 0x1;
  erh_isir->oam        = (value >> 25) & 0x1;
  erh_isir->rcos       = (value >> 11) & 0x7;
  erh_isir->rdp        = (value >> 9)  & 0x3;
  erh_isir->s          = (value >> 8)  & 0x1;
  erh_isir->ttl_excidx = (value >> 0)  & 0xff;

  erh_isir->entry.length = 12;

  return 0;
}
int encode_erh_isir (erh_isir_t *erh_isir, uint8 *data) {
  uint32 value;

 /* word 0 */
  value = 0;
  value = (((erh_isir->ksop       & 0xff)   << 24) |
           ((erh_isir->et         & 0x1)    << 23) |
           ((erh_isir->en         & 0x1)    << 22) |
           ((erh_isir->t          & 0x1)    << 21) |
           ((erh_isir->mc         & 0x1)    << 20) |
           ((erh_isir->lenadj     & 0xF)    << 16) |
           ((erh_isir->res        & 0x3)    << 14) |
           ((erh_isir->qid        & 0x3fff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);

 /* word 1 */
  value = 0;
  value = (((erh_isir->out_union  & 0xffff) << 16) |
           ((erh_isir->sid        & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data + 4, &value, 4);

 /* word 2 */
  value = 0;
  value = (((erh_isir->fdp        & 0x3)    << 30) |
           ((erh_isir->fcos       & 0x7)    << 27) |
           ((erh_isir->hdrcompr   & 0x1)    << 26) |
           ((erh_isir->oam        & 0x1)    << 25) |
           ((erh_isir->rcos       & 0x7)    << 11) |
           ((erh_isir->rdp        & 0x3)    <<  9) |
           ((erh_isir->s          & 0x1)    <<  8) |
           ((erh_isir->ttl_excidx & 0xff)   <<  0));
  value = soc_htonl (value);
  sal_memcpy (data + 8, &value, 4);


  return 0;
}


/*
 * Egress Sirius ERH
 *
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |      ksop             | et|en| t|mc|  lenadj  |     out_union (OI/VLAN/Multicast_Id)          |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |  |   dest_port        |                       |         sid                             |     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | fdp |  fcos  |               -                   |hc| rcos   | rdp | s|     ttl_excidx        |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * hc - hdrcompr
 */
int decode_erh_esir (erh_esir_t *erh_esir, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);

  erh_esir->ksop       = (value >> 24) & 0xff;
  erh_esir->et         = (value >> 23) & 0x1;
  erh_esir->en         = (value >> 22) & 0x1;
  erh_esir->t          = (value >> 21) & 0x1;
  erh_esir->mc         = (value >> 20) & 0x1;
  erh_esir->lenadj     = (value >> 16) & 0xf;
  erh_esir->out_union  = (value >> 0)  & 0xffff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_esir->dest_port  = (value >> 24) & 0x7f;
  erh_esir->sid        = (value >> 2)  & 0x3fff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_esir->fdp        = (value >> 30) & 0x3;
  erh_esir->fcos       = (value >> 27) & 0x7;
  erh_esir->hdrcompr   = (value >> 26) & 0x1;
  erh_esir->oam        = (value >> 25) & 0x1;
  erh_esir->rcos       = (value >> 11) & 0x7;
  erh_esir->rdp        = (value >> 9)  & 0x3;
  erh_esir->s          = (value >> 8)  & 0x1;
  erh_esir->ttl_excidx = (value >> 0)  & 0xff;

  erh_esir->entry.length = 12;

  return 0;
}

int encode_erh_esir (erh_esir_t *erh_esir, uint8 *data) {
  uint32 value;

 /* word 0 */
  value = 0;
  value = (((erh_esir->ksop       & 0xff)   << 24) |
           ((erh_esir->et         & 0x1)    << 23) |
           ((erh_esir->en         & 0x1)    << 22) |
           ((erh_esir->t          & 0x1)    << 21) |
           ((erh_esir->mc         & 0x1)    << 20) |
           ((erh_esir->lenadj     & 0xF)    << 16) |
           ((erh_esir->out_union  & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);

 /* word 1 */
  value = 0;
  value = (((erh_esir->dest_port  & 0x7f)   << 24) |
           ((erh_esir->sid        & 0x3fff) <<  2));
  value = soc_htonl (value);
  sal_memcpy (data + 4, &value, 4);

 /* word 2 */
  value = 0;
  value = (((erh_esir->fdp        & 0x3)    << 30) |
           ((erh_esir->fcos       & 0x7)    << 27) |
           ((erh_esir->hdrcompr   & 0x1)    << 26) |
           ((erh_esir->oam        & 0x1)    << 25) |
           ((erh_esir->rcos       & 0x7)    << 11) |
           ((erh_esir->rdp        & 0x3)    <<  9) |
           ((erh_esir->s          & 0x1)    <<  8) |
           ((erh_esir->ttl_excidx & 0xff)   <<  0));
  value = soc_htonl (value);
  sal_memcpy (data + 8, &value, 4);

  return 0;
}


/*
 * Ingress ARAD ERH
 *
 * KSOP (HiGig)
 * +--+--+--+--+--+--+--+--+
 * | 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+
 * |      ksop             |
 * +--+--+--+--+--+--+--+--+
 *
 * PTCH-1
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | f|  attr  |  reserved |                 System Source Port            |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * ITMH
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | type| m| snoop cmd |   cos  |  dp |        forwarding action destination data                 |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * Pad (Interlaken)
 * +--+--+--+--+--+--+--+--+
 * | 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+
 * |        pad            |
 * +--+--+--+--+--+--+--+--+
 *
 * PPH
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |         Outheader_Index  / Vlan               |                  sid                    |     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | fdp | fcos   |               ?                      | rcos   | rdp | s|     ttl  (excidx)     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
int decode_erh_iarad (erh_iarad_t *erh_iarad, uint8 *data) {
  uint32 value;

  if (is_higig()) { /* HiGig */
    /* word 0, HiGig + PTCH-1 */
    sal_memcpy (&value, data, 4);
    value = soc_ntohl (value);

    erh_iarad->ksop         = (value >> 24) & 0xff;
    erh_iarad->format       = (value >> 23) & 0x1;
    erh_iarad->attr         = (value >> 20) & 0x7;
    erh_iarad->reserved     = (value >> 16) & 0xf;
    erh_iarad->sys_src_port = (value >> 00) & 0xffff;

    /* word 1, ITMH */
    sal_memcpy (&value, data + 4, 4);
    value = soc_ntohl (value);
    erh_iarad->type       = (value >> 30) & 0x3;
    erh_iarad->m          = (value >> 29) & 0x1;
    erh_iarad->snoop_cmd  = (value >> 25) & 0xf;
    erh_iarad->cos        = (value >> 22) & 0x7;
    erh_iarad->dp         = (value >> 20) & 0x3;
    erh_iarad->dest_data  = (value >> 00) & 0xfffff;
  } else { /* Interlaken */
    /* word 0, PTCH-1 + ITHM */
    sal_memcpy (&value, data, 4);
    value = soc_ntohl (value);

    erh_iarad->format       = (value >> 31) & 0x1;
    erh_iarad->attr         = (value >> 28) & 0x7;
    erh_iarad->reserved     = (value >> 24) & 0xf;
    erh_iarad->sys_src_port = (value >> 8) & 0xffff;

    erh_iarad->type       = (value >> 6) & 0x3;
    erh_iarad->m          = (value >> 5) & 0x1;
    erh_iarad->snoop_cmd  = (value >> 1) & 0xf;
    erh_iarad->cos        = (value >> 0) & 0x1;

    /* word 1, ITMH + PAD(Interlaken)*/
    sal_memcpy (&value, data + 4, 4);
    value = soc_ntohl (value);
    erh_iarad->cos        =((value >> 30) & 0x3) + (erh_iarad->cos << 2);
    erh_iarad->dp         = (value >> 28) & 0x3;
    erh_iarad->dest_data  = (value >>  8) & 0xfffff;

    erh_iarad->pad        = (value >>  0) & 0xff;
  }
  /* word 2, PPH 1/2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_iarad->out_union  = (value >> 16) & 0xffff;
  erh_iarad->sid        = (value >>  2) & 0x3fff;

  /* word 3, PPH 2/2 */
  sal_memcpy (&value, data + 12, 4);
  value = soc_ntohl (value);
  erh_iarad->fdp        = (value >> 30) & 0x3;
  erh_iarad->fcos       = (value >> 27) & 0x7;
  erh_iarad->hdrcompr   = (value >> 26) & 0x1;
  erh_iarad->oam        = (value >> 25) & 0x1;
  erh_iarad->rcos       = (value >> 11) & 0x7;
  erh_iarad->rdp        = (value >> 9)  & 0x3;
  erh_iarad->s          = (value >> 8)  & 0x1;
  erh_iarad->ttl_excidx = (value >> 0)  & 0xff;

  erh_iarad->entry.length = 16;

  return 0;
}
int encode_erh_iarad (erh_iarad_t *erh_iarad, uint8 *data) {
  uint32 value;

  if (is_higig()) { /* Higig */
    /* word 0 */
    value = 0;
    value = (((erh_iarad->ksop         & 0xff)   << 24) |
             ((erh_iarad->format       & 0x1)    << 23) |
             ((erh_iarad->attr         & 0x7)    << 20) |
             ((erh_iarad->reserved     & 0xf)    << 16) |
             ((erh_iarad->sys_src_port & 0xffff) << 00));
    value = soc_htonl (value);
    sal_memcpy (data, &value, 4);

    /* word 1 */
    value = 0;
    value = (((erh_iarad->type      & 0x3)    << 30) |
             ((erh_iarad->m         & 0x1)    << 29) |
             ((erh_iarad->snoop_cmd & 0xf)    << 25) |
             ((erh_iarad->cos       & 0x7)    << 22) |
             ((erh_iarad->dp        & 0x3)    << 20) |
             ((erh_iarad->dest_data & 0xfffff)<< 00));
    value = soc_htonl (value);
    sal_memcpy (data + 4, &value, 4);
  } else { /* Interlaken */
    /* word 0 */
    value = 0;
    value = (((erh_iarad->format       & 0x1)    << 31) |
             ((erh_iarad->attr         & 0x7)    << 28) |
             ((erh_iarad->reserved     & 0xf)    << 24) |
             ((erh_iarad->sys_src_port & 0xffff) <<  8) |

             ((erh_iarad->type        & 0x3)    << 6) |
             ((erh_iarad->m           & 0x1)    << 5) |
             ((erh_iarad->snoop_cmd   & 0xf)    << 1) |
             (((erh_iarad->cos  >> 2) & 0x1)    << 0));
    value = soc_htonl (value);
    sal_memcpy (data, &value, 4);

    /* word 1 */
    value = 0;
    value = (((erh_iarad->cos       & 0x3)     << 30) |
             ((erh_iarad->dp        & 0x3)     << 28) |
             ((erh_iarad->dest_data & 0xfffff) <<  8) |
             ((erh_iarad->pad       & 0xff)    <<  0));
    value = soc_htonl (value);
    sal_memcpy (data + 4, &value, 4);
  }

  /* word 2 */
  value = 0;
  value = (((erh_iarad->out_union & 0xffff)    << 16) |
           ((erh_iarad->sid       & 0x3fff)    <<  2));
  value = soc_htonl (value);
  sal_memcpy (data + 8, &value, 4);

   /* word 3 */
   value = 0;
   value = (((erh_iarad->fdp        & 0x3)    << 30) |
            ((erh_iarad->fcos       & 0x7)    << 27) |
            ((erh_iarad->hdrcompr   & 0x1)    << 26) |
            ((erh_iarad->oam        & 0x1)    << 25) |
            ((erh_iarad->rcos       & 0x7)    << 11) |
            ((erh_iarad->rdp        & 0x3)    << 9)  |
            ((erh_iarad->s          & 0x1)    << 8)  |
            ((erh_iarad->ttl_excidx & 0xff)   << 0));
   value = soc_htonl (value);
   sal_memcpy (data + 12, &value, 4);

  return 0;
}


/*
 * Egress Arad ERH
 *
 * +--+--+--+--+--+--+--+--+
 * | 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+
 * |      ksop             |
 * +--+--+--+--+--+--+--+--+
 *
 * OTHM + Extensions
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | act |  dp | m|  class |   Destination Port    |             SRC_SYS_PORT                      |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |   cud reserved  |                       CUD[17:0]                     |     ***
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * PPH
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |   Outheader_Index  / Vlan                     |                  sid                    |     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | fdp | fcos   |               ?                      | rcos   | rdp | s|     ttl  (excidx)     |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
int decode_erh_earad (erh_earad_t *erh_earad, uint8 *data) {
  uint32 value;

  if (is_higig()) { /* HiGig */
    /* word 0 */
    sal_memcpy (&value, data, 4);
    value = soc_ntohl (value);

    erh_earad->ksop         = (value >> 24) & 0xff;
    erh_earad->act          = (value >> 22) & 0x3;
    erh_earad->dp           = (value >> 20) & 0x3;
    erh_earad->m            = (value >> 19) & 0x1;
    erh_earad->class        = (value >> 16) & 0x7;
    erh_earad->dest_port    = (value >>  8) & 0xff;
    erh_earad->sys_src_port = (value >>  0) & 0xff;

   /* word 1 */
    sal_memcpy (&value, data + 4, 4);
    value = soc_ntohl (value);
    erh_earad->sys_src_port = ((value >> 24) & 0xff) | (erh_earad->sys_src_port << 8);
    erh_earad->cud_reserved =  (value >> 18) & 0x3f;
    erh_earad->cud          =  (value >>  0) & 0x3ffff;
  } else { /* Interlaken */
    /* word 0 */
    sal_memcpy (&value, data, 4);
    value = soc_ntohl (value);

    erh_earad->act          = (value >> 30) & 0x3;
    erh_earad->dp           = (value >> 28) & 0x3;
    erh_earad->m            = (value >> 27) & 0x1;
    erh_earad->class        = (value >> 24) & 0x7;
    erh_earad->dest_port    = (value >> 16) & 0xff;
    erh_earad->sys_src_port = (value >>  0) & 0xffff;

    /* word 1 */
    sal_memcpy (&value, data + 4, 4);
    value = soc_ntohl (value);
    erh_earad->cud_reserved =  (value >> 26) & 0x3f;
    erh_earad->cud          =  (value >>  8) & 0x3ffff;

    erh_earad->pad          =  (value >>  0) & 0xff;
  }

 /* word 2, PPH 1/2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_earad->out_union  = (value >> 16) & 0xffff;
  erh_earad->sid        = (value >> 02) & 0x3fff;

 /* word 3, PPH 2/2 */
  sal_memcpy (&value, data + 12, 4);
  value = soc_ntohl (value);
  erh_earad->fdp        = (value >> 30) & 0x3;
  erh_earad->fcos       = (value >> 27) & 0x7;
  erh_earad->hdrcompr   = (value >> 26) & 0x1;
  erh_earad->oam        = (value >> 25) & 0x1;
  erh_earad->rcos       = (value >> 11) & 0x7;
  erh_earad->rdp        = (value >> 9)  & 0x3;
  erh_earad->s          = (value >> 8)  & 0x1;
  erh_earad->ttl_excidx = (value >> 0)  & 0xff;

  erh_earad->entry.length = 16;

  return 0;
}

int encode_erh_earad (erh_earad_t *erh_earad, uint8 *data) {
  uint32 value;

  if (is_higig()) { /* HiGig */
    /* word 0 */
    value = 0;
    value = (((erh_earad->ksop               & 0xff) << 24) |
             ((erh_earad->act                & 0x3 ) << 22) |
             ((erh_earad->dp                 & 0x3 ) << 20) |
             ((erh_earad->m                  & 0x1 ) << 19) |
             ((erh_earad->class              & 0x7 ) << 16) |
             ((erh_earad->dest_port          & 0xff) <<  8) |
             ((erh_earad->sys_src_port       & 0xff) <<  0));
    value = soc_htonl (value);
    sal_memcpy (data, &value, 4);

    /* word 1 */
    value = 0;
    value = (((erh_earad->sys_src_port & 0xff   ) << 24) |
             ((erh_earad->cud_reserved & 0x3f   ) << 18) |
             ((erh_earad->cud          & 0x3ffff) <<  0));
    value = soc_htonl (value);
    sal_memcpy (data + 4, &value, 4);
  } else { /* Interlaken */
    /* word 0 */
    value = 0;
    value = (((erh_earad->act            & 0x3 )  << 30) |
             ((erh_earad->dp             & 0x3 )  << 28) |
             ((erh_earad->m              & 0x1 )  << 27) |
             ((erh_earad->class          & 0x7 )  << 24) |
             ((erh_earad->dest_port      & 0xff)  << 16) |
             ((erh_earad->sys_src_port   & 0xffff)<< 0));
    value = soc_htonl (value);
    sal_memcpy (data, &value, 4);

    /* word 1 */
    value = 0;
    value = (((erh_earad->cud_reserved & 0x3f   ) << 26) |
             ((erh_earad->cud          & 0x3ffff) <<  8) |
             ((erh_earad->pad          & 0xff   ) <<  0 ));
        value = soc_htonl (value);
    sal_memcpy (data + 4, &value, 4);
  }


 /* word 2 */
  value = 0;
  value = (((erh_earad->out_union  & 0xffff)    << 16) |
           ((erh_earad->sid        & 0x3fff)    <<  2));
  value = soc_htonl (value);
  sal_memcpy (data + 8, &value, 4);

 /* word 3 */
  value = 0;
  value = (((erh_earad->fdp        & 0x3)    << 30) |
           ((erh_earad->fcos       & 0x7)    << 27) |
           ((erh_earad->hdrcompr   & 0x1)    << 26) |
           ((erh_earad->oam        & 0x1)    << 25) |
           ((erh_earad->rcos       & 0x7)    << 11) |
           ((erh_earad->rdp        & 0x3)    << 9)  |
           ((erh_earad->s          & 0x1)    << 8)  |
           ((erh_earad->ttl_excidx & 0xff)   << 0));
   value = soc_htonl (value);
   sal_memcpy (data + 12, &value, 4);

  return 0;
}


int decode_erh_ss (erh_ss_t *erh_ss, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_ss->ksop      = (value >> 24) & 0xff;
  erh_ss->ect       = (value >> 23) & 0x1;
  erh_ss->ecn       = (value >> 22) & 0x1;
  erh_ss->test      = (value >> 21) & 0x1;
  erh_ss->mc        = (value >> 20) & 0x1;
  erh_ss->len_adj   = (value >> 16) & 0xf;
  erh_ss->out_union = (value >> 0)  & 0xffff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  /* erh_ss->rsvd0   = (value >> 30) & 0x3; */
  erh_ss->qid     = (value >> 16) & 0x3fff;  /* includes fcos */
  erh_ss->sid     = (value >>  0) & 0xffff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  /* erh_ss->rsvd1  = (value >> 24) & 0xff; */
  erh_ss->fdp    = (value >> 22) & 0x3;
  erh_ss->fcos2  = (value >> 19) & 0x7;
  erh_ss->lbid   = (value >> 16) & 0x7;
  erh_ss->rcos   = (value >> 12) & 0x7;
  erh_ss->rdp    = (value >>  9) & 0x3;
  erh_ss->s      = (value >>  8) & 0x1;
  erh_ss->ttl    = (value >>  0) & 0xff;

  erh_ss->entry.length = 12;
  return 0;
}
int encode_erh_ss (erh_ss_t *erh_ss, uint8 *data) {
  return 0;
}

int decode_erh_qess (erh_qess_t *erh_qess, uint8 *data) {
  uint32 value;
  uint16 value16;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_qess->qid     = (value >> 18) & 0x3fff;  /* includes fcos */
  erh_qess->fdp     = (value >> 16) & 0x3;
  erh_qess->ecn     = (value >> 15) & 0x1;
  erh_qess->test    = (value >> 14) & 0x1;
  erh_qess->frm_len = (value >>  0) & 0x3ff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_qess->len_adj   = (value >> 28) & 0xf;
  erh_qess->out_union = (value >> 14) & 0x3fff;
  erh_qess->mc        = (value >>  0) & 0x1;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_qess->sid    = (value >> 16) & 0xffff;
  /* erh_qess->fdp2   = (value >> 14) & 0x3; */
  erh_qess->fcos2  = (value >> 11) & 0x7;
  erh_qess->lbid   = (value >>  8) & 0x7;
  erh_qess->zero   = (value >>  0) & 0xff;

  sal_memcpy (&value16, data + 12, 2);
  value16 = soc_ntohs (value);
  erh_qess->rcos   = (value16 >> 12) & 0x7;
  erh_qess->rdp    = (value16 >>  9) & 0x3;
  erh_qess->s      = (value16 >>  8) & 0x1;
  erh_qess->ttl    = (value16 >>  0) & 0xff;

  erh_qess->entry.length = 14;
  return 0;
}
int encode_erh_qess (erh_qess_t *erh_qess, uint8 *data) {
  return 0;
}

int decode_higig (higig_t *higig, uint8 *data) {
  uint8  hgicng1hxlsm6dm6;
  uint16 pricfivlan;
  uint8  sm40op;
  uint8  pfmsp50;
  uint8  dp40cos;
  uint8  hfcng0dm40;
  uint8  ttgidtm;
  uint8  sm5dm5l3lpl1916;
  uint16 l150;

  higig->entry.length = 12;

  /* word 1 */
  higig->start = data[0];
  hgicng1hxlsm6dm6 = data[1];
  sal_memcpy(&pricfivlan, data+2, 2);
  pricfivlan = soc_ntohs (pricfivlan);

  /* word 2 */
  sm40op = data[4];
  pfmsp50 = data[5];
  dp40cos = data[6];
  hfcng0dm40 = data[7];
  
  /* word 3 */
  ttgidtm = data[8];
  sm5dm5l3lpl1916 = data[9];
  sal_memcpy(&l150, data+10, 2);
  l150 = soc_ntohs(l150);

  higig->hgi = ((hgicng1hxlsm6dm6) >> 6) & 0x3;
  higig->dstport = dp40cos >> 3;
  higig->dstmod = ((hgicng1hxlsm6dm6 & 1) << 6) + 
                   (((sm5dm5l3lpl1916 >> 6) & 1) << 5) + 
                   (hfcng0dm40 & 0x1f);
  higig->srcport = pfmsp50 & 0x3f;
  higig->srcmod = (((hgicng1hxlsm6dm6 >> 1) & 1) << 6) +
                  (((sm5dm5l3lpl1916 >> 7) & 1) << 5) +
                  ((sm40op >> 3) & 0x1f);
  higig->hdrformat = (hfcng0dm40 >> 6) & 3;
  higig->cng = (((hgicng1hxlsm6dm6 >> 5) & 0x1) << 1) +
               ((hfcng0dm40 >> 5) & 0x1);
  higig->hdrextlen = (hgicng1hxlsm6dm6 >> 2) & 0x7;
  higig->vlan = pricfivlan & 0xfff;
  higig->cfi = (pricfivlan >> 12) & 1;
  higig->pri = (pricfivlan >> 13) & 0x7;
  higig->opcode = sm40op & 7;
  higig->pfm = (pfmsp50 >> 6) & 3;
  higig->cos = dp40cos & 7;
  higig->mirror = ttgidtm & 1;
  higig->mirrordone = (ttgidtm >> 1) & 1;
  higig->mirroronly = (ttgidtm >> 2) & 1;
  higig->ingresstagged = (ttgidtm >> 3) & 1;
  higig->dsttgid = (ttgidtm >> 4) & 7;
  higig->dstt = (ttgidtm >> 7) & 1;
  higig->vclabel = ((sm5dm5l3lpl1916 & 0xf) << 16) + l150;
  higig->labelpresent = (sm5dm5l3lpl1916 >> 4) & 1;
  higig->l3 = (sm5dm5l3lpl1916 >> 5) & 1;

  return 0;
}

int encode_higig (higig_t *higig, uint8 *data) {
  uint8  hgicng1hxlsm6dm6 = 0;
  uint16 pricfivlan = 0;
  uint8  sm40op = 0;
  uint8  pfmsp50 = 0;
  uint8  dp40cos = 0;
  uint8  hfcng0dm40 = 0;
  uint8  ttgidtm = 0;
  uint8  sm5dm5l3lpl1916 = 0;
  uint16 l150 = 0;

  hgicng1hxlsm6dm6 = ((higig->hgi & 0x3) << 6) |
                     ((higig->dstmod >> 6) & 0x1) |
                     (((higig->srcmod >> 6) & 0x1) << 1) |
                     (((higig->cng >> 1) & 0x1) << 5) |
                     ((higig->hdrextlen & 0x7) << 2);
  pricfivlan = (higig->vlan & 0xfff) | 
               ((higig->cfi & 0x1) << 12) | 
               ((higig->pri & 0x7) << 13);
  sm40op = ((higig->srcmod & 0x1f) << 3) |
           (higig->opcode & 0x7);
  pfmsp50 = (higig->srcport & 0x3f) |
            ((higig->pfm & 0x3) << 6);
  dp40cos = ((higig->dstport & 0x1f) << 3) |
            (higig->cos & 0x7);
  hfcng0dm40 = (higig->dstmod & 0x1f) |
               ((higig->hdrformat & 0x3) << 6) |
               ((higig->cng & 0x1) << 5);
  ttgidtm = (higig->mirror & 0x1) |
            ((higig->mirrordone & 0x1) << 1) |
            ((higig->mirroronly & 0x1) << 2) |
            ((higig->ingresstagged & 0x1) << 3) |
            ((higig->dsttgid & 0x7) << 4) |
            ((higig->dstt & 0x1) << 7);
  sm5dm5l3lpl1916 = (((higig->dstmod >> 5) & 0x1) << 6) |
                    (((higig->srcmod >> 5) & 0x1) << 7) |
                    ((higig->vclabel >> 16) & 0xf) |
                    ((higig->labelpresent & 0x1) << 4) |
                    ((higig->l3 & 0x1) << 5);
  l150 = higig->vclabel & 0xffff;

  /* word 1 */
  data[0] = higig->start;
  data[1] = hgicng1hxlsm6dm6;
  pricfivlan = soc_htons (pricfivlan);
  sal_memcpy (data + 2, &pricfivlan, 2);

  /* word 2 */
  data[4] = sm40op;
  data[5] = pfmsp50;
  data[6] = dp40cos;
  data[7] = hfcng0dm40;

  /* word 3 */
  data[8] = ttgidtm;
  data[9] = sm5dm5l3lpl1916;
  l150 = soc_htons (l150);
  sal_memcpy (data + 10, &l150, 2);

  return 0;
}

int decode_higig2 (higig2_t *higig2, uint8 *data) {
  uint8 value8 = 0;

  higig2->entry.length = 8;
  /* word 1 */
  higig2->start = data[0];
  value8 = data[1];
  higig2->mcst = (value8 >> 4) & 0x1;
  higig2->tc = value8 & 0xf;
  higig2->dstmod = data[2];
  higig2->dstport = data[3];

  /* word 2 */
  higig2->srcmod = data[4];
  higig2->srcport = data[5];
  higig2->lbid = data[6];
  value8 = data[7];
  higig2->dp = value8 >> 6;
  higig2->hdrextvalid = (value8 >> 5) & 0x1;
  higig2->ppdtype = value8 & 0x7;

  if (higig2->ppdtype != 0) {
    return 0;
  }
  higig2->entry.length = 16;

  /* word 3 */
  value8 = data[8];
  higig2->dstt = value8 >> 7;
  higig2->dsttgid = (value8 >> 4) & 0x7;
  higig2->ingresstagged = (value8 >> 3) & 0x1;
  higig2->mirroronly = (value8 >> 2) & 0x1;
  higig2->mirrordone = (value8 >> 1) & 0x1;
  higig2->mirror = (value8 >> 0) & 0x1;
  value8 = data[9];
  higig2->lblovltype = (value8 >> 6) & 0x3;
  higig2->l3 = (value8 >> 5) & 0x1;
  higig2->labelpresent = (value8 >> 4) & 0x1;
  higig2->vclabel = (value8 & 0xf) << 16;
  value8 = data[10];
  higig2->vclabel += (value8 << 8);
  value8 = data[11];
  higig2->vclabel += value8;

  /* word 4 */
  value8 = data[12];
  higig2->vlan = value8 << 8;
  value8 = data[13];
  higig2->vlan += value8;
  value8 = data[14];
  higig2->pfm = (value8 >> 6) & 0x3;
  higig2->srct = (value8 >> 5) & 0x1;
  higig2->preservedscp = (value8 >> 4) & 0x1;
  higig2->preservedot1p = (value8 >> 3) & 0x1;
  higig2->opcode = (value8 >> 0) & 0x7;
  value8 = data[15];
  higig2->hdrextlen = (value8 >> 5) & 0x7;
  
  return 0;
}

int encode_higig2 (higig2_t *higig2, uint8 *data) {

  /* word 1 */
  data[0] = higig2->start;
  data[1] = ((higig2->mcst & 0x1) << 4) |
            (higig2->tc & 0xf);
  data[2] = higig2->dstmod;
  data[3] = higig2->dstport;

  /* word 2 */
  data[4] = higig2->srcmod;
  data[5] = higig2->srcport;
  data[6] = higig2->lbid;
  data[7] = ((higig2->dp & 0x3) << 6) |
            ((higig2->hdrextvalid & 0x1) << 5) |
            (higig2->ppdtype & 0x7);

  if (higig2->ppdtype != 0) {
    return 0;
  }

  /* word 3 */
  data[8] = ((higig2->dstt & 0x1) << 7) |
            ((higig2->dsttgid & 0x7) << 4) |
            ((higig2->ingresstagged & 0x1) << 3) |
            ((higig2->mirroronly & 0x1) << 2) |
            ((higig2->mirrordone & 0x1) << 1) |
            ((higig2->mirror & 0x1) << 0);
  data[9] = ((higig2->lblovltype & 0x3) << 6) |
            ((higig2->l3 & 0x1) << 5) |
            ((higig2->labelpresent & 0x1) << 4) |
            ((higig2->vclabel >> 16) & 0xf);
  data[10] = (higig2->vclabel >> 8) & 0xff;
  data[11] = (higig2->vclabel >> 0) & 0xff;

  /* word 4 */
  data[12] = (higig2->vlan >> 8) & 0xf;
  data[13] = higig2->vlan & 0xff;
  data[14] = ((higig2->pfm & 0x3) << 6) |
             ((higig2->srct & 0x1) << 5) |
             ((higig2->preservedscp & 0x1) << 4) |
             ((higig2->preservedot1p & 0x1) << 3) |
             (higig2->opcode & 0x7);
  data[15] = (higig2->hdrextlen & 0x7) << 5;
  
  return 0;
}

int decode_mac (mac_t *mac, uint8 *data) {
  sal_memcpy (mac->dmac, data, 6);
  sal_memcpy (mac->smac, data + 6, 6);
  mac->entry.length = 12;
  return 0;
}
int encode_mac (mac_t *mac, uint8 *data) {
  sal_memcpy (data, mac->dmac, 6);
  sal_memcpy (data + 6, mac->smac, 6);
  return 0;
}

int decode_vlan (vlan_t *vlan, uint32 value) {
  vlan->tpid = (value >> 16) & 0xffff;
  vlan->pri  = (value >> 13) & 0x7;
  vlan->cfi  = (value >> 12) & 0x1;
  vlan->vid  = (value >>  0) & 0xfff;
  vlan->entry.length = 4;
  return 0;
}
int encode_vlan (vlan_t *vlan, uint8 *data) {
  uint32 value;
  value = (((vlan->tpid & 0xffff) << 16) | 
           ((vlan->pri  & 0x7)    << 13) |
           ((vlan->cfi  & 0x1)    << 12) |
           ((vlan->vid  & 0xfff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_stag (stag_t *stag, uint32 value) {
  stag->tpid = (value >> 16) & 0xffff;
  stag->pcp  = (value >> 13) & 0x7;
  stag->dei  = (value >> 12) & 0x1;
  stag->vid  = (value >>  0) & 0xfff;
  stag->entry.length = 4;
  return 0;
}
int encode_stag (stag_t *stag, uint8 *data) {
  uint32 value;
  value = (((stag->tpid & 0xffff) << 16) |
           ((stag->pcp  & 0x7)    << 13) |
           ((stag->dei  & 0x1)    << 12) |
           ((stag->vid  & 0xfff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_etype (etype_t *etype, unsigned int value) {
  etype->etype = (value >> 16) & 0xffff;
  etype->entry.length = 2;
  return 0;
}
int encode_etype (etype_t *etype, uint8 *data) {
  uint16 value;
  value = ((etype->etype & 0xffff)); 
  value = soc_htons (value);
  sal_memcpy (data, &value, 2);
  return 0;
}

int decode_mpls (mpls_t *mpls, uint32 value) {
  mpls->label = (value >> 12) & 0xfffff;
  mpls->exp   = (value >>  9) & 0x7;
  mpls->s     = (value >>  8) & 0x1;
  mpls->ttl   = (value >>  0) & 0xff;
  mpls->entry.length = 4;
  return 0;
}
int encode_mpls (mpls_t *mpls, uint8 *data) {
  uint32 value;
  value = (((mpls->label & 0xfffff)  << 12) |
           ((mpls->exp   &     0x7)  <<  9) |
           ((mpls->s     &     0x1)  <<  8) |
           ((mpls->ttl   &    0xff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_llc (llc_t *llc, uint8 *data) {
  uint16 value16;

  sal_memcpy (&value16, data, 2);
  value16     = soc_ntohs (value16);
  llc->len    = value16;
  sal_memcpy (&value16, data + 2, 2);
  value16     = soc_ntohs (value16);
  llc->ssap   = value16;
  sal_memcpy (&value16, data + 4, 2);
  value16     = soc_ntohs (value16);
  llc->dsap   = value16;
  llc->ctrl    = data[6];

  llc->entry.length = 7;
  return 0;
}
int encode_llc (llc_t *llc, uint8 *data) {
  uint16 value16;

  value16 = llc->len;
  value16 = soc_htons (value16);
  sal_memcpy (data, &value16, 2);

  value16 = llc->ssap;
  value16 = soc_htons (value16);
  sal_memcpy (data + 2, &value16, 2);

  value16 = llc->dsap;
  value16 = soc_htons (value16);
  sal_memcpy (data + 4, &value16, 2);

  data[6] = llc->ctrl;

  return 0;
}

int decode_snap (snap_t *snap, uint8 *data) {
  sal_memcpy (&snap->oui, data, 3);
  snap->entry.length = 3;
  return 0;
}
int encode_snap (snap_t *snap, uint8 *data) {
  sal_memcpy (data, &snap->oui, 3);
  return 0;
}

int decode_oam_ccm (oam_ccm_t *oam_ccm, uint8 *data) {
  uint32 value;

  /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  oam_ccm->lvl           = (value >> 29) & 0x7;
  oam_ccm->version       = (value >> 24) & 0x1f;
  oam_ccm->opcode        = (value >> 16) & 0xff;
  oam_ccm->flags_rdi     = (value >> 15) & 0x1;
  oam_ccm->flags_rsvd    = (value >> 11) & 0xf;
  oam_ccm->flags_period  = (value >> 8)  & 0x7;
  oam_ccm->tlv_offset    = (value >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  oam_ccm->seq_number    = value;

  /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  oam_ccm->mep_id        = (value >> 16) & 0xffff;

  /* maid */
  sal_memcpy (oam_ccm->maid, data + 10, 48);

  sal_memcpy (&value, data + 58, 4);
  oam_ccm->tx_fcf        = value;

  sal_memcpy (&value, data + 62, 4);
  oam_ccm->rx_fcb        = value;

  sal_memcpy (&value, data + 66, 4);
  oam_ccm->tx_fcb        = value;

  sal_memcpy (&value, data + 70, 4);
  oam_ccm->reserved      = value;

  oam_ccm->end_tlv       = data[74];

  oam_ccm->entry.length = 75;

  return 0;

}

int encode_oam_ccm (oam_ccm_t *oam_ccm, uint8 *data) {
  uint16 value16;
  uint32 value32;

  /* word 0 */
  value32 = (((oam_ccm->lvl          & 0x7)     << 29) |
         ((oam_ccm->version      & 0x1f)    << 24) |
         ((oam_ccm->opcode       & 0xff)    << 16) |
         ((oam_ccm->flags_rdi    & 0x1)     << 15) |
         ((oam_ccm->flags_rsvd   & 0xf)     << 11) |
         ((oam_ccm->flags_period & 0x7)     << 8)  |
         ((oam_ccm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_ccm->seq_number ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  value16 = oam_ccm->mep_id;
  value16 = soc_htons (value16);
  sal_memcpy (data + 8, &value16, 2);

  /* maid */
  sal_memcpy (data + 10, oam_ccm->maid, 48);

  value32 = oam_ccm->tx_fcf;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 58, &value32, 4);

  value32 = oam_ccm->rx_fcb;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 62, &value32, 4);

  value32 = oam_ccm->tx_fcb;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 66, &value32, 4);

  value32 = oam_ccm->reserved;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 70, &value32, 4);

  data[74] = oam_ccm->end_tlv;


  return 0;
}

int decode_oam_lbm (oam_lbm_t *oam_lbm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  oam_lbm->lvl           = (value32 >> 29) & 0x7;
  oam_lbm->version       = (value32 >> 24) & 0x1f;
  oam_lbm->opcode        = (value32 >> 16) & 0xff;
  oam_lbm->flags         = (value32 >> 8)  & 0xff;
  oam_lbm->tlv_offset    = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  oam_lbm->seq_number    = value32;

  /* word 2 */
  oam_lbm->end_tlv       = data[8];

  oam_lbm->entry.length = 9;

  return 0;
}

int encode_oam_lbm (oam_lbm_t *oam_lbm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((oam_lbm->lvl          & 0x7)     << 29) |
         ((oam_lbm->version      & 0x1f)    << 24) |
         ((oam_lbm->opcode       & 0xff)    << 16) |
         ((oam_lbm->flags        & 0xff)    << 8)  |
         ((oam_lbm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_lbm->seq_number ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  data[8] = oam_lbm->end_tlv;

  return 0;
}

int decode_oam_ltm (oam_ltm_t *oam_ltm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  oam_ltm->lvl            = (value32 >> 29) & 0x7;
  oam_ltm->version        = (value32 >> 24) & 0x1f;
  oam_ltm->opcode         = (value32 >> 16) & 0xff;
  oam_ltm->flags          = (value32 >> 8)  & 0xff;
  oam_ltm->tlv_offset     = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  oam_ltm->seq_number     = value32;

  /* word 2 */
  oam_ltm->ttl            = data[8];

  if(OPCODE_LTM == oam_ltm->opcode) {
    sal_memcpy (oam_ltm->origin_mac, data + 9, 6);
    sal_memcpy (oam_ltm->target_mac, data + 15, 6);
    oam_ltm->end_tlv      = data[21];
    oam_ltm->entry.length = 22;
  } else {
    oam_ltm->relay_action = data[9];
    oam_ltm->entry.length = 10;
  }

  return 0;
}

int encode_oam_ltm (oam_ltm_t *oam_ltm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((oam_ltm->lvl          & 0x7)     << 29) |
         ((oam_ltm->version      & 0x1f)    << 24) |
         ((oam_ltm->opcode       & 0xff)    << 16) |
       ((oam_ltm->flags        & 0xff)    << 8)  |
         ((oam_ltm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_ltm->seq_number ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  data[8] = oam_ltm->ttl;
  if(OPCODE_LTM == oam_ltm->opcode) {
    sal_memcpy (data + 9, oam_ltm->origin_mac, 6);
    sal_memcpy (data + 15, oam_ltm->target_mac, 6);
    data[21] = oam_ltm->end_tlv;
  } else {
    data[9] = oam_ltm->relay_action;
  }

  return 0;
}

int decode_oam_lmm (oam_lmm_t *oam_lmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  oam_lmm->lvl            = (value32 >> 29) & 0x7;
  oam_lmm->version        = (value32 >> 24) & 0x1f;
  oam_lmm->opcode         = (value32 >> 16) & 0xff;
  oam_lmm->flags          = (value32 >> 8)  & 0xff;
  oam_lmm->tlv_offset     = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  oam_lmm->tx_fcf         = value32;

  /* word 2 */
  sal_memcpy (&value32, data + 8, 4);
  oam_lmm->rx_fcf         = value32;

  /* word 3 */
  sal_memcpy (&value32, data + 12, 4);
  oam_lmm->tx_fcb         = value32;

  /* word 4 */
  oam_lmm->end_tlv        = data[16];

  oam_lmm->entry.length   = 17;

  return 0;
}

int encode_oam_lmm (oam_lmm_t *oam_lmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((oam_lmm->lvl          & 0x7)     << 29) |
         ((oam_lmm->version      & 0x1f)    << 24) |
         ((oam_lmm->opcode       & 0xff)    << 16) |
       ((oam_lmm->flags        & 0xff)    << 8)  |
         ((oam_lmm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_lmm->tx_fcf ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  value32 = oam_lmm->rx_fcf ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 8, &value32, 4);

  /* word 3 */
  value32 = oam_lmm->tx_fcb ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 12, &value32, 4);

  /* word 4 */
  data[16] = oam_lmm->end_tlv;

  return 0;
}

int decode_oam_1dmm (oam_1dmm_t *oam_1dmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  oam_1dmm->lvl               = (value32 >> 29) & 0x7;
  oam_1dmm->version           = (value32 >> 24) & 0x1f;
  oam_1dmm->opcode            = (value32 >> 16) & 0xff;
  oam_1dmm->flags             = (value32 >> 8)  & 0xff;
  oam_1dmm->tlv_offset        = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  oam_1dmm->tx_timestamp_sec  = value32;

  /* word 2 */
  sal_memcpy (&value32, data + 8, 4);
  oam_1dmm->tx_timestamp_nano = value32;

  /* word 3 */
  sal_memcpy (&value32, data + 12, 4);
  oam_1dmm->rx_timestamp_sec  = value32;

  /* word 4 */
  sal_memcpy (&value32, data + 16, 4);
  oam_1dmm->rx_timestamp_nano = value32;

  /* word 5 */
  oam_1dmm->end_tlv           = data[20];

  oam_1dmm->entry.length   = 21;

  return 0;
}

int encode_oam_1dmm (oam_1dmm_t *oam_1dmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((oam_1dmm->lvl          & 0x7)     << 29) |
         ((oam_1dmm->version      & 0x1f)    << 24) |
         ((oam_1dmm->opcode       & 0xff)    << 16) |
       ((oam_1dmm->flags        & 0xff)    << 8)  |
         ((oam_1dmm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_1dmm->tx_timestamp_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  value32 = oam_1dmm->tx_timestamp_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 8, &value32, 4);

  /* word 3 */
  value32 = oam_1dmm->rx_timestamp_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 12, &value32, 4);

  /* word 4 */
  value32 = oam_1dmm->rx_timestamp_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 16, &value32, 4);

  /* word 5 */
  data[20] = oam_1dmm->end_tlv;

  return 0;
}

int decode_oam_2dmm (oam_2dmm_t *oam_2dmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  oam_2dmm->lvl                 = (value32 >> 29) & 0x7;
  oam_2dmm->version             = (value32 >> 24) & 0x1f;
  oam_2dmm->opcode              = (value32 >> 16) & 0xff;
  oam_2dmm->flags               = (value32 >> 8)  & 0xff;
  oam_2dmm->tlv_offset          = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  oam_2dmm->tx_timestamp_f_sec  = value32;

  /* word 2 */
  sal_memcpy (&value32, data + 8, 4);
  oam_2dmm->tx_timestamp_f_nano = value32;

  /* word 3 */
  sal_memcpy (&value32, data + 12, 4);
  oam_2dmm->rx_timestamp_f_sec  = value32;

  /* word 4 */
  sal_memcpy (&value32, data + 16, 4);
  oam_2dmm->rx_timestamp_f_nano = value32;

  /* word 5 */
  sal_memcpy (&value32, data + 20, 4);
  oam_2dmm->tx_timestamp_b_sec  = value32;

  /* word 6 */
  sal_memcpy (&value32, data + 24, 4);
  oam_2dmm->tx_timestamp_b_nano = value32;

  /* word 7 */
  sal_memcpy (&value32, data + 28, 4);
  oam_2dmm->rx_timestamp_b_sec  = value32;

  /* word 8 */
  sal_memcpy (&value32, data + 32, 4);
  oam_2dmm->rx_timestamp_b_nano = value32;

  /* word 9 */
  oam_2dmm->end_tlv             = data[36];

  oam_2dmm->entry.length        = 37;

  return 0;
}

int encode_oam_2dmm (oam_2dmm_t *oam_2dmm, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((oam_2dmm->lvl          & 0x7)     << 29) |
         ((oam_2dmm->version      & 0x1f)    << 24) |
         ((oam_2dmm->opcode       & 0xff)    << 16) |
       ((oam_2dmm->flags        & 0xff)    << 8)  |
         ((oam_2dmm->tlv_offset   & 0xff)    << 0));

  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = oam_2dmm->tx_timestamp_f_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  value32 = oam_2dmm->tx_timestamp_f_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 8, &value32, 4);

  /* word 3 */
  value32 = oam_2dmm->rx_timestamp_f_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 12, &value32, 4);

  /* word 4 */
  value32 = oam_2dmm->rx_timestamp_f_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 16, &value32, 4);

  /* word 5 */
  value32 = oam_2dmm->tx_timestamp_b_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 20, &value32, 4);
  
  /* word 6 */
  value32 = oam_2dmm->tx_timestamp_b_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 24, &value32, 4);

  /* word 7 */
  value32 = oam_2dmm->rx_timestamp_b_sec ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 28, &value32, 4);

  /* word 8 */
  value32 = oam_2dmm->rx_timestamp_b_nano ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 32, &value32, 4);

  /* word 9 */
  data[36] = oam_2dmm->end_tlv;

  return 0;
}

int decode_bfd (bfd_t *bfd, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  bfd->version      = (value32 >> 29) & 0x7;
  bfd->diag         = (value32 >> 24) & 0x1f;
  bfd->state        = (value32 >> 22) & 0x3;
  bfd->flags        = (value32 >> 16) & 0x3f;
  bfd->detect_mult  = (value32 >> 8)  & 0xff;
  bfd->length       = (value32 >> 0)  & 0xff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  value32 = soc_ntohl (value32);
  bfd->my_discrim   = value32;

  /* word 2 */
  sal_memcpy (&value32, data + 8, 4);
  value32 = soc_ntohl (value32);
  bfd->your_discrim = value32;

  /* word 3 */
  sal_memcpy (&value32, data + 12, 4);
  value32 = soc_ntohl (value32);
  bfd->min_tx       = value32;

  /* word 4 */
  sal_memcpy (&value32, data + 16, 4);
  value32 = soc_ntohl (value32);
  bfd->min_rx       = value32;

  /* word 5 */
  sal_memcpy (&value32, data + 20, 4);
  value32 = soc_ntohl (value32);
  bfd->echo         = value32;

  bfd->entry.length   = 24;

  return 0;
}

int encode_bfd (bfd_t *bfd, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((bfd->version & 0x7)    << 29) |
         ((bfd->diag          & 0x1f)   << 24) |
         ((bfd->state         & 0x3)    << 22) |
       ((bfd->flags         & 0x3f)   << 16) |
         ((bfd->detect_mult   & 0xff)   << 8)  |
         ((bfd->length        & 0xff)   << 0));
  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = bfd->my_discrim ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);
  
  /* word 2 */
  value32 = bfd->your_discrim ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 8, &value32, 4);

  /* word 3 */
  value32 = bfd->min_tx ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 12, &value32, 4);

  /* word 4 */
  value32 = bfd->min_rx ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 16, &value32, 4);

  /* word 5 */
  value32 = bfd->echo ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 20, &value32, 4);

  return 0;
}

int decode_bpdu (bpdu_t *bpdu, uint8 *data) {
  uint32 value32;
  uint16 value16;

  sal_memcpy (&value16, data, 2);
  value16              = soc_ntohs (value16);
  bpdu->protocol_id    = value16;
  bpdu->version        = data[2];
  bpdu->message_type   = data[3];
  bpdu->flags          = data[4];
  sal_memcpy (&value16, data + 5, 2);
  value16              = soc_ntohs (value16);
  bpdu->root_pri       = value16;
  sal_memcpy (bpdu->root_mac, data + 7, 6);
  sal_memcpy (&value32, data + 13, 4);
  value32              = soc_ntohl (value32);
  bpdu->root_path_cost = value32;
  sal_memcpy (&value16, data + 17, 2);
  value16              = soc_ntohs (value16);
  bpdu->bridge_pri     = value16;
  sal_memcpy (bpdu->bridge_mac, data + 19, 6);
  sal_memcpy (&value16, data + 25, 2);
  value16              = soc_ntohs (value16);
  bpdu->port_id        = value16;
  sal_memcpy (&value16, data + 27, 2);
  value16              = soc_ntohs (value16);
  bpdu->message_age    = value16;
  sal_memcpy (&value16, data + 29, 2);
  value16              = soc_ntohs (value16);
  bpdu->max_age        = value16;
  sal_memcpy (&value16, data + 31, 2);
  value16              = soc_ntohs (value16);
  bpdu->hello_time     = value16;
  sal_memcpy (&value16, data + 33, 2);
  value16              = soc_ntohs (value16);
  bpdu->forward_delay  = value16;

  bpdu->entry.length   = 35;

  return 0;
}

int encode_bpdu (bpdu_t *bpdu, uint8 *data) {
  uint32 value32;
  uint16 value16;

  value16 = bpdu->protocol_id;
  value16 = soc_htons (value16);
  sal_memcpy (data, &value16, 2);
  data[2] = bpdu->version;
  data[3] = bpdu->message_type;
  data[4] = bpdu->flags;
  value16 = bpdu->root_pri;
  value16 = soc_htons (value16);
  sal_memcpy (data + 5, &value16, 2);
  sal_memcpy (data + 7, bpdu->root_mac, 6);
  value32 = bpdu->root_path_cost ;
  value32 = soc_htonl (value32);
  sal_memcpy (data + 13, &value32, 4);
  value16 = bpdu->bridge_pri;
  value16 = soc_htons (value16);
  sal_memcpy (data + 17, &value16, 2);
  sal_memcpy (data + 19, bpdu->bridge_mac, 6);
  value16 = bpdu->port_id;
  value16 = soc_htons (value16);
  sal_memcpy (data + 25, &value16, 2);
  value16 = bpdu->message_age;
  value16 = soc_htons (value16);
  sal_memcpy (data + 27, &value16, 2);
  value16 = bpdu->max_age;
  value16 = soc_htons (value16);
  sal_memcpy (data + 29, &value16, 2);
  value16 = bpdu->hello_time;
  value16 = soc_htons (value16);
  sal_memcpy (data + 31, &value16, 2);
  value16 = bpdu->forward_delay;
  value16 = soc_htons (value16);
  sal_memcpy (data + 33, &value16, 2);

  return 0;
}

int decode_slow (slow_t *slow, uint8 *data) {

  slow->sub_type     = data[0];

  slow->entry.length = 1;

  return 0;
}

int encode_slow (slow_t *slow, uint8 *data) {

  data[0] = slow->sub_type;

  return 0;
}


int decode_ipv4 (ipv4_t *ipv4, uint8 *data) {
  uint32 value;
 
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  ipv4->ver = (value >> 28) & 0xf;
  ipv4->ihl = (value >> 24) & 0xf;
  ipv4->tos = (value >> 16) & 0xff;
  ipv4->length = (value >> 0) & 0xffff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  ipv4->id = (value >> 16) & 0xffff;
  ipv4->df = (value >> 14) & 0x1;
  ipv4->mf = (value >> 13) & 0x1;
  ipv4->offset  = (value >> 0) & 0x1fff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  ipv4->ttl = (value >> 24) & 0xff;
  ipv4->proto = (value >> 16) & 0xff;
  ipv4->checksum = (value >> 0) & 0xffff;

 /* word 3 and 4 */
  sal_memcpy (&value, data + 12, 4);
  ipv4->sa = soc_ntohl (value);
  sal_memcpy (&value, data + 16, 4);
  ipv4->da = soc_ntohl (value);

  ipv4->entry.length = 20;
  return 0;
}
int encode_ipv4 (ipv4_t *ipv4, uint8 *data) {
  uint32 value;
  uint16 value16;
  uint8  value8;
  
  /* word 0 */
  value8 = (((ipv4->ver & 0xf) << 4) |
            ((ipv4->ihl & 0xf) << 0));
  sal_memcpy (data, &value8, 1);
  value8 = (ipv4->tos & 0xff);
  sal_memcpy (data + 1, &value8, 1);
  value16 = (ipv4->length & 0xffff);
  value16 = soc_htons (value16);
  sal_memcpy (data + 2, &value16, 2);

  /* word 1 */
  sal_memcpy (data + 4, &ipv4->id, 2);
  value16 = (((0         &    0x1)  << 15) |
             ((ipv4->df  &    0x1)  << 14) |
             ((ipv4->mf  &    0x1)  << 13) |
             ((ipv4->offset & 0x1fff)  <<  0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 6, &value16, 2);

  /* word 2 */
  value8 = (ipv4->ttl & 0xff);
  sal_memcpy (data + 8, &value8, 1);
  value8 = (ipv4->proto & 0xff);
  sal_memcpy (data + 9, &value8, 1);
  /* checksum 0*/
  value16 = 0;
  sal_memcpy (data + 10, &value16, 2);

  /* word 3 and 4 */
  value = soc_htonl (ipv4->sa);
  sal_memcpy (data + 12, &value, 4);
  value = soc_htonl (ipv4->da);
  sal_memcpy (data + 16, &value, 4);

  /* update checksum */
  value16 = ipv4_checksum(data, 20);
  value16 = soc_htons (value16 & 0xffff);
  sal_memcpy (data + 10, &value16, 2);

  return 0;
}

int decode_ipv6 (ipv6_t *ipv6, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  ipv6->ver = (value32 >> 28) & 0xf;
  ipv6->tos = (value32 >> 20) & 0xff;
  ipv6->flow_label = (value32 >> 0) & 0xfffff;

  /* word 1 */
  sal_memcpy (&value32, data + 4, 4);
  value32 = soc_ntohl (value32);
  ipv6->length = (value32 >> 16) & 0xffff;
  ipv6->next_header = (value32 >> 8) & 0xff;
  ipv6->ttl = (value32 >> 0) & 0xff;

  /* word 2, 3, 4, 5 */
  sal_memcpy (ipv6->sa, data + 8, 16);

  /* word 6, 7, 8, 9 */
  sal_memcpy (ipv6->da, data + 24, 16);

  ipv6->entry.length = 40;

  return 0;
}
int encode_ipv6 (ipv6_t *ipv6, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32 = (((ipv6->ver & 0xf) << 28) |
            ((ipv6->tos & 0xff) << 20) |
            ((ipv6->flow_label & 0xfffff) << 0));
  value32 = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = (((ipv6->length & 0xffff) << 16) |
            ((ipv6->next_header & 0xff) << 8) |
            ((ipv6->ttl & 0xff) << 0));
  value32 = soc_htonl (value32);
  sal_memcpy (data + 4, &value32, 4);

  /* word 2, 3, 4, 5 */
  sal_memcpy (data + 8, ipv6->sa, 16);

  /* word 6, 7, 8, 9 */
  sal_memcpy (data + 24, ipv6->da, 16);

  return 0;
}


int decode_udp (udp_t *udp, uint8 *data) {
  uint32 value;

  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  udp->sport = (value >> 16) & 0xffff;
  udp->dport = (value >> 0) & 0xffff;

  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  udp->len = (value >> 16) & 0xffff;
  udp->checksum = (value >> 0) & 0xffff;

  udp->entry.length = 8;
  return 0;
}
int encode_udp (udp_t *udp, uint8 *data) {
  uint32 value32;
  uint16 value16;

  value32 = ((udp->sport & 0xffff) << 16) +
          (udp->dport & 0xffff);
  value32 = soc_htonl(value32);
  sal_memcpy (data, &value32, 4);

  value16 = (udp->len & 0xffff);
  value16 = soc_htons (value16);
  sal_memcpy (data + 4, &value16, 2);

  value16 = (udp->checksum & 0xffff);
  value16 = soc_htons (value16);
  sal_memcpy (data + 6, &value16, 2);

  return 0;
}


int decode_tcp (tcp_t *tcp, uint8 *data) {
  uint32 value;
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  tcp->sport = (value >> 16) & 0xffff;
  tcp->dport = (value >> 0) & 0xffff;

  sal_memcpy (&value, data + 4, 4);
  tcp->seqn = soc_ntohl (value);

  sal_memcpy (&value, data + 8, 4);
  tcp->ackn = soc_ntohl (value);

  sal_memcpy (&value, data + 12, 4);
  value = soc_ntohl (value);
  tcp->dofs = (value >> 28) & 0xf;
  tcp->ecn  = (value >> 22) & 0x7;
  tcp->ctrl = (value >> 16) & 0x3f;
  tcp->wind = (value >>  0) & 0xffff;

  sal_memcpy (&value, data + 16, 4);
  value = soc_ntohl (value);
  tcp->checksum = (value >> 16) & 0xffff;
  tcp->urgp     = (value >>  0) & 0xffff;

  tcp->entry.length = 20;
  return 0;
}
int encode_tcp (tcp_t *tcp, uint8 *data) {
  uint32 value32;

  value32 = ((tcp->sport & 0xffff) << 16)
          + (tcp->dport & 0xffff);
  value32 = soc_htonl(value32);
  sal_memcpy (data, &value32, 4);

  value32 = soc_htonl(tcp->seqn);
  sal_memcpy (data + 4, &value32, 4);

  value32 = soc_htonl(tcp->ackn);
  sal_memcpy (data + 8, &value32, 4);

  value32 = tcp->dofs;
  value32 = value32 << 3;
  value32 = (value32 << 3) + tcp->ecn;
  value32 = (value32 << 6) + tcp->ctrl;
  value32 = (value32 << 16) + tcp->wind;
  value32 = soc_htonl(value32);
  sal_memcpy (data + 12, &value32, 4);

  value32 = (tcp->checksum << 16) + (tcp->urgp & 0xffff);
  value32 = soc_htonl(value32);
  sal_memcpy (data + 16, &value32, 4);
  return 0;
}

int decode_igmp (igmp_t *igmp, uint8 *data) {
  igmp->entry.length = 20;
  return 0;
}
int encode_igmp (igmp_t *igmp, uint8 *data) {
  return 0;
}

int decode_itag (itag_t *itag, uint32 value) {
  itag->ipcp = (value >> 29) & 0x7;
  itag->idei = (value >> 28) & 0x1;
  itag->nca  = (value >> 27) & 0x1;
  itag->isid = (value >>  0) & 0xffffff;
  itag->entry.length = 4;
  return 0;
}
int encode_itag (itag_t *itag, uint8 *data) {
  uint32 value;
  value = (((itag->ipcp & 0x7) << 29) |
           ((itag->idei & 0x1) << 28) |
           ((itag->nca  & 0x1) << 27) |
           ((itag->isid & 0xffffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_pwe_data_ach (pwe_data_ach_t *pwe_data_ach, uint32 value) {
  pwe_data_ach->flag = (value >> 24) & 0xf;
  pwe_data_ach->frg = (value >> 22) & 0x3;
  pwe_data_ach->length  = (value >> 16) & 0x3f;
  pwe_data_ach->seqno = (value >>  0) & 0xffff;
  pwe_data_ach->entry.length = 4;
  return 0;
}
int encode_pwe_data_ach (pwe_data_ach_t *pwe_data_ach, uint8 *data) {
  uint32 value;
  value = (((pwe_data_ach->flag & 0xf) << 24) |
           ((pwe_data_ach->frg & 0x3) << 22) |
           ((pwe_data_ach->length  & 0x3f) << 16) |
           ((pwe_data_ach->seqno & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_pwe_ctrl_ach (pwe_ctrl_ach_t *pwe_ctrl_ach, uint32 value) {
  pwe_ctrl_ach->ctrl = (value >> 28) & 0xf;
  pwe_ctrl_ach->ver = (value >> 24) & 0xf;
  pwe_ctrl_ach->rsvd = (value >> 16) & 0xff;
  pwe_ctrl_ach->channel = (value >>  0) & 0xffff;
  pwe_ctrl_ach->entry.length = 4;
  return 0;
}
int encode_pwe_ctrl_ach (pwe_ctrl_ach_t *pwe_ctrl_ach, uint8 *data) {
  uint32 value;
  value = (((pwe_ctrl_ach->ctrl & 0xf) << 28) |
           ((pwe_ctrl_ach->ver & 0xf) << 24) |
           ((pwe_ctrl_ach->rsvd  & 0xff) << 16) |
           ((pwe_ctrl_ach->channel & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_gach (gach_t *gach, uint32 value) {
  gach->ver = (value >> 24) & 0xf;
  gach->rsvd = (value >> 16) & 0xff;
  gach->channel = (value >>  0) & 0xffff;
  gach->entry.length = 4;
  return 0;
}
int encode_gach (gach_t *gach, uint8 *data) {
  uint32 value;
  value = (((gach->ver & 0xf) << 24) |
           ((gach->rsvd  & 0xff) << 16) |
           ((gach->channel & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_tlvhdr (tlvhdr_t *tlvhdr, uint32 value) {
  tlvhdr->length = (value >> 16) & 0xffff;
  tlvhdr->rsvd = (value >> 0) & 0xffff;
  tlvhdr->entry.length = 4;
  return 0;
}
int encode_tlvhdr (tlvhdr_t *tlvhdr, uint8 *data) {
  uint32 value;
  value = (((tlvhdr->length & 0xffff) << 16) |
           ((tlvhdr->rsvd & 0xffff) <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_psc (psc_t *psc, uint32 value) {
  psc->ver = (value >> 30) & 0x3;
  psc->request = (value >> 26) & 0xf;
  psc->pt = (value >> 24) & 0x3;
  psc->r = (value >> 23) & 0x1;
  psc->rsvd = (value >> 16) & 0x7f;
  psc->fpath = (value >> 8) & 0xff;
  psc->path = (value >> 0) & 0xff;
  psc->entry.length = 4;
  return 0;
}
int encode_psc (psc_t *psc, uint8 *data) {
  uint32 value;
  value = (((psc->ver & 0x3) << 30) |
           ((psc->request & 0xf) << 26) |
           ((psc->pt & 0x3) << 24) |
           ((psc->r & 0x1) << 23) |
           ((psc->rsvd & 0x7f) << 16) |
           ((psc->fpath & 0xff) << 8) |           
           ((psc->path & 0xff) << 0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_oam_mpls (oam_mpls_t *oam_mpls, uint8 *data) {
  uint16 value16;

  oam_mpls->func_type = data[0];

  sal_memcpy (oam_mpls->ttsi, data+4, TTSI_LENGTH);

  oam_mpls->frequency = data[24];

  sal_memcpy (&value16, data + 42, 2);
  value16 = soc_ntohs (value16);
  oam_mpls->bip16 = value16;

  oam_mpls->entry.length   = 44;

  return 0;
}
int encode_oam_mpls (oam_mpls_t *oam_mpls, uint8 *data) {
  uint16 value16;

  sal_memset(data, 0, 44);
  
  data[0] = oam_mpls->func_type;

  sal_memcpy (data+4, oam_mpls->ttsi, TTSI_LENGTH);

  data[24] = oam_mpls->frequency;

  value16 = oam_mpls->bip16;
  value16 = soc_htons (value16);
  sal_memcpy (data+42, &value16, 2);
  return 0;
}

int decode_ip_option (ip_option_t *ip_option, uint8 *data) {
  ip_option->option_code = data[0];
  ip_option->length      = data[1];

  sal_memcpy (ip_option->data, data+2, ip_option->length-1);

  ip_option->entry.length   = ip_option->length+1;

  return 0;
}
int encode_ip_option (ip_option_t *ip_option, uint8 *data) {
  data[0] = ip_option->option_code;
  data[1] = ip_option->length;

  sal_memcpy (data+2, ip_option->data, ip_option->length-1);

  return 0;
}

int decode_hop (hop_t *hop, uint8 *data) {
  uint32 value32;

  hop->entry.length = 8;
  hop->next_header = data[0];
  hop->length      = data[1];
  hop->opt_type    = data[2];
  hop->opt_len     = data[3];
  
  sal_memcpy (&value32, data+4, 4);
  value32 = soc_ntohl (value32);

  hop->data        = value32;

  return 0;
}
int encode_hop (hop_t *hop, uint8 *data) {
  uint32 value32;

  data[0] = hop->next_header;
  data[1] = hop->length;
  data[2] = hop->opt_type;
  data[3] = hop->opt_len;

  value32 = hop->data;
  value32 = soc_htonl (value32);
  sal_memcpy (data+4, &value32, 4);

  return 0;
}

int decode_ptp (ptp_t *ptp, uint8 *data) {
  uint32 value32;
  uint16 value16;

  ptp->entry.length         = 34;
  ptp->message_type         = data[0];
  ptp->version_ptp          = data[1];
  sal_memcpy (&value16, data + 2, 2);
  ptp->message_length       = soc_ntohs (value16);
  ptp->domain_number        = data[4];
  sal_memcpy (&value16, data + 6, 2);
  ptp->flag_field           = soc_ntohs (value16);
  sal_memcpy (&value32, data + 8, 4);
  ptp->correction_field1    = soc_ntohl (value32);
  sal_memcpy (&value32, data + 12, 4);
  ptp->correction_field2    = soc_ntohl (value32);
  sal_memcpy (&value32, data + 20, 4);
  ptp->sourceport_identity1 = soc_ntohl (value32);
  sal_memcpy (&value32, data + 24, 4);
  ptp->sourceport_identity2 = soc_ntohl (value32);
  sal_memcpy (&value16, data + 28, 2);
  ptp->sourceport_identity3 = soc_ntohs (value16);
  sal_memcpy (&value16, data + 30, 2);
  ptp->sequence_id          = soc_ntohs (value16);
  ptp->control_field        = data[32];
  ptp->logmessage_interval  = data[33];
  
  return 0;
}
int encode_ptp (ptp_t *ptp, uint8 *data) {
  uint32 value32;
  uint16 value16;

  sal_memset(data, 0, 34);
  data[0]  = ptp->message_type;
  data[1]  = ptp->version_ptp;
  value16  = ptp->message_length;
  value16  = soc_htons (value16);
  sal_memcpy (data+2, &value16, 2);
  data[4]  = ptp->domain_number;
  value16  = ptp->flag_field;
  value16  = soc_htons (value16);
  sal_memcpy (data+6, &value16, 2);
  value32  = ptp->correction_field1;
  value32  = soc_htonl (value32);
  sal_memcpy (data+8, &value32, 4);
  value32  = ptp->correction_field2;
  value32  = soc_htonl (value32);
  sal_memcpy (data+12, &value32, 4);
  value32  = ptp->sourceport_identity1;
  value32  = soc_htonl (value32);
  sal_memcpy (data+20, &value32, 4);
  value32  = ptp->sourceport_identity2;
  value32  = soc_htonl (value32);
  sal_memcpy (data+24, &value32, 4);
  value16  = ptp->sourceport_identity3;
  value16  = soc_htons (value16);
  sal_memcpy (data+28, &value16, 2);
  value16  = ptp->sequence_id;
  value16  = soc_htons (value16);
  sal_memcpy (data+30, &value16, 2);
  data[32] = ptp->control_field;
  data[33] = ptp->logmessage_interval;

  return 0;
}

int decode_itmh (itmh_t *itmh, uint8 *data) {
    uint32 value32;

    /* word 0 */
    sal_memcpy (&value32, data, 4);
    value32 = soc_ntohl (value32);
    itmh->type         = (value32 >> 30) & 0x3;
    itmh->p_t          = (value32 >> 28) & 0x3;
    itmh->pkt_head_len = (value32 >> 22) & 0x3f;
    itmh->dp           = (value32 >> 19) & 0x7;

    if (itmh->type == SBX_PKT_ITMH_TYPE_UC)
    {
        itmh->uc_flow_id    = (value32 >> 0) & 0x3fff;
    }
    else
    {
        itmh->mc_t_c        = (value32 >> 16)& 0x7;
        itmh->multicast_id  = (value32 >> 0) & 0x1fff;
    }
    itmh->entry.length = 4;

    return 0;
}
int encode_itmh (itmh_t *itmh, uint8 *data) {
    uint32 value32;

    sal_memset(data, 0, 4);
    if (itmh->type == SBX_PKT_ITMH_TYPE_UC)
    {
        value32 = ((itmh->type & 0x3) << 30) |
                  ((itmh->p_t & 0x3) << 28) |
                  ((itmh->pkt_head_len & 0x3f) << 22) |
                  ((itmh->dp & 0x7) << 19) |
                  ((itmh->uc_flow_id & 0x3fff) << 0);
    }
    else
    {
        value32 = ((itmh->type & 0x3) << 30) |
                  ((itmh->p_t & 0x3) << 28) |
                  ((itmh->pkt_head_len & 0x3f) << 22) |
                  ((itmh->dp & 0x7) << 19) |
                  ((itmh->mc_t_c & 0x7) << 16) |
                  ((itmh->multicast_id & 0x1fff) << 0);
    }
    value32  = soc_htonl (value32);
    sal_memcpy (data, &value32, 4);

    return 0;
}

int decode_nph (nph_t *nph, uint8 *data) 
{
    uint32 value32;

      nph->entry.length  = 16;
    /* word 0 */
    sal_memcpy (&value32, data, 4);
    value32 = soc_ntohl (value32);
    nph->m_u_flag         = (value32 >> 31) & 0x1;
    nph->main_type        = (value32 >> 28) & 0x7;
    nph->sub_type         = (value32 >> 24) & 0xf;
    nph->ptp_1588flag     = (value32 >> 21) & 0x1;
    nph->cos_dp           = (value32 >> 16) & 0x1F;
    nph->slot_in          = (value32 >> 9) & 0x7f;
    nph->trunk_flag       = (value32 >> 8) & 0x1;
    nph->port_or_trunk_id = (value32 >> 0) & 0xff;
    /* word 1 */
    sal_memcpy (&value32, data+4, 4);
    value32 = soc_ntohl (value32);
    nph->vlan_status              = (value32 >> 30) & 0x3;
    nph->del_length               = (value32 >> 24) & 0x3f;
    nph->default_pri_cfi          = (value32 >> 20) & 0xf;
    nph->protect_status_section   = (value32 >> 19) & 0x1;
    nph->protect_status_lsp       = (value32 >> 18) & 0x1;
    nph->protect_status_pw_uni    = (value32 >> 17) & 0x1;
    nph->output_fp                = (value32 >> 0) & 0x1ffff;

    /* word 2 */
    sal_memcpy (&value32, data+8, 4);
    value32 = soc_ntohl (value32);
    nph->root_leaf     = (value32 >> 31) & 0x1;
    nph->hub_spoke     = (value32 >> 30) & 0x1;
    nph->tag_num       = (value32 >> 28) & 0x3;
    nph->b_f           = (value32 >> 27) & 0x1;
    nph->port_out      = (value32 >> 16) & 0x7ff;
    nph->lrn_on        = (value32 >> 15) & 0x1;
    nph->mstp_Lrn      = (value32 >> 14) & 0x1;
    nph->msp_opo       = (value32 >> 13) & 0x1;
    nph->vpn_id        = (value32 >> 0) & 0x1fff;

    /* word 3 */
    sal_memcpy (&value32, data+12, 4);
    value32 = soc_ntohl (value32);
    nph->nph_word3    = value32;

    return 0;
}
int encode_nph (nph_t *nph, uint8 *data) 
{
    uint32  value32;

    sal_memset(data, 0, 16);
    /* word 0 */
    value32 = ((nph->m_u_flag & 0x1) << 31) |
              ((nph->main_type & 0x7) << 28) |
              ((nph->sub_type & 0xf) << 24) |
              ((nph->ptp_1588flag & 0x1) << 21) |
              ((nph->cos_dp & 0x1F) << 16) |
              ((nph->slot_in & 0x7f) << 9) |
              ((nph->trunk_flag & 0x1) << 8) |
              ((nph->port_or_trunk_id & 0xff) << 0);
    value32  = soc_htonl (value32);
    sal_memcpy (data, &value32, 4);

    /* word 1 */    
    value32 = ((nph->vlan_status & 0x3) << 30) |
              ((nph->del_length & 0x3f) << 24) |
              ((nph->default_pri_cfi & 0xf) << 20) |
              ((nph->protect_status_section & 0x1) << 19) |
              ((nph->protect_status_lsp & 0x1) << 18) |
              ((nph->protect_status_pw_uni & 0x1) << 17) |
              ((nph->output_fp & 0x1ffff) << 0);
    value32  = soc_htonl (value32);
    sal_memcpy (data+4, &value32, 4);
    
    /* word 2 */
    value32 = ((nph->root_leaf & 0x1) << 31) |
              ((nph->hub_spoke & 0x1) << 30) |
              ((nph->tag_num & 0x3) << 28) |
              ((nph->b_f & 0x1) << 27) |
              ((nph->port_out & 0x7ff) << 16) |
              ((nph->lrn_on & 0x1) << 15) |
              ((nph->mstp_Lrn & 0x1) << 14) |
              ((nph->msp_opo & 0x1) << 13) |
              ((nph->vpn_id & 0x1fff) << 0);
    value32  = soc_htonl (value32);
    sal_memcpy (data+8, &value32, 4);

    /* word 3 */
    value32 = nph->nph_word3;
    value32  = soc_htonl (value32);
    sal_memcpy (data+12, &value32, 4);

    return 0;
}

int decode_nfh (nfh_t *nfh, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  nfh->destpoint     = (value32 >> 28) & 0xf;
  nfh->linktype      = (value32 >> 24) & 0xf;
  nfh->packettype_to = (value32 >> 16) & 0xff;
  nfh->packetlen     = (value32 >> 19) & 0xffff;

  /* word 1 */
  sal_memcpy (&value32, data+4, 4);
  value32 = soc_ntohl (value32);
  nfh->packettype   = (value32 >> 30) & 0x3;
  nfh->vlan_status  = (value32 >> 28) & 0x3;
  nfh->port         = (value32 >> 17) & 0x7ff;
  nfh->fp           = (value32 >> 0) & 0x1ffff;

  /* word 2 */
  sal_memcpy (&value32, data+8, 4);
  value32 = soc_ntohl (value32);
  nfh->desttype     = (value32 >> 31) & 0x1;
  nfh->vpnid        = (value32 >> 16) & 0x7fff;
  nfh->flowtype     = (value32 >> 8) & 0xff;
  nfh->greencap     = (value32 >> 7) & 0x1;
  nfh->slot         = (value32 >> 0) & 0x7f;

  /* word 3 */
  sal_memcpy (&value32, data+12, 4);
  value32 = soc_ntohl (value32);
  nfh->to_fpga      = (value32 >> 8) & 0xffffff;
  nfh->cookielen    = (value32 >> 0) & 0xff;

  /* word 4 */
  sal_memcpy (&value32, data+16, 4);
  value32 = soc_ntohl (value32);
  nfh->cookie       = value32;
  
  nfh->entry.length         = 20;

  return 0;
}
int encode_nfh (nfh_t *nfh, uint8 *data) {
  uint32 value32;

  sal_memset(data, 0, 20);
  /* word 0 */
  value32 = ((nfh->destpoint & 0xf) << 28) |
            ((nfh->linktype & 0xf) << 24) |
            ((nfh->packettype_to & 0xff) << 16) |
            ((nfh->packetlen & 0xffff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32 = ((nfh->packettype & 0x3) << 30) |
            ((nfh->vlan_status & 0x3) << 28) |
            ((nfh->port & 0x7ff) << 17) |
            ((nfh->fp & 0x1ffff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+4, &value32, 4);

  /* word 2 */
  value32 = ((nfh->desttype & 0x1) << 31) |
            ((nfh->vpnid & 0x7fff) << 16) |
            ((nfh->flowtype & 0xff) << 8) |
            ((nfh->greencap & 0x1) << 7) |
            ((nfh->slot & 0x7f) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+8, &value32, 4);

  /* word 3 */
  value32 = ((nfh->to_fpga & 0xffffff) << 8) |
            ((nfh->cookielen & 0xff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+12, &value32, 4);

  /* word 4 */
  value32 = nfh->cookie;
  value32  = soc_htonl (value32);
  sal_memcpy (data+16, &value32, 4);

  return 0;
}

int decode_fnh_full_fwd (fnh_full_fwd_t *fnh_full_fwd, uint8 *data) {
  uint32 value32;
  uint16 value16;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  fnh_full_fwd->cmheader = value32;

  /* word 1 */
  sal_memcpy (&value32, data+4, 4);
  value32 = soc_ntohl (value32);
  fnh_full_fwd->rsv0  = (value32 >> 27) & 0x1f;
  fnh_full_fwd->ftype = (value32 >> 24) & 0x7;
  fnh_full_fwd->rsv1  = (value32 >> 19) & 0x1f;
  fnh_full_fwd->ptype = (value32 >> 16) & 0x7;
  fnh_full_fwd->rsv2  = (value32 >> 11) & 0x1f;
  fnh_full_fwd->port  = (value32 >> 0) & 0x7ff;

  /* word 2 */
  sal_memcpy (&value16, data+8, 2);
  value16 = soc_htons (value16);
  fnh_full_fwd->rsv3  = (value16 >> 15) & 0x1;
  fnh_full_fwd->slot  = (value16 >> 8) & 0x7f;
  fnh_full_fwd->rsv4  = (value16 >> 3) & 0x1f;
  fnh_full_fwd->cos   = (value16 >> 0) & 0x7;

  fnh_full_fwd->entry.length = 10;

  return 0;
}
int encode_fnh_full_fwd (fnh_full_fwd_t *fnh_full_fwd, uint8 *data) {
  uint32 value32;
  uint16 value16;

  /* word 0 */
  value32  = fnh_full_fwd->cmheader;
  value32  = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32  = ((fnh_full_fwd->rsv0 & 0x1f) << 27) |
             ((fnh_full_fwd->ftype & 0x7) << 24) |
             ((fnh_full_fwd->rsv1 & 0x1f) << 19) |
             ((fnh_full_fwd->ptype & 0x7) << 16) |
             ((fnh_full_fwd->rsv2 & 0x1f) << 11) |
             ((fnh_full_fwd->port & 0x7ff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+4, &value32, 4);

  /* word 2 */
  value16  = ((fnh_full_fwd->rsv3 & 0x1) << 15) |
             ((fnh_full_fwd->slot & 0x7f) << 8) |
             ((fnh_full_fwd->rsv4 & 0x1f) << 3) |
             ((fnh_full_fwd->cos & 0x7) << 0);
  value16  = soc_htons (value16);
  sal_memcpy (data+8, &value16, 2);
  
  return 0;
}

int decode_fnh_half_fwd (fnh_half_fwd_t *fnh_half_fwd, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  fnh_half_fwd->cmheader = value32;

  /* word 1 */
  sal_memcpy (&value32, data+4, 4);
  value32 = soc_ntohl (value32);
  fnh_half_fwd->rsv0  = (value32 >> 27) & 0x1f;
  fnh_half_fwd->ftype = (value32 >> 24) & 0x7;
  fnh_half_fwd->rsv1  = (value32 >> 19) & 0x1f;
  fnh_half_fwd->ptype = (value32 >> 16) & 0x7;
  fnh_half_fwd->rsv2  = (value32 >> 15) & 0x1;
  fnh_half_fwd->protect_status = (value32 >> 14) & 0x1;
  fnh_half_fwd->dp    = (value32 >> 12) & 0x3;
  fnh_half_fwd->rsv3  = (value32 >> 11) & 0x1;
  fnh_half_fwd->cos   = (value32 >> 8) & 0x7;
  fnh_half_fwd->oampduoffset   = (value32 >> 0) & 0xff;

  /* word 2 */
  sal_memcpy (&value32, data+8, 4);
  value32 = soc_ntohl (value32);
  fnh_half_fwd->rsv4  = (value32 >> 17) & 0x7fff;
  fnh_half_fwd->outfp = (value32 >> 0) & 0x1ffff;

  fnh_half_fwd->entry.length = 12;

  return 0;
}
int encode_fnh_half_fwd (fnh_half_fwd_t *fnh_half_fwd, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32  = fnh_half_fwd->cmheader;
  value32  = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32  = ((fnh_half_fwd->rsv0 & 0x1f) << 27) |
             ((fnh_half_fwd->ftype & 0x7) << 24) |
             ((fnh_half_fwd->rsv1 & 0x1f) << 19) |
             ((fnh_half_fwd->ptype & 0x7) << 16) |
             ((fnh_half_fwd->rsv2 & 0x1) << 15) |
             ((fnh_half_fwd->protect_status & 0x1) << 14) |
             ((fnh_half_fwd->dp & 0x3) << 12) |
             ((fnh_half_fwd->rsv3 & 0x1) << 11) |
             ((fnh_half_fwd->cos & 0x7) << 8) |
             ((fnh_half_fwd->oampduoffset & 0xff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+4, &value32, 4);

  /* word 2 */
  value32  = ((fnh_half_fwd->rsv4 & 0x7fff) << 17) |
             ((fnh_half_fwd->outfp & 0x1ffff) << 0);  

  value32  = soc_htonl (value32);
  sal_memcpy (data+8, &value32, 4);
  
  return 0;
}

int decode_fnh_non_fwd (fnh_non_fwd_t *fnh_non_fwd, uint8 *data) {
    uint32 value32;
    uint16 value16;
    
    /* word 0 */
    sal_memcpy (&value32, data, 4);
    value32 = soc_ntohl (value32);
    fnh_non_fwd->cmheader = value32;
    
    /* word 1 */
    sal_memcpy (&value32, data+4, 4);
    value32 = soc_ntohl (value32);
    fnh_non_fwd->ftype = (value32 >> 24) & 0x7;
    fnh_non_fwd->ptype = (value32 >> 16) & 0x7;
    fnh_non_fwd->slot  = (value32 >> 0) & 0x7f;
    
    /* word 2 */
    sal_memcpy (&value16, data+8, 2);
    value16 = soc_htons (value16);
    fnh_non_fwd->packets = value16;
    
    fnh_non_fwd->entry.length = 10;
    
    return 0;
}
int encode_fnh_non_fwd (fnh_non_fwd_t *fnh_non_fwd, uint8 *data) {
    uint32 value32;
    uint16 value16;
    
    /* word 0 */
    value32  = fnh_non_fwd->cmheader;
    value32  = soc_htonl (value32);
    sal_memcpy (data, &value32, 4);
    
    /* word 1 */
    value32  = ((fnh_non_fwd->ftype & 0x7) << 24) |
               ((fnh_non_fwd->ptype & 0x7) << 16) |
               ((fnh_non_fwd->slot & 0xffff) << 0);
    value32  = soc_htonl (value32);
    sal_memcpy (data+4, &value32, 4);
    
    /* word 2 */
    value16  = fnh_non_fwd->packets;
    value16  = soc_htons (value16);
    sal_memcpy (data+8, &value16, 2);
    
    return 0;
}

int decode_fnh_np_term (fnh_np_term_t *fnh_np_term, uint8 *data) {
  uint32 value32;

  /* word 0 */
  sal_memcpy (&value32, data, 4);
  value32 = soc_ntohl (value32);
  fnh_np_term->cmheader = value32;

  /* word 1 */
  sal_memcpy (&value32, data+4, 4);
  value32 = soc_ntohl (value32);
  fnh_np_term->rsv0  = (value32 >> 27) & 0x1f;
  fnh_np_term->ftype = (value32 >> 24) & 0x7;
  fnh_np_term->reserved = (value32 >> 0) & 0xffffff;

  fnh_np_term->entry.length = 8;

  return 0;
}
int encode_fnh_np_term (fnh_np_term_t *fnh_np_term, uint8 *data) {
  uint32 value32;

  /* word 0 */
  value32  = fnh_np_term->cmheader;
  value32  = soc_htonl (value32);
  sal_memcpy (data, &value32, 4);

  /* word 1 */
  value32  = ((fnh_np_term->rsv0 & 0x1f) << 27) |
             ((fnh_np_term->ftype & 0x7) << 24) |
             ((fnh_np_term->reserved & 0xffffff) << 0);
  value32  = soc_htonl (value32);
  sal_memcpy (data+4, &value32, 4);

  return 0;
}


int decode_raw_data (raw_data_t *raw_data, uint8 *data) {
  sal_memcpy (raw_data->raw_data, data, raw_data->entry.length);
  return 0;
}
int encode_raw_data (raw_data_t *raw_data, uint8 *data) {
  sal_memcpy (data, raw_data->raw_data, raw_data->entry.length);
  return 0;
} 

int decode_hex_data (hex_data_t *hex_data, uint8 *data) {
  sal_memcpy (hex_data->hex_data, data, hex_data->entry.length);
  return 0;
}
int encode_hex_data (hex_data_t *hex_data, uint8 *data) {
  sal_memcpy (data, hex_data->hex_data, hex_data->entry.length);
  return 0;
} 


int get_value (uint8 *raw_data, int *offset) {
  return ( (raw_data[*offset + 0] << 24) + (raw_data[*offset + 1] << 16) + \
           (raw_data[*offset + 2] <<  8) + (raw_data[*offset + 3] <<  0) );
  return 0;
}

/* ************************
 * allocates header and fills in type
 * ************************/
entry_desc_t* header_alloc (header_type_e hdr_type, uint8 *pkt_data, int length, 
                            int *offset, header_type_e *next_type) {

  unsigned int  value;
  unsigned int  opcode;
  entry_desc_t * hdr_ptr = NULL;

  erh_qe_t   *erh_qe_ptr;
  erh_isir_t *erh_isir_ptr;
  erh_esir_t *erh_esir_ptr;
  erh_iarad_t *erh_iarad_ptr;
  erh_earad_t *erh_earad_ptr;
  erh_ss_t   *erh_ss_ptr;
  erh_qess_t *erh_qess_ptr;
  higig_t *higig_ptr;
  higig2_t *higig2_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  oam_ccm_t  *oam_ccm_ptr;
  oam_lbm_t  *oam_lbm_ptr;
  oam_ltm_t  *oam_ltm_ptr;
  oam_lmm_t  *oam_lmm_ptr;
  oam_1dmm_t  *oam_1dmm_ptr;
  oam_2dmm_t  *oam_2dmm_ptr;
  bfd_t   *bfd_ptr;
  bpdu_t  *bpdu_ptr;
  slow_t  *slow_ptr;
  ipv4_t  *ipv4_ptr;
  ipv6_t  *ipv6_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;
  pwe_data_ach_t *pwe_data_ach_ptr;
  pwe_ctrl_ach_t *pwe_ctrl_ach_ptr;
  gach_t *gach_ptr;
  tlvhdr_t *tlvhdr_ptr;
  psc_t *psc_ptr;
  oam_mpls_t *oam_mpls_ptr;
  ip_option_t *ip_option_ptr;
  hop_t *hop_ptr;
  ptp_t *ptp_ptr;
  itmh_t *itmh_ptr;
  nph_t *nph_ptr;
  nfh_t *nfh_ptr;
  fnh_full_fwd_t *fnh_full_fwd_ptr;
  fnh_half_fwd_t *fnh_half_fwd_ptr;
  fnh_non_fwd_t *fnh_non_fwd_ptr;
  fnh_np_term_t *fnh_np_term_ptr;

  raw_data_t *raw_data_ptr;
  hex_data_t *hex_data_ptr;

  value = get_value (pkt_data, offset);

  if (pkt_verbose) {
    printf ("header alloc: %x at offset %d \n", value, *offset);
  }

  if (hdr_type == UNKNOWN) {
    if ((value >> 16) < 1500) {
      hdr_type = LLC;
    } else {
      switch ((value >> 16) & 0xffff) {
        case 0x8100:
          hdr_type = VLAN;
         break;
        case 0x9100:
        case 0x88A8:
          hdr_type = STAG;
         break;
        case 0x0800:
          hdr_type = ETYPE;
          *next_type = IPV4;
          break;
        case 0x86DD:
          hdr_type = ETYPE;
          *next_type = IPV6;
          break;
        case 0x8847:
          hdr_type = ETYPE;
          *next_type = MPLS;
          break;
        case 0x8809:
          hdr_type = ETYPE;
          *next_type = SLOW;
          break;
        case 0x88E7:
          hdr_type = ETYPE;
          *next_type = ITAG;
          break;
        case 0x88F7:
          hdr_type = ETYPE;
          *next_type = PTP;
          break;
       case 0x8902:
          hdr_type = ETYPE;
          opcode = value & 0xff;
          switch(opcode) {
            case OPCODE_CCM:
              *next_type = OAM_CCM;
              break;
            case OPCODE_LBR:
            case OPCODE_LBM:
              *next_type = OAM_LBM;
              break;
            case OPCODE_LTR:
            case OPCODE_LTM:
              *next_type = OAM_LTM;
              break;
            case OPCODE_1DM:
              *next_type = OAM_1DMM;
              break;
            case OPCODE_DMR:
            case OPCODE_DMM:
              *next_type = OAM_2DMM;
              break;
            default:
              *next_type = UNKNOWN;              
          }

          break;
        default:
          hdr_type = ETYPE;
          *next_type = RAW_DATA;
          break;
        }
     }
  }


  switch (hdr_type) {
    case  ERH_QE:
      erh_qe_ptr = sal_alloc(sizeof(erh_qe_t),"erh_qe");
      erh_qe_ptr->entry.type = ERH_QE;
      decode_erh_qe (erh_qe_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_qe_ptr;
      *next_type = MAC;
      break;
    case  ERH_ISIR:
      erh_isir_ptr = sal_alloc(sizeof(erh_isir_t),"erh_isir");
      erh_isir_ptr->entry.type = ERH_ISIR;
      decode_erh_isir (erh_isir_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_isir_ptr;
      *next_type = MAC;
      break;
    case  ERH_ESIR:
      erh_esir_ptr = sal_alloc(sizeof(erh_esir_t),"erh_esir");
      erh_esir_ptr->entry.type = ERH_ESIR;
      decode_erh_esir (erh_esir_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_esir_ptr;
      *next_type = MAC;
      break;
    case  ERH_IARAD:
      erh_iarad_ptr = sal_alloc(sizeof(erh_iarad_t),"erh_iarad");
      erh_iarad_ptr->entry.type = ERH_IARAD;
      decode_erh_iarad (erh_iarad_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_iarad_ptr;
      *next_type = MAC;
      break;
    case  ERH_EARAD:
      erh_earad_ptr = sal_alloc(sizeof(erh_earad_t),"erh_earad");
      erh_earad_ptr->entry.type = ERH_EARAD;
      decode_erh_earad (erh_earad_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_earad_ptr;
      *next_type = MAC;
      break;
    case  ERH_SS:
      erh_ss_ptr = sal_alloc(sizeof(erh_ss_t),"erh_ss");
      erh_ss_ptr->entry.type = ERH_SS;
      decode_erh_ss (erh_ss_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_ss_ptr;
      *next_type = MAC;
      break;
    case  ERH_QESS:
      erh_qess_ptr = sal_alloc(sizeof(erh_qess_t),"erh_qess");
      erh_qess_ptr->entry.type = ERH_QESS;
      decode_erh_qess (erh_qess_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_qess_ptr;
      *next_type = MAC;
      break;
    case  HIGIG:
      higig_ptr = sal_alloc(sizeof(higig_t),"higig");
      higig_ptr->entry.type = HIGIG;
      decode_higig (higig_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)higig_ptr;
      *next_type = MAC;
      break;
    case  HIGIG2:
      higig2_ptr = sal_alloc(sizeof(higig2_t),"higig2");
      higig2_ptr->entry.type = HIGIG2;
      decode_higig2 (higig2_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)higig2_ptr;
      *next_type = MAC;
      break;
    case  MAC:
      mac_ptr = sal_alloc(sizeof(mac_t),"mac");
      mac_ptr->entry.type = MAC;
      decode_mac (mac_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)mac_ptr;
      *next_type = UNKNOWN;
      break;
    case  VLAN:
      vlan_ptr = sal_alloc(sizeof(vlan_t),"vlan");
      vlan_ptr->entry.type = VLAN;
      decode_vlan (vlan_ptr, value);
      hdr_ptr = (entry_desc_t *)vlan_ptr;
      *next_type = UNKNOWN;
      break;
    case  STAG:
      stag_ptr = sal_alloc(sizeof(stag_t),"stag");
      stag_ptr->entry.type = STAG;
      decode_stag (stag_ptr, value);
      hdr_ptr = (entry_desc_t *)stag_ptr;
      *next_type = UNKNOWN;
      break;
    case  ETYPE:
      etype_ptr = sal_alloc(sizeof(etype_t),"etype");
      etype_ptr->entry.type = ETYPE;
      decode_etype (etype_ptr, value);
      hdr_ptr = (entry_desc_t *)etype_ptr;
      break;
    case  LLC:
      llc_ptr = sal_alloc(sizeof(llc_t),"llc");
      llc_ptr->entry.type = LLC;
      decode_llc (llc_ptr, pkt_data + *offset); 
      hdr_ptr = (entry_desc_t *)llc_ptr;
      if ((llc_ptr->ssap == 0xaa) && (llc_ptr->dsap == 0xaa)) {
        *next_type = SNAP;
      } else if ((llc_ptr->ssap == 0x42) && (llc_ptr->dsap == 0x42)) {
        *next_type = BPDU;  
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  SNAP:
      snap_ptr = sal_alloc(sizeof(snap_t),"snap");
      snap_ptr->entry.type = SNAP;
      decode_snap (snap_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)snap_ptr;
      *next_type = UNKNOWN;
      break;
    case  SLOW:
      slow_ptr = sal_alloc(sizeof(slow_t),"slow");
      slow_ptr->entry.type = SLOW;
      decode_slow (slow_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)slow_ptr;
      *next_type = UNKNOWN;
      break;
    case  BPDU:
      bpdu_ptr = sal_alloc(sizeof(bpdu_t),"bpdu");
      bpdu_ptr->entry.type = BPDU;
      decode_bpdu (bpdu_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)bpdu_ptr;
      *next_type = UNKNOWN;
      break;
    case  OAM_CCM:      
      oam_ccm_ptr = sal_alloc(sizeof(oam_ccm_t),"oam_ccm");
      oam_ccm_ptr->entry.type = OAM_CCM;
      decode_oam_ccm (oam_ccm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_ccm_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_LBM:      
      oam_lbm_ptr = sal_alloc(sizeof(oam_lbm_t),"oam_lbm");
      oam_lbm_ptr->entry.type = OAM_LBM;
      decode_oam_lbm (oam_lbm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_lbm_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_LTM:      
      oam_ltm_ptr = sal_alloc(sizeof(oam_ltm_t),"oam_ltm");
      oam_ltm_ptr->entry.type = OAM_LTM;
      decode_oam_ltm (oam_ltm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_ltm_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_LMM:      
      oam_lmm_ptr = sal_alloc(sizeof(oam_lmm_t),"oam_lmm");
      oam_lmm_ptr->entry.type = OAM_LMM;
      decode_oam_lmm (oam_lmm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_lmm_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_1DMM:      
      oam_1dmm_ptr = sal_alloc(sizeof(oam_1dmm_t),"oam_1dmm");
      oam_1dmm_ptr->entry.type = OAM_1DMM;
      decode_oam_1dmm (oam_1dmm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_1dmm_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_2DMM:      
      oam_2dmm_ptr = sal_alloc(sizeof(oam_2dmm_t),"oam_2dmm");
      oam_2dmm_ptr->entry.type = OAM_2DMM;
      decode_oam_2dmm (oam_2dmm_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_2dmm_ptr;
      *next_type = RAW_DATA;
      break;
    case  BFD:      
      bfd_ptr = sal_alloc(sizeof(bfd_t),"bfd");
      bfd_ptr->entry.type = BFD;
      decode_bfd (bfd_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)bfd_ptr;
      *next_type = RAW_DATA;
      break;
    case  IPV4:
      ipv4_ptr = sal_alloc(sizeof(ipv4_t),"ipv4");
      ipv4_ptr->entry.type = IPV4;
      decode_ipv4 (ipv4_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)ipv4_ptr;
      if (ipv4_ptr->ihl > 5) {
        *next_type = IP_OPTION;
      }
      else if (ipv4_ptr->proto == 0x11) {
        *next_type = UDP;
      } else if (ipv4_ptr->proto == 0x6) {
        *next_type = TCP;
      } else if (ipv4_ptr->proto == 0x2) {
        *next_type = IGMP;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  IP_OPTION:
      ip_option_ptr = sal_alloc(sizeof(ip_option_t),"ip_option");
      ip_option_ptr->entry.type = IP_OPTION;
      decode_ip_option (ip_option_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)ip_option_ptr;
      *next_type = RAW_DATA;
      break;
    case  IPV6:
      ipv6_ptr = sal_alloc(sizeof(ipv6_t),"ipv6");
      ipv6_ptr->entry.type = IPV6;
      decode_ipv6 (ipv6_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)ipv6_ptr;
      if (ipv6_ptr->next_header == 0x0) {
        *next_type = HOP;
      } else if (ipv6_ptr->next_header == 0x2) {
        *next_type = IGMP;
      } else if (ipv6_ptr->next_header == 0x6) {
        *next_type = TCP;
      } else if (ipv6_ptr->next_header == 0x11) {
        *next_type = UDP;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  ITAG:
      itag_ptr = sal_alloc(sizeof(itag_t),"itag");
      itag_ptr->entry.type = ITAG;
      decode_itag (itag_ptr, value);
      hdr_ptr = (entry_desc_t *)itag_ptr;
      *next_type = MAC;
      break;
    case  MPLS:
      mpls_ptr = sal_alloc(sizeof(mpls_t),"mpls");
      mpls_ptr->entry.type = MPLS;
      decode_mpls (mpls_ptr, value); 
      hdr_ptr = (entry_desc_t *)mpls_ptr;
      if (mpls_ptr->s == 0) {
        *next_type = MPLS;
      } else if (mpls_ptr->label == 0x000D) {
        *next_type = GACH;
      } else if (mpls_ptr->label == 0x000E) {
        *next_type = OAM_MPLS;
      } else {
        uint8 load_type = pkt_data[*offset];
        load_type = (load_type >> 4) & 0xf;
        if (0 == load_type) {
          *next_type = PWE_DATA_ACH;
        } else if (1 == load_type) {
          *next_type = PWE_CTRL_ACH;
        } else if (4 == load_type) {
          *next_type = IPV4;
        } else if (6 == load_type) {
          *next_type = IPV6;
        } else {
          *next_type = MAC;
        }
      }
      break;
    case  UDP:
      udp_ptr = sal_alloc(sizeof(udp_t),"udp");
      udp_ptr->entry.type = UDP;
      decode_udp (udp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)udp_ptr;
      if ((3784 == udp_ptr->dport) || (4784 == udp_ptr->dport)) {
        *next_type = BFD;
      } else if (319 == udp_ptr->dport) {
        *next_type = PTP;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  TCP:
      tcp_ptr = sal_alloc(sizeof(tcp_t),"tcp");
      tcp_ptr->entry.type = TCP;
      decode_tcp (tcp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)tcp_ptr;
      *next_type = RAW_DATA;
      break;
    case  IGMP:
      igmp_ptr = sal_alloc(sizeof(igmp_t),"igmp");
      igmp_ptr->entry.type = IGMP;
      decode_igmp (igmp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)igmp_ptr;
      *next_type = RAW_DATA;
      break;
    case  HOP:
      hop_ptr = sal_alloc(sizeof(hop_t),"hop");
      hop_ptr->entry.type = HOP;
      decode_hop (hop_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)hop_ptr;
      *next_type = RAW_DATA;
      break;
    case  PTP:
      ptp_ptr = sal_alloc(sizeof(ptp_t),"ptp");
      ptp_ptr->entry.type = PTP;
      decode_ptp (ptp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)ptp_ptr;
      *next_type = RAW_DATA;
      break;
    case  PWE_DATA_ACH:
      pwe_data_ach_ptr = sal_alloc(sizeof(pwe_data_ach_t),"pwe_data_ach");
      pwe_data_ach_ptr->entry.type = PWE_DATA_ACH;
      decode_pwe_data_ach (pwe_data_ach_ptr, value);
      hdr_ptr = (entry_desc_t *)pwe_data_ach_ptr;
      *next_type = RAW_DATA;
      break;
    case  PWE_CTRL_ACH:
      pwe_ctrl_ach_ptr = sal_alloc(sizeof(pwe_ctrl_ach_t),"pwe_ctrl_ach");
      pwe_ctrl_ach_ptr->entry.type = PWE_CTRL_ACH;
      decode_pwe_ctrl_ach (pwe_ctrl_ach_ptr, value);
      hdr_ptr = (entry_desc_t *)pwe_ctrl_ach_ptr;
      *next_type = RAW_DATA;
      break;
    case  GACH:
      gach_ptr = sal_alloc(sizeof(gach_t),"gach");
      gach_ptr->entry.type = GACH;
      decode_gach (gach_ptr, value);
      hdr_ptr = (entry_desc_t *)gach_ptr;
      if (gach_ptr->channel == 0x7ff9) {
        *next_type = TLVHDR;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  TLVHDR:
      tlvhdr_ptr = sal_alloc(sizeof(tlvhdr_t),"tlvhdr");
      tlvhdr_ptr->entry.type = TLVHDR;
      decode_tlvhdr (tlvhdr_ptr, value);
      hdr_ptr = (entry_desc_t *)tlvhdr_ptr;
      *next_type = PSC;
      break;
    case  PSC:
      psc_ptr = sal_alloc(sizeof(psc_t),"psc");
      psc_ptr->entry.type = PSC;
      decode_psc (psc_ptr, value);
      hdr_ptr = (entry_desc_t *)psc_ptr;
      *next_type = RAW_DATA;
      break;
    case  OAM_MPLS:
      oam_mpls_ptr = sal_alloc(sizeof(oam_mpls_t),"oam_mpls");
      oam_mpls_ptr->entry.type = OAM_MPLS;
      decode_oam_mpls (oam_mpls_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)oam_mpls_ptr;
      *next_type = RAW_DATA;
      break;
    case  ITMH:
      itmh_ptr = sal_alloc(sizeof(itmh_t),"itmh");
      itmh_ptr->entry.type = ITMH;
      decode_itmh (itmh_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)itmh_ptr;
      *next_type = NPH;
      break;
    case  NPH:
      nph_ptr = sal_alloc(sizeof(nph_t),"nph");
      nph_ptr->entry.type = NPH;
      decode_nph (nph_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)nph_ptr;
      *next_type = RAW_DATA;
      break;
    case  NFH:
      nfh_ptr = sal_alloc(sizeof(nfh_t),"nfh");
      nfh_ptr->entry.type = NFH;
      decode_nfh (nfh_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)nfh_ptr;
      *next_type = MAC;
      break;
    case  FNH_FULL_FWD:
      fnh_full_fwd_ptr = sal_alloc(sizeof(fnh_full_fwd_t),"fnh_full_fwd");
      fnh_full_fwd_ptr->entry.type = FNH_FULL_FWD;
      decode_fnh_full_fwd (fnh_full_fwd_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)fnh_full_fwd_ptr;
      *next_type = MAC;
      break;
    case  FNH_HALF_FWD:
      fnh_half_fwd_ptr = sal_alloc(sizeof(fnh_half_fwd_t),"fnh_half_fwd");
      fnh_half_fwd_ptr->entry.type = FNH_HALF_FWD;
      decode_fnh_half_fwd (fnh_half_fwd_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)fnh_half_fwd_ptr;
      *next_type = MAC;
      break;
    case  FNH_NON_FWD:
      fnh_non_fwd_ptr = sal_alloc(sizeof(fnh_non_fwd_t),"fnh_non_fwd");
      fnh_non_fwd_ptr->entry.type = FNH_NON_FWD;
      decode_fnh_non_fwd (fnh_non_fwd_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)fnh_non_fwd_ptr;
      *next_type = MAC;
      break;
    case  FNH_NP_TERM:
      fnh_np_term_ptr = sal_alloc(sizeof(fnh_np_term_t),"fnh_np_term");
      fnh_np_term_ptr->entry.type = FNH_NP_TERM;
      decode_fnh_np_term (fnh_np_term_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)fnh_np_term_ptr;
      *next_type = MAC;
      break;
    case RAW_DATA:
      raw_data_ptr = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data_ptr->entry.type = RAW_DATA;
      raw_data_ptr->entry.next = NULL;
      if (length > *offset) {
        raw_data_ptr->entry.length = length - *offset;
      } else {
        raw_data_ptr->entry.length = 0;
      }
      decode_raw_data (raw_data_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)raw_data_ptr;
      break;
    case HEX_DATA:
      hex_data_ptr = sal_alloc(sizeof(hex_data_t),"hex_data");
      hex_data_ptr->entry.type = HEX_DATA;
      if (length > *offset) {
        hex_data_ptr->entry.length = length - *offset;
      } else {
        hex_data_ptr->entry.length = 0;
      }
      decode_hex_data (hex_data_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)hex_data_ptr;
      *next_type = RAW_DATA;
      break;
    default:
      hdr_ptr = NULL;
      printf("unexpected htype %d\n", hdr_type);
      return (hdr_ptr);
  }

  *offset += hdr_ptr->length;
  return (hdr_ptr);
}

/* ***********************
 * From Byte - convert network byte string to Packet linked list
 * ***********************/
int sbxpkt_from_byte (header_type_e start_type, uint8 *pkt_data, int length, sbxpkt_t *return_pkt) {

  int offset=0;
  header_type_e next_type;
  entry_desc_t *hdr_ptr;
  entry_desc_t *next_hdr_ptr;

  /* printf ("from byte %x %x %x %x for %d \n", pkt_data[0], pkt_data[1], pkt_data[2], pkt_data[3], length); */
  /* alloc and fill first_hdr_t  */
  hdr_ptr = header_alloc (start_type, pkt_data, length, &offset, &next_type);
  /*printf("start_type=%d, next_type=%d\n", (int)start_type, (int)next_type);*/

  /* link first_hdr_t to sbxpkt_t  */
  return_pkt->entry.next = hdr_ptr;

  /*printf ("header pointer %p  type %d next %p at offset %d  length=%d\n", hdr_ptr, hdr_ptr->type, hdr_ptr->next, offset, length);*/

  /* determine next header type or exit  */
  while (offset <= length) { 
    /* alloc and fill next_hdr_t */
    next_hdr_ptr = header_alloc (next_type, pkt_data, length, &offset, &next_type);
    /*printf("*start_type=%d, next_type=%d\n", (int)start_type, (int)next_type);*/

    hdr_ptr->next = next_hdr_ptr;

    /*determine next heder type or exit */
    hdr_ptr = next_hdr_ptr;
    if (hdr_ptr->type == RAW_DATA) break;
    
    /*    printf ("header pointer %p  type %d next %p at offset %d\n", hdr_ptr, hdr_ptr->type, hdr_ptr->next, offset);  */
  }

  if (hdr_ptr->type != RAW_DATA) {
    printf ("unexpected packet length - terminating decoding headers hdr_ptr->type=%d\n",hdr_ptr->type);
    hdr_ptr->next = NULL;
  }

  return_pkt->entry.length = offset;

  return 0;
}

/* ***********************
 * To Byte - convert a packet linked list to network byte string
 * ***********************/
int sbxpkt_to_byte (sbxpkt_t *packet, uint8 *pkt_data) {
 
  entry_desc_t *hdr_ptr;

  erh_qe_t   *erh_qe_ptr;
  erh_isir_t *erh_isir_ptr;
  erh_esir_t *erh_esir_ptr;
  erh_iarad_t *erh_iarad_ptr;
  erh_earad_t *erh_earad_ptr;
  /* erh_ss_t   *erh_ss_ptr; */
  /* erh_qess_t *erh_qess_ptr; */
  higig_t *higig_ptr;
  higig2_t *higig2_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  oam_ccm_t  *oam_ccm_ptr;
  oam_lbm_t  *oam_lbm_ptr;
  oam_ltm_t  *oam_ltm_ptr;
  oam_lmm_t  *oam_lmm_ptr;
  oam_1dmm_t  *oam_1dmm_ptr;
  oam_2dmm_t  *oam_2dmm_ptr;
  bfd_t   *bfd_ptr;
  bpdu_t  *bpdu_ptr;
  slow_t  *slow_ptr;
  ipv4_t  *ipv4_ptr;
  ipv6_t  *ipv6_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;
  pwe_data_ach_t *pwe_data_ach_ptr;
  pwe_ctrl_ach_t *pwe_ctrl_ach_ptr;
  gach_t *gach_ptr;
  tlvhdr_t *tlvhdr_ptr;
  psc_t *psc_ptr;
  oam_mpls_t *oam_mpls_ptr;
  ip_option_t *ip_option_ptr;
  hop_t *hop_ptr;
  ptp_t *ptp_ptr;
  itmh_t *itmh_ptr;
  nph_t *nph_ptr;
  nfh_t *nfh_ptr;
  fnh_full_fwd_t *fnh_full_fwd_ptr;
  fnh_half_fwd_t *fnh_half_fwd_ptr;
  fnh_non_fwd_t *fnh_non_fwd_ptr;
  fnh_np_term_t *fnh_np_term_ptr;

  raw_data_t *raw_data_ptr;
  hex_data_t *hex_data_ptr;


  if (packet->normalize) {
    /* normalize_packet (packet); */
  }

  hdr_ptr = packet->entry.next;

  while (hdr_ptr) {
    switch (hdr_ptr->type) {
      case  ERH_QE:
        erh_qe_ptr = (erh_qe_t *)hdr_ptr;
        encode_erh_qe (erh_qe_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  ERH_ISIR:
        erh_isir_ptr = (erh_isir_t *)hdr_ptr;
        encode_erh_isir (erh_isir_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  ERH_ESIR:
        erh_esir_ptr = (erh_esir_t *)hdr_ptr;
        encode_erh_esir (erh_esir_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  ERH_IARAD:
        erh_iarad_ptr = (erh_iarad_t *)hdr_ptr;
        encode_erh_iarad (erh_iarad_ptr, pkt_data);
        pkt_data += 16;
        break;
      case  ERH_EARAD:
        erh_earad_ptr = (erh_earad_t *)hdr_ptr;
        encode_erh_earad (erh_earad_ptr, pkt_data);
        pkt_data += 16;
        break;
      case  ERH_SS:
        /* erh_ss_ptr = (erh_ss_t *)hdr_ptr; */
        /* encode_erh_ss_header (erh_ss_ptr); */
        pkt_data += 12;
        break;
      case  ERH_QESS:
        /* erh_qess_ptr = (erh_qess_t *)hdr_ptr; */
        /* encode_erh_qess_header (erh_qess_ptr); */
        pkt_data += 14;
        break;
      case  HIGIG:
        higig_ptr = (higig_t *)hdr_ptr;
        encode_higig (higig_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  HIGIG2:
        higig2_ptr = (higig2_t *)hdr_ptr;
        encode_higig2 (higig2_ptr, pkt_data);
        pkt_data += 8;
        if (higig2_ptr->ppdtype == 0) {
          pkt_data += 8;
        }
        break;
      case  MAC:
        mac_ptr = (mac_t *)hdr_ptr;
        encode_mac (mac_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  VLAN:
        vlan_ptr = (vlan_t *)hdr_ptr;
        encode_vlan (vlan_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  STAG:
        stag_ptr = (stag_t *)hdr_ptr;
        encode_stag (stag_ptr, pkt_data);
       pkt_data += 4;
        break;
      case  ETYPE:
        etype_ptr = (etype_t *)hdr_ptr;
        encode_etype (etype_ptr, pkt_data);
        pkt_data += 2;
        break;
      case  LLC:
        llc_ptr = (llc_t *)hdr_ptr;
        encode_llc (llc_ptr, pkt_data);
        pkt_data += 7;
        break;
      case  SNAP:
        snap_ptr = (snap_t *)hdr_ptr;
        encode_snap (snap_ptr, pkt_data);
        pkt_data += 3;
        break;
      case  BPDU:
        bpdu_ptr = (bpdu_t *)hdr_ptr;
        encode_bpdu (bpdu_ptr, pkt_data);
        pkt_data += 35;
        break;
      case  SLOW:
        slow_ptr = (slow_t *)hdr_ptr;
        encode_slow (slow_ptr, pkt_data);
        pkt_data += 1;
        break;
      case  OAM_CCM:
        oam_ccm_ptr = (oam_ccm_t *)hdr_ptr;
        encode_oam_ccm (oam_ccm_ptr, pkt_data);
        pkt_data += oam_ccm_ptr->entry.length;
        break;
      case  OAM_LBM:
        oam_lbm_ptr = (oam_lbm_t *)hdr_ptr;
        encode_oam_lbm (oam_lbm_ptr, pkt_data);
        pkt_data += oam_lbm_ptr->entry.length;
        break;
      case  OAM_LTM:
        oam_ltm_ptr = (oam_ltm_t *)hdr_ptr;
        encode_oam_ltm (oam_ltm_ptr, pkt_data);
        pkt_data += oam_ltm_ptr->entry.length;
        break;
      case  OAM_LMM:
        oam_lmm_ptr = (oam_lmm_t *)hdr_ptr;
        encode_oam_lmm (oam_lmm_ptr, pkt_data);
        pkt_data += oam_lmm_ptr->entry.length;
        break;
      case  OAM_1DMM:
        oam_1dmm_ptr = (oam_1dmm_t *)hdr_ptr;
        encode_oam_1dmm (oam_1dmm_ptr, pkt_data);
        pkt_data += oam_1dmm_ptr->entry.length;
        break;
      case  OAM_2DMM:
        oam_2dmm_ptr = (oam_2dmm_t *)hdr_ptr;
        encode_oam_2dmm (oam_2dmm_ptr, pkt_data);
        pkt_data += oam_2dmm_ptr->entry.length;
        break;
      case  BFD:
        bfd_ptr = (bfd_t *)hdr_ptr;
        encode_bfd (bfd_ptr, pkt_data);
        pkt_data += bfd_ptr->entry.length;
        break;
      case  IPV4:
        ipv4_ptr = (ipv4_t *)hdr_ptr;
        encode_ipv4 (ipv4_ptr, pkt_data);
        pkt_data += 20;
        break;
      case  IPV6:
        ipv6_ptr = (ipv6_t *)hdr_ptr;
        encode_ipv6 (ipv6_ptr, pkt_data);
        pkt_data += 40;
        break;
      case  ITAG:
        itag_ptr = (itag_t *)hdr_ptr;
        encode_itag (itag_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  MPLS:
        mpls_ptr = (mpls_t *)hdr_ptr;
        encode_mpls (mpls_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  UDP:
        udp_ptr = (udp_t *)hdr_ptr;
        encode_udp (udp_ptr, pkt_data);
        pkt_data += 8;
        break;
      case  TCP:
        tcp_ptr = (tcp_t *)hdr_ptr;
        encode_tcp (tcp_ptr, pkt_data);
        pkt_data += 20;
        break;
      case  IGMP:
        igmp_ptr = (igmp_t *)hdr_ptr;
        encode_igmp (igmp_ptr, pkt_data);
        break;
      case  HOP:
        hop_ptr = (hop_t *)hdr_ptr;
        encode_hop (hop_ptr, pkt_data);
        pkt_data += 8;
        break;
      case  PTP:
        ptp_ptr = (ptp_t *)hdr_ptr;
        encode_ptp (ptp_ptr, pkt_data);
        pkt_data += 34;
        break;
      case  PWE_DATA_ACH:
        pwe_data_ach_ptr = (pwe_data_ach_t *)hdr_ptr;
        encode_pwe_data_ach (pwe_data_ach_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  PWE_CTRL_ACH:
        pwe_ctrl_ach_ptr = (pwe_ctrl_ach_t *)hdr_ptr;
        encode_pwe_ctrl_ach (pwe_ctrl_ach_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  GACH:
        gach_ptr = (gach_t *)hdr_ptr;
        encode_gach (gach_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  TLVHDR:
        tlvhdr_ptr = (tlvhdr_t *)hdr_ptr;
        encode_tlvhdr (tlvhdr_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  PSC:
        psc_ptr = (psc_t *)hdr_ptr;
        encode_psc (psc_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  OAM_MPLS:
        oam_mpls_ptr = (oam_mpls_t *)hdr_ptr;
        encode_oam_mpls (oam_mpls_ptr, pkt_data);
        pkt_data += 44;
        break;
      case  IP_OPTION:
        ip_option_ptr = (ip_option_t *)hdr_ptr;
        encode_ip_option (ip_option_ptr, pkt_data);
        pkt_data += ip_option_ptr->length+1;
        break;
      case  ITMH:
        itmh_ptr = (itmh_t *)hdr_ptr;
        encode_itmh (itmh_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  NPH:
        nph_ptr = (nph_t *)hdr_ptr;
        encode_nph (nph_ptr, pkt_data);
        pkt_data += 16;
        break;
      case  NFH:
        nfh_ptr = (nfh_t *)hdr_ptr;
        encode_nfh (nfh_ptr, pkt_data);
        pkt_data += 20;
        break;
      case  FNH_FULL_FWD:
        fnh_full_fwd_ptr = (fnh_full_fwd_t *)hdr_ptr;
        encode_fnh_full_fwd (fnh_full_fwd_ptr, pkt_data);
        pkt_data += 10;
        break;
      case  FNH_HALF_FWD:
        fnh_half_fwd_ptr = (fnh_half_fwd_t *)hdr_ptr;
        encode_fnh_half_fwd (fnh_half_fwd_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  FNH_NON_FWD:
        fnh_non_fwd_ptr = (fnh_non_fwd_t *)hdr_ptr;
        encode_fnh_non_fwd (fnh_non_fwd_ptr, pkt_data);
        pkt_data += 10;
        break;
      case  FNH_NP_TERM:
        fnh_np_term_ptr = (fnh_np_term_t *)hdr_ptr;
        encode_fnh_np_term (fnh_np_term_ptr, pkt_data);
        pkt_data += 8;
        break;
      case RAW_DATA:
        raw_data_ptr = (raw_data_t *)hdr_ptr;
        encode_raw_data (raw_data_ptr, pkt_data);
        break;
      case HEX_DATA:
        hex_data_ptr = (hex_data_t *)hdr_ptr;
        encode_hex_data (hex_data_ptr, pkt_data);
        pkt_data += hex_data_ptr->entry.length;
        break;        
      default:
       printf("sbxpkt_to_byte: unexpected htype %d\n", hdr_ptr->type);
       break;
    }

   hdr_ptr = hdr_ptr->next;
  }

  
 /* printf ("to byte: packet at %x for %d\n", packet, packet->entry.length);  */
  return 0;
}

/* ********************************
 * Allocate and Free Packet
 * ********************************/
sbxpkt_t* sbxpkt_alloc (void) {
  sbxpkt_t *packet;
  packet = sal_alloc(sizeof(sbxpkt_t),"pkt");
  if (packet == NULL) {
    printf ("unable to allocate memory for packet\n");
  } else {
    sal_memset(packet, 0, sizeof(*packet));
  }
  return (packet);
}

int sbxpkt_free (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;
  entry_desc_t *next_hdr;

  if (packet == NULL) return -1;

  cur_hdr = packet->entry.next;
  sal_free (packet);

  while (cur_hdr) {
   next_hdr = cur_hdr->next;
   sal_free (cur_hdr);
   cur_hdr = next_hdr;
  }
  return 0;
}

int header_print (entry_desc_t *hdr_ptr) {

  erh_qe_t   *erh_qe_ptr;
  erh_isir_t *erh_isir_ptr;
  erh_esir_t *erh_esir_ptr;
  erh_iarad_t *erh_iarad_ptr;
  erh_earad_t *erh_earad_ptr;
  erh_ss_t   *erh_ss_ptr;
  erh_qess_t *erh_qess_ptr;
  higig_t  *higig_ptr;
  higig2_t *higig2_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  oam_ccm_t  *oam_ccm_ptr;
  oam_lbm_t  *oam_lbm_ptr;
  oam_ltm_t  *oam_ltm_ptr;
  oam_lmm_t  *oam_lmm_ptr;
  oam_1dmm_t  *oam_1dmm_ptr;
  oam_2dmm_t  *oam_2dmm_ptr;
  bfd_t   *bfd_ptr;
  bpdu_t  *bpdu_ptr;
  slow_t  *slow_ptr;
  ipv4_t  *ipv4_ptr;
  ipv6_t  *ipv6_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;
  pwe_data_ach_t  *pwe_data_ach_ptr;
  pwe_ctrl_ach_t  *pwe_ctrl_ach_ptr;
  gach_t  *gach_ptr;
  tlvhdr_t *tlvhdr_ptr;
  psc_t  *psc_ptr;
  oam_mpls_t *oam_mpls_ptr;
  ip_option_t *ip_option_ptr;
  hop_t *hop_ptr;
  ptp_t *ptp_ptr;
  itmh_t *itmh_ptr;
  nph_t *nph_ptr;
  nfh_t *nfh_ptr;
  fnh_full_fwd_t *fnh_full_fwd_ptr;
  fnh_half_fwd_t *fnh_half_fwd_ptr;
  fnh_non_fwd_t *fnh_non_fwd_ptr;
  fnh_np_term_t *fnh_np_term_ptr;

  raw_data_t *raw_data_ptr;
  hex_data_t *hex_data_ptr;

  switch (hdr_ptr->type) {
    case  ERH_QE:
      erh_qe_ptr = (erh_qe_t *)hdr_ptr;
      print_erh_qe_header (erh_qe_ptr);
      break;
    case  ERH_ISIR:
      erh_isir_ptr = (erh_isir_t *)hdr_ptr;
      print_erh_isir_header (erh_isir_ptr);
      break;
    case  ERH_ESIR:
      erh_esir_ptr = (erh_esir_t *)hdr_ptr;
      print_erh_esir_header (erh_esir_ptr);
      break;
    case  ERH_IARAD:
      erh_iarad_ptr = (erh_iarad_t *)hdr_ptr;
      print_erh_iarad_header (erh_iarad_ptr);
      break;
    case  ERH_EARAD:
      erh_earad_ptr = (erh_earad_t *)hdr_ptr;
      print_erh_earad_header (erh_earad_ptr);
      break;
    case  ERH_SS:
      erh_ss_ptr = (erh_ss_t *)hdr_ptr;
      print_erh_ss_header (erh_ss_ptr);
      break;
    case  ERH_QESS:
      erh_qess_ptr = (erh_qess_t *)hdr_ptr;
      print_erh_qess_header (erh_qess_ptr);
      break;
    case  HIGIG:
      higig_ptr = (higig_t *)hdr_ptr;
      print_higig_header (higig_ptr);
      break; 
    case  HIGIG2:
      higig2_ptr = (higig2_t *)hdr_ptr;
      print_higig2_header (higig2_ptr);
      break; 
    case  MAC:
      mac_ptr = (mac_t *)hdr_ptr;
      print_mac_header (mac_ptr);
      break;
    case  VLAN:
      vlan_ptr = (vlan_t *)hdr_ptr;
      print_vlan_header (vlan_ptr);
      break;
    case  STAG:
      stag_ptr = (stag_t *)hdr_ptr;
      print_stag_header (stag_ptr);
      break;
    case  ETYPE:
      etype_ptr = (etype_t *)hdr_ptr;
      print_etype_header (etype_ptr);
      break;
    case  LLC:
      llc_ptr = (llc_t *)hdr_ptr;
      print_llc_header (llc_ptr);
      break;
    case  SNAP:
      snap_ptr = (snap_t *)hdr_ptr;
      print_snap_header (snap_ptr);
      break;
    case  BPDU:
      bpdu_ptr = (bpdu_t *)hdr_ptr;
      print_bpdu_header (bpdu_ptr);
      break;
    case  SLOW:
      slow_ptr = (slow_t *)hdr_ptr;
      print_slow_header (slow_ptr);
      break;
    case  OAM_CCM:
      oam_ccm_ptr = (oam_ccm_t *)hdr_ptr;
      print_oam_ccm_pdu (oam_ccm_ptr);
      break;
    case  OAM_LBM:
      oam_lbm_ptr = (oam_lbm_t *)hdr_ptr;
      print_oam_lbm_pdu (oam_lbm_ptr);
      break;
    case  OAM_LTM:
      oam_ltm_ptr = (oam_ltm_t *)hdr_ptr;
      print_oam_ltm_pdu (oam_ltm_ptr);
      break;
    case  OAM_LMM:
      oam_lmm_ptr = (oam_lmm_t *)hdr_ptr;
      print_oam_lmm_pdu (oam_lmm_ptr);
      break;
    case  OAM_1DMM:
      oam_1dmm_ptr = (oam_1dmm_t *)hdr_ptr;
      print_oam_1dmm_pdu (oam_1dmm_ptr);
      break;
    case  OAM_2DMM:
      oam_2dmm_ptr = (oam_2dmm_t *)hdr_ptr;
      print_oam_2dmm_pdu (oam_2dmm_ptr);
      break;
    case  BFD:
      bfd_ptr = (bfd_t *)hdr_ptr;
      print_bfd_header (bfd_ptr);
      break;
    case  IPV4:
      ipv4_ptr = (ipv4_t *)hdr_ptr;
      print_ipv4_header (ipv4_ptr);
      break;
    case  IPV6:
      ipv6_ptr = (ipv6_t *)hdr_ptr;
      print_ipv6_header (ipv6_ptr);
      break;
    case  ITAG:
      itag_ptr = (itag_t *)hdr_ptr;
      print_itag_header (itag_ptr);
      break;
    case  MPLS:
      mpls_ptr = (mpls_t *)hdr_ptr;
      print_mpls_header (mpls_ptr);
      break;
    case  UDP:
      udp_ptr = (udp_t *)hdr_ptr;
      print_udp_header (udp_ptr);
      break;
    case  TCP:
      tcp_ptr = (tcp_t *)hdr_ptr;
      print_tcp_header (tcp_ptr);
      break;
    case  IGMP:
      igmp_ptr = (igmp_t *)hdr_ptr;
      print_igmp_header (igmp_ptr);
      break;
    case  HOP:
      hop_ptr = (hop_t *)hdr_ptr;
      print_hop_header (hop_ptr);
      break;
    case  PTP:
      ptp_ptr = (ptp_t *)hdr_ptr;
      print_ptp_header (ptp_ptr);
      break;
    case  PWE_DATA_ACH:
      pwe_data_ach_ptr = (pwe_data_ach_t *)hdr_ptr;
      print_pwe_data_ach_header (pwe_data_ach_ptr);
      break;
    case  PWE_CTRL_ACH:
      pwe_ctrl_ach_ptr = (pwe_ctrl_ach_t *)hdr_ptr;
      print_pwe_ctrl_ach_header (pwe_ctrl_ach_ptr);
      break;
    case  GACH:
      gach_ptr = (gach_t *)hdr_ptr;
      print_gach_header (gach_ptr);
      break;
    case  TLVHDR:
      tlvhdr_ptr = (tlvhdr_t *)hdr_ptr;
      print_tlvhdr_header (tlvhdr_ptr);
      break;
    case  PSC:
      psc_ptr = (psc_t *)hdr_ptr;
      print_psc_header (psc_ptr);
      break;
    case  OAM_MPLS:
      oam_mpls_ptr = (oam_mpls_t *)hdr_ptr;
      print_oam_mpls_header (oam_mpls_ptr);
      break;
    case  IP_OPTION:
      ip_option_ptr = (ip_option_t *)hdr_ptr;
      print_ip_option_header (ip_option_ptr);
      break;
    case  ITMH:
      itmh_ptr = (itmh_t *)hdr_ptr;
      print_itmh_header (itmh_ptr);
      break;
    case  NPH:
      nph_ptr = (nph_t *)hdr_ptr;
      print_nph_header (nph_ptr);
      break;
    case  NFH:
      nfh_ptr = (nfh_t *)hdr_ptr;
      print_nfh_header (nfh_ptr);
      break;
    case  FNH_FULL_FWD:
      fnh_full_fwd_ptr = (fnh_full_fwd_t *)hdr_ptr;
      print_fnh_full_fwd_header (fnh_full_fwd_ptr);
      break;
    case  FNH_HALF_FWD:
      fnh_half_fwd_ptr = (fnh_half_fwd_t *)hdr_ptr;
      print_fnh_half_fwd_header (fnh_half_fwd_ptr);
      break;
    case  FNH_NON_FWD:
      fnh_non_fwd_ptr = (fnh_non_fwd_t *)hdr_ptr;
      print_fnh_non_fwd_header (fnh_non_fwd_ptr);
      break;
    case  FNH_NP_TERM:
      fnh_np_term_ptr = (fnh_np_term_t *)hdr_ptr;
      print_fnh_np_term_header (fnh_np_term_ptr);
      break;
    case RAW_DATA:
      raw_data_ptr = (raw_data_t *)hdr_ptr;
      print_raw_data_header (raw_data_ptr);
      break;
    case HEX_DATA:
      hex_data_ptr = (hex_data_t *)hdr_ptr;
      print_hex_data_header (hex_data_ptr);
      break;      
    default:
     printf("header print: unexpected htype %d\n", hdr_ptr->type);
     break;
  }
  return 0;
}

int sbxpkt_print (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;
  entry_desc_t *next_hdr;

  if (packet == NULL) return -1;

  /* printf ("packet at %x - type %x next %x \n", packet, packet->entry.type, packet->entry.next); */
  printf ("Packet Information:  len=%d (0x%x)\n", packet->entry.length, packet->entry.length);

  cur_hdr = packet->entry.next;

  while (cur_hdr) {
   next_hdr = cur_hdr->next;

   /* printf ("header at %x - type %x next %x \n", cur_hdr, cur_hdr->type, cur_hdr->next); */
   /*printf("cur_hdr=%#X, next_hdr=%3x\n\r", (int)cur_hdr, (int)next_hdr);*/
   header_print (cur_hdr);

   cur_hdr = next_hdr;
  }
  return 0;
}



int sbxpkt_get_type (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;

  if (packet == NULL) return -1;

  cur_hdr = packet->entry.next;

  if(cur_hdr) {
    return cur_hdr->type;
  }
  else {
    return -1;
  }

}

int sbxpkt_check_ipv4 (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;
  int rc = 0;

  if (packet == NULL) {
    return rc;
  }

  cur_hdr = packet->entry.next;
  while (cur_hdr) {
    if (IPV4 == cur_hdr->type) {
      return 1;
    }
    cur_hdr = cur_hdr->next;
  }
  return rc;
}


int debug_print_diff (uint8 * pkt_data, int length, int diff_pos) {
  int i;
  int line_len = 16;

  printf ("\npacket dump for length = %d\n", length);

  /* if (length > 128) length = 128;  */

  for (i=0; i<length; i++) { 

    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x  ", i); 
    }
    if( (diff_pos == (i-1)) || (diff_pos == i) ) {
      printf("*");
    }
    else {
      printf(" ");
    }
    printf ("%02x", pkt_data[i]);

  }

  printf ("\n\n");
  return 0;
}

int debug_print (uint8 * pkt_data, int length) {
  int i;
  int line_len = 16;

  printf ("\npacket dump for length = %d\n", length);

  /* if (length > 128) length = 128;  */

  for (i=0; i<length; i++) { 
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x  ", i); 
    }
    printf ("%02x ", pkt_data[i]);
  }

  printf ("\n\n");
  return 0;
}


int sbxpkt_compare_data(sbxpkt_t *tx_pkt, uint8 *rx_pkt_data, int rx_pkt_len) 
{
    int     i;
    uint8   tx_pkt_data[2000];

    if ((tx_pkt == NULL) || (rx_pkt_data == NULL))
    {
        return -1;
    }

    if (tx_pkt->entry.length == rx_pkt_len) 
    {
        sbxpkt_to_byte (tx_pkt, tx_pkt_data);
        for (i=0; i< (tx_pkt->entry.length); i++) 
        {
            if (tx_pkt_data[i] != rx_pkt_data[i]) 
            {
                printf ("Expected Packet: \n");
                debug_print_diff(tx_pkt_data, tx_pkt->entry.length, i);
                printf ("Received Packet: \n");
                debug_print_diff(rx_pkt_data, rx_pkt_len, i);
                return -1;
            }
        }
        return 0;
    }

    printf ("Expected Packet len=%d, Received Packet len=%d\n", tx_pkt->entry.length, rx_pkt_len);
    return -1;
}




int sbxpkt_compare (sbxpkt_t *tx_pkt, sbxpkt_t *rx_pkt) {
  int i;
  uint8 tx_pkt_data[2000];
  uint8 rx_pkt_data[2000];

  if ((tx_pkt == NULL) || (rx_pkt == NULL)) return -1;


  if (tx_pkt->entry.length == rx_pkt->entry.length) {
    sbxpkt_to_byte (tx_pkt, tx_pkt_data);
    sbxpkt_to_byte (rx_pkt, rx_pkt_data);
    for (i=0; i< (tx_pkt->entry.length); i++) {
      if (tx_pkt_data[i] != rx_pkt_data[i]) {
        printf ("Expected Packet: \n");
        sbxpkt_print(tx_pkt);
        debug_print_diff(tx_pkt_data, tx_pkt->entry.length, i);
        printf ("Received Packet: \n");
        sbxpkt_print(rx_pkt);
        debug_print_diff(rx_pkt_data, rx_pkt->entry.length, i);
        return -1;
      }
    }
    return 0;
  }

  printf ("Expected Packet: \n");
  sbxpkt_print(tx_pkt);
  printf ("Received Packet: \n");
  sbxpkt_print(rx_pkt);

  return -1;
}

int sbxpkt_compare_ext (sbxpkt_t *tx_pkt, sbxpkt_t *rx_pkt, int length) {
  int i;
  uint8 tx_pkt_data[2000];
  uint8 rx_pkt_data[2000];

  if ((tx_pkt == NULL) || (rx_pkt == NULL)) return -1;


  if (tx_pkt->entry.length == rx_pkt->entry.length) {
    sbxpkt_to_byte (tx_pkt, tx_pkt_data);
    sbxpkt_to_byte (rx_pkt, rx_pkt_data);
    for (i=0; i< length; i++) {
      if (tx_pkt_data[i] != rx_pkt_data[i]) {
        printf ("Expected Packet: \n");
        sbxpkt_print(tx_pkt);
        debug_print_diff(tx_pkt_data, tx_pkt->entry.length, i);
        printf ("Received Packet: \n");
        sbxpkt_print(rx_pkt);
        debug_print_diff(rx_pkt_data, rx_pkt->entry.length, i);
        return -1;
      }
    }
    return 0;
  }

  printf ("Expected Packet: \n");
  sbxpkt_print(tx_pkt);
  printf ("Received Packet: \n");
  sbxpkt_print(rx_pkt);

  return -1;
}


int p_build (op_e op, sbxpkt_t *packet, char *char_data) {
  int rc;
  int i;
  int argc;
  char **argv;

  if (packet == NULL) return -1;

  /* allocate array of pointers */
  /* coverity[suspicious_sizeof] */
  argv = sal_alloc(MAXFIELD * MAXARGS * sizeof(argv),"argv");

  /* allocate storage for input */
  for (i=0; i<MAXFIELD; i++) {
    argv[i] = sal_alloc(MAXFIELD * sizeof(char),"argv");
  }

  convert_string (char_data, &argc, argv);

  rc = header_build (op, packet, argc, argv);

  for (i=0; i<MAXFIELD; i++) {
    sal_free (argv[i]);
  }
  sal_free (argv);

  return rc;
}

int sbxpkt_create (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (CREATE, packet, char_data);

  return rc;
}

int sbxpkt_prepend (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (PREPEND, packet, char_data);

  return rc;
}

int sbxpkt_append (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (APPEND, packet, char_data);

  return rc;
}

void sbxpkt_test_udp(void) {
  sbxpkt_t *pkt;
  uint8 pkt_data[2000];

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--mac -smac 00:11:22:33:44:55 --stag --vlan -pri 5 -vid 18 --etype -etype 0x09bb --ipv4");

  sbxpkt_print(pkt);

  sbxpkt_to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_append (pkt, "--udp -sport 0x1111 -dport 0x2222");
  sbxpkt_print(pkt);

  sbxpkt_to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);
  return;
}

void sbxpkt_test_itag(void) {
  sbxpkt_t *pkt;
  uint8 pkt_data[2000];

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--mac -dmac 00:00:ab:cd:ef:ff --etype -etype 0x8847 --mpls");

  sbxpkt_print(pkt);

  sbxpkt_to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_prepend (pkt, "--mac --vlan -vid 0xabc --etype -etype 0x88e7 --itag -isid 0x12345");

  sbxpkt_print(pkt);

  sbxpkt_to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);

  return;
}

void sbxpkt_test_erh1(void) {
  sbxpkt_t *pkt;
  uint8 pkt_data[2000];

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--erh_qe --mac --vlan --etype --raw_data -length 100");

  sbxpkt_print(pkt);

  sbxpkt_to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);

  return;
}

void sbxpkt_test_cmpmpls(void) {
  int rc = 0;
  sbxpkt_t *pkt;
  sbxpkt_t *i_pkt;
  uint8 eth_data[128] = {0,1,2,3,4,5,0,1,2,3,4,6,0x81,0x00,0,3,
                         0x88,0x47,0xab,0xcd,0xe1,0x40,0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x08, 0x11,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  sbxpkt_from_byte (MAC, eth_data, 64, pkt);

  i_pkt = sbxpkt_alloc();
  sbxpkt_create (i_pkt, "--mac -dmac 00:01:02:03:04:05 -smac 00:01:02:03:04:06 --vlan -pri 0 -vid 3 \
                    --etype -etype 0x8847 --mpls -label 0xabcde -s 1 -ttl 0x40 \
                    --mac -dmac 00:aa:aa:aa:aa:aa -smac 00:bb:bb:bb:bb:bb --etype -etype 0x811 \
                    --raw_data -length 28 -value 0x55555555 -mode 1");
  rc = sbxpkt_compare (i_pkt, pkt);
  if (rc != 0) {
    printf("%s failed.\n", __FUNCTION__);
  }

  sbxpkt_free(i_pkt);
  sbxpkt_free(pkt);

  return;
}

void sbxpkt_test_data(void) {
  sbxpkt_t *pkt;
  uint8 eth_data2[128] = {0x00, 0xe0, 0xfc, 0x00, 0x44, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                     0x08, 0x00, 0x45, 0x00, 0x00, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
                     0x74, 0x7c, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
                     0x03, 0x04, 0x00, 0x04, 0x01, 0x00, 0x56, 0xf1, 0x39, 0x00, 0x00,
                     0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  sbxpkt_from_byte (MAC, eth_data2, 0, pkt);
  sbxpkt_from_byte (MAC, eth_data2, 20, pkt);
  sbxpkt_from_byte (MAC, eth_data2, 120, pkt);

  sbxpkt_print(pkt);
  sbxpkt_free(pkt);

  return;
}

void sbxpkt_test_erh2(void) {
  sbxpkt_t *pkt;
  uint8 erh_data[128] = {0x01,0x23,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,0,1,2,3,4,6,0x81,0x00,0,3,
                         0x88,0x47,0xab,0xcd,0xe1,0x40,1,2,3,4,5,6,7,8,9};

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  sbxpkt_from_byte (ERH_QE, erh_data, 64, pkt);

  sbxpkt_print(pkt);
  sbxpkt_free(pkt);

  return;
}

int sbxpkt_test_hex(void) {
  int i = 0;
  int rc = 0;
  sbxpkt_t *pkt;
  sbxpkt_t *i_pkt;
  char tmp_str[150] = {0};
  char pkt_str[300] = {0};
  uint8 eth_data[128] = {0,1,2,3,4,5,0,1,2,3,4,6,0x81,0x00,0,3,
                         0x88,0x47,0xab,0xcd,0xe1,0x40,0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                         0x00, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x08, 0x11,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                         0x55, 0x55, 0x55, 0x55};


  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  for(i = 0; i < 64; i++) {
    /* coverity[secure_coding] */
    sprintf(tmp_str, "%s%02X", tmp_str, eth_data[i]);
  }
  /* coverity[secure_coding] */
  sprintf(pkt_str, "--hex_data -length 64 -hex %s", tmp_str);
  sbxpkt_create (pkt, pkt_str);


  i_pkt = sbxpkt_alloc();
  sbxpkt_create (i_pkt, "--mac -dmac 00:01:02:03:04:05 -smac 00:01:02:03:04:06 --vlan -pri 0 -vid 3 \
                    --etype -etype 0x8847 --mpls -label 0xabcde -s 1 -ttl 0x40 \
                    --mac -dmac 00:aa:aa:aa:aa:aa -smac 00:bb:bb:bb:bb:bb --etype -etype 0x811 \
                    --raw_data -length 28 -value 0x55555555 -mode 1");
  rc = sbxpkt_compare (i_pkt, pkt);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }

  sbxpkt_free(i_pkt);
  sbxpkt_free(pkt);

  return rc;
}

int sbxpkt_test_ccm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_ccm -lvl 7 -opcode 1 -seq_number 0x101");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);

  return rc;
}

int sbxpkt_test_lbm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_lbm -lvl 7 -opcode 3 -seq_number 0x101");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);

  return rc;
}

int sbxpkt_test_ltm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_ltm -lvl 7 -opcode 5 -seq_number 0x101 -ttl 0xff -target_mac 01:00:5e:00:00:01");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_lmm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_lmm -lvl 7 -opcode 43 -tx_fcf 0x1000 -rx_fcf 0x800");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_1dmm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_1dmm -lvl 7 -opcode 45 -tx_timestamp_sec 1 -tx_timestamp_nano 2");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_2dmm(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x8902");
  sbxpkt_append (pkt1, "--oam_2dmm -lvl 7 -opcode 45 -rx_timestamp_b_sec 7 -rx_timestamp_b_nano 8");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_bfd(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:BB:CC:DD:EE:FF --stag --vlan -pri 5 -vid 18 --etype -etype 0x0800 \
    --ipv4 -proto 0x11 --udp -sport 0x1111 -dport 3784");
  sbxpkt_append (pkt1, "--bfd -flags 0x1111 -state 1 -my_discrim 0x77777 -your_discrim 0x888888");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_ipv6(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x86DD");
  sbxpkt_append (pkt1, "--ipv6 -ver 6 -tos 0x77 -length 10 -flow_label 0x3456 -ttl 0xff \
    -sa 1111:2222:3333:4444:5555:6666:7777:8888 -da 0123:4567:89ab:cdef:0123:4567:89ab:cdef");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_bpdu(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:00 --etype -etype 64 --llc -ssap 0x42 -dsap 0x42");
  sbxpkt_append (pkt1, "--bpdu");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_slow(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--slow");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_pwe_data_ach(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--mpls -label 0x00001 -s 1");
  sbxpkt_append (pkt1, "--pwe_data_ach -flag 1 -frg 3 -seqno 0x123");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_pwe_ctrl_ach(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--mpls -label 0x00001 -s 1");
  sbxpkt_append (pkt1, "--pwe_ctrl_ach -ctrl 3 -ver 1 -rsvd 0xff -channel 0x1234");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_gach(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--mpls -label 0x0000D -s 1");
  sbxpkt_append (pkt1, "--gach -ver 1 -rsvd 0xff -channel 0x1234");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_psc(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--mpls -label 0x0000D -s 1");
  sbxpkt_append (pkt1, "--gach -ver 1 -rsvd 0xff -channel 0x7ff9");
  sbxpkt_append (pkt1, "--tlvhdr -length 0 -rsvd 0xffff");
  sbxpkt_append (pkt1, "--psc -ver 1 -rsvd 0xff -fpath 0x66 -path 0x88");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_oam_mpls(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -dmac 01:80:c2:00:00:02 --etype -etype 0x8809");
  sbxpkt_append (pkt1, "--mpls -label 0x0000E -s 1");
  sbxpkt_append (pkt1, 
    "--oam_mpls -func_type 5 -ttsi 0102030405060708090A0B0C0D0E0F1011121314 -bip16 0xabcd");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_ip_option(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create(pkt1, "--mac -smac 00:11:22:33:44:55 --stag --vlan -pri 5 -vid 18 --etype -etype 0x0800");

  sbxpkt_append(pkt1, "--ipv4 -ihl 10");
  sbxpkt_append(pkt1, "--ip_option -option_code 111 -length 19 -data 0102030405060708090A0B0C0D0E0F101112");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_hop(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x86DD");
  sbxpkt_append (pkt1, "--ipv6 -ver 6 -next_header 0");
  sbxpkt_append (pkt1, "--hop -next_header 0x11 -data 0x1122eeff");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_ptp(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac -smac 00:A9:87:65:43:21 --etype -etype 0x88F7");
  sbxpkt_append (pkt1, "--ptp -message_type 1 -message_length 34 -correction_field1 0x11 -correction_field2 0x11110000");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(MAC, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}


int sbxpkt_test_higig(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--higig -start 0xff -cos 7 -dstport 9 -srcport 6 -vlan 100 -labelpresent 1 -vclabel 0xfffff");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(HIGIG, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_higig2(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--higig2 -start 0xee -dstport 7 -srcport 5 -vlan 100 -hdrextlen 7");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(HIGIG, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_itmh_nph(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--nph -m_u 1 -sub_t 15 -slot_in 64 -tag_num 1 -l2vpn_id 257 ");
  sbxpkt_prepend(pkt1, "--itmh -type 3 -pkt_head_len 4");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(ITMH, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_itmh_nph_nfh(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--nfh -destpoint 1 -packetlen 28 -cookielen 32 -cookie 88888 ");
  sbxpkt_prepend(pkt1, "--nph -m_u 1 -sub_t 15 -slot_in 64 -tag_num 1 -l2vpn_id 257 ");
  sbxpkt_prepend(pkt1, "--itmh -type 3 -pkt_head_len 4");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(ITMH, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_nph(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--nph -m_u 1 -sub_t 15 -slot_in 64 -tag_num 1 -l2vpn_id 257 ");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(NPH, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_nph_nfh(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--nfh -destpoint 1 -packetlen 28 -cookielen 32 -cookie 88888 ");
  sbxpkt_prepend(pkt1, "--nph -m_u 1 -sub_t 15 -slot_in 64 -tag_num 1 -l2vpn_id 257 ");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(NPH, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_fnh_full_fwd(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--fnh_full_fwd -ftype 1 -ptype 2 -slot 99 -cos 6 ");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(FNH_FULL_FWD, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_fnh_half_fwd(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--fnh_half_fwd -ftype 3 -ptype 4 -cos 6 -outfp 65535");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(FNH_HALF_FWD, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_fnh_non_fwd(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--fnh_non_fwd -ftype 5 -ptype 6 -slot 98 -packets 65501");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(FNH_NON_FWD, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}

int sbxpkt_test_fnh_np_term(void) {
  sbxpkt_t *pkt1, *pkt2;
  uint8 pkt_data[2000];
  int rc = 0;

  pkt1 = sbxpkt_alloc();
  pkt2 = sbxpkt_alloc();

  sbxpkt_create (pkt1, "--mac --vlan --etype --raw_data -length 100");
  sbxpkt_prepend(pkt1, "--fnh_np_term -cmheader 666666 -ftype 7");

  sbxpkt_print(pkt1);

  sbxpkt_to_byte (pkt1, pkt_data);
  debug_print(pkt_data, pkt1->entry.length);
  sbxpkt_from_byte(FNH_NP_TERM, pkt_data, pkt1->entry.length, pkt2);

  rc = sbxpkt_compare (pkt1, pkt2);
  if (rc != 0) {
    printf("%s failed.\n", FUNCTION_NAME());
  }
  
  sbxpkt_free(pkt1);
  sbxpkt_free(pkt2);
  return rc;
}


/* Under BCM shell, run "TestRun c3sw test=sbx_pkt_ut id=0" to invoke this test */
int sbxpkt_test_main(int unit, void *testdata) {
  int rc = 0;

  sbxpkt_test_udp();
  sbxpkt_test_itag();
  sbxpkt_test_erh1();
  sbxpkt_test_cmpmpls();
  sbxpkt_test_data();
  sbxpkt_test_erh2();
  rc |= sbxpkt_test_hex();
  rc |= sbxpkt_test_ccm();
  rc |= sbxpkt_test_lbm();
  rc |= sbxpkt_test_ltm();
  rc |= sbxpkt_test_lmm();
  rc |= sbxpkt_test_1dmm();
  rc |= sbxpkt_test_2dmm();
  rc |= sbxpkt_test_ipv6();
  rc |= sbxpkt_test_bfd();
  rc |= sbxpkt_test_bpdu();
  rc |= sbxpkt_test_slow();
  rc |= sbxpkt_test_pwe_data_ach();
  rc |= sbxpkt_test_pwe_ctrl_ach();
  rc |= sbxpkt_test_gach();
  rc |= sbxpkt_test_psc();
  rc |= sbxpkt_test_oam_mpls();
  rc |= sbxpkt_test_ip_option();
  rc |= sbxpkt_test_hop();
  rc |= sbxpkt_test_ptp();
  rc |= sbxpkt_test_higig();
  rc |= sbxpkt_test_higig2();
  rc |= sbxpkt_test_itmh_nph();
  rc |= sbxpkt_test_itmh_nph_nfh();
  rc |= sbxpkt_test_nph();
  rc |= sbxpkt_test_nph_nfh();
  rc |= sbxpkt_test_fnh_full_fwd();
  rc |= sbxpkt_test_fnh_half_fwd();
  rc |= sbxpkt_test_fnh_non_fwd();
  rc |= sbxpkt_test_fnh_np_term();
  
  return rc;
}

#ifdef BCM_CALADAN3_G3P1_SUPPORT

/* Under BCM shell, run "TestRun c3sw test=sbx_pkt_ut id=1 case=0" to invoke this test */
int sbxpkt_test_header(int unit, void *testdata) {
  int rc;
  sbxpkt_t *pkt;
  sbxpkt_t *p_pkt;
  uint8 pkt_data[2000];
  c3sw_test_info_t *info = (c3sw_test_info_t *)testdata;
  uint32 itlk_offset = 0;

  soc_sbx_g3p1_interlaken_offset_get(0, &itlk_offset);

  printf ("SBX Packet test \n");

  if (info->testCase == 0 ) info->testCase = -1;

  /* Bit 0 & 1 : HiGig */
  if (info->testCase & 3) soc_sbx_g3p1_interlaken_offset_set(0, 0);

  if ((info->testCase >> 0) & 1) {
      printf ("\n*************** test 1 ARAD Ingress HiGig *******************\n");

      pkt = sbxpkt_alloc();

      sbxpkt_create (pkt, "--erh_iarad -ksop 0xfb -out_union 0x10 -dest_data 0xc2020 -fcos 0x7 -sid 0x40 -sys_src_port 0x50 --mac --etype");

      sbxpkt_print(pkt);

      sbxpkt_to_byte (pkt, pkt_data);
      debug_print(pkt_data, pkt->entry.length);

      p_pkt = sbxpkt_alloc();
      p_pkt->entry.type = PACKET;
      sbxpkt_from_byte (ERH_IARAD, pkt_data, 64, p_pkt);

      sbxpkt_print(p_pkt);

      rc = sbxpkt_compare (pkt, p_pkt);
      if (rc != 0) {
        printf("%s failed.\n", __FUNCTION__);
      }   
      sbxpkt_free(p_pkt);
      sbxpkt_free(pkt);
  }

  if ((info->testCase >> 1) & 1) {
      printf ("\n*************** test 2 ARAD Egress HiGig *******************\n");

      pkt = sbxpkt_alloc();

      sbxpkt_create (pkt, "--erh_earad -ksop 0xfb -out_union 0x10 -cud 0x20 -fcos 0x7 -sid 0x40 -sys_src_port 0x50 -dest_port 0x60 --mac --etype");

      sbxpkt_print(pkt);

      sbxpkt_to_byte (pkt, pkt_data);
      debug_print(pkt_data, pkt->entry.length);

      p_pkt = sbxpkt_alloc();
      p_pkt->entry.type = PACKET;
      sbxpkt_from_byte (ERH_EARAD, pkt_data, 64, p_pkt);

      sbxpkt_print(p_pkt);

      rc = sbxpkt_compare (pkt, p_pkt);
      if (rc != 0) {
        printf("%s failed.\n", __FUNCTION__);
      }
      sbxpkt_free(pkt);
      sbxpkt_free(p_pkt);
  }

  /* Bit 3 & 4 : Interlink */
  if ((info->testCase >> 2) & 3) soc_sbx_g3p1_interlaken_offset_set(0, 1);

  if ((info->testCase >> 2) & 1) {
      printf ("\n*************** test 3 ARAD Ingress Interlink *******************\n");

      pkt = sbxpkt_alloc();

      sbxpkt_create (pkt, "--erh_iarad -pad 0xfb -out_union 0x10 -dest_data 0x20 -fcos 0x7 -sid 0x40 --mac --etype");

      sbxpkt_print(pkt);

      sbxpkt_to_byte (pkt, pkt_data);
      debug_print(pkt_data, pkt->entry.length);

      p_pkt = sbxpkt_alloc();
      p_pkt->entry.type = PACKET;
      sbxpkt_from_byte (ERH_IARAD, pkt_data, 64, p_pkt);

      sbxpkt_print(p_pkt);

      rc = sbxpkt_compare (pkt, p_pkt);
      if (rc != 0) {
        printf("%s failed.\n", __FUNCTION__);
      }  
      sbxpkt_free(p_pkt);
      sbxpkt_free(pkt);
  }

  if ((info->testCase >> 3) & 1) {
      printf ("\n*************** test 4 ARAD Egress Interlink *******************\n");

      pkt = sbxpkt_alloc();

      sbxpkt_create (pkt, "--erh_earad -pad 0xfb -out_union 0x10 -cud 0x20 -fcos 0x7 -sid 0x40 -sys_src_port 0x50 -dest_port 0x60 --mac --etype");

      sbxpkt_print(pkt);

      sbxpkt_to_byte (pkt, pkt_data);
      debug_print(pkt_data, pkt->entry.length);

      p_pkt = sbxpkt_alloc();
      p_pkt->entry.type = PACKET;
      sbxpkt_from_byte (ERH_EARAD, pkt_data, 64, p_pkt);

      sbxpkt_print(p_pkt);

      rc = sbxpkt_compare (pkt, p_pkt);
      if (rc != 0) {
        printf("%s failed.\n", __FUNCTION__);
      }  
      sbxpkt_free(pkt);
      sbxpkt_free(p_pkt);
  }

  /* Clean up */
  soc_sbx_g3p1_interlaken_offset_set(0, itlk_offset);

  return 0;
}

#endif /*BCM_CALADAN3_G3P1_SUPPORT*/


/*********************/
/*     Test Runner   */
/*********************/

/* test database */
test_call_back_t sbx_pkt_test_cback_array[] = {
         {0, NULL, sbxpkt_test_main, NULL}
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        ,{1, NULL, sbxpkt_test_header, NULL}
#endif /*BCM_CALADAN3_G3P1_SUPPORT*/
};

int c3_ut_pkt_test_run(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(sbx_pkt_test_cback_array)) return SOC_E_PARAM;

    if (sbx_pkt_test_cback_array[testinfo->testid].run)
      rv = sbx_pkt_test_cback_array[testinfo->testid].run(testinfo->unit, (void *)testinfo);

    return rv;
}

int c3_ut_pkt_test_init(c3sw_test_info_t *testinfo, void *userdata)
{
  int rv=SOC_E_NONE;

  if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(sbx_pkt_test_cback_array)) return SOC_E_PARAM;

  if (sbx_pkt_test_cback_array[testinfo->testid].init)
    rv = sbx_pkt_test_cback_array[testinfo->testid].init(testinfo->unit, (void*)testinfo);

  return rv;
}

int c3_ut_pkt_test_done(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(sbx_pkt_test_cback_array)) return SOC_E_PARAM;

    if (sbx_pkt_test_cback_array[testinfo->testid].clean)
        rv = sbx_pkt_test_cback_array[testinfo->testid].clean(testinfo->unit, userdata);

    return rv;
}

#else
int _src_appl_test_sbx_pkt_c_not_empty;
#endif /* BCM_SBX_SUPPORT */

