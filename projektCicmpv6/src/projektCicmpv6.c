#include <stdio.h>
#include <stdlib.h>
#include "lib.h"
#include <dlfcn.h>
#include <netinet/in.h>
#include <netinet/ip6.h>

int main (void) {

	char pakiety [32];
	int *licz = malloc (sizeof (int));

	void * IPv6_biblioteka = malloc(sizeof (void)); // rezerwacja pamieci dla biblioteki
	void * ICMPv6_biblioteka = malloc(sizeof (void)); // rezerwacja pamieci dla biblioteki

	struct ip6_hdr *ip6_naglowek; //powołanie wskaznika na sturkture naglowka
	unsigned char *datagram;

	unsigned char * (*ipv6) () = malloc (sizeof (unsigned short *));
	void (*icmp6) (unsigned char *) = malloc (sizeof (void));

	datagram = malloc (sizeof (unsigned char *)); //rezerwacja pamiecie

	IPv6_biblioteka = LoadIPV6LIB(); //ładowanie biblioteki dynamicznej dla IPv6
	ipv6 = dlsym(IPv6_biblioteka, "CREATEIPv6");
	datagram = (*ipv6)();
	ip6_naglowek = (struct ip6_hdr *) datagram;

	if ( ip6_naglowek->ip6_ctlun.ip6_un1.ip6_un1_nxt == IPPROTO_ICMP){
		ICMPv6_biblioteka = LoadICMPV6LIB(); //ładowanie biblioteki dynamicznej dla ICMPv6
		icmp6 = dlsym(ICMPv6_biblioteka, "CREATEICMPv6");
		(*icmp6) (datagram);
	}

	printf ("Jak dużo pakietów chcesz wysłać ???: ");
	fgets (pakiety, 32, stdin);
	*licz = atoi(pakiety);
	LoadLIST(licz, datagram);
	SendPACKET(datagram);
	EXIT_SUCCESS;
}
