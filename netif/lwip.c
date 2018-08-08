/**************************************************************************
*                                                                         *
*   AUTHOR      : ljp@bupt                                       *
*                                                                         *
**************************************************************************/

#include "tcpip.h"
#include "inet.h"
#include "dm9000if.h"

/* ********************************************************************* */
/* Global definitions */


/* ********************************************************************* */
/* File local definitions */


/* ********************************************************************* */
/* Local functions */

/* ********************************************************************* */
/* Global functions */

#define CONFIG_IP_UNITS 1	/*max interface*/

struct user_ip_config  {
	char* ipaddr;/*"0.0.0.0 for dhcp"*/
	char* gw;
	char* netmask;
	int inDefault;	/*defualt netif?*/
};
struct user_ip_config user_ip_configs[CONFIG_IP_UNITS] = 
{
/*	{"192.168.0.73", "192.168.0.42", "255.255.255.0", 1},*/
	{"10.105.243.73", "10.105.243.1", "255.255.255.0", 1},
//	{"192.168.1.73", "192.168.1.8", "255.255.255.0", 1},
};

void* tq2440_eth_load(struct dme_softc *sc, eth_config_t *config);
void tq2440_dm9000_init(void* arg);
void tq2440_dm9000_stop(void* arg);
void tq2440_dm9000_start(void* arg); 
void tq2440_dm9000_int_clear(void* arg);

#define INT_LVL_EINT_4_7   4

eth_config_t eth_configs[] = 
{
	{
		tq2440_eth_load,
		INT_LVL_EINT_4_7,
		{'L', 'W', 'I', 'P', '-',0x01},
		0x20000000,
		tq2440_dm9000_init,
		tq2440_dm9000_stop,
		tq2440_dm9000_start,
		tq2440_dm9000_int_clear,
	},
	{0, 0, {0}, 0, 0, 0, 0}
};

struct dme_softc dm9k_drvs[CONFIG_IP_UNITS];
struct netif     dm9k_netif[CONFIG_IP_UNITS];

/*init driver*/
void netif_drv_init(void)
{
	struct ip_addr gw;
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	int 		count;
	eth_config_t* eth_config_entry;
	
    for (count = 0, eth_config_entry = eth_configs; eth_config_entry->eth_load != (0)  ;
         eth_config_entry++, count++)
    {
		void *dm9k_drv;
		struct netif *dm9k_if;
		struct user_ip_config *ip_config;

		dm9k_drv = &dm9k_drvs[count];
    	dm9k_if = &dm9k_netif[count];
		ip_config = &user_ip_configs[count];
		
		 /*1.board dependent init*/
		dm9k_drv = eth_config_entry->eth_load(dm9k_drv, eth_config_entry);
		if (dm9k_drv == (0))
			continue;
		/*set ip*/
		gw.addr = inet_addr(ip_config->gw);
		ipaddr.addr = inet_addr(ip_config->ipaddr);
		netmask.addr = inet_addr(ip_config->netmask);
		/*2.init netif*/
		netif_add(dm9k_if, &ipaddr, &netmask, &gw, dm9k_drv, dm9000if_init, tcpip_input);
		/*	Registers the default network interface.*/
		if(ip_config->inDefault)
			netif_set_default(dm9k_if);

#if LWIP_DHCP
		if (ipaddr.addr == 0)
		{
			/*	Creates a new DHCP client for this interface on the first call.
			Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
			the predefined regular intervals after starting the client.
			You can peek in the netif->dhcp struct for the actual DHCP status.*/
			dhcp_start(dm9k_if);
		}
#endif /* LWIP_DHCP */

#if LWIP_NETIF_STATUS_CALLBACK
		netif_set_status_callback(dm9k_if, dm9000if_status_callback);
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
		netif_set_link_callback(dm9k_if, dm9000if_link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_NETIF_REMOVE_CALLBACK
		netif_set_remove_callback(dm9k_if, dm9000if_stop);
#endif /* LWIP_NETIF_REMOVE_CALLBACK */
	
		/*3.start netif, When the netif is fully configured this function must be called.*/
		netif_set_up(dm9k_if);
  	}

#if 0
	/*2.lwip netif init*/
#if LWIP_DHCP
		{
		  IP4_ADDR(&gw, 0,0,0,0);
		  IP4_ADDR(&ipaddr, 0,0,0,0);
		  IP4_ADDR(&netmask, 0,0,0,0);
	  
	  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
				struct ip_addr *netmask, struct ip_addr *gw,
				void *state, err_t (* init)(struct netif *netif),
				err_t (* input)(struct pbuf *p, struct netif *netif))
		
	   Adds your network interface to the netif_list. Allocate a struct
	  netif and pass a pointer to this structure as the first argument.
	  Give pointers to cleared ip_addr structures when using DHCP,
	  or fill them with sane numbers otherwise. The state pointer may be NULL.
	
	  The init function pointer must point to a initialization function for
	  your ethernet netif interface. The following code illustrates it's use.*/
		  netif_add(&dm9k_netif[0], &ipaddr, &netmask, &gw, NULL, dm9000if_init, tcpip_input);
		  /*  Registers the default network interface.*/
		  netif_set_default(&dm9k_netif[0]);
		  /*  Creates a new DHCP client for this interface on the first call.
		  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
		  the predefined regular intervals after starting the client.
		  You can peek in the netif->dhcp struct for the actual DHCP status.*/
		  dhcp_start(&dm9k_netif[0]);
		}
#else /* LWIP_DHCP */
		/*default ip*/
		IP4_ADDR(&gw, 192,168,0,42);
		IP4_ADDR(&ipaddr, 192,168,0,73);
		IP4_ADDR(&netmask, 255,255,255,0);
	
		netif_add(&dm9k_netif[0],&ipaddr, &netmask, &gw, NULL, dm9000if_init, tcpip_input);
		netif_set_default(&dm9k_netif[0]);
#endif /* LWIP_DHCP */
	
#if LWIP_NETIF_STATUS_CALLBACK
	  netif_set_status_callback(&dm9k_netif[0], status_callback);
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
	  netif_set_link_callback(&dm9k_netif[0], link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_NETIF_REMOVE_CALLBACK
	  netif_set_remove_callback(&dm9k_netif[0], dm9000if_stop);
#endif /* LWIP_NETIF_REMOVE_CALLBACK */
	
		/*	When the netif is fully configured this function must be called.*/
		netif_set_up(&dm9k_netif[0]);

	/*3.start*/
#endif
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void lwip_main_task(void * arg)
{
  LWIP_UNUSED_ARG(arg);

  /*1.init lwip*/
  tcpip_init(NULL, NULL);

  printf("Lwip TCP/IP Core initialized.\n");

  /*2.init driver*/
  netif_drv_init();
  printf("Lwip Eth Driver (DM9000) initialized.\n");

	//led_set(1, 1, 0, 0);
/*
	httpd_init();
	printf("httpd started on port 80.\n");
*/
#if 1
	/*3.init app*/
#if 1
#if LWIP_UDP  
	udpecho_init();
	printf("udpecho_thread started on port 7.\n");
#endif 
#if LWIP_TCP  
	tcpecho_init();
	printf("tcpecho_thread started on port 7.\n");

/*
	shell_init();
	printf("shell_thread started on port 23.\n");
*/
	//led_set(1, 0, 0, 0);

	httpd_init();
	printf("httpd started on port 80.\n");
//	http_server_netconn_init();
#endif
#endif

#endif

	//led_set(0, 0, 0, 0);

	printf("Lwip TCP/IP Applications initialized.\n");
	/*	sys_timeout(5000, tcp_debug_timeout, NULL);*/
}

