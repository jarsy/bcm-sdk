/*
 * $Id: sbxpkt_test.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

void sbxpkt_init() {
  bshell(1, "nicconfig ingress");
  sbxpkt_rx_sync_start (0);
  sbxpkt_rx_sync_start (1);
}

void sbxpkt_config() {
  int rc;
  int unit = 1;
  int  port = 31; 
  int  vid = 3;

  soc_sbx_g2p3_pv2e_t pv2e;
  soc_sbx_g2p3_pv2e_t_init(pv2e);
  pv2e.vlan = 3;
  pv2e.stpstate = 0;
  pv2e.untagged_strip = 1;
  rc = soc_sbx_g2p3_pv2e_set(unit, vid, port, pv2e);

  soc_sbx_g2p3_ft_t ft;
  rc = soc_sbx_g2p3_ft_get(unit, 0x4000 + vid, &ft);
  ft.excidx = 0;
  ft.qid = 0xabc;
  ft.oi = 0x19876;
  rc = soc_sbx_g2p3_ft_set(unit, 0x4000 + vid, ft);
}

void sbxpkt_test1() {
  int rc;
  int unit = 1;
  int pkt_len;
  int pkt_flags;

  bcm_pkt_t *rx_pkt;
  rc = bcm_pkt_alloc(unit,pkt_len, pkt_flags, rx_pkt);

  rc = sbxpkt_rx_sync (1, rx_pkt, 0);
  printf ("packet rx: %d\n", rc);

  sbxpkt_t *sbx_tx_pkt;
  uint8 pkt_data[2000];
  sbx_tx_pkt = sbxpkt_alloc();
  sbxpkt_create (sbx_tx_pkt, "--mac -dmac 00:00:ab:cd:ef:ff --vlan -vid 3 --etype -etype 0x8847 --mpls");
  sbxpkt_print(sbx_tx_pkt);
  to_byte (sbx_tx_pkt, (auto)pkt_data);

  sbxpkt_prepend (sbx_tx_pkt, "--erh_qe -qid 0xabc -oi 0x19876;");

  pkt_len = sbx_tx_pkt->entry.length;
  pkt_flags = BCM_TX_CRC_ALLOC;

  bcm_pkt_t *tx_pkt;
  rc = bcm_pkt_alloc(unit,pkt_len, pkt_flags, tx_pkt);

  void * cookie = (auto)1;
  tx_pkt->call_back = 0;
  tx_pkt->blk_count = 1;
  tx_pkt->unit = 1;

  bcm_pkt_memcpy (tx_pkt, 0, (auto)pkt_data, pkt_len);

  rc = bcm_tx (unit, tx_pkt, cookie);
  printf ("packet tx: %d\n", rc);

  rc = sbxpkt_rx_sync (1, rx_pkt, 0);
  printf ("packet rx: %d\n", rc);

  pkt_len = rx_pkt->pkt_len;
  sbxpkt_data_memget (rx_pkt, 0, pkt_data, pkt_len);

  sbxpkt_t *sbx_rx_pkt;
  sbx_rx_pkt = sbxpkt_alloc();
  sbx_rx_pkt->entry.type = 0;
  from_byte (1, pkt_data, pkt_len, sbx_rx_pkt);
  sbxpkt_print(sbx_rx_pkt);

  sbxpkt_compare (sbx_tx_pkt, sbx_rx_pkt);

  rc = sbxpkt_rx_sync (1, rx_pkt, 0);
  printf ("packet rx: %d\n", rc);
}

