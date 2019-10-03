#ifndef LIB_H_
#define LIB_H_

struct Node {
	int id;
	unsigned char *datagram;
	struct Node *next;
	struct Node *prev;
};

void * LoadIPV6LIB();	//wczytywanie biblioteki IPv6

void * LoadICMPV6LIB();	//wczytywanie biblioteki TCP

void SendPACKET (unsigned char *interface);	//służy do wysłania pakietu, ktory znajduje sie w liście wiązanej

void LoadLIST (int *count, unsigned char *dtgr);	//dodawanie elementow do listy

struct Node *ReserveMEM(unsigned char *datagram);	//rezerwacja pamieci

void AddTOLIST(unsigned char *datagram);	//dodawanie elementu na koniec listy

void ShowLIST();	//wyswietlanie listy

void DelLIST();	//funkcja do usuniecia listy wiazanej

struct Node * StartLIST (); //funkcja do zwrocenia poczatku listy wiazanej

#endif
