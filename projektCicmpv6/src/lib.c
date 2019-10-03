#include <dlfcn.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <net/if.h>
#include "lib.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <errno.h>

struct Node *glowa = NULL;	//początek listy

uint16_t icmp6_checksum (struct ip6_hdr *iphdr, struct icmp6_hdr *icmp6hdr);
uint16_t checksum (uint16_t *addr, int len);

//funkcja do wczytania biblioteki IPv6
void * LoadIPV6LIB(){

	void *ip6_biblioteka = malloc ( sizeof (void ) );	//do zwrocenia wczytanej biblioteki

	ip6_biblioteka = dlopen("/home/mati/eclipse-workspace/IPv6/src/IPv6.so", RTLD_LAZY);
		if (!ip6_biblioteka){
			printf ("Blad otwarcia biblioteki ipv6.\n");
			return 0;
		}
		else{
			return ip6_biblioteka;
		}
}

//funkcja do wczytania biblioteki ICMP
void * LoadICMPV6LIB(){

	void *icmp6_bibioteka = malloc ( sizeof (void ) );	//do zwrocenia wczytanej biblioteki

	icmp6_bibioteka = dlopen("/home/mati/eclipse-workspace/ICMPv6/src/ICMPv6.so", RTLD_LAZY);
		if (!icmp6_bibioteka){
			printf ("Blad otwarcia biblioteki icmp.\n");
			return 0;
		}
		else{
			return icmp6_bibioteka;
		}
}

void SendPACKET ( unsigned char *datagram ){
	  int i, status, frame_length, sd, bytes;
	  char *interface, *src_ip, *dst_ip, cmd [30], input [128];
	  struct ip6_hdr *iphdr;
	  uint8_t *src_mac, *dst_mac;
	  struct addrinfo hints;
	  struct icmp6_hdr *naglowek_icmp = (struct icmp6_hdr *)(datagram + sizeof (struct ip6_hdr));
	  struct sockaddr_ll device;
	  struct ifreq ifr;
	  char str[80];

	  iphdr = (struct ip6_hdr *) datagram;
	  src_mac = malloc (6);
	  dst_mac = malloc (6);
	  interface = malloc (INET6_ADDRSTRLEN);
	  src_ip = malloc (INET6_ADDRSTRLEN);
	  dst_ip = malloc (INET6_ADDRSTRLEN);

	  printf ("\n");

	  //wyswietlenie interfejsow
	  sprintf(cmd, "/bin/ip link");
	  system(cmd);

	  printf ("\n");
	  printf ("PODAJ NAZWĘ JEDENGO Z POWYŻSZYCH INTERFEJSÓW.\n");
	  printf ("\n");

	  printf ("Nazwa interfejsu:");
	  fgets (input, 32, stdin);	//pobierz nazwe interfejsu
	  memcpy (interface, input, strlen (input)-1);

	  printf ("\n");

	  // gniazdo
	  if ((sd = socket (AF_INET6, SOCK_RAW, IPPROTO_RAW)) < 0) {
	    perror ("blad gniazda uruchom jako sudo");
	    exit (EXIT_FAILURE);
	  }

	  //  ioctl() pobranie adresu mac podanego interfejsu
	  memset (&ifr, 0, sizeof (ifr));
	  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
	  if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
	    perror ("Blad pobrania adresu mac wybranego interfejsu ");
	    printf ("uruchom ponownie program i podaj poprawna nazwe interfejsu.\n");
	  }
	  close (sd);

	  // skopiowanie zrodlowego adresu mac
	  memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));

	  // wyswietlenie adresu mac.
	  printf ("adres MAC dla danego interfejsu %s to: ", interface);
	  for (i=0; i<5; i++) {
	    printf ("%02x:", src_mac[i]);
	  }
	  printf ("%02x\n", src_mac[5]);

	  // znalezienie indexu wybranego interfejsu ktory bedzie uzyty w sendto()
	  memset (&device, 0, sizeof (device));
	  if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
	    perror ("nie udalo sie odczytac indexu podanego interfejsu ");
	    exit (EXIT_FAILURE);
	  }
	  printf ("Index dla interfejsu %s to: %i\n", interface, device.sll_ifindex);

	  // ustawienie docelowego adresu MAC
	  dst_mac[0] = 0x12;
	  dst_mac[1] = 0xaf;
	  dst_mac[2] = 0x32;
	  dst_mac[3] = 0x22;
	  dst_mac[4] = 0x33;
	  dst_mac[5] = 0x68;

	  printf ("\n");

	  //tworzenie polecenia do wyswietlenia adresu IP wybranego interfejsu
	  strcpy(str, "ip -6 addr show ");
	  strcat(str, interface);
	  strcat(str, " | grep -oP '(?<=inet6\\s)[\\da-f:]+'");

	  printf("Adres IP wybranego interfejsu : ");
	  printf("\n");
	  sprintf(cmd, str);
	  system(cmd);

	  // Zrodlowy adres IPv6
	  printf ("Podaj zrodlowy adres IP : ");
	  fgets(input, INET6_ADDRSTRLEN, stdin);
	  if (strlen(input) != 0){
		  memcpy(src_ip, input, strlen (input)-1);
	  }
	  else{
		  strcpy(src_ip, "1000:1000::1000:1000:1000:1000");
	  }

	  // Docelowy adres IPv6
	  printf ("Podaj docelowy adres IP : ");
	  fgets (input, INET6_ADDRSTRLEN, stdin);
	  if (strlen(input) != 0){
		  memcpy(dst_ip, input, strlen (input)-1);
	  }
	  else{
		  strcpy(dst_ip, "1000:1000::1000:1000:1000:1000");
	  }

	  memset (&hints, 0, sizeof (hints));
	  hints.ai_family = AF_INET6;
	  hints.ai_socktype = SOCK_STREAM;
	  hints.ai_flags = hints.ai_flags | AI_CANONNAME;

	  device.sll_family = AF_PACKET;
	  device.sll_protocol = htons (ETH_P_IPV6);
	  memcpy (device.sll_addr, dst_mac, 6 * sizeof (uint8_t));
	  device.sll_halen = 6;

	  if ((status = inet_pton (AF_INET6, src_ip, &(iphdr->ip6_src))) != 1) {
	    fprintf (stderr, "Blad funkcji inet_pton().\nError:: %s", strerror (status));
	    exit (EXIT_FAILURE);
	  }

	  if ((status = inet_pton (AF_INET6, dst_ip, &(iphdr->ip6_dst))) != 1) {
	    fprintf (stderr, "Blad funkcji inet_pton().\nError: %s", strerror (status));
	    exit (EXIT_FAILURE);
	  }

	  frame_length = sizeof (struct ip6_hdr)+ntohs(iphdr->ip6_ctlun.ip6_un1.ip6_un1_plen);

	  //suma kontrolna
	  if (iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt == IPPROTO_ICMP){
		  naglowek_icmp->icmp6_cksum = 0;
		  naglowek_icmp->icmp6_cksum = icmp6_checksum (iphdr, naglowek_icmp);
	  }

	  // otworzenie gniazda
	  if ((sd = socket (PF_PACKET, SOCK_DGRAM, htons (ETH_P_ALL))) < 0) {
	    perror ("Blad funkcji socket() ");
	    exit (EXIT_FAILURE);
	  }
	  ShowLIST();
	  // wyslanie elementow listy wiazanej przez gniazdo
	  while (glowa != NULL) {
	  if ((bytes = sendto (sd, datagram, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
	    perror ("Blad funkcji wysyłania(), nie udalo sie wyslac.");
	    exit (EXIT_FAILURE);
	  }
	  else{
		  printf ("Przesłano bajtów: %d\n", bytes);
	  }

	  glowa = glowa -> next;

	  }
	  close (sd);
	  DelLIST();
}

void LoadLIST (int *count, unsigned char *dtgr){
	for (int i = 0; i != *count; i++)
		AddTOLIST(dtgr);
}

//funkcja do rezerwowania pamieci dla nowego elementu listy wiazanej
struct Node *ReserveMEM ( unsigned char *datagram ){

	//stworzenie nowego elementu
	struct Node *new_node;
	static int id = 1;

	//rezerwacja pamieci dla niego
	new_node = malloc(sizeof (struct Node));

	if (new_node == NULL) {
		printf("Nie mozna utworzyc nowego elementu");
		return NULL;
	}

	//dodanie danych do utworzonego elementu
	new_node->id = id;
	id++;
	new_node-> datagram = datagram;
	new_node->next = NULL;
	new_node->prev = NULL;

	//zwrocenie adresu elementu
	return new_node;
}

//funkcja do dodania utworzonego elementu na koncu listy wiazanej
void AddTOLIST(unsigned char *datagram){

	struct Node *roboczy = glowa;
	struct Node *nowy_element = ReserveMEM(datagram);

	if (glowa == NULL){
		glowa = nowy_element;
		return;
	}

	while (roboczy->next != NULL)
		roboczy = roboczy->next;

	roboczy->next = nowy_element;
	nowy_element->prev = roboczy;
	nowy_element->next = NULL;
}

//funkcja do wyswietlenia listy wizanej
void ShowLIST () {

	struct Node *temp = glowa;

	if ( temp == NULL)
		printf ("\nLista jest pusta\n");

	while (temp != NULL){
		printf ("id: %d, datagram: %p\n", temp->id, (void *)(temp-> datagram) );
		temp = temp->next;
	}
}

//funkcja zwracajaca poczatek listy
struct Node * StartLIST() {
	return glowa;
}

//funkcja do usuniecia listy wiazanej
void DelLIST (){

	struct Node *temp = glowa;
	struct Node *del = NULL;

	while ( temp != NULL ){
		del = temp;
		temp = temp -> next;
		free (del);
	}
	glowa = NULL;

}

uint16_t
icmp6_checksum (struct ip6_hdr *iphdr, struct icmp6_hdr *icmp6hdr)
{
  char buf[4096];
  char *ptr;
  int chksumlen = 0;
  //int i;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (128 bits)
  memcpy (ptr, &iphdr->ip6_src, sizeof (iphdr->ip6_src));
  ptr += sizeof (iphdr->ip6_src);
  chksumlen += sizeof (iphdr->ip6_src);

  // Copy destination IP address into buf (128 bits)
  memcpy (ptr, &iphdr->ip6_dst, sizeof (iphdr->ip6_dst));
  ptr += sizeof (iphdr->ip6_dst);
  chksumlen += sizeof (iphdr->ip6_dst);

  // Copy Upper Layer Packet length into buf (32 bits).
  // Should not be greater than 65535 (i.e., 2 bytes).
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  *ptr = (sizeof (struct icmp6_hdr)) / 256;
  ptr++;
  *ptr = (sizeof (struct icmp6_hdr)) % 256;
  ptr++;
  chksumlen += 4;

  // Copy zero field to buf (24 bits)
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 3;

  // Copy next header field to buf (8 bits)
  memcpy (ptr, &iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt, sizeof (iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt));
  ptr += sizeof (iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt);
  chksumlen += sizeof (iphdr->ip6_ctlun.ip6_un1.ip6_un1_nxt);

  // Copy ICMPv6 type to buf (8 bits)
  memcpy (ptr, &icmp6hdr->icmp6_type, sizeof (icmp6hdr->icmp6_type));
  ptr += sizeof (icmp6hdr->icmp6_type);
  chksumlen += sizeof (icmp6hdr->icmp6_type);

  // Copy ICMPv6 code to buf (8 bits)
  memcpy (ptr, &icmp6hdr->icmp6_code, sizeof (icmp6hdr->icmp6_code));
  ptr += sizeof (icmp6hdr->icmp6_code);
  chksumlen += sizeof (icmp6hdr->icmp6_code);

  // Copy ICMPv6 ID to buf (16 bits)
  memcpy (ptr, &icmp6hdr->icmp6_dataun.icmp6_un_data16 [0], sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [0]));
  ptr += sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [0]);
  chksumlen += sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [0]);

  // Copy ICMPv6 sequence number to buff (16 bits)
  memcpy (ptr, &icmp6hdr->icmp6_dataun.icmp6_un_data16 [1], sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [1]));
  ptr += sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [1]);
  chksumlen += sizeof (icmp6hdr->icmp6_dataun.icmp6_un_data16 [1]);

  // Copy ICMPv6 checksum to buf (16 bits)
  // Zero, since we don't know it yet.
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;


  return checksum ((uint16_t *) buf, chksumlen);
}

uint16_t checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}
