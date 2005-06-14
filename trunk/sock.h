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

// Brief Protocol Description
// '0x01' <<-- Version of Mmant Protocol
// int signed <<-- length of command
// buffer command <<-- Command
// int signed <<-- length of data (if any)
// buffer data <<<-- Data (if any)
// EOF. <<-- 


// Client commands &
// Parameters
// AUTH		<<<--- Rudimentary PASSWORD
// password
// SENDONE	<<<--- Send one file
// path file
// SENDMUL	<<<--- Send multiple files
// path of files
// KILLDAEMON	<<<--- Kill main daemon
// VERSION	<<<--- Version of Mmant
// CLEANCAMALL	<<<--- Remove all data of 1 cam
// cam number
// MRPROPER	<<<--- Remove ALL data of ALL cams.
// cam number
// REMOVEONE	<<<--- Remove one file
// path of file
// FREESPACE	<<<--- Space free of directory.
// directory

#define Mmant_PROTOCOL_VERSION	0x01

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
	int Recv(char *msg);
	int Send(char *msg);
	int RecvHeader(int *i);
	int MiniRecv(char **msg, int len);	
private:	
	int SendMsgLen(int *msg);
	int MiniSend(char *msg);
	
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
	CSockClient client;
	// Server
	int Bind();
	int Listen(int backlog=10);
	// MAIN FUNCTION OF SERVER
	static void * HeartServer(void *p_m);

};
#endif
