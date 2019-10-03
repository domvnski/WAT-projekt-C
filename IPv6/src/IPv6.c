#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip6.h>

unsigned char * CREATEIPv6(){

	unsigned char *datagram = malloc (4096);
	struct ip6_hdr *iphdr = (struct ip6_hdr *) datagram;
	char dane[32];

	printf ("START\n");
	printf ("IP WERSJA: 6\n");
	printf ("\n");

	iphdr->ip6_ctlun.ip6_un1.ip6_un1_flow = htonl ((6 << 28) | (0 << 20) | 0);

	printf ("Podaj limit skoków (Domyślnie to 255) : ");

	fgets (dane, 32, stdin);

	if (atoi(dane)== 0){
		iphdr->ip6_ctlun.ip6_un1.ip6_un1_hlim = 255;
	}
	else{
		iphdr->ip6_ctlun.ip6_un1.ip6_un1_hlim = atoi (dane);
	}

	printf ("Payload zostanie obliczony\n");

	iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt = IPPROTO_ICMP;

	iphdr->ip6_ctlun.ip6_un1.ip6_un1_plen = htons((short)strlen(dane)-1);

	return datagram;
}
