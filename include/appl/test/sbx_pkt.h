/*
 * $Id: sbx_pkt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == sbx_pkt.h - SBX Packet Class Functions ==
 */

#ifndef _SBXPKT_H_
#define _SBXPKT_H_

#define MAXARGS 100
#define MAXLEN 2000
#define MAXUSRSTR 4096
#define MAXFIELD 256
#define TTSI_LENGTH 20
#define IP_OPTION_LENGTH 40


#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <soc/mem.h>

#define printf cli_out

typedef enum header_type_s {
  PACKET,
  ERH_QE,
  ERH_SS,
  ERH_QESS,
  ERH_ISIR,
  ERH_ESIR,
  ERH_IARAD,
  ERH_EARAD,
  MAC,
  VLAN,
  STAG,
  ETYPE,
  LLC,
  SNAP,
  IPV4,
  IPV6,
  ITAG,
  MPLS,
  UDP,
  TCP,
  IGMP,
  OAM_CCM,
  OAM_LBM,
  OAM_LTM,
  OAM_LMM,
  OAM_1DMM,
  OAM_2DMM,
  BFD,
  BPDU,
  SLOW,
  PWE_DATA_ACH,
  PWE_CTRL_ACH,
  GACH,
  TLVHDR,
  PSC,
  OAM_MPLS,
  IP_OPTION,
  HOP,
  HIGIG,
  HIGIG2,
  PTP,
  ITMH,
  NPH,
  NFH,
  FNH_FULL_FWD,
  FNH_HALF_FWD,
  FNH_NON_FWD,
  FNH_NP_TERM,
  RAW_DATA,
  HEX_DATA,
  UNKNOWN
} header_type_e;

typedef enum op_s {
  CREATE,
  MODIFY,
  PREPEND,
  APPEND
} op_e;

typedef enum fill_mode_s {
  INCREMENT,
  FIXED,
  RANDOM
} fill_mode_e;

typedef enum oam_opcode_s {
  OPCODE_CCM = 1,
  OPCODE_LBR = 2,
  OPCODE_LBM = 3,
  OPCODE_LTR = 4,
  OPCODE_LTM = 5,
  OPCODE_AIS = 33,
  OPCODE_TST = 37,
  OPCODE_LMR = 42,
  OPCODE_LMM = 43,
  OPCODE_1DM = 45,
  OPCODE_DMR = 46,
  OPCODE_DMM = 47
} oam_opcode_e;

typedef enum ptp_type_s {
  PTP_SYNC                  = 0,
  PTP_DELAY_REQ             = 1,
  PTP_PDELAY_REQ            = 2,
  PTP_PDELAY_RESP           = 3,
  PTP_FOLLOW_UP             = 8,
  PTP_DELAY_RESP            = 9,
  PTP_PDELAY_RESP_FOLLOW_UP = 10,
  PTP_ANNOUNCE              = 11,
  PTP_SIGNALING             = 12,
  PTP_MANAGEMENT            = 13
} ptp_type_e;


/*
 * Public: Entry description
 * entry_desc_t
 */
typedef struct entry_desc_s {
    int    type;
    void   *next;
    uint32 length;
} entry_desc_t;

/* 
 * Public: Packet Header
 * sbxpkt_t
 */
typedef struct sbxpkt_s {
  entry_desc_t entry;
  uint32   normalize;
} sbxpkt_t;

/* 
 * Public Functions 
 *
 */ 
int sbxpkt_cmic_port(int unit);
int sbxpkt_create (sbxpkt_t *packet, char *char_data);
int sbxpkt_prepend (sbxpkt_t *packet, char *char_data);
int sbxpkt_append (sbxpkt_t *packet, char *char_data);
sbxpkt_t* sbxpkt_alloc (void);
int sbxpkt_free (sbxpkt_t *packet);
int sbxpkt_print (sbxpkt_t *packet);
int sbxpkt_compare (sbxpkt_t *sbxtx_pkt, sbxpkt_t *rx_pkt);
int sbxpkt_compare_data(sbxpkt_t *tx_pkt, uint8 *rx_pkt_data, int rx_pkt_len);
int sbxpkt_compare_ext (sbxpkt_t *sbxtx_pkt, sbxpkt_t *rx_pkt, int length);
int sbxpkt_get_type (sbxpkt_t *packet);
int sbxpkt_check_ipv4 (sbxpkt_t *packet);
int sbxpkt_to_byte (sbxpkt_t *packet, unsigned char *pkt_data);
int sbxpkt_from_byte (header_type_e start_type, unsigned char *pkt_data, int length, sbxpkt_t *return_pkt);
int debug_print (unsigned char * pkt_data, int length);

/*
 ***raw_data
 */
typedef struct raw_data_s {
  entry_desc_t entry;
  uint32   value;
  uint32   mode;
  uint32   flags;
  unsigned char raw_data[MAXLEN];
} raw_data_t;

/*
 ***hex_data
 */
typedef struct hex_data_s {
  entry_desc_t entry;
  unsigned char hex_data[MAXLEN];
} hex_data_t;

/* 
 ***mac
 */
typedef struct mac_s {
  entry_desc_t entry;
  unsigned char dmac[6];
  unsigned char smac[6];
} mac_t;

/* 
 ***vlan
 */
typedef struct vlan_s {
  entry_desc_t entry;
  uint32 tpid;
  uint32 vid;
  uint32 cfi;
  uint32 pri;
} vlan_t;

/* 
 ***stag
 */
typedef struct stag_s { 
  entry_desc_t entry;
  uint32 tpid;
  uint32 vid;
  uint32 dei;
  uint32 pcp;
} stag_t;

/* 
 ***etype
 */
typedef struct etype_s { 
  entry_desc_t entry;
  uint32 etype;
} etype_t;

/* 
 ***llc 
 */
typedef struct llc_s {
  entry_desc_t entry;
  uint32 len;
  uint32 ssap;
  uint32 dsap;
  uint32 ctrl;
} llc_t;

/* 
 ***snap
 */
typedef struct snap_s {
  entry_desc_t entry;
  uint32 oui;
} snap_t;

/* 
 ***ipv4
 */
typedef struct ipv4_s { 
entry_desc_t entry;
uint32 ver;
uint32 ihl;
uint32 tos;
uint32 length;
uint32 id;
uint32 df;
uint32 mf;
uint32 offset;
uint32 ip_opt;
uint32 proto;
uint32 ttl;
uint32 checksum;
uint32 sa;
uint32 da;
} ipv4_t;

/* 
 ***ipv6
 */
typedef struct ipv6_s { 
entry_desc_t entry;
uint32 ver;
uint32 tos;
uint32 length;
uint32 flow_label;
uint32 ttl;
uint32 next_header;
uint16 sa[8];
uint16 da[8];
} ipv6_t;

/* 
 ***bfd
 */
typedef struct bfd_s { 
entry_desc_t entry;
uint32 version;
uint32 diag;
uint32 flags;
uint32 state;
uint32 detect_mult;
uint32 length;
uint32 my_discrim;
uint32 your_discrim;
uint32 min_tx;
uint32 min_rx;
uint32 echo;
} bfd_t;

/* 
 ***itag 
 */
typedef struct itag_s {
 entry_desc_t entry;
 uint32 ipcp;
 uint32 idei;
 uint32 nca;
 uint32 isid;
} itag_t;

/* 
 ***mpls 
 */
typedef struct mpls_s{
 entry_desc_t entry;
 uint32 label;
 uint32 exp;
 uint32 s;
 uint32 ttl;
} mpls_t;

/* 
 *** udp
 */
typedef struct udp_s {
 entry_desc_t entry;
 uint32 sport; 
 uint32 dport; 
 uint32 len; 
 uint32 checksum; 
} udp_t;

/* 
 *** tcp 
 */
typedef struct tcp_s {
 entry_desc_t entry;
 uint32 sport;
 uint32 dport;
 uint32 seqn;
 uint32 ackn;
 uint32 dofs;
 uint32 ecn;
 uint32 ctrl;
 uint32 wind;
 uint32 checksum;
 uint32 urgp;
 uint32 option;
} tcp_t;

/* 
 *** igmp
 */
typedef struct igmp_s { 
 entry_desc_t entry;
 uint32 ver; 
 uint32 type; 
 uint32 checksum; 
 uint32 group;
} igmp_t;

/* 
 *** erh_qe
 */
typedef struct erh_qe_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 sid;
 uint32 mc;
 uint32 out_union;
 uint32 len_adj;
 uint32 frm_len;
 uint32 test;
 uint32 e;
 uint32 fdp;
 uint32 qid;
} erh_qe_t;


/* 
 *** erh_isir
 */
typedef struct erh_isir_s {
 entry_desc_t entry;
 uint32 ksop;
 uint32 et;
 uint32 en;
 uint32 t;
 uint32 mc;
 uint32 lenadj;
 uint32 res;
 uint32 qid;
 uint32 out_union;
 uint32 sid;
 uint32 fdp;
 uint32 fcos;
 uint32 hdrcompr;
 uint32 oam;
 uint32 rcos;
 uint32 rdp;
 uint32 s;
 uint32 ttl_excidx;
} erh_isir_t;

/* 
 *** erh_esir
 */
typedef struct erh_esir_s {
 entry_desc_t entry;
 uint32 ksop;
 uint32 et;
 uint32 en;
 uint32 t;
 uint32 mc;
 uint32 lenadj;
 uint32 out_union;
 uint32 dest_port;
 uint32 sid;
 uint32 fdp;
 uint32 fcos;
 uint32 hdrcompr;
 uint32 oam;
 uint32 rcos;
 uint32 rdp;
 uint32 s;
 uint32 ttl_excidx;
} erh_esir_t;

/* 
 *** erh_iarad
 */
typedef struct erh_iarad_s {
 entry_desc_t entry;
 /* HiGig */
 uint32 ksop;
 /* PTCH-1 */
 uint32 format;
 uint32 attr;
 uint32 reserved;
 uint32 sys_src_port;
 /* ITMH */
 uint32 type;
 uint32 m;
 uint32 snoop_cmd;
 uint32 cos;
 uint32 dp;
 uint32 dest_data;
 /* Interlaken */
 uint32 pad;
 /* PPH */
 uint32 out_union;
 uint32 sid;
 uint32 fdp;
 uint32 fcos;
 uint32 hdrcompr;
 uint32 oam;
 uint32 rcos;
 uint32 rdp;
 uint32 s;
 uint32 ttl_excidx;
} erh_iarad_t;

/*
 *** erh_earad
 */
typedef struct erh_earad_s {
 entry_desc_t entry;
 /* HiGig */
 uint32 ksop;
 /* OTMH */
 uint32 act;
 uint32 dp;
 uint32 m;
 uint32 class;
 uint32 dest_port;
 uint32 sys_src_port;
 uint32 cud_reserved;
 uint32 cud;
 /* PAD(Interlaken) */
 uint32 pad;
 /* PPH */
 uint32 out_union;
 uint32 sid;
 uint32 fdp;
 uint32 fcos;
 uint32 hdrcompr;
 uint32 oam;
 uint32 rcos;
 uint32 rdp;
 uint32 s;
 uint32 ttl_excidx;
} erh_earad_t;

/*
 *** erh_ss
 */
typedef struct erh_ss_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 lbid;
 uint32 fcos2;
 uint32 fdp;
 uint32 sid;
 uint32 out_union;
 uint32 qid;
 uint32 len_adj;
 uint32 mc;
 uint32 test;
 uint32 ecn;
 uint32 ect;
 uint32 ksop;
} erh_ss_t;


/* 
 *** erh_qess
 */
typedef struct erh_qess_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 lbid;
 uint32 fcos2;
 uint32 sid;
 uint32 out_union;
 uint32 zero;
 uint32 ect;
 uint32 mc;
 uint32 len_adj;
 uint32 frm_len;
 uint32 test;
 uint32 ecn;
 uint32 fdp;
 uint32 qid;
} erh_qess_t;

/* 
 *** higig
 */
typedef struct higig_s {
  entry_desc_t entry;
  uint32 start;
  uint32 hgi;
  uint32 opcode;
  uint32 cos;
  uint32 cng;
  uint32 ingresstagged;
  uint32 l3;
  uint32 dstport;
  uint32 dstmod;
  uint32 srcport;
  uint32 srcmod;
  uint32 vlan;
  uint32 cfi;
  uint32 pri;
  uint32 hdrformat;
  uint32 hdrextlen;
  uint32 pfm;
  uint32 mirror;
  uint32 mirrordone;
  uint32 mirroronly;
  uint32 dsttgid;
  uint32 dstt;
  uint32 labelpresent;
  uint32 vclabel;
} higig_t;

/* 
 *** higig2
 */
typedef struct higig2_s {
  entry_desc_t entry;
  uint32 start;
  uint32 mcst;
  uint32 tc;
  uint32 dstmod;
  uint32 dstport;
  uint32 srcmod;
  uint32 srcport;
  uint32 lbid;
  uint32 dp;
  uint32 hdrextvalid;
  uint32 ppdtype;
  uint32 dstt;
  uint32 dsttgid;
  uint32 ingresstagged;
  uint32 mirroronly;
  uint32 mirrordone;
  uint32 mirror;
  uint32 lblovltype;
  uint32 l3;
  uint32 labelpresent;
  uint32 vclabel;
  uint32 vlan;
  uint32 pfm;
  uint32 srct;
  uint32 preservedscp;
  uint32 preservedot1p;
  uint32 opcode;
  uint32 hdrextlen;
} higig2_t;

/* 
 *** oam_ccm
 */
typedef struct oam_ccm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags_rdi;
  uint32 flags_rsvd;
  uint32 flags_period;
  uint32 tlv_offset;
  uint32 seq_number;
  uint32 mep_id;
  unsigned char maid[48];
  uint32 tx_fcf;
  uint32 rx_fcb;
  uint32 tx_fcb;
  uint32 reserved;
  uint32 end_tlv;
} oam_ccm_t;

/* 
 *** oam_lbm
 */
typedef struct oam_lbm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags;
  uint32 tlv_offset;
  uint32 seq_number;
  uint32 end_tlv;
} oam_lbm_t;

/* 
 *** oam_ltm
 */
typedef struct oam_ltm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags;
  uint32 tlv_offset;
  uint32 seq_number;
  uint32 ttl;
  uint8  origin_mac[6];
  uint8  target_mac[6];
  uint32 relay_action;
  uint32 end_tlv;
} oam_ltm_t;

/* 
 *** oam_lmm
 */
typedef struct oam_lmm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags;
  uint32 tlv_offset;
  uint32 tx_fcf;
  uint32 rx_fcf;
  uint32 tx_fcb;
  uint32 end_tlv;
} oam_lmm_t;

/* 
 *** oam_1dmm
 */
typedef struct oam_1dmm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags;
  uint32 tlv_offset;
  uint32 tx_timestamp_sec;
  uint32 tx_timestamp_nano;
  uint32 rx_timestamp_sec;
  uint32 rx_timestamp_nano;
  uint32 end_tlv;
} oam_1dmm_t;

/* 
 *** oam_2dmm
 */
typedef struct oam_2dmm_s { 
entry_desc_t entry;
  uint32 lvl;
  uint32 version;
  uint32 opcode;
  uint32 flags;
  uint32 tlv_offset;
  uint32 tx_timestamp_f_sec;
  uint32 tx_timestamp_f_nano;
  uint32 rx_timestamp_f_sec;
  uint32 rx_timestamp_f_nano;
  uint32 tx_timestamp_b_sec;
  uint32 tx_timestamp_b_nano;
  uint32 rx_timestamp_b_sec;
  uint32 rx_timestamp_b_nano;
  uint32 end_tlv;
} oam_2dmm_t;

/* 
 *** bpdu
 */
typedef struct bpdu_s { 
entry_desc_t entry;
  uint32 protocol_id;
  uint32 version;
  uint32 message_type;
  uint32 flags;
  uint32 root_pri;
  uint8  root_mac[6];
  uint32 root_path_cost;
  uint32 bridge_pri;
  uint8  bridge_mac[6];
  uint32 port_id;
  uint32 message_age;
  uint32 max_age;
  uint32 hello_time;
  uint32 forward_delay;
} bpdu_t;

/* 
 *** slow
 */
typedef struct slow_s { 
entry_desc_t entry;
  uint32 sub_type;
} slow_t;

/* 
 *** pwe_data_ach
 */
typedef struct pwe_data_ach_s { 
entry_desc_t entry;
  uint32 flag;
  uint32 frg;
  uint32 length;
  uint32 seqno;
} pwe_data_ach_t;

/* 
 *** pwe_ctrl_ach
 */
typedef struct pwe_ctrl_ach_s { 
entry_desc_t entry;
  uint32 ctrl;
  uint32 ver;
  uint32 rsvd;
  uint32 channel;
} pwe_ctrl_ach_t;

/* 
 *** gach
 */
typedef struct gach_s { 
entry_desc_t entry;
  uint32 ver;
  uint32 rsvd;
  uint32 channel;
} gach_t;

/* 
 *** tlvhdr
 */
typedef struct tlvhdr_s { 
entry_desc_t entry;
  uint32 length;
  uint32 rsvd;
} tlvhdr_t;

/* 
 *** psc
 */
typedef struct psc_s { 
entry_desc_t entry;
  uint32 ver;
  uint32 request;
  uint32 pt;
  uint32 r;
  uint32 rsvd;
  uint32 fpath;
  uint32 path;
} psc_t;

/* 
 *** oam_mpls
 */
typedef struct oam_mpls_s { 
entry_desc_t entry;
  uint32 func_type;
  uint8  ttsi[TTSI_LENGTH];
  uint32 frequency;
  uint32 bip16;
} oam_mpls_t;

/* 
 *** ip_option
 */
typedef struct ip_option_s { 
entry_desc_t entry;
  uint32 option_code;
  uint32 length;
  uint8  data[IP_OPTION_LENGTH];
} ip_option_t;

/* 
 *** hop
 */
typedef struct hop_s { 
entry_desc_t entry;
  uint32 next_header;
  uint32 length;
  uint32 opt_type;
  uint32 opt_len;
  uint32 data;
} hop_t;

/* 
 *** ptp
 */
typedef struct ptp_s { 
entry_desc_t entry;
  uint32 message_type;
  uint32 version_ptp;
  uint32 message_length;
  uint32 domain_number;
  uint32 flag_field;
  uint32 correction_field1;
  uint32 correction_field2;
  uint32 sourceport_identity1;
  uint32 sourceport_identity2;
  uint32 sourceport_identity3;
  uint32 sequence_id;
  uint32 control_field;
  uint32 logmessage_interval;
} ptp_t;

/* 
 *** itmh
 */
#define SBX_PKT_ITMH_TYPE_MC    2
#define SBX_PKT_ITMH_TYPE_UC    3
typedef struct itmh_s 
{ 
    entry_desc_t entry;

    uint32  type;
    uint32  p_t;
    uint32  pkt_head_len;
    uint32  dp;

    /* for uc itmh */
    uint32  uc_flow_id;

    /* for mc itmh */
    uint32  mc_t_c;
    uint32  multicast_id;

} itmh_t;



typedef struct nph_s {
    entry_desc_t entry;

    /* word 0 */
    uint32      m_u_flag;
    uint32      main_type;
    uint32      sub_type;
    uint32      ptp_1588flag;
    uint32      cos_dp;
    uint32      slot_in;
    uint32      trunk_flag;
    uint32      port_or_trunk_id;

    /* word 1 */
    uint32      vlan_status;
    uint32      del_length;
    uint32      default_pri_cfi;
    uint32      protect_status_section;
    uint32      protect_status_lsp;
    uint32      protect_status_pw_uni;
    uint32      output_fp;

    /* word 2 */
    uint32      root_leaf;
    uint32      hub_spoke;
    uint32      tag_num;
    uint32      b_f;
    uint32      port_out;
    uint32      lrn_on;
    uint32      mstp_Lrn;
    uint32      msp_opo;
    uint32      vpn_id;

    uint32      nph_word3;

}nph_t;

/* 
 *** nfh
 */
typedef struct nfh_s { 
entry_desc_t entry;
  uint32 destpoint;
  uint32 linktype;
  uint32 packettype_to;
  uint32 packetlen;
  uint32 packettype;
  uint32 vlan_status;
  uint32 port;
  uint32 fp;
  uint32 desttype;
  uint32 vpnid;
  uint32 flowtype;
  uint32 greencap;
  uint32 slot;
  uint32 to_fpga;
  uint32 cookielen;
  uint32 cookie;
} nfh_t;

/* 
 *** fnh_full_fwd
 */
typedef struct fnh_full_fwd_s { 
entry_desc_t entry;
  uint32 cmheader;
  uint32 rsv0;
  uint32 ftype;
  uint32 rsv1;
  uint32 ptype;
  uint32 rsv2;
  uint32 port;
  uint32 rsv3;
  uint32 slot;
  uint32 rsv4;
  uint32 cos;
} fnh_full_fwd_t;

/* 
 *** fnh_half_fwd
 */
typedef struct fnh_half_fwd_s { 
entry_desc_t entry;
  uint32 cmheader;
  uint32 rsv0;
  uint32 ftype;
  uint32 rsv1;
  uint32 ptype;
  uint32 rsv2;
  uint32 protect_status;
  uint32 dp;
  uint32 rsv3;
  uint32 cos;
  uint32 oampduoffset;
  uint32 rsv4;
  uint32 outfp;
} fnh_half_fwd_t;

/* 
 *** fnh_non_fwd
 */
typedef struct fnh_non_fwd_s 
{ 
    entry_desc_t entry;
    uint32  cmheader;
    uint32  ftype;
    uint32  ptype;
    uint32  slot;
    uint32  packets;
} fnh_non_fwd_t;

/* 
 *** fnh_np_term
 */
typedef struct fnh_np_term_s { 
entry_desc_t entry;
  uint32 cmheader;
  uint32 rsv0;
  uint32 ftype;
  uint32 reserved;
} fnh_np_term_t;





#endif /* _SBXPKT_H_ */

