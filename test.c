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
#include <sys/time.h>

#include <linux/ieee80211.h>

#define BUFLEN 2000
struct timeval tv;
char buf[BUFLEN];

int main(int argc, char **argv)
{
	int i, skfd, n;
	struct 'a' *eth;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int stat;
	unsigned char thismac[ETH_ALEN];
	unsigned char destmac[ETH_ALEN];
	unsigned char srcmac[ETH_ALEN];

	if (2 != argc) {
		printf("Usage: %s netdevName\n", argv[0]);
		exit(1);
	}

	/* init */
	if(0>(skfd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))){
        perror("Create Error");
        exit(1);
    }

	bzero(&sll, sizeof(sll));
	bzero(&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, argv[1]);
	
	if (-1 == ioctl(skfd, SIOCGIFINDEX, &ifr)) {
		printf("get dev index error\n");
		exit(1);
	}
	if (-1 == ioctl(skfd, SIOCGIFHWADDR, &ifr)) {
		printf("get dev MAC addr error\n");
		exit(1);
	}
	if (-1 == ioctl(skfd, SIOCGIFFLAGS, &ifr)) {
		printf("get dev FLAGS error\n");
		exit(1);
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if (-1 == ioctl(skfd, SIOCGIFFLAGS, &ifr)) {
		printf("get dev FLAGS error\n");
		exit(1);
	}
	memcpy(thismac,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
    printf("MAC :%02x-%02x-%02x-%02x-%02x-%02x\n",
    	thismac[0],thismac[1],thismac[2],thismac[3],thismac[4],thismac[5]);

	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_ALL);
	
	bind(skfd, (struct sockaddr*)&sll, sizeof(struct sockaddr));
	while (1) {
		memset(buf, 0, BUFLEN);
		recvfrom(skfd, buf, ETH_FRAME_LEN, 0, NULL, NULL);
		gettimeofday(&tv,NULL);
		printf("%u.%u: ",tv.tv_sec,tv.tv_usec);
		eth = (struct ethhdr*)buf;

		for (i = 0; i < ETH_ALEN; i++)
			printf("%02x-", eth->h_source[i]);
		printf("-------->");
		for (i = 0; i < ETH_ALEN; i++)
			printf("%02x-", eth->h_dest[i]);
		memcpy(destmac, eth->h_dest, ETH_ALEN);
		memcpy(srcmac, eth->h_source, ETH_ALEN);
		if ( 0 != strcmp(thismac, destmac) && 0 != strcmp(thismac, srcmac) ) {
			printf("Catched!!\n");
		}
		printf("\n");
	}

	close(skfd);
	return 0;

}