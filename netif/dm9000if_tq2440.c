/*-
 * Copyright (c) 2012 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Paul Fleischer <paul@xpg.dk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/snmp.h"
#include "netif/etharp.h"

#include "dm9000var.h"
#include "dm9000reg.h"

#include "dm9000if.h"


#define IFNAME0 'd'
#define IFNAME1 'm'
#define HOSTNAME 'lwip'




static int dm9000if_intr(void *arg)
{
	struct dme_softc *sc = arg;

	//led_flip(1);

	/*clear intr*/
	if(sc->eth_config->int_clear)
		sc->eth_config->int_clear(sc);
	sys_sem_signal(&sc->sem_signal);
}

/**
 *
 *
 * @return error code
 * - ERR_OK: packet transferred to hardware
 * - ERR_CONN: no link or link failure
 * - ERR_IF: could not transfer to link (hardware buffer full?)
 */
static err_t dm9000if_output(struct netif *netif, struct pbuf *p)
{
	struct dme_softc *sc = (struct dme_softc *)netif->state;

	if(!p)
		return ERR_BUF;
	
#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

		/*put in send q*/
		sys_sem_wait(&sc->sem_prot);
		pbuf_ref(p);
		if (list_push(sc->if_snd, p) == 0) {
		  pbuf_free(p);
	
		  LWIP_DEBUGF(DM9000IF_DEBUG, ("sc_output: drop\n"));	
		  LINK_STATS_INC(link.drop);
	
		} else {
		  LWIP_DEBUGF(DM9000IF_DEBUG, ("sc_output: on list\n"));
		}
		sys_sem_signal(&sc->sem_prot);

	/*try send pbuf in q*/
	dme_start_output(netif);

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	return ERR_OK;
}

void dm9000if_input_handler(struct netif *netif)
{
	struct dme_softc *sc;

	sc = (struct dme_softc *)netif->state;

	/*clear intr
	if(sc->eth_config->int_clear)
		sc->eth_config->int_clear(sc);
	*/
	/*service intr*/
	dme_intr(sc);
}

/*-----------------------------------------------------------------------------------*/
static void 
dm9000if_thread(void *arg)
{
  struct netif *netif;
  struct dme_softc *sc;

  LWIP_DEBUGF(DM9000IF_DEBUG, ("dm9000if_thread: started.\n"));

  netif = (struct netif *)arg;
  sc = (struct dme_softc *)netif->state;

  while (1) {
    sys_sem_wait(&sc->sem_signal);
    dm9000if_input_handler(netif);
  }
}

/**
 * Initialize the DM9000 Ethernet MAC/PHY and its device driver.
 *
 * @param netif The lwIP network interface data structure belonging to this device.
 *
 */
err_t dm9000if_init(struct netif *netif)
{
  struct dme_softc *sc = (struct dme_softc *)netif->state;

  /* 1.initialize lwip network interface ... */
#if LWIP_SNMP
  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100*1000*1000);/*6, 100Mbps*/
#endif

	/* administrative details */
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
#if LWIP_NETIF_HOSTNAME  
	  netif.hostname = HOSTNAME;
#endif

  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = dm9000if_output;
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  /* set MAC hardware address */
  memcpy(netif->hwaddr, sc->eth_config->enetAddr, sizeof(sc->eth_config->enetAddr));
  /* maximum transfer unit */
  netif->mtu = 1500;
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /*2.initialize the hardware chip driver*/
  sc->if_snd = list_new(DM9000IF_QUEUELEN);
  if(sys_sem_new(&sc->sem_signal, 0) != ERR_OK) {
    LWIP_ASSERT("Failed to create semaphore", 0);
  }
  if(sys_sem_new(&sc->sem_prot, 1) != ERR_OK) {
    LWIP_ASSERT("Failed to create semaphore", 0);
  }
  /*for intr handle*/
  sys_thread_new("dm9000if_thread", dm9000if_thread, netif, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);/* use defualt size*/

#if (DM9000_STATS > 0)
	/* number of interrupt service routine calls */
	sc->interrupts = 0;
	sc->missed = 0;
	sc->dropped = 0;
	sc->collisions = 0;
	sc->sentpackets = 0;
	sc->sentbytes = 0;
#endif
  sc->sc_dev = netif;
  dme_attach(sc, sc->eth_config->enetAddr);
  dme_init(netif);

  /*3.start*/
  if(sc->eth_config->board_start)
  	sc->eth_config->board_start(sc);
  
  return ERR_OK;
}

void dm9000if_stop(struct netif *netif)
{
	struct dme_softc *sc = (struct dme_softc *)netif->state;

	/*disable int*/
	if(sc->eth_config->board_stop)
		sc->eth_config->board_stop(sc);
	/*remove ISR?*/
	/*stop driver*/
	dme_stop(netif, 1);
	/*free mem*/
	list_delete(sc->if_snd);
	sys_sem_free(&sc->sem_signal);
	sys_sem_free(&sc->sem_prot);
}


#if LWIP_NETIF_STATUS_CALLBACK
void dm9000if_status_callback(struct netif *netif)
{
  if (netif_is_up(netif)) {
    printf("status_callback==UP, local interface IP is %s\n", ip_ntoa(&netif->ip_addr));
  } else {
    printf("status_callback==DOWN\n");
  }
}
#endif /* LWIP_NETIF_STATUS_CALLBACK */

#if LWIP_NETIF_LINK_CALLBACK
void dm9000if_link_callback(struct netif *netif)
{
  if (netif_is_link_up(netif)) {
    printf("link_callback==UP\n");
#if USE_DHCP
    if (netif->dhcp != NULL) {
      dhcp_renew(netif);
    }
#endif /* USE_DHCP */
  } else {
    printf("link_callback==DOWN\n");
  }
}
#endif /* LWIP_NETIF_LINK_CALLBACK */


/***********************************************************************/

#define INT_LVL_EINT_4_7   4
#define BIT_EINT4_7		(0x1<<4)

#define rGPFCON    (*(volatile unsigned *)0x56000050)	//Port F control
#define rEXTINT0   (*(volatile unsigned *)0x56000088)	//External interrupt control register 0
#define rEINTMASK  (*(volatile unsigned *)0x560000a4)	//External interrupt mask
#define rEINTPEND  (*(volatile unsigned *)0x560000a8)	//External interrupt pending
#define rSRCPND     (*(volatile unsigned *)0x4a000000)	//Interrupt request status
#define rINTPND     (*(volatile unsigned *)0x4a000010)	//Interrupt request status

#define	ClearPending(bit) {\
			rSRCPND = bit;\
			rINTPND = bit;\
			rINTPND;\
		}		

/*@for tq2440*/
void tq2440_dm9000_init(void* arg)
{
	/*gpio*/
	rGPFCON = (rGPFCON & (~(0x03<<14))) | (0x02<<14);
	/*int*/
	rEXTINT0 = (rEXTINT0 & (~(0x07<<28))) | (0x04<<28);
	rEINTMASK = rEINTMASK & (~(0x01<<7));
	rSRCPND = 1<<INT_LVL_EINT_4_7;
	rINTPND = 1<<INT_LVL_EINT_4_7;
}   
   
void tq2440_dm9000_stop(void* arg)
{   
	/*disable intr*/
	rEINTMASK |= (0x01<<7);
}
void tq2440_dm9000_start(void* arg) 
{
	struct dme_softc *sc = arg;
    BSP_Int_Enable(sc->eth_config->ivec);   
}

/*board specific configs, return dme_softc*/
void* tq2440_eth_load(struct dme_softc *sc, eth_config_t *config)
{
	/*ether config*/
	sc->eth_config = config;

	/*mem address*/
	sc->sc_iot = 0;/*not used*/
	sc->sc_ioh = config->devAdrs;/*base address of dm9000 at tq2440 board*/
	sc->dme_io = 0x0;/*io port offset*/
	sc->dme_data = 0x4;/*data port offset*/

	/*int*/
	BSP_Int_Connect(sc->eth_config->ivec, dm9000if_intr, sc);
	
	/*gpio*/
	if(sc->eth_config->board_init)
		sc->eth_config->board_init(sc);

	return sc;
}

void tq2440_dm9000_int_clear(void* arg)
{
	rEINTPEND = 0x1<<7;/*EINT7 for dm9000*//*EINT0-1-2-4=keypad, EINT19=camera*/
	ClearPending(BIT_EINT4_7);/*clear status for int4-7*/
}


