/*
 * $Id: simintf.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SBX SIM SDK interface
 */

#ifndef __SOC_SBX_SIM_
#define __SOC_SBX_SIM_



/*
 * Function:
 *    soc_sbx_caladan3_sim_block_encode_simple
 * Purpose
 *    Routine to encode a block, table info into text for messaging
 *    returns error status
 */
int soc_sbx_caladan3_sim_block_encode_simple(char *buffer, char *block, char *table, char *op);

/*
 * Function:
 *    soc_sbx_caladan3_sim_block_encode
 * Purpose
 *    Routine to encode a block, table info into text for messaging
 *    returns error status
 */
int soc_sbx_caladan3_sim_block_encode(char *buffer, char *block,
                                    char *table, char *op, uint32 addr);

/*
 * Function:
 *    soc_sbx_caladan3_sim_block_decode
 * Purpose
 *    Routine to decode a block, table info into text for messaging
 *    returns error status
 */
int soc_sbx_caladan3_sim_block_decode(char *buffer, char *block,
                                    char *table, char *op, uint32 *addr);
/*
 * Function:
 *    soc_sbx_caladan3_sim_field_encode
 * Purpose
 *    Routine to encode a {field value} pair into text for messaging
 *    returns error status
 */
int soc_sbx_caladan3_sim_field_encode(char *buffer, char *field,
                                    uint8 *value, int width);

/*
 * Function:
 *    soc_sbx_caladan3_sim_field_decode
 * Purpose
 *    Routine to decode and extract value from {field value} pair from a text message
 * message
 *    Returns error status
 */
int soc_sbx_caladan3_sim_field_decode(char *buffer, int size, char *field,
                                     uint8 *value, int width);

/*
 * Function:
 *    soc_sbx_caladan3_sim_sendrcv
 * Purpose
 *    Send a message to the sim server and get the response back
 *    No processing of the message happens here, its up to the client
 */
int soc_sbx_caladan3_sim_sendrcv(int unit, char *buffer, int *size);

/*
 * Function:
 *    soc_sbx_caladan3_sim_status_decode
 * Purpose
 *    Routine to decode a response status, from message
 *    returns error status
 */
int soc_sbx_caladan3_sim_status_decode(char *buffer, char *verb, int *parsedlen);

/*
 * Function:
 *    soc_sbx_caladan3_sim_verb_decode
 * Purpose
 *    Routine to decode a verb,value from message
 *    returns value and the length of message consumed in parsedlen
 */
int soc_sbx_caladan3_sim_verb_decode(char *buffer, char *verb, int *parsedlen);

/*
 * Function:
 *    soc_sbx_caladan3_sim_verb_encode
 * Purpose
 *    Routine to encode verb value into message if verb given 
 *    or just encode the value if no verb given
 *    returns value and length of message generated
 */
int
soc_sbx_caladan3_sim_verb_encode(char *buffer, char *verb, int value);

/*
 * Function:
 *   soc_sbx_caladan3_sim_split
 * Purpose
 *   Break a string of data into field,value pairs
 */
int
soc_sbx_caladan3_sim_split(char *buffer, int size, char**fields, char**values);

/*
 * Function:
 *    soc_sbx_caladan3_sim_keyword_encode
 * Purpose
 *    Routine to encode keyword value into message 
 */
int
soc_sbx_caladan3_sim_keyword_encode(char *buffer, char *keyword);

/*
 * Function:
 *    soc_sbx_caladan3_sim_keyword_decode
 * Purpose
 *    Routine to check keyword in message ,
 */
int
soc_sbx_caladan3_sim_keyword_decode(char *buffer, char *keyword);

/*
 *   Function
 *     sbx_caladan3_cop_policer_create_sim
 *   Purpose
 *      COP create a policer in simulation
 */
int 
soc_sbx_caladan3_cop_policer_create_sim(int unit,
					uint32 cop,
					uint32 segment,
					uint32 policer,
					soc_sbx_caladan3_cop_policer_config_t *config,
					uint32 *handle);

#endif
