
/*******************************************************************

Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 

File Name: 
    ag_basic_types.h

File Description: 

$Revision: 1.1.2.1 $ - Visual SourceSafe revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    12/18/2002
*******************************************************************/
#ifndef AG_BASIC_TYPES_H 
#define AG_BASIC_TYPES_H 

/*Main include files */
#ifdef AG_GCC
typedef unsigned long 		AG_U64; /*TEMP standard C for U64 is "long long" - for some reason does not work in GCC*/
typedef long 				AG_S64;
#else
typedef unsigned __int64 	AG_U64; 
typedef __int64 			AG_S64;
#endif
typedef unsigned long 		AG_U32;
typedef long 				AG_S32;
typedef unsigned short 		AG_U16;
typedef short 				AG_S16;
#if (defined AG_MNT || defined AG_PC_PROG)
typedef unsigned char 	AG_U8;
typedef char 			AG_S8;
#else
/* In ARM, the default compiler option is to set char to unsigned char,
   if one wants to use char as signed a special option -zc is needed (which we DO NOT use)
   For this reason, and to avoid the compiler errors/warnings of types casting between char
   (used a lot by Nucleus) and unsigned-char, we define AG_U8 as simple char and we 
   compile under ARM with no special switch (NOT USING -zc) so all chars in the code are
   handled as unsigned-char - and EVERYONE is Happy! 
   If, for some reason, one will chose to compile with this special switch, the resulting
   behavior of the code in NOT GAURANTEED !!!!
*/
typedef char 			AG_U8;
typedef signed char 	AG_S8;
#endif

/* The type for working with characters */
typedef char            AG_CHAR;

/* 
The standard return value that each function must return.
See ag_results for standard return values.
*/
typedef AG_U32          AgResult;

typedef AG_U32 			AG_BOOL; /*boolean value*/

typedef AG_U16          AG_ID;

typedef volatile AG_U32 AG_REGISTER;

/*used for AG_BOOL*/
#define AG_FALSE   0
#define AG_TRUE    1

#ifndef NULL
#define NULL   0
#endif


#define AG_MAX_U32 ((AG_U32)0xFFFFFFFF)
#define AG_MAX_U16 ((AG_U16)0xFFFF)
#define AG_MAX_U8  ((AG_U8)0xFF)
#define UINT32_ROLL_OVER(a1,a2) ((a1)>=(a2))?((a1)-(a2)):(((a1)-(a2))+AG_MAX_U32)

#define AG_UNKNOWN_ID   AG_MAX_U16

/*AG internal frame header size equals to the size of BMH + PIH */
#define AG_FRM_HDR_SIZE   ((AG_U8)32) 
#define AG_MAX_QUEUE_NUM  255

typedef enum AgSetClear_E
{ AG_CLEAR,
  AG_SET,
  AG_NUM_SETCLR
}AgSetClear;

/* AG Communication interfaces: Interface-a, interface-b and interfac-c
AG_IF_A mode: HDLC or Ethernet or Bitstream
AG_IF_B mode: HDLC or Bitstream
AG_IF_C mode: Ethernet
*/
enum AgComIntf_E
{
    AG_IF_A=1,
    AG_IF_B=2,
    AG_IF_C=0,

    /* old names */
    AG_LAN = 0,
    AG_WAN1 = 1,
    AG_WAN2 = 2,
    /* end of old names */

    /* highest interface number */
    AG_HIGHEST_IF_NUM = 2,

    /* maximum interfaces */
    AG_MAX_IF = 3
};
typedef enum AgComIntf_E AgComIntf;

/* System base mode */
enum AgBaseMode_E
{    
    AG_BASE_GATEWAY = 1,
	AG_BASE_SWITCH
};
typedef enum AgBaseMode_E AgBaseMode;

enum AgEthIntf_E
{
    AG_LAN_ETHERNET = 0,
    AG_WAN_ETHERNET = 1,
	AG_ETHERNET_IF_C = 0,
	AG_ETHERNET_IF_A = 1
};
typedef enum AgEthIntf_E AgEthIntf;

/* the UART are 2 more interfaces in RX100, we want to make sure that the enums of
   the AG_IF and AG_UART do not overlap */
enum AgUart_E
{
    /* lowest UART number - we set it to be the next number after AG_IF_A, AG_IF_B AND AG_IF_C */
    AG_LOWEST_UART_NUM = (AG_HIGHEST_IF_NUM+1),

    AG_UART_A = AG_LOWEST_UART_NUM,
	AG_UART_B,

    /* highest UART number */
    AG_HIGHEST_UART_NUM = AG_UART_B, 
    /* create kind of ENUM between all RX-if, the net traffic and UARTS */
    AG_HIGHEST_IF_UART_NUM = AG_HIGHEST_UART_NUM,  

    /* maximum number of UARTS */
    AG_MAX_UARTS = 2

};
typedef enum AgUart_E AgUart;

/* The Host Comm Tcp Ip Interfaces */
enum AgHostTcpIpIntf_E
{
	AG_TCPIP_IF_C = AG_IF_C,
	AG_TCPIP_IF_A = AG_IF_A,
	AG_TCPIP_IF_B = AG_IF_B,
	AG_TCPIP_UART_A = AG_UART_A,
	AG_TCPIP_UART_B = AG_UART_B
};
typedef enum AgHostTcpIpIntf_E AgHostTcpIpIntf;

typedef enum AgEnable_E
{ AG_DISABLE=0,
  AG_ENABLE =1,
  AG_MAX_ENABLE     /* For validation purpose */
}AgEnable;


/* support 2 names for the same type for backward compatability. */
typedef enum AgEnable_E AgDeviceEnable;

/*what action to do with the bundle when the sw q is full*/
enum AgQFullAction_E
{    
    AG_QFULL_OVERWRITE,
	AG_QFULL_DISCARD,
	AG_QFULL_NONE,
	AG_QFULL_MAX_VAL    /* For validation */
};
typedef enum AgQFullAction_E AgQFullAction;


/******** general data structure for declaring const strings ******/

typedef struct AgCharStringPtr_S
{
	AG_CHAR			*p_str;
} AgCharStringPtr;

/* Type used for massage tables */
typedef const AG_CHAR* AgSCharStringPtr;


#endif  /* AG_BASIC_TYPES_H */

