#include "main.h"

// Clase de socket.

CSock::CSock() {
	protocol = SOCK_STREAM;
}

	// Set and init Addr
int CSock::SetAddr(unsigned short int port, char *address) {

	inet.sin_family = AF_INET;
	inet.sin_port = htons(port);
	
	if ( address != NULL)
		inet.sin_addr.s_addr = inet_addr(address);
	else
		inet.sin_addr.s_addr = INADDR_ANY;
	memset(&(inet.sin_zero), '\0', 8);     	

}

int CSock::InitSocket() {
	
	sock_fd = socket(AF_INET, protocol, 0);
	if ( sock_fd == -1 ) {
		cerr << "CSock error: I can't init socket" << endl;
		perror("socket");
		exit(1);
	}
	int yes=1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
}

int CSockClient::Connect() {

	if ( connect(sock_fd, (struct sockaddr *)&inet, sizeof (struct sockaddr) ) == -1 ) {
		perror ("connect");
		exit(1);
	} 
}

int CSockClient::SendMsgLen(int *msg) {
	int n;
	n = send(sock_fd, msg, sizeof(int), 0);
	return n == -1?-1:0;	
}

int CSockClient::MiniSend(char *msg) {
	
	int total = 0;
	int bytesleft = strlen(msg);
	int n;
	
	while (total < strlen(msg) ) {
		n = send(sock_fd, msg+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	return n == -1?-1:0;
}


int CSockClient::Send(char *msg) {
	int len,protovers,i_send;
	protovers = htons(Mmant_PROTOCOL_VERSION);
	len = strlen(msg);
	len = htons(len);
	i_send = send(sock_fd, &protovers, sizeof(int), 0);
	if (i_send == -1)
		cerr << "CSockClient error: I cant't send header!!" << endl;
	if ( SendMsgLen(&len) == -1 ) {
		cerr << "CSockClient error: I can't send header" << endl;
		exit (-1);
	}
	
	if ( MiniSend(msg) == -1 ) {
		cerr << "CSockClient error: I can't send data" << endl;
		exit (-1);
	}
	return 0;
}

int CSockClient::RecvHeader(int *i) {
	int stat;
	stat = recv(sock_fd, i, sizeof(int), 0);
perror("recv");
	*i = ntohs(*i);
	return stat;
}

int CSockClient::MiniRecv(char **msg, int len) {
	int total = 0;
	char *ptr;
	*msg = (char *)malloc(len);
	int bytesleft = len;
	int n;
	while ( total < len  ) {
		n = recv(sock_fd, *msg+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	ptr = *msg+len;
	*ptr = '\0';
	return n == -1?-1:0;
}


// Clase CSockServer

int CSockServer::Bind() {
	
	if ( bind(sock_fd, (struct sockaddr *)&inet, sizeof(struct sockaddr)) == -1 ) {
		cerr << "CSockServer error: I can't bind socket" << endl;
		perror("bind");
		exit(-1);
	} 
	cout << "Wait connections on port " << ntohs(inet.sin_port) << endl;
	return 0;
}

int CSockServer::Listen(int backlog) {

	if ( listen(sock_fd, backlog) == -1) {
		cerr << "CSockServer error: I can't listen socket" << endl;
		perror("listen");
		exit(-1);
	}
}

int CSockServer::AttachMmant(Mmant *z) {
	mmant=z;
}

int CSockServer::InitServer() {
	socklen_t sin_size;
	time_t	now;
	Bind();
	Listen();
	pthread_t p_socket;
	CSockServer *cs_ptr = this;
//	cs_ptr = this;

	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		if ( (client.sock_fd = accept (sock_fd, (struct sockaddr *)&client.inet, &sin_size)) == -1 ) {
			perror("accept");
			continue;
		}
//		cs_ptr = &client;
		time(&now);
		cout << "Got connection from " << inet_ntoa(client.inet.sin_addr) << " at " << ctime(&now); 

// DEPRECATED
#ifdef WITH_FORK	
		if (!fork()) { //Proceso hijo
			cout << "Proceso hijo iniciado\n" << endl;
			close (sock_fd);
			int a,x;
			char *msg=NULL;
			do {
				x = client.RecvHeader(&a);
				cout << "Recibido header: " << a << endl;
				x = client.MiniRecv(&msg, a);
				if (x == -1)
					cout << "Error el recibir los datos" << endl;
				cout << "Mensaje: " << msg << endl;
//				close(client.sock_fd);
			} while ( strcmp((const char*)msg,"END") != 0 );
			exit(0);
		}
#endif
#ifdef WITH_PTHREAD
		if ( (pthread_create(&p_socket,NULL,HeartServer,cs_ptr)) == -1 ) {
			cerr << "ERROR: I can't create server process" << endl;
			exit(1);
		}
#endif
		close(client.sock_fd);
	}
	return 0;
}

void * CSockServer::HeartServer(void *ps){
	CSockServer *p_this = (CSockServer *)ps;
	cout << "Thread hijo iniciado\n" << endl;
	int a,x;
	char *msg=NULL;
//	do {
		if ( p_this->client.RecvHeader(&a) == -1 )
			cout << "CSockServer ERROR: RecvHeader" << endl;
		else
			cout << "Recibido head: " << a << endl;
/*		if ( a == 0)
			break;
		x = p_this->MiniRecv(p_this->client.sock_fd, &msg, a);
		if (x == -1)
			cout << "Error el recibir los datos" << endl;
		cout << "Mensaje: " << msg << endl;
//				close(client.sock_fd);
	} while ( strcmp((const char*)msg,"END") != 0 );
*/
	pthread_exit(NULL);
}
