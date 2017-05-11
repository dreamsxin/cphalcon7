
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          Didier Bertrand <diblibre@gmail.com>                          |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "kernel/io/sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel/io/networks.h"
#include "kernel/io/threads.h"

#define ADDRESS_SIZE 32

void phalcon_io_assign_networks(void *tpi, char *networks_list )
{
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *) tpi;		// THREAD_INFO and phalcon_io_poller_info have the same first elements
	char *pn, *ps;
	int b1, b2, b3, b4;
	static char* masks[] = { "0.0.0.0", "255.0.0.0", "255.255.0.0", "255.255.255.0", "255.255.255.255" };
	char address[ADDRESS_SIZE];

	if (networks_list==NULL || *networks_list=='\0')
		return;

	if (ti->networks_info!=NULL)
		phalcon_io_release_networks (tpi);

	// allocate networks
	pn=networks_list;
	ti->networks_size = 0;
	while ((pn = strchr (pn, ':')) != NULL) {
		++pn;
		++ti->networks_size;
	}
	ti->networks_info = calloc (ti->networks_size+1, sizeof(phalcon_io_network_info));

	pn=networks_list;
	ti->networks_size = 0;
	do {
		ps = strchr (pn, ':');
		if (ps != NULL) {
			strncpy (address, pn, ps-pn);
			++ ps;
		}
		else
			strcpy (address, pn);
		pn = ps;

		b1 = b2 = b3 = b4 = 0;
		int parts = sscanf (address, "%d.%d.%d.%d", &b1, &b2, &b3, &b4);
		if (parts<0)
			parts = 0;				// adresse vide.
		snprintf (address, ADDRESS_SIZE, "%d.%d.%d.%d", b1, b2, b3, b4);
		phalcon_io_network_info *ni = &((phalcon_io_network_info *)ti->networks_info)[ti->networks_size];

		inet_aton(address, &ni->addr);
		inet_aton(masks[parts], &ni->mask);
		++ti->networks_size;

		pn = ps;					// passe aux autres r�seaux
	} while (pn != NULL);
}

void phalcon_io_release_networks ( void *tpi )
{
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *) tpi;		// THREAD_INFO and phalcon_io_poller_info have the save 5 first elements
	if (ti->networks_info)
		free(ti->networks_info);
	ti->networks_info = NULL;
	ti->networks_size = 0;
}

int phalcon_io_validate_network ( void *tpi, struct sockaddr *client_addr, char *address_string )
{
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *) tpi;		// THREAD_INFO and phalcon_io_poller_info have the save 5 first elements
	int valid = PHALCON_IO_FALSE, pn;
	struct sockaddr_in *client_addr_in = (struct sockaddr_in *) client_addr;

	if (ti->networks_info==NULL)
		return PHALCON_IO_TRUE;

	for (pn=0; pn<ti->networks_size; pn++) {
		phalcon_io_network_info *ni = &((phalcon_io_network_info *)ti->networks_info)[pn];

		if (ni->addr.s_addr == (client_addr_in->sin_addr.s_addr & ni->mask.s_addr) )
			valid = PHALCON_IO_TRUE;
	}
	if (!valid) {
		strcpy (address_string, inet_ntoa(client_addr_in->sin_addr));
		return PHALCON_IO_FALSE;
	}
	return PHALCON_IO_TRUE;
}
