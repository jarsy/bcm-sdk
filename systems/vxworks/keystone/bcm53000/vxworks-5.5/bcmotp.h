/*
 * OTP support.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: bcmotp.h,v 1.2 Broadcom SDK $
 */

/* OTP regions */
#define OTP_HW_RGN	1
#define OTP_SW_RGN	2
#define OTP_CI_RGN	4
#define OTP_FUSE_RGN	8

#if !defined(BCMDONGLEHOST)
extern void *otp_init(si_t *sih);
#endif /* !defined(BCMDONGLEHOST) */

extern int otp_status(void *oh);
extern int otp_size(void *oh);

extern int otp_read_region(void *oh, int region, uint16 *data, uint *wlen);
extern uint16 otpr(void *oh, chipcregs_t *cc, uint wn);
extern int otp_nvread(void *oh, char *data, uint *len);

#ifdef BCMNVRAMW
#if !defined(BCMDONGLEHOST)
extern int otp_write_region(void *oh, int region, uint16 *data, uint wlen);
extern int otp_fix_word16(void *oh, uint wn, uint16 mask, uint16 val);
extern int otp_write_rde(void *oh, int rde, uint bit, uint val);
extern int otpw(void *oh, int wn, uint16 data);
extern int otp_nvwrite(void *oh, uint16 *data, uint wlen);
#endif /* !defined(BCMDONGLEHOST) */
#endif /* BCMNVRAMW */

#if defined(WLTEST) || defined (BCMINTERNAL)
extern int otp_dump(void *oh, int arg, char *buf, uint size);
#endif
