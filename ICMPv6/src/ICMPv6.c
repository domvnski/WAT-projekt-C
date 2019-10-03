#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

void CREATEICMPv6 (unsigned char *datagram) {

	char input [32];
	struct ip6_hdr *iphdr = (struct ip6_hdr*) datagram;
	struct icmp6_hdr *icmphdr = (struct icmp6_hdr *) (datagram + sizeof (struct ip6_hdr));


	printf ("Poadj typ ICMP (domyślnie ustawiony jest 128 - PING): ");

	fgets (input, 32, stdin);
	if (atoi(input)== 0)
			icmphdr->icmp6_type = 128;
	else
			icmphdr->icmp6_type = atoi (input);


	printf ("ICMP kod (domyślnie ustawiona wartość to 0):");

	fgets (input, 32, stdin);
	if (atoi(input)== 0)
			icmphdr->icmp6_code = 0;
	else
			icmphdr->icmp6_code = atoi (input);


	printf ("ICMP identyfikator:");
		fgets (input, 32, stdin);
		if (atoi(input)== 0)
			icmphdr->icmp6_dataun.icmp6_un_data16 [0] = htons (12345);
		else
			icmphdr->icmp6_dataun.icmp6_un_data16 [0] = htons (atoi (input));

		printf ("Podaj numer sekwencji ICMP (domyślnie ustawiona wartość to 0):");
		fgets (input, 32, stdin);
		if (atoi(input)== 0)
			icmphdr->icmp6_dataun.icmp6_un_data16 [1] = htons (0);
		else
			icmphdr->icmp6_dataun.icmp6_un_data16 [1] = htons (atoi (input));

		iphdr->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(sizeof (struct icmp6_hdr));

		icmphdr->icmp6_cksum = 0;

}
