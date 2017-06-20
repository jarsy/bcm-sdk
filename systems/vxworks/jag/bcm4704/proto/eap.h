/*
 * Extensible Authentication Protocol (EAP) definitions
 *
 * See
 * RFC 2284: PPP Extensible Authentication Protocol (EAP)
 *
 * Copyright (C) 2002 Broadcom Corporation
 *
 * $Id: eap.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 */

#ifndef _eap_h_
#define _eap_h_

/* EAP packet format */
typedef struct {
	unsigned char code;	/* EAP code */
	unsigned char id;	/* Current request ID */
	unsigned short length;	/* Length including header */
	unsigned char type;	/* EAP type (optional) */
	unsigned char data[1];	/* Type data (optional) */
} eap_header_t;

#define EAP_HEADER_LEN 4

/* EAP codes */
#define EAP_REQUEST	1
#define EAP_RESPONSE	2
#define EAP_SUCCESS	3
#define EAP_FAILURE	4

/* EAP types */
#define EAP_IDENTITY		1
#define EAP_NOTIFICATION	2
#define EAP_NAK			3
#define EAP_MD5			4
#define EAP_OTP			5
#define EAP_GTC			6
#define EAP_TLS			13
#ifdef  CCX
#define EAP_LEAP		17

#define LEAP_VERSION		1
#define LEAP_CHALLENGE_LEN	8
#define LEAP_RESPONSE_LEN	24

/* LEAP challenge */
typedef struct {
	unsigned char version;	/* should be value of LEAP_VERSION */
	unsigned char reserved;	/* not used */
	unsigned char chall_len;/* always value of LEAP_CHALLENGE_LEN */
	unsigned char challenge[LEAP_CHALLENGE_LEN]; /* random */
	unsigned char username[1];
} leap_challenge_t;

#define LEAP_CHALLENGE_HDR_LEN	12

/* LEAP challenge reponse */
typedef struct {
	unsigned char version;	/* should be value of LEAP_VERSION */
	unsigned char reserved;	/* not used */
	unsigned char resp_len;	/* always value of LEAP_RESPONSE_LEN */
	/* MS-CHAP hash of challenge and user's password */
	unsigned char response[LEAP_RESPONSE_LEN];
	unsigned char username[1];
} leap_response_t;

#define LEAP_RESPONSE_HDR_LEN	28

#endif /* CCX */

#endif /* _eap_h_ */
