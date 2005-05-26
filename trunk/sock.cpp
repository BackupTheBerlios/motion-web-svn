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

int CSockClient::SendHeader(int s, int *msg) {
	int n;
	n = send(s, msg, sizeof(int), 0);
	return n == -1?-1:0;	
}

int CSockClient::MiniSend(int s, char *msg) {
	
	int total = 0;
	int bytesleft = strlen(msg);
	int n;
	
	while (total < strlen(msg) ) {
		n = send(s, msg+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	return n == -1?-1:0;
}


int CSockClient::Send(int s, char *msg) {
	int len;
	len = strlen(msg);
	len = htons(len);
	if ( SendHeader(s, &len) == -1 ) {
		cerr << "CSockClient error: I can't send header" << endl;
		exit (-1);
	}
	
	if ( MiniSend(s, msg) == -1 ) {
		cerr << "CSockClient error: I can't send data" << endl;
		exit (-1);
	}
	return 0;
}

int CSockClient::RecvHeader(int s, int *i) {
	int stat;
	stat = recv(s, i, sizeof(int), 0);
	*i = ntohs(*i);
	return stat;
}

int CSockClient::MiniRecv(int s, char **msg, int len) {
	int total = 0;
	char *ptr;
	*msg = (char *)malloc(len);
	int bytesleft = len;
	int n;
	while ( total < len  ) {
		n = recv(s, *msg+total, bytesleft, 0);
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
	
	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		if ( (client.sock_fd = accept (sock_fd, (struct sockaddr *)&client.inet, &sin_size)) == -1 ) {
			perror("accept");
			continue;
		}
		//DEBUG
		time(&now);
		cout << "Got connection from " << inet_ntoa(client.inet.sin_addr) << " at " << ctime(&now); 
	
		if (!fork()) { //Proceso hijo
			cout << "Proceso hijo iniciado\n" << endl;
			close (sock_fd);
			int a,x;
			char *msg=NULL;
			do {
				x = RecvHeader(client.sock_fd, &a);
				cout << "Recibido head: " << a << endl;
				x = MiniRecv(client.sock_fd, &msg, a);
				if (x == -1)
					cout << "Error el recibir los datos" << endl;
				cout << "Mensaje: " << msg << endl;
//				close(client.sock_fd);
			} while ( strcmp((const char*)msg,"END") != 0 );
			exit(0);
		}
		close(client.sock_fd);
	}
	return 0;
}
