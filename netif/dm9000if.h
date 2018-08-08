/*
 * 
 * Author: ljp@bupt
 *
 */
 
#ifndef __NETIF_DM9000IF_H__
#define __NETIF_DM9000IF_H__

#include "lwip/netif.h"
#include "dm9000var.h"

err_t dm9000if_init(struct netif *netif);
void dm9000if_stop(struct netif *netif);
void dm9000if_status_callback(struct netif *netif);
void dm9000if_link_callback(struct netif *netif);

typedef struct eth_config {
	void* (*eth_load)(struct dme_softc *sc, struct eth_config *config);
    int     ivec;                   /* interrupt vector */
    unsigned char	enetAddr[6];		/* ethernet address */
    unsigned int  devAdrs;                /* device structure address */
	void (*board_init)(void *arg);	/*hardware chip init, eg:gpio*/
	void (*board_stop)(void *arg);	/*hardware chip stop, eg:intr*/
	void (*board_start)(void *arg);	/*hardware chip init, eg:intr*/
	void (*int_clear)(void*arg);	/*clear int*/
}eth_config_t;


#define DM9000IF_QUEUELEN 6

#endif /* __NETIF_DM9000IF_H__ */

