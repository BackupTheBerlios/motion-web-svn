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

int CSockClient::SendHMsgLen(char *msg, unsigned long len) {
	int n;
	if (!len)	// If len=0 is a binary data and length is passthru
		len = strlen(msg);
#ifdef DEBUG
	cout << "Sending Header: MSG LEN: " << len << endl;
#endif
	len = htonl (len);
	n = send(sock_fd, &len, sizeof(unsigned long), 0);
#ifdef DEBUG
	perror("send");
#endif
	return n == -1?-1:0;	
}

int CSockClient::MiniSend(char *msg, unsigned long len) {
	
	int total = 0;
	int bytesleft;
	if (!len) {
		bytesleft = strlen(msg);
		len = bytesleft;
	}
	else
		bytesleft = len;
	int n;
	
	while (total < len ) {
		n = send(sock_fd, msg+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	return n == -1?-1:0;
}

int CSockClient::SendHProtocolVersion() {
	int n;
#ifdef DEBUG
	cout << "Sending Header: PROTOCOL_VERSION: " << Mmant_PROTOCOL_VERSION << endl;
#endif
	protocol = htons(Mmant_PROTOCOL_VERSION);
	n = send(sock_fd, &protocol, sizeof(int), 0);
#ifdef DEBUG
	perror("send");
#endif
	return n == -1?-1:0;
}


int CSockClient::Send(char *msg, unsigned long len) {
	int i_send;
	// We send the protocol version
	i_send = SendHProtocolVersion();
	if (i_send == -1) {
#ifdef DEBUG
		cerr << "CSockClient error: I cant't send protocol version!!" << endl;
#endif
		return -1;
	}
	// We send the length of command
	if ( SendHMsgLen(msg, len) == -1 ) {
#ifdef DEBUG
		cerr << "CSockClient error: I can't send header" << endl;
#endif
		return -1;
	}
	// We send the message
	if ( MiniSend(msg, len) == -1 ) {
#ifdef DEBUG
		cerr << "CSockClient error: I can't send command" << endl;
#endif
		return -1;
	}
	return 0;
}

int CSockClient::RecvHPRotocolVersion(long *protocol) {
	int stat;
	struct pollfd my_pfds[1];
	my_pfds[0].fd = sock_fd;
	my_pfds[0].events = POLLIN;
	if (poll(my_pfds, 1, TIMEOUT_RECV) == 1) {
		stat = recv(sock_fd, protocol, sizeof(int), 0);
#ifdef DEBUG
		perror("recv");
#endif
		*protocol = ntohs(*protocol);
		return stat == 0?-1:0;
	}
#ifdef DEBUG
	else
		cout << "RecvHPRotocolVersion: TIMEOUT_RECV ERROR" << endl;
#endif
	return -1;
}

int CSockClient::RecvHLengthCommand(unsigned long *length) {
	int stat;
	*length = 2;
	struct pollfd my_pfds[1];
	my_pfds[0].fd = sock_fd;
	my_pfds[0].events = POLLIN;
	if (poll(my_pfds, 1, TIMEOUT_RECV) == 1) {
		stat = recv(sock_fd, length, sizeof(unsigned long), 0);
#ifdef DEBUG
		cout << "Recibidos " << stat << " bytes" << endl;
		perror("recv");
#endif
	*length = ntohl(*length);
	} else
		return -1;
	return stat == 0?-1:0;
}

int CSockClient::RecvHCommand(char **msg, int len) {
	return MiniRecv(msg, len);
}

int CSockClient::RecvLengthData(int *length) {

}

int CSockClient::RecvData(char **msg, int len) {

}

int CSockClient::RecvHeader(int *i) {
	int stat;
	struct pollfd my_pfds[1];
	my_pfds[0].fd = sock_fd;
	my_pfds[0].events = POLLIN;
	if (poll(my_pfds, 1, 10) == 1) {
		stat = recv(sock_fd, i, sizeof(int), 0);
		perror("recv");
		*i = ntohs(*i);
		return stat;
	}
	return -1;
}

int CSockClient::MiniRecv(char **msg, int len) {
	int total = 0;
	int i = 0;
	char *ptr;
	*msg = (char *)malloc(len+1);
	int bytesleft = len;
	int n;
	struct pollfd my_pfds[1];
	my_pfds[0].fd = sock_fd;
	my_pfds[0].events = POLLIN;
	while ( total < len  ) {
#ifdef DEBUG
		cout << "1Total: " << total << " len: " << len << endl;
#endif
		if (poll(my_pfds, 1, TIMEOUT_RECV) == 1) {
			n = recv(sock_fd, *msg+total, bytesleft, 0);
			if (n == -1) { break; }
			total += n;
			bytesleft -= n;
#ifdef DEBUG
			cout << "2Total: " << total << " len: " << len << endl;
			perror("recv");
#endif
			i++;
		} else {
		break;
		}
			if (i>9)
				break;
	}
	ptr = *msg+len;
	*ptr = '\0';
	return n == -1?-1:0;
}


int CSockClient::Read(char **cmd) {
	long protocol;
	unsigned long l_command;
	if ( RecvHPRotocolVersion(&protocol) == -1 ) {
#ifdef DEBUG
		cout << "CSockServer ERROR: RecvProtocol" << endl;
#endif
		return -1;
	}
#ifdef DEBUG
	else
		cout << "Recibido protocol: " << protocol << endl;
#endif
	if ( protocol != PROTOCOL_VERSION ) {
#ifdef DEBUG
		cerr << "Packet discarded. Protocol not matching!!" << endl;
//		RecvHPRotocolVersion(&protocol);	// Workaround for "sucksets" PHP
#endif
		return -1;
	}
	if ( (RecvHLengthCommand(&l_command) == -1) || l_command == 0) {
#ifdef DEBUG
		cerr << "CSockServer ERROR: RecvHLengthCommand: " << l_command << endl;
#endif
		return -1;
	}
#ifdef DEBUG
	else
		cout << "Recibido length Header: " << l_command << endl;
#endif
	if ( RecvHCommand(cmd, l_command) == -1 ) {
#ifdef DEBUG
		cout << "CSockServer ERROR: RecvHCommand" << endl;
#endif
		return -1;
	}
#ifdef DEBUG
	else {
		cout << "Recibido comando: " << *cmd << endl;
	}
#endif
return l_command;

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
	CSockServer *ptr_CSock;

	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		if ( (client.sock_fd = accept (sock_fd, (struct sockaddr *)&client.inet, &sin_size)) == -1 ) {
			perror("accept");
			continue;
		}
		ptr_CSock = new CSockServer();
		if ( ptr_CSock == NULL)
			cerr << "FATAL ERROR" << endl;
		ptr_CSock->sock_fd = client.sock_fd;
		ptr_CSock->inet = client.inet;
		ptr_CSock->protocol = client.protocol;
		ptr_CSock->parent = this;
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
				x = client->RecvHeader(&a);
					cout << "Recibido header: " << a << endl;
				x = client->MiniRecv(&msg, a);
				if (x == -1)
					cout << "Error el recibir los datos" << endl;
				cout << "Mensaje: " << msg << endl;
//				close(client.sock_fd);
			} while ( strcmp((const char*)msg,"END") != 0 );
			exit(0);
		}
#endif
#ifdef WITH_PTHREAD
	#ifdef DEBUG
		cout << "Creating thread" << endl;
	#endif
		if ( (pthread_create(&p_socket,NULL,HearthServer,ptr_CSock)) == -1 ) {
			cerr << "ERROR: I can't create server process" << endl;
			exit(1);
		}
#endif
//		close(client.sock_fd);
	}
	return 0;
}

void * CSockServer::HearthServer(void *ps){
	CSockServer *ptr_CSock = (CSockServer *)ps;
	int p_len;
	char *msg=NULL;
	char *msg_prev=NULL;
	do {
		if (msg != NULL)
			free(msg);
		p_len = ptr_CSock->Read(&msg);
		if (  p_len == -1 ) {
			ptr_CSock->Send("900 FATAL ERROR");
			break;
		} else {
			ptr_CSock->ProcessCommand(&msg, p_len);
		}
	} while ( strcmp((const char*)msg,"END") != 0 );
	ptr_CSock->Send("200 CLOSING CONNECTION");
	if ( msg != NULL )
		free(msg);
	close(ptr_CSock->sock_fd);
	if ( ptr_CSock != NULL )
		delete(ptr_CSock);
	cout << "Cerrando conexion y thread" << endl;
	pthread_exit(NULL);
}

int CSockServer::ProcessCommand(char **cmd, int p_len) {
	string	list,command;
	char *name_image_file;
	unsigned char *buff,*btmp;
	int i_cam;
	FILE *img_fd;
	command = *cmd;
 	cout << "Procesando comando: " << command << endl;
	if ( strcmp( (const char*)*cmd, "LIST") == 0 ) {
		parent->ActiveJobsList.push_back(*cmd);		// Add the command to the array of current jobs
#ifdef DEBUG
		cout << " Tamaño de list: " << parent->ActiveJobsList.size() << endl;
#endif
		CleanActiveCommandList(&list);
		if ( Send((char *)list.c_str()) ){
 			return -1;
		}
		parent->ActiveJobsList.pop_back();
	} else
	if ( ( i_cam = command.rfind("DELETE CAM") ) == 0 ) {
		sscanf(command.c_str(), "DELETE CAM %d",&i_cam);
		cout << "200 OK.DELETE CAM: " << i_cam << endl;
		if ( CheckActiveCommand(command) ) {
			if ( Send("900 No es posible procesar el comando. Ya se está haciendo ese trabajo") )
				return -1;
		return 0;
		}
		parent->ActiveJobsList.push_back(*cmd);		// Add the command to the array the current jobs to satisfy LIST
		parent->mmant->SetEraseAll(i_cam);
		parent->mmant->DeleteFiles();
		if ( Send("200 CMD DELETE ACCEPTED") )
			return -1;
	} else
	if ( ( i_cam = command.rfind("STOP DELETE") ) == 0 ) {
		cout << "200 OK STOP DELETE" << endl;
		if ( parent->mmant->LOCK == false ) {
			if ( Send("900 No se está borrando ninguna cámara") )
				return 0;
			return 0;
		} else {
			parent->mmant->DeleteStop(1);
			if ( Send("200 CMD STOP DELETE ACCEPTED") )
				return -1;
		}
	}else
	if ( ( i_cam = command.rfind("STATUS") ) == 0 ) {
		if (parent->mmant->LOCK == true ) {
			string s_send = "200 ";
			s_send += parent->mmant->CheckDelete(1);
			if ( Send((char*)s_send.c_str()) )
				return -1;
		} else {
			if ( Send("200 IDLE") )
				return -1;
		}
	} else
	if ( ( i_cam = command.rfind("SEND IMG") ) == 0 ) {
		name_image_file = (char *)malloc((p_len-8)*sizeof(char));
		sscanf(command.c_str(), "SEND IMG %s", name_image_file);
		cout << "200 OK. SENDING IMG: " << name_image_file << endl;
		img_fd = fopen ( name_image_file, "rb");
		if ( img_fd == NULL ) {
			Send("900 ERROR OPENING FILE");
			free(name_image_file);
			return -1;
		}
		struct stat s_img_file;
		stat(name_image_file,&s_img_file);
		cout << "Tamaño del archivo: " << s_img_file.st_size << endl;
		unsigned int a;
		buff = (unsigned char *)malloc(s_img_file.st_size+1);
		btmp = (unsigned char *)malloc(s_img_file.st_size+1);
		if ( btmp == NULL || buff == NULL) {
			Send("900 ERROR ALLOCATING MEMORY");
			free(name_image_file);
			fclose(img_fd);
			return -1;
		}
//		do {
			a = fread(buff, 1, s_img_file.st_size+1, img_fd);
//			if ( a )
//				buff=(unsigned char *)memcpy(buff+(buff),btmp,a);
			cout << "Leído: " << a << endl;
//		}
//		while (a == s_img_file.st_size);
		cout << "szie buff: " << strlen((const char*)buff) << endl;
//		cout << "BUFF: " << buff << endl;
		if ( !feof(img_fd) ) {
			Send("900 ERROR READING FILE");
			fclose(img_fd);
			free(name_image_file);
			free(buff);
			free(btmp);
			return -1;
		}
		if ( Send((char *)buff,s_img_file.st_size) )
			return -1;
		free(name_image_file);
		free(buff);
		free(btmp);
		fclose(img_fd);
		return 0;
	}
}

int CSockServer::CheckActiveCommand(string cmd) {
#ifdef DEBUG
	cout << "Comparando: " << cmd << endl;
#endif
	for (int a=0; a != parent->ActiveJobsList.size(); a++) {
		if ( parent->ActiveJobsList[a] == cmd )
			return 1;
	}
return 0;
}

int CSockServer::CleanActiveCommandList(string *list) {
	vector<string> v_tmp;
	for (int a=0; a != parent->ActiveJobsList.size();a++) {
		if ( (parent->ActiveJobsList[a].find("DELETE CAM") != string::npos ) && parent->mmant->LOCK == false ) {
		} else {
		*list = *list + parent->ActiveJobsList[a] + "|";
		v_tmp.push_back(parent->ActiveJobsList[a].c_str());
		}
	}
	parent->ActiveJobsList = v_tmp;
}
