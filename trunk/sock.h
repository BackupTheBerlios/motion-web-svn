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
#include <poll.h>
#include "mmant.h"

// Brief Protocol Description
// '0x01' <<-- Version of Mmant Protocol (Server IMPAR - client - PAR);
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
// LIST		<<<--- List current jobs

#define Mmant_PROTOCOL_VERSION	0x0A
#define TIMEOUT_RECV 10000

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
	int Read(char **cmd);
private:
	int SendHProtocolVersion();
	int SendHMsgLen(char *msg);
	int SendCommand(char *cmd);
	int RecvHPRotocolVersion(long *protocol);
	int RecvHLengthCommand(long *length);
	int RecvHCommand(char **msg, int len);
	int RecvLengthData(int *length);
	int RecvData(char **msg, int len);
	int RecvHeader(int *i);
	int MiniRecv(char **msg, int len);	
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
	int ProcessCommand(char **cmd);	

private:
	CSockServer* parent;	//Pointer to CsockServer base
	CSock client;
	vector<string> ActiveJobsList;
	// Server
	int Bind();
	int Listen(int backlog=10);
	// MAIN FUNCTION OF SERVER
	static void * HearthServer(void *p_m);
	int CheckActiveCommand(string cmd);
	int CleanActiveCommandList(string *list);

};
#endif
