/*
 * $Id: txit.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

sbxpkt_t *pkt;
unsigned char pkt_data[2000];
pkt = sbxpkt_alloc();
sbxpkt_create (pkt, "--mac -dmac 00:00:ab:cd:ef:ff --etype -etype 0x8847 --mpls");
sbxpkt_print(pkt);
to_byte (pkt, pkt_data);

int unit = 1;
int pkt_flags;
int pkt_len = pkt->entry.length;
print pkt_len;

pkt_flags = BCM_TX_CRC_ALLOC;

int rc;
bcm_pkt_t *tx_pkt;
rc = bcm_pkt_alloc(unit,pkt_len, pkt_flags, tx_pkt);
print rc;

void * cookie = (auto)1; 
tx_pkt->call_back = 0;

tx_pkt->blk_count = 1;             
tx_pkt->unit = 1;           

bcm_pkt_memcpy (tx_pkt, 0, (auto)pkt_data, pkt_len); 
       
bcm_tx (unit, tx_pkt, cookie);
bshell(1, "hd raw");  
bshell (1,"hg queue=0x39 dumpold=1");

