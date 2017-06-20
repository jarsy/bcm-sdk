/*
 * $Id: txrx.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        txrx.c
 * Purpose:     First cut at some sbx commands
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <appl/diag/system.h>
#include <soc/defs.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/hal_user.h>

#include <bcm/error.h>
#include <bcm/tx.h>
#include <bcm/rx.h>
#include <bcm/pkt.h>


#define RT_HDR_SZ 8
#define SHIM_HDR_SZ 4
#define HIGIG_HDR_SZ 12
#define TXRX_BUF_SIZE 16384

#define SA_ADDR_SZ   6
#define DA_ADDR_SZ   6
#define ETHERTYPE_SZ 2

enum {
    sbx_hdr_Rh,
    sbx_hdr_Shim,
    sbx_hdr_Higig,
    sbx_hdr_num_types
} sbx_headers_e;


static char **cmd_sbx_txrx_bufs = NULL;

static int
cmd_sbx_txrx_init_buf (int unit)
{
    if (!cmd_sbx_txrx_bufs) {
        cmd_sbx_txrx_bufs
            = sal_alloc(sizeof(char *) * BCM_MAX_NUM_UNITS,
                        "SBX diag txrx bufs");
        if (!cmd_sbx_txrx_bufs) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "cmd_sbx_txrx_init_buf: buffer pointer alloc"
                                  " failed.\n")));
            return CMD_FAIL;
        }
        sal_memset(cmd_sbx_txrx_bufs, 0, sizeof(char *) * BCM_MAX_NUM_UNITS);
    }

    if (!cmd_sbx_txrx_bufs[unit]) {
        cmd_sbx_txrx_bufs[unit]
            = soc_cm_salloc(unit, TXRX_BUF_SIZE, "SBX diag packet buffer");
        if (!cmd_sbx_txrx_bufs[unit]) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "cmd_sbx_txrx_init_buf: DMAable buffer alloc"
                                  " failed. \n")));
            return CMD_FAIL;
        }
        sal_memset(cmd_sbx_txrx_bufs[unit], 0, sizeof(char) * TXRX_BUF_SIZE);
    }

   return CMD_OK;
}


void
sbx_dump_pkt(unsigned char* pBuf, int nBufLen,
             int hdrExists[sbx_hdr_num_types],
             int hdrShow[sbx_hdr_num_types],
             int nShowLen)
{
    unsigned char* pCurLoc = pBuf;
    int hdrSize[sbx_hdr_num_types];
    char *hdrName[sbx_hdr_num_types];
    int i, nHdr;

    hdrSize[sbx_hdr_Rh] = 8;
    hdrSize[sbx_hdr_Shim] = 4;
    hdrSize[sbx_hdr_Higig] = 12;

    hdrName[sbx_hdr_Rh] = "RH";
    hdrName[sbx_hdr_Shim] = "SHIM";
    hdrName[sbx_hdr_Higig] = "HIGIG";

    cli_out("Packet Len=%d -------------------------------------------------\n",
            nBufLen);
    if (hdrExists[0] || hdrExists[1]) {
        cli_out("RAW:");
        for (i=0; i<nBufLen; i++) {
            if (i % 32 == 0) {
                cli_out("\n0x");
            }
            cli_out("%02x", pBuf[i]);
            if ((i&3)==3) {
            cli_out(" ");
            }
        }
        cli_out("\n");
    }

    for (nHdr=0; nHdr < sbx_hdr_num_types; nHdr++) {
        if (hdrExists[nHdr]) {
            if(hdrShow[nHdr]) {
                cli_out("%-5s: 0x", hdrName[nHdr]);
                for (i=0; i<hdrSize[nHdr]; i++) {
                    cli_out("%02x", *pCurLoc++);
                    if ((i&3) == 3) {
                        cli_out(" ");
                    }
                }
                cli_out("\n");
            }
            pCurLoc += hdrSize[nHdr];
            nBufLen -= hdrSize[nHdr];
        }
    }

    if (nShowLen < 0 || nShowLen > nBufLen) {
        nShowLen = nBufLen;
    }

    cli_out("Payload:");
    for (i=0; i<nShowLen; i++) {
        if (i % 32 == 0) {
          cli_out("\n0x");
        }
        cli_out("%02x", *pCurLoc++);
        if ((i & 3) == 3) {
            cli_out(" ");
        }
    }
    cli_out("\n");

}


bcm_rx_t test_rx_cb(int unit, bcm_pkt_t *pkt, void *cookie)
{

    int q_depth;

    bcm_rx_queue_packet_count_get(unit, 0, &q_depth);

    cli_out("DiagRxCallBack: pkt len=%d rxUnit=%d rxPort=%d rxReason=%d qlen=%d\n",
            pkt->pkt_len, pkt->rx_unit, pkt->rx_port, 
            pkt->rx_reason, q_depth);

#if 0
    /* cli_out("Freeing buffer Enqueue\n");*/
    bcm_rx_free_enqueue(unit, pkt->pkt_data->data);
    pkt->pkt_data->data = NULL;
#else
    /*cli_out("Freeing buffer directly\n");*/
    bcm_rx_free(unit, pkt->pkt_data->data);
    pkt->pkt_data->data = NULL;
#endif

    return BCM_RX_HANDLED;
}


static int
sbx_tx_value_parse(char *data, char *s, int *len)
{
    int max_len = *len;
    int str_len;
    int i;
    int octet;

    if ((!s) || ((str_len = (strlen(s))) < 0)) {
        return BCM_E_PARAM;
    }
    
    if ((*s != '0') || ((*(s+1) != 'x') && (*(s + 1) != 'X'))) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("1: Value not specified in hex format: %c%c\n"), *s, *(s+1)));
        return BCM_E_PARAM;
    }


    s+=2; /* ignore 0x */

    for (i = 0; ((i < (max_len * 2)) && (i < (str_len -2))); i++) {
        octet = ((*(s + i) >= 'a') ? (*(s + i) - 'a' + 10) :
	           (*(s + i) >= 'A') ? (*(s + i) - 'A' + 10) :
	             (*(s + i) - '0'));

        if ((octet < 0) || (octet > 0xf)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("2: Value not specified in hex format \n")));
            return BCM_E_PARAM;
        }

        if (i%2) {
            data[i/2] = (data[i/2] & 0xf0) | octet;
        } else {
            data[i/2] = (data[i/2] & 0x0f) | (octet << 4);
        }
    }

    *len = i/2;

    return BCM_E_NONE;
}

typedef enum {
    NO_FILL,
    NO_FILL_RAW,
    WORD_INCR_FILL,
    CONST_FILL,
    RAND_FILL
} pkt_fill_mode_t;


static void
sbx_debug_dump_buffer(unsigned char* pBuf, int nBufLen)
{
    unsigned char* pCurLoc = pBuf;
    int i;

    cli_out("Packet Len=%d -------------------------------------------------\n", nBufLen);
    cli_out("Payload:");
    for (i=0; i<nBufLen; i++) {
        if (i % 32 == 0) {
          cli_out("\n0x");
        }
        cli_out("%02x", *pCurLoc++);
        if ((i & 3) == 3) {
            cli_out(" ");
        }
    }
    cli_out("\n");
}

static int
sbx_tx_l2_pkt_init(int nPktLen, char *da_addr, char *sa_addr, int crc_append, 
                   char *pBuf, char *payload, pkt_fill_mode_t fill_mode)
{
    int rem, cur, len;
    char *da_default = "0x000506000009";
    char *sa_default = "0x000506000000";
    char *ethertype_default = "0x0801";
    char *ethertype = ethertype_default;
    char *hdr = NULL;
    /*
     * Note that the ethertype is set to 0x0801 because we need an innnocuous value for the
     * L2 packet.  Using ethertype of 0x0800 was causing the IP checksum value to be reset
     * to 0x0000 before transmission.  This posed a problem because the CRC had already
     * been calculated due to crc_append==1.
     */

    if (!da_addr) {
        da_addr = da_default;
    }
    if (!sa_addr) {
        sa_addr = sa_default;
    }
    
    cur = 0;
    rem = nPktLen;
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: BEGIN cur=0, rem=%d\n"),
                 FUNCTION_NAME(), rem));
    
    /* The Math of this is as follows...  
     * If DA is provided then it is 6 bytes long.
     * If SA is provided then it is 6 bytes long.
     */
    /* len = rem because we expect payload to fillup the buf until the desired
     * nPktLen.  It is possible that user specified a pattern or noincr, in which
     * case sbx_tx_value_parse() will return the parsed length in the len var,
     * which will be less than or equal to nPktLen.
     */
    hdr = &pBuf[cur];
    len = DA_ADDR_SZ;
    if (sbx_tx_value_parse(hdr, da_addr, &len)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Invalid specification of Destination Address.\n")));
        return BCM_E_PARAM;
    }
    cur += DA_ADDR_SZ;
    rem -= DA_ADDR_SZ;
    if (cur > nPktLen) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Specified length(%d) too small\n"), nPktLen));
        return BCM_E_PARAM;
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: DA added, cur=%d, rem=%d, len=%d\n"),
                 FUNCTION_NAME(), cur, rem, len));

    hdr = &pBuf[cur];
    len = SA_ADDR_SZ;
    if (sbx_tx_value_parse(hdr, sa_addr, &len)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Invalid specification of Source Address.\n")));
        return BCM_E_PARAM;
    }
    cur += SA_ADDR_SZ;
    rem -= SA_ADDR_SZ;
    if (cur > nPktLen) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Specified length(%d) too small\n"), nPktLen));
        return BCM_E_PARAM;
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: SA added, cur=%d, rem=%d, len=%d\n"),
                 FUNCTION_NAME(), cur, rem, len));

    hdr = &pBuf[cur];
    len = ETHERTYPE_SZ;
    if (sbx_tx_value_parse(hdr, ethertype, &len)) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Invalid specification of Ethertype.\n")));
        return BCM_E_PARAM;
    }
    cur += ETHERTYPE_SZ;
    rem -= ETHERTYPE_SZ;
    if (cur > nPktLen) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Specified length(%d) too small\n"), nPktLen));
        return BCM_E_PARAM;
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: Etype added, cur=%d, rem=%d, len=%d\n"),
                 FUNCTION_NAME(), cur, rem, len));

    /* At this point, we have the l2 Header.  Now we want to fill the rest of
     * the L2 packet body with data.
     */
    hdr = &pBuf[cur];
    if (crc_append) {
        /* User has asked to append CRC at the end of the packet. Take that 
         * into account when creating the packet. So the pkt looks as follows.
         *  |<- DA ->|<- SA ->|<- ETYPE ->|<- PAYLOAD ->|<- CRC ->|
         *  |<- 6B ->|<- 6B ->|<-- 2B --->|<-- xBytes ->|<- 4B -->|
         *  |<---------------- nPktLen -------------------------->|
         */
        len = rem -4;
    } else {
        /* User does not want CRC at the end of the packet. Take that 
         * into account when creating the packet. So the pkt looks as follows.
         *  |<- DA ->|<- SA ->|<- ETYPE ->|<- PAYLOAD ->|
         *  |<- 6B ->|<- 6B ->|<-- 2B --->|<-- xBytes ->|
         *  |<---------------- nPktLen ---------------->|
         */
        len = rem;
    }

    if (fill_mode == RAND_FILL) {
        packet_random_store((uint8*)hdr, len);
        cur += len;
        rem -= len;
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("DEBUG: %s: Rand Payload added, cur=%d, rem=%d, len=%d\n"),
                     FUNCTION_NAME(), cur, rem, len));
    } else if (fill_mode == NO_FILL) {
        /* The user has provided the payload, copy it over and then check if
         * there is still some space in the buffer.
         */
        int nTmpLen = len;

        if (sbx_tx_value_parse(hdr, payload, &len)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("Invalid payload contents. \n")));
            return BCM_E_PARAM;
        }
        if (len < nTmpLen) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("User provided payload came up short. Aborting...\n")));
            return BCM_E_PARAM;
        }

        cur += nTmpLen;  /* Note: This points to current locn on buf and *not* hdr */
        rem -= nTmpLen;
    } else {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Only random or pay=0x... options allowed for l2 tx. Aborting...\n")));
        return BCM_E_PARAM;
    }

    if (crc_append) {
        int ii;
        uint32 nCrc32 = 0;
        uint32 nCrc32_r = 0;
	uint32 crc_table[16] = {
            0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0, 
            0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320, 
            0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190, 
            0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000 
        };

        for (ii = 0; ii < cur; ii++) {
            nCrc32 = (nCrc32 >> 4) ^ crc_table[(nCrc32 ^ (pBuf[ii] >> 0)) & 0x0F];  /* lower nibble */ 
            nCrc32 = (nCrc32 >> 4) ^ crc_table[(nCrc32 ^ (pBuf[ii] >> 4)) & 0x0F];  /* upper nibble */
        }

        /* Remember to reflect the order i.e. change to N/W Byte order.
         */
        ((char *) &nCrc32_r)[0] = ((char *) &nCrc32)[3];
        ((char *) &nCrc32_r)[1] = ((char *) &nCrc32)[2];
        ((char *) &nCrc32_r)[2] = ((char *) &nCrc32)[1];
        ((char *) &nCrc32_r)[3] = ((char *) &nCrc32)[0];

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("DEBUG: %s: Crc Calc: CRC=0x%x, CRC_r=0x%x\n"),
                     FUNCTION_NAME(), nCrc32, nCrc32_r));
        /* Now put the CRC32 value into the packet buffer.
         */
        pBuf[cur+0] = ((char *) &nCrc32_r)[0];
        pBuf[cur+1] = ((char *) &nCrc32_r)[1];
        pBuf[cur+2] = ((char *) &nCrc32_r)[2];
        pBuf[cur+3] = ((char *) &nCrc32_r)[3];
        cur += 4; /* 4Bytes being the length of the CRC */
        rem -= 4;

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("DEBUG: %s: Crc added: pay_len=%d, cur=%d, rem=%d, CRC=0x%x\n"),
                     FUNCTION_NAME(), len, cur, rem, nCrc32_r));
    }

    if (LOG_CHECK(BSL_LS_APPL_COMMON | BSL_VERBOSE)) {
        sbx_debug_dump_buffer((unsigned char*)pBuf, nPktLen);
    }

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: END2 cur=%d, rem=%d, len=%d\n"),
                 FUNCTION_NAME(), cur, rem, len));


    return BCM_E_NONE;
}


static int
sbx_tx_pkt_init(int pktlen, char *hdr, char *rh, char *shim, char *higig,
                char *buf, char *payload, pkt_fill_mode_t fill_mode)
{
    int rem, cur, len, i;
    uint32 fill_word = 0;

    cur = 0;
    rem = pktlen;
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: BEGIN cur=0, rem=%d\n"),
                 FUNCTION_NAME(), pktlen));
    if (hdr) {
        if (rh) {
            len = RT_HDR_SZ;
            if (sbx_tx_value_parse(hdr, rh, &len)) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Invalid specification of Route Header.\n")));
                return BCM_E_PARAM;
            }
            cur += RT_HDR_SZ;
            rem -= RT_HDR_SZ;
            if (cur > pktlen) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Specified length(%d) too small\n"), pktlen));
                return BCM_E_PARAM;
            }
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META("DEBUG: %s: rh=%s, rh_len=%d, cur=%d, rem=%d\n"),
                      FUNCTION_NAME(), rh, len, cur, rem));
        }

        if (shim) {
            len = SHIM_HDR_SZ;
            if (sbx_tx_value_parse(&hdr[cur], shim, &len))  {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Invalid specification of Shim Header. \n")));
                return BCM_E_PARAM;
            }
            cur += SHIM_HDR_SZ;
            rem -= SHIM_HDR_SZ;
            if (cur > pktlen) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Specified length(%d) too small\n"), pktlen));
                return BCM_E_PARAM;
            }
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META("DEBUG: %s: shim=%s, shim_len=%d, cur=%d, rem=%d\n"),
                      FUNCTION_NAME(), shim, len, cur, rem));
        }

        if (higig) {
            len = HIGIG_HDR_SZ;
            if (sbx_tx_value_parse(&hdr[cur], higig, &len))  {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Invalid specification of HiGig Header. \n")));
                return BCM_E_PARAM;
            }
            cur += HIGIG_HDR_SZ;
            rem -= HIGIG_HDR_SZ;
            if (cur > pktlen) {
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("Specified length(%d) too small\n"), pktlen));
                return BCM_E_PARAM;
            }
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META("DEBUG: %s: higig=%s, higig_len=%d, cur=%d, rem=%d\n"),
                      FUNCTION_NAME(), higig, len, cur, rem));
        }
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: END1 cur=%d, rem=%d\n"),
                 FUNCTION_NAME(), cur, rem));
        
    /* len = rem because we expect payload to fillup the buf until the desired
     * pktlen.  It is possible that user specified a pattern or noincr, in which
     * case sbx_tx_value_parse() will return the parsed length in the len var,
     * which will be less than or equal to pktlen.
     */
    len = rem;
    if (fill_mode == NO_FILL_RAW) {
        /* The user has supplied raw data.  Just copy it verbatim into 'buf' */
        for (cur=0; cur < len; cur++) {
            buf[cur] = payload[cur];
            rem--;
        }
    } else {
        if (sbx_tx_value_parse(buf, payload, &len)) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("Invalid payload contents. \n")));
            return BCM_E_PARAM;
        }
        cur = len;  /* Note: This points to current locn on buf and *not* hdr */
        rem -= len;
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META("DEBUG: %s: payload=%s, pay_len=%d, cur=%d, rem=%d\n"),
                  FUNCTION_NAME(), payload, len, cur, rem));
        
        if (fill_mode == WORD_INCR_FILL) {
            if (len == 0 && rem > 0) {
                buf[0] = 0;
                cur++;
                len++;
                rem--;
            }
            while (len < 4 && rem > 0) {
                buf[cur+1] = buf[cur];
                cur++;
                len++;
                rem--;
            }

            fill_word =
                ((((unsigned char *) buf)[cur - 4] << 24) 
                 | (((unsigned char *) buf)[cur - 3] << 16)
                 | (((unsigned char *) buf)[cur - 2] << 8)
                 | ((unsigned char *) buf)[cur - 1])
                + 1;
        }
        
        if (fill_mode != NO_FILL) {
            while (rem > 0) {
                if (fill_mode == WORD_INCR_FILL) {
                    i = 0;
                    switch (rem > 4 ? 4 : rem) {
                    case 4:
                        buf[cur+3] = ((char *) &fill_word)[3];
                        i++;
                    case 3:
                        buf[cur+2] = ((char *) &fill_word)[2];
                        i++;
                    case 2:
                        buf[cur+1] = ((char *) &fill_word)[1];
                        i++;
                    case 1:
                        buf[cur+0] = ((char *) &fill_word)[0];
                        i++;
                    }
                    fill_word++;
                    rem -= i;
                    cur += i;
                } else {
                    buf[cur] = buf[cur - len];
                    rem--;
                    cur++;
                }
            }
        }
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("DEBUG: %s: END2 cur=%d, rem=%d\n"),
                 FUNCTION_NAME(), cur, rem));

    return BCM_E_NONE;
}

char cmd_soc_sbx_rx_usage[] =
"DBG Version of RX\n"
"rx [t=<wait time>] [rh[=show]] [shim[=show]] [payload=<show length>]\n"
"  t       - wait time in ms, default 0\n"
"  rh      - specify if route header exists in pci packet\n"
"  shim    - specify if shim header exists in pci packet\n"
"  higig   - specify if higig header exists in pci packet\n"
"  payload - specify length of payload to show, all by default\n"
;

cmd_result_t
cmd_soc_sbx_rx(int unit, args_t *args)
{
    int rv;
    int buf_len = TXRX_BUF_SIZE;
    char *pArg;
    int i;
    int hdrExists[sbx_hdr_num_types];
    int hdrShow[sbx_hdr_num_types];
    int nShowPayload = -1;
    int nWaitTimeUs = 0;

    sal_memset(hdrExists, 0, sizeof(int)*sbx_hdr_num_types);
    sal_memset(hdrShow, 0, sizeof(int)*sbx_hdr_num_types);

    while ((pArg = ARG_GET(args))) {

        i = -1;
        if (strstr(pArg, "rh") || strstr(pArg, "RH")) {
            i = sbx_hdr_Rh;
            pArg += 2; /* past 'rh' */
        }
        else if (strstr(pArg, "shim") || strstr(pArg, "SHIM")) {
            i = sbx_hdr_Shim;
            pArg += 4; /* past shim */
        }
        else if (strstr(pArg, "higig") || strstr(pArg, "HIGIG")) {
            i = sbx_hdr_Higig;
            pArg += 5; /* past higig */
        }
        else if (strstr(pArg, "payload=") || strstr(pArg, "PAYLOAD=")) {
            pArg += 8; /* past payload= */
            nShowPayload = parse_integer(pArg);
        }
        else if (strstr(pArg, "t=") || strstr(pArg, "T=")) {
            pArg += 2; /* past "t=" */
            /* Convert wait time to microseconds */
            nWaitTimeUs = parse_integer(pArg) * 1000;
        }
        else {
            cli_out("ignored option: %s\n", pArg);
            return CMD_USAGE;
        }

        if (i >= 0) {
            hdrExists[i] = 1;
            if (strstr(pArg, "=show") || strstr(pArg, "=SHOW")) {
                hdrShow[i] = 1;
            }
        }
    }

    /* Allocate packet buffer in PCI memory space */
    if (cmd_sbx_txrx_init_buf(unit) != CMD_OK) {
        return CMD_FAIL;
    }

    rv = soc_sbx_txrx_sync_rx(unit, cmd_sbx_txrx_bufs[unit],
                              &buf_len, nWaitTimeUs);

    if (rv) {
        return CMD_FAIL;
    }

    cli_out("%d:sbx_rx: received a packet\n", unit);
    sbx_dump_pkt((unsigned char*)cmd_sbx_txrx_bufs[unit],
                 buf_len, hdrExists, hdrShow, nShowPayload);

    return CMD_OK;
}


char cmd_soc_sbx_tx_usage[] =
"DBG Version of TX \n"
"Tx <count> Options \n"
"   count   - number of packets to send \n"
"   RH      - route header (hex format) \n"
"   SHIM    - shim header (hex format) \n"
"   HIGIG   - higig header (hex format) \n"
"   LEN     - length (including RH & SHIM) of packet \n"
"   PAY     - packet payload (hex format)\n"
"   PAT     - 4 byte pattern to increment & fill the payload (hex format)\n"
"   NOINCR  - n byte pattern to fill the payload (hex format)\n";

cmd_result_t
cmd_soc_sbx_tx(int unit, args_t *args)
{
    char    *arg, *value_str, *rh = NULL, *shim = NULL, *higig = NULL;
    int     rv;
    uint32  num = 1;
    uint32  *value_holder = NULL;
    char    *pay = "0xba5eba11";
    int     hdrlen, len = 64;
    pkt_fill_mode_t fill_mode = WORD_INCR_FILL;
    char    hdr[24];

    arg = ARG_GET(args);
    while (arg) {
        value_str =  NULL;
        value_holder = NULL;

        if ((strstr(arg, "RH=")) || (strstr(arg, "rh="))) {
            rh = arg + 3;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: rh=%s\n"),
                      FUNCTION_NAME(), rh));
            continue;
        } else if ((strstr(arg, "SHIM=")) || (strstr(arg, "shim="))) {
            shim = arg + 5;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: shim=%s\n"),
                      FUNCTION_NAME(), shim));
            continue;
        } else if ((strstr(arg, "HIGIG=")) || (strstr(arg, "higig="))) {
            higig = arg + 6;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: higig=%s\n"),
                      FUNCTION_NAME(), higig));
            continue;
        } else if ((strstr(arg, "LEN=")) || (strstr(arg, "len="))) {
            value_str = arg + 4;
            value_holder = (uint32 *) &len;
        } else if ((strstr(arg, "RAW=")) || (strstr(arg, "raw="))) {
            pay = arg + 4;
            fill_mode = NO_FILL_RAW;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: Raw Packet, possibly read from a Binary File\n"),
                      FUNCTION_NAME()));
            continue;
        } else if ((strstr(arg, "PAY=")) || (strstr(arg, "pay="))) {
            pay = arg + 4;
            fill_mode = NO_FILL;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: pay=%s\n"),
                      FUNCTION_NAME(), pay));
            continue;
        } else if ((strstr(arg, "PAT=")) || (strstr(arg, "pat="))) {
            pay = arg + 4;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: pat=%s\n"),
                      FUNCTION_NAME(), pay));
            continue;
        } else if ((strstr(arg, "NOINCR=")) || (strstr(arg, "noincr="))) {
            fill_mode = CONST_FILL;
            pay = arg + 7;
            arg = ARG_GET(args);
            LOG_INFO(BSL_LS_APPL_TESTS,
                     (BSL_META_U(unit,
                                 "DEBUG: %s: noincr=%s\n"),
                      FUNCTION_NAME(), pay));
            continue;
        } else if (isint(arg)) {
          value_str = arg;
          value_holder = &num;
        } else {
            return CMD_USAGE;
        }

        if (((!value_str) || (!value_holder)) || (!isint(value_str))) {
            return CMD_USAGE;
        }

        *value_holder = parse_integer(value_str);

        arg = ARG_GET(args);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "DEBUG: %s: num/len=%d\n"),
                  FUNCTION_NAME(), *value_holder));
    }

    /* Allocate packet buffer in PCI memory space */
    if (cmd_sbx_txrx_init_buf(unit) != CMD_OK) {
        return CMD_FAIL;
    }

    /* write the pattern to the packet buffer */
    if (sbx_tx_pkt_init(len, hdr, rh, shim, higig, cmd_sbx_txrx_bufs[unit],
                        pay, fill_mode)) {
        cli_out("cmd_sbx_tx: sbx_tx_pkt_init failed. \n");
        return CMD_FAIL;
    }

    hdrlen = (rh ? RT_HDR_SZ : 0) + (shim ? SHIM_HDR_SZ : 0) + (higig ? HIGIG_HDR_SZ : 0);
    len -= hdrlen;

    if (LOG_CHECK(BSL_LS_APPL_TESTS | BSL_INFO)) {
      int hdrExists[sbx_hdr_num_types];
      int hdrShow[sbx_hdr_num_types];
      int nShowPayload = len;
      int buf_len = len;

      hdrExists[0] = 0;
      hdrExists[1] = 0;
      hdrExists[2] = 0;
      hdrShow[0] = 0;
      hdrShow[1] = 0;
      hdrShow[2] = 0;

      /* Dump the packet before it goes out */
      LOG_INFO(BSL_LS_APPL_TESTS,
               (BSL_META_U(unit,
                           "DEBUG: %s: Outgoing Tx packet\n"),
                FUNCTION_NAME()));
      sbx_dump_pkt((unsigned char*)cmd_sbx_txrx_bufs[unit],
                   buf_len, hdrExists, hdrShow, nShowPayload);
    }

    rv = soc_sbx_txrx_sync_tx(unit, hdr, hdrlen,
                              cmd_sbx_txrx_bufs[unit], len, 1000000);
    if (rv) {
        cli_out("cmd_sbx_tx: failed rv(%d). \n", rv);
        return CMD_FAIL;
    }

    return CMD_OK;
}

char cmd_sbx_rx_init_usage[] =
    "RXInit [unit=override-unit]\n"
    "    Start the BCM RX thread by calling bcm_rx_init and bcm_rx_start\n"
    "    on the optionally given override unit. \n";

cmd_result_t
cmd_sbx_rx_init(int unit, args_t *args)
{
    int override_unit = unit;
    int debug = 1;
    int rv;

    if (ARG_CNT(args)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "unit", PQ_DFL | PQ_INT,
                        0, &override_unit, NULL);
        parse_table_add(&pt, "debug", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &debug, NULL);

        if (!parseEndOk(args, &pt, &ret_code)) {
            return ret_code;
        }
    }

    rv = bcm_rx_init(override_unit);
    if (rv < 0) {
        cli_out("ERROR:  bcm_rx_init(%d) returns %d: %s\n",
                override_unit, rv, bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (debug) {
        rv = bcm_rx_start(override_unit, 0);
        if (rv <0) {
            cli_out("ERROR: bcm_rx_start(%d) failed:%d:%s\n",
                    override_unit, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        rv = bcm_rx_register(unit, "TestRx CB",
                             test_rx_cb, 0x40, (void*)0x0c00c613, BCM_RCO_F_ALL_COS);

        if (rv <0) {
            cli_out("ERROR: bcm_register(%d) failed:%d:%s\n",
                    override_unit, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

char cmd_sbx_rx_stop_usage[] =
    "rxstop [override-unit]\n"
    "    Stop the BCM RX thread on the optionally given override unit. \n";

cmd_result_t
cmd_sbx_rx_stop(int unit, args_t *args)
{
    char *ch;
    int override_unit = unit;
    int rv;

    if ((ch = ARG_GET(args))) {
        override_unit = strtoul(ch, NULL, 0);
    }

    rv = bcm_rx_stop(override_unit, NULL);
    if (rv < 0) {
        cli_out("ERROR:  bcm_rx_stop(%d) returns %d: %s\n",
                override_unit, rv, bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}


char cmd_sbx_tx_usage[] =
"DBG Version of TX \n"
"Tx <count> Options \n"
#ifndef COMPILER_STRING_CONST_LIMIT
"   count   - number of packets to send \n"
"   NODE    - Destination node (default: current unit)\n"
"   PORT    - Destination port (default: CPU)\n"
"   LEN     - length of packet payload (not including RH & SHIM)\n"
"   PAY     - packet payload (hex format)\n"
"   PAT     - 4 byte pattern to increment & fill the payload (hex format)\n"
"   NOINCR  - n byte pattern to fill the payload (hex format)\n"
"   L2      - This indicates a Layer2 packet\n"
"   RANDOM  - payload is filled with random data\n"
"   SA      - 6 byte Source Address\n"
"   DA      - 6 byte Destination Address\n"
"   CRC_APPEND - A 4 Byte CRC (CRC32) will be appended to packet\n"
#endif
;

cmd_result_t
cmd_sbx_tx(int unit, args_t *args)
{
    char    *arg, *value_str;
    int     rv, i;
    char    *pay = "0xba5eba11";
    uint32  *value_holder = NULL;
    int     num = 0, len = 64, node = unit, port = 49, mod_id=0;
    pkt_fill_mode_t fill_mode = WORD_INCR_FILL;
    bcm_pkt_t  pkt;
    /* The following to be used for custom l2 packets. */
    int l2_pkt = 0, crc_append = 0;
    char *da_addr = NULL;  /* = "0x000506000009"; */
    char *sa_addr = NULL;  /* = "0x000506000000"; */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SBX_SIRIUS(unit)) {
      int hgig = 0;
      port = 0;
      for (hgig=0; hgig < 4; hgig++) {
          port += SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[hgig];
      }
    }
#endif

    arg = ARG_GET(args);
    if (arg == NULL) {
        return CMD_USAGE;
    }

    if (isint(arg)) {
        /* Check for valid number */
        num = parse_integer(arg);

        if (num <= 0) {
            cli_out("cmd_sbx_tx: Invalid number of packets (%d) "
                    "specified\n", num);
            return CMD_USAGE;
        }
    }

    arg = ARG_GET(args);

    while (arg) {
        value_str =  NULL;
        value_holder = NULL;

        if ((strstr(arg, "NODE=")) || (strstr(arg, "node="))) {
            value_str = arg + 5;
            value_holder = (uint32 *) &node;
        } else if ((strstr(arg, "PORT=")) || (strstr(arg, "port="))) {
            value_str = arg + 5;
            value_holder = (uint32 *) &port;
        } else if ((strstr(arg, "LEN=")) || (strstr(arg, "len="))) {
            value_str = arg + 4;
            value_holder = (uint32 *) &len;
        } else if ((strstr(arg, "PAY=")) || (strstr(arg, "pay="))) {
            pay = arg + 4;
            fill_mode = NO_FILL;
            arg = ARG_GET(args);
            continue;
        } else if ((strstr(arg, "PAT=")) || (strstr(arg, "pat="))) {
            /* fill_mode = WORD_INCR_FILL; This is probably the default. */
            /* gsrao 032610
             * Note that when PAT=xxxxx is specified then the fill mode is 
             * WORD_INCR_FILL or when NOINCR=xxxxx is specified then the 
             * fill mode is CONST_FILL.  Both of these can be utilized
             * along with RAND_FILL to generate a tailor made pattern.
             */
            pay = arg + 4;
            arg = ARG_GET(args);
            continue;
        } else if ((strstr(arg, "NOINCR=")) || (strstr(arg, "noincr="))) {
            fill_mode = CONST_FILL;
            pay = arg + 7;
            arg = ARG_GET(args);
            continue;
        } else if (!sal_strcasecmp("random", arg)) {
            /* User knows what they are doing. Set a flag. */
            fill_mode = RAND_FILL;
            arg = ARG_GET(args);
            continue;
        } else if (!sal_strcasecmp("L2", arg)) {
            /* User knows what they are doing. Set a flag. User sets this flag
             * to indicate a desire to generate a custom L2 packet with specific
             * SA/DA and with/without CRC32.
             */
            l2_pkt = 1;
            arg = ARG_GET(args);
            continue;
        } else if (!sal_strcasecmp("crc_append", arg)) {
            /* User knows what they are doing. Set a flag. In this mode, the 
             * CRC32 value will be calculated and appended to the packet.
             */
            crc_append = 1;
            arg = ARG_GET(args);
            continue;
        } else if ((strstr(arg, "SA=")) || (strstr(arg, "sa="))) {
            /* User knows what they are doing. Set the Source Address. */
            sa_addr = arg + 3;
            arg = ARG_GET(args);
            continue;
        } else if ((strstr(arg, "DA=")) || (strstr(arg, "da="))) {
            /* User knows what they are doing. Set the Destination Address. */
            da_addr = arg + 3;
            arg = ARG_GET(args);
            continue;
        } else {
            return CMD_USAGE;
        }

        if (((!value_str) || (!value_holder)) || (!isint(value_str))) {
            return CMD_USAGE;
        }

        *value_holder = parse_integer(value_str);

        arg = ARG_GET(args);
    }

    /* Allocate packet buffer in PCI memory space */
    if (cmd_sbx_txrx_init_buf(unit) != CMD_OK) {
        return CMD_FAIL;
    }

    /* Use the following to create a custom L2 packet with a specific SA and/or
     * DA. 
     */
    if (l2_pkt) {
        if (sbx_tx_l2_pkt_init(len, da_addr, sa_addr, crc_append, cmd_sbx_txrx_bufs[unit],
                               pay, fill_mode)) {
            cli_out("cmd_sbx_tx: sbx_tx_l2_pkt_init failed. \n");
            return CMD_FAIL;
        }
    } else {
        /* write the pattern to the packet buffer */
        if (sbx_tx_pkt_init(len, NULL, NULL, NULL, NULL, cmd_sbx_txrx_bufs[unit], 
                            pay, fill_mode)) {
            cli_out("cmd_sbx_tx: sbx_tx_pkt_init failed. \n");
            return CMD_FAIL;
        }
    }

    sal_memset(&pkt, 0, sizeof(bcm_pkt_t));
    pkt.pkt_data = &pkt._pkt_data;

    pkt.blk_count = 1;
    pkt.pkt_data->data = (uint8 *) cmd_sbx_txrx_bufs[unit];
    pkt.pkt_data->len = len;

#ifndef BCM_SIRIUS_SUPPORT
    SOC_SBX_MODID_FROM_NODE(node, mod_id);
#endif

    pkt.dest_mod = mod_id;
    pkt.dest_port = port;
    pkt.cos = 0;
    pkt.flags |= BCM_PKT_F_TEST | BCM_TX_CRC_ALLOC;
    pkt.opcode = BCM_PKT_OPCODE_CPU;

    if (LOG_CHECK(BSL_LS_APPL_TESTS | BSL_VERBOSE)) {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "DEBUG: %s: Begin Dumping Final Packet.\n"),
                  FUNCTION_NAME()));
        sbx_debug_dump_buffer((unsigned char*)pkt.pkt_data->data, pkt.pkt_data->len);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "DEBUG: %s: Begin Dumping Final Packet. Done...\n"),
                  FUNCTION_NAME()));
    }

    for (i=0; i<num; i++) {
        rv = bcm_tx_pkt_setup(unit, &pkt);
        if (BCM_FAILURE(rv)) {
            cli_out("bcm_tx_pkt_setup error:%d %s\n", rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
#ifdef BCM_SIRIUS_SUPPORT
	if (SOC_IS_SBX_SIRIUS(unit)) {
	    pkt.flags |= (BCM_TX_HG_READY | BCM_TX_NO_PAD);
	}
#endif

        rv = bcm_tx(unit, &pkt, NULL);
        if (BCM_FAILURE(rv)) {
            cli_out("bcm_tx error:%d %s\n", rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}


char cmd_sbx_rx_usage[] =
"DBG Version of RX\n"
"rx [t=<wait time>] [rh[=show]] [shim[=show]] [payload=<show length>]\n"
"  t       - wait time in ms, default 0\n"
"  rh      - specify if route header exists in pci packet\n"
"  shim    - specify if shim header exists in pci packet\n"
"  payload - specify length of payload to show, all by default\n"
;

cmd_result_t
cmd_sbx_rx(int unit, args_t *args)
{
    return cmd_soc_sbx_rx(unit, args);
}

static void
cmd_sbx_print_tx_ring_entry (int unit, int entry)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint8 *entryp = (uint8 *) &sbx->tx_ring[entry * 4];
    uint32 i, l;

    cli_out("TX ring entry %d: ", entry);

    i = entryp[3];
    cli_out("type=%s%s%s", (i & (1 << 7)) ? "imm" : "sg",
            (i & (1 << 6)) ? ", sop" : "",
            (i & (1 << 5)) ? ", eop" : "");
    l = (entryp[2] << 8) + entryp[1];
    cli_out(", len=%d", l);
    if (i & (1 << 7)) {
        if (l > 12) {
            cli_out(" (invalid, too large)");
            l = 12;
        }
        cli_out("\n  data:");
        for (i = 0; i < l; i++) {
            cli_out(" 0x%02x", entryp[4 + i]);
        }
        cli_out("\n");
    } else {
        cli_out(", ptr=0x%08x\n",
                (entryp[7] << 24)
                | (entryp[6] << 16)
                | (entryp[5] << 8)
                | entryp[4]);
    }
    cli_out("  raw: ");
    for(i = 0; i < 4; i++) {
        cli_out("0x%08x ",
                (entryp[i * 4 + 3] << 24)
                | (entryp[i * 4 + 2] << 16)
                | (entryp[i * 4 + 1] << 8)
                | entryp[i * 4 + 0]);
    }
    cli_out("\n");
}

char cmd_sbx_tx_ring_get_usage[] =
        "print TX ring \n"
        "gettxring <options> \n"
        "   ENTRY   - entry # to print\n";

cmd_result_t
cmd_sbx_tx_ring_get (int unit, args_t *args)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    char    *arg, *value_str, *value_holder;
    int entry = -1;
    int i;

    if (!sbx->tx_ring) {
        cli_out("ERROR:  SBX TX ring unininitialized\n");
        return CMD_FAIL;
    }

    arg = ARG_GET(args);

    while (arg) {
        value_str = NULL;
        value_holder = NULL;
        if ((strstr(arg, "ENTRY=")) || (strstr(arg, "entry="))) {
            value_str = arg + strlen("ENTRY=");
            value_holder = (char *) &entry;
        } else {
            return CMD_USAGE;
        }

        if (((!value_str) || (!value_holder)) || (!isint(value_str))) {
            return CMD_USAGE;
        }

        *value_holder = parse_integer(value_str);

        arg = ARG_GET(args);
    }

    cli_out("TX ring producer=%d, consumer=%d\n",
            SAND_HAL_READ_OFFS(sbx->sbhdl,
            sbx->tx_ring_producer_reg)/16,
            SAND_HAL_READ_OFFS(sbx->sbhdl,
            sbx->tx_ring_consumer_reg)/16);

    if (entry == -1) {
        for (i = 0; i < sbx->tx_ring_entries; i++) {
            cmd_sbx_print_tx_ring_entry(unit, i);
        }
    } else if (entry < sbx->tx_ring_entries) {
        cmd_sbx_print_tx_ring_entry(unit, entry);
    } else {
        return CMD_USAGE;
    }

    return CMD_OK;
}

static void
cmd_sbx_print_completion_ring_entry (int unit, int entry)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    unsigned char *entryp = (unsigned char *) &sbx->completion_ring[entry];
    uint32 i;

    cli_out("Completion ring entry %d: ", entry);

    i = entryp[3];
    cli_out("type=%s%s", (i & (1 << 7)) ? "tx" : "rx",
            (i & (1 << 6)) ? " error" : "");
    i = (entryp[1] >> 6) + (entryp[2] << 8);
    cli_out(" bufs=%d", i);
    i = ((entryp[1] & 0x3f) << 8) + (entryp[0]);
    cli_out(" len=%d\n", i);
    cli_out("  raw: ");
    cli_out("0x%08x ",
            (entryp[3] << 24)
            | (entryp[2] << 16)
            | (entryp[1] << 8)
            | entryp[0]);
    cli_out("\n");
}

char cmd_sbx_completion_ring_get_usage[] =
        "print completion ring \n"
        "getcompring <options> \n"
        "   ENTRY   - entry # to print\n";

cmd_result_t
cmd_sbx_completion_ring_get (int unit, args_t *args)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    char    *arg, *value_str, *value_holder;
    int entry = -1;
    int i;

    if (!sbx->completion_ring) {
        cli_out("ERROR:  SBX completion ring unininitialized\n");
        return CMD_FAIL;
    }

    arg = ARG_GET(args);

    while (arg) {
        value_str = NULL;
        value_holder = NULL;
        if ((strstr(arg, "ENTRY=")) || (strstr(arg, "entry="))) {
            value_str = arg + strlen("ENTRY=");
            value_holder = (char *) &entry;
        } else {
            return CMD_USAGE;
        }

        if (((!value_str) || (!value_holder)) || (!isint(value_str))) {
            return CMD_USAGE;
        }

        *value_holder = parse_integer(value_str);

        arg = ARG_GET(args);
    }

    cli_out("Completion ring producer: %d, completion ring consumer: %d\n",
            SAND_HAL_READ_OFFS(sbx->sbhdl,
            sbx->completion_ring_producer_reg)/4,
            SAND_HAL_READ_OFFS(sbx->sbhdl,
            sbx->completion_ring_consumer_reg)/4);

    if (entry == -1) {
        for (i = 0; i < sbx->completion_ring_entries; i++) {
            cmd_sbx_print_completion_ring_entry(unit, i);
        }
    } else if (entry < sbx->completion_ring_entries) {
        cmd_sbx_print_completion_ring_entry(unit, entry);
    } else {
        return CMD_USAGE;
    }

    return CMD_OK;
}
