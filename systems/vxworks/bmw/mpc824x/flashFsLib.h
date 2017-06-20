/* $Id: flashFsLib.h,v 1.3 2011/07/21 16:14:08 yshtil Exp $ */
#ifndef	FLASH_FS_LIB_H
#define	FLASH_FS_LIB_H

STATUS flashFsLibInit(void);
STATUS flashFsSync(void);
IMPORT STATUS sysHasDOC();

#define	FLASH_FS_NAME	((sysHasDOC()) ? "flsh:":"flash:")

#define FIOFLASHSYNC	0x10000
#define FIOFLASHINVAL	0x10001

#endif	/* !FLASH_FS_LIB_H */
