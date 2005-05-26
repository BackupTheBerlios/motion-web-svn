#ifndef _SOCK_H
#define _SOCK_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include "mmant.h"

// Clase de sockets

using namespace std;
class CSock {

public:
	// Constructor
	CSock();
	
	// Struct with address and port
	struct sockaddr_in inet;
	int sock_fd;
	int protocol;
	
	// Set Addr and Port
	int SetAddr(unsigned short int port, char *address=NULL);
	// Set protocol: TCP or UDP: Default TCP
	int SetProtocol(int protocol);
	// Call socket();
	int InitSocket();
	
};

class CSockClient:public CSock {

public:
	int Connect();
	int Recv(int s, char *msg);
	int Send(int s, char *msg);
	int RecvHeader(int s, int *i);
	int MiniRecv(int s, char **msg, int len);	
private:	
	int SendHeader(int s, int *msg);

	int MiniSend(int s, char *msg);
	
};

class CSockServer:public CSockClient {

public:
	// Función genérica que arranca el server.
	int InitServer();

	// Mmant Functions
	// Recoge el puntero hacia un objeto Mmant.
	// Lo ideal sería realizar una lista de objetos y manejar distintos para poder realizar varias operaciones en cola.
	// Después se podrían consultar, eliminar, etc.
	Mmant *mmant;
	int AttachMmant(Mmant *z);

private:
	CSock client;
	// Server
	int Bind();
	int Listen(int backlog=10);

};
#endif
