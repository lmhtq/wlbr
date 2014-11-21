#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#define BUFLEN 2000

char buf[BUFLEN];

int main(int argc, char **argv)
{
	int i, skfd, n;
	struct ethhdr *eth;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int stat;
	char thismac[ETH_ALEN];

	char *ifname = *(++argv);

	if (2 != argc) {
		printf("Usage: %s netdevName\n", argv[0]);
		exit(1);
	}

	/* init */
	if(0>(skfd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))){
        perror("Create Error");
        exit(1);
    }

	memset(&sll, 0, sizeof(sll));
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, argv[1]);
	
	stat = ioctl(skfd, SIOCGIFINDEX, &ifr);
	if (-1 == stat) {
		printf("get dev index error\n");
		exit(1);
	}

	stat = ioctl(skfd, SIOCGIFHWADDR, &ifr);
	if (-1 == stat) {
		printf("get dev MAC addr error\n");
		exit(1);
	}
	memcpy(thismac,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    printf("MAC :%02X-%02X-%02X-%02X-%02X-%02X\n",
    	thismac[0],thismac[1],thismac[2],thismac[3],thismac[4],thismac[5]);

	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_ALL);

	bind(skfd, (struct sockaddr*)&sll, sizeof(struct sockaddr));
	while (1) {
		memset(buf, 0, BUFLEN);
		printf("run\n");
		recvfrom(skfd, buf, ETH_FRAME_LEN, 0, NULL, NULL);
		eth = (struct ethhdr*)buf;

		for (i = 0; i < ETH_ALEN; i++)
			printf("%02x-", eth->h_source[i]);
		printf("-------->");
		for (i = 0; i < ETH_ALEN; i++)
			printf("%02x-", eth->h_source[i]);
		printf("\n");
	}

	close(skfd);
	return 0;

}