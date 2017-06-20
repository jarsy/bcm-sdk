/*
 * $Id: simsock.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SBX SIM Socket client interface
 * 
 */

#include <shared/bsl.h>

#ifdef VXWORKS
#include <vxWorks.h>
#include <sockLib.h>
#include <selectLib.h>
#include <errnoLib.h>
#else
#include <unistd.h>
#endif

#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef VXWORKS
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#endif

#include <ctype.h>


#include <sal/core/sync.h>
#include <sal/appl/io.h>
#include <soc/defs.h>
#include <soc/debug.h>
#include <soc/error.h>

#include "simsock.h"

/*
 *  Send "n" bytes to a descriptor.
 */
int sendn(int fd, char *ptr, int nbytes)
{
#ifdef BCM_CALADAN3_SIM
    int nleft, nwritten;
    nleft = nbytes;
    while (nleft > 0){
	nwritten = write(fd, ptr, nleft);
	if (nwritten <= 0)
	    return(nwritten);               
	nleft -= nwritten;
	ptr = ptr + nwritten;
    }
    return(nbytes - nleft);
#else
    return SOC_E_UNAVAIL;
#endif
}


/*
 *  Receive "n" bytes from a descriptor.
 */
int rcvn(int fd, char* ptr, int nbytes)
{
#ifdef BCM_CALADAN3_SIM
    int nleft, nread;
    int dataok = 0;
    nleft = nbytes;
    while (nleft > 0) {
	nread = read(fd, ptr, nleft);
	if (nread < 0 && !dataok)
	    return(nread);          	/* error, return < 0 */
	else if (dataok)
	    break;                  	/* EOF */
	nleft -= nread;
        dataok = 1;
	ptr = (char *)ptr + nread;
    }
    return(nbytes - nleft);         	/* return >= 0 */
#else
    return SOC_E_UNAVAIL;
#endif
}


/*
 * Wait for and get a response from the sim server
 * Parameters:
 *  sockfd -socket file descriptor.
 * Returns:
 *	-1 on error 
 */
int get_response(int sockfd, struct timeval* tv, char* buffer, int *bsize)
{
#ifdef BCM_CALADAN3_SIM
    int size;
    fd_set read_vect;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("get_response: Waiting for sockfd=%d\n"),
                 sockfd));

    assert(sockfd > 2);	

    /* Setup bitmap for read notification ... */
    FD_ZERO(&read_vect);
    FD_SET(sockfd, &read_vect);

    while (1) {
        if(select(sockfd+1,&read_vect,(fd_set*)0x0,(fd_set*)0x0,tv)<0){
            if (errno == EINTR) {
                /* 
                 * Interrupted by a signal such as a GPROF alarm so
                 * restart the call
                 */
                continue;
            }
            perror("get_response: select error");
            goto fail;
        } else {
            break;
        }
    }
    /*
     * Data ready to be read.
     */
    if( FD_ISSET(sockfd, &read_vect)){

        if ((*bsize > 0) && (*bsize < 8192)) {
	    size = rcvn(sockfd, buffer, *bsize);
        } else {
	    size = rcvn(sockfd, buffer, 8192);
        }
        if (*bsize && size < *bsize) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META("get_response: failed, got %d bytes expected %d bytes"),
                         size, *bsize));
	    goto fail;
	} 
        *bsize = size;
	return SOC_E_NONE;
    }

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("get_response: Timeout no response on sockfd=%d\n"),
               sockfd));
    return SOC_E_TIMEOUT;	/* Time expire with no response */

 fail:
    return SOC_E_PARAM;
#else
    return SOC_E_UNAVAIL;
#endif
}



/*
 * Connection management 
 */
typedef struct connection_info_s  {
    int valid;
    int sockfd;
    struct addrinfo *serverinfo;
} connection_info_t;

connection_info_t sbx_sim_conn_info[SOC_MAX_NUM_DEVICES];

#define UNIT_VALID(u) (u < SOC_MAX_NUM_DEVICES) ? 1 : 0
#define IS_CONNECTED(u) (sbx_sim_conn_info[(u)].valid)

/*
 * Connect to a sim server located at serverip and listening on port
 */
int 
soc_sbx_caladan3_sim_connect(int unit, char* port, char* serverip)
{
#ifdef BCM_CALADAN3_SIM
    int rv = SOC_E_NONE;
    int sockfd = -1;
    int one = 1;
    struct addrinfo params;
    struct addrinfo *server, *serverp;

    memset(&params, 0, sizeof(params));
    params.ai_family = AF_UNSPEC;
    params.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(serverip, port, &params, &server)) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_sim_connect: getaddrinfo failed: %s\n"), 
                   gai_strerror(rv)));
        return SOC_E_PARAM;
    }

    /*
     * We may get ipv4 as well as ipv6 address info
     * connect on whichever reaches the server 
     */
    for(serverp = server; serverp != NULL; serverp = serverp->ai_next) {
        sockfd = socket(serverp->ai_family, 
                        serverp->ai_socktype,
                        serverp->ai_protocol);
        if (sockfd < 0) {
            continue;
        }

        if (connect(sockfd, serverp->ai_addr, serverp->ai_addrlen) == -1) {
            close(sockfd);
            sockfd=-1;
            continue;
        }

        break;
    }
    
    /* Did we fail to connect */
    if (sockfd < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("soc_sbx_caladan3_sim_connect: connect failed: %d\n"),
                   rv));
		freeaddrinfo(server);		   
        return SOC_E_PARAM; 
    }

    /* do we need this */
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
		   (char *)&one, sizeof (one)) < 0) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("soc_sbx_caladan3_sim_connect: setsockopt failed: %d\n"),
               rv));
        
    } 
    /* Set as non-blocking */
    /* coverity[unchecked_value] */
    (void)fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK); 

    sbx_sim_conn_info[unit].valid = 1;
    sbx_sim_conn_info[unit].sockfd = sockfd;

    freeaddrinfo(server);
    return SOC_E_NONE;
#else
    return SOC_E_UNAVAIL;
#endif
}

int 
soc_sbx_caladan3_sim_disconnect(int unit) 
{
#ifdef BCM_CALADAN3_SIM
    if (IS_CONNECTED(unit)) {
        close(sbx_sim_conn_info[unit].sockfd);
        freeaddrinfo(sbx_sim_conn_info[unit].serverinfo);
    } 
    return SOC_E_NONE;
#else
    return SOC_E_UNAVAIL;
#endif
}

int 
soc_sbx_caladan3_sim_uninit(int unit) 
{
#ifdef BCM_CALADAN3_SIM
    if (!UNIT_VALID(unit)) {
        return SOC_E_PARAM;
    }
    if (IS_CONNECTED(unit)) {
        soc_sbx_caladan3_sim_disconnect(unit);
    }
    return SOC_E_NONE;
#else
    return SOC_E_UNAVAIL;
#endif
}

int 
soc_sbx_caladan3_sim_init(int unit)
{
#ifdef BCM_CALADAN3_SIM
    char *s, *host;
    char* port;
    char tmp[128];
    int rv, p = 0;

    if (!UNIT_VALID(unit)) {
       return SOC_E_PARAM;
    }
    if (IS_CONNECTED(unit)) {
        soc_sbx_caladan3_sim_uninit(unit);
    }
    memset(&sbx_sim_conn_info[unit], 0, sizeof(connection_info_t));

 
    s = getenv("SOC_TARGET_PORT");
    if (!s) {
      /* coverity[secure_coding] */
	sprintf(tmp, "SOC_TARGET_PORT%d", unit);
	s = getenv(tmp);
    }
    if (!s) {
       port = SOC_SBX_SIM_DEFAULT_PORT;     
    } else {
       /*
        * SOC_TARGET_PORT + 1 ==> TCL
        * SOC_TARGET_PORT + 2 ==> SDK 
        */
       p = atoi(s) + 2;
       /* coverity[secure_coding] */
       sprintf(tmp, "%d", p);
       port = tmp;
    } 

    /* Assume localhost for now, export the host, port from PLI later */
    host = "localhost";
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("SbxSim using host: %s port %s\n"),
                 host, port));
    rv = soc_sbx_caladan3_sim_connect(unit, port, host); 
    return rv; 
#else
    return SOC_E_UNAVAIL;
#endif

}

/*
 * Send a message to the server, connect if not connected 
 * Parameters:
 *  unit - sim server dev no
 *  buffer - message
 *  bsize - size of message
 * Returns:
 */
int 
send_message(int unit, char* buffer, int bsize)
{
#ifdef BCM_CALADAN3_SIM
    int rv = SOC_E_NONE;

    if (!UNIT_VALID(unit)) {
       return SOC_E_PARAM;
    }
    if (!IS_CONNECTED(unit)) {
        rv = soc_sbx_caladan3_sim_init(unit);
    }
    if (SOC_SUCCESS(rv)) {
        rv = sendn(sbx_sim_conn_info[unit].sockfd, buffer, bsize);
        if (rv != bsize) {
           /* Not all data sent */
           return SOC_E_INTERNAL;
        }
    } 
    return rv;
#else
    return SOC_E_UNAVAIL;
#endif
}

/*
 * Get a message from the server
 * Parameters:
 *  unit - sim server dev no
 *  buffer - buffer to write response from server
 *  size - expected size of response
 */
int 
recv_message(int unit, char* buffer, int *bsize)
{
#ifdef BCM_CALADAN3_SIM
    int rv = SOC_E_NONE;
    struct timeval tv = {3,0}; /* give select 1 sec to timeout */

    if (!UNIT_VALID(unit)) {
       return SOC_E_PARAM;
    }
    if (!IS_CONNECTED(unit)) {
        rv = soc_sbx_caladan3_sim_init(unit);
    }
    if (SOC_SUCCESS(rv)) {
        rv = get_response(sbx_sim_conn_info[unit].sockfd, &tv, buffer, bsize);
        if (rv != SOC_E_NONE) {
           /* Incomplete data received */
           return rv;
        }
    } 
    return SOC_E_NONE;
#else
    return SOC_E_UNAVAIL;
#endif
}


