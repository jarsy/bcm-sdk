/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SBX SIM Socket client interface
 *
 */



#define MAXDATASIZE 10*1024

#define SOC_SBX_SIM_DEFAULT_PORT "1588"   /* Note string format */

/*
 * Send a message to the server, connect if not connected
 * Parameters:
 *  unit - sim server dev no
 *  buffer - message 
 *  bsize - size of message
 * Returns:
 */
int send_message(int unit, char* buffer, int bsize);


/*
 * Get message  from sim server
 * Parameters:
 *  unit - sim server dev no
 *  buffer - buffer to write response from server
 *  size - expected size of response
 */
int recv_message(int unit, char* buffer, int *size);

