#include "main.h"

//#define NOCLIENT

const string HOST = "localhost";
const string USER = "root";
const string PASS = "";
const string DATABASE = "motion";

extern "C" { static void signhandler(int sig);}

#ifdef CLIENT
CSockClient o_sock;
#endif
#ifndef CLIENT
CSockServer o_sock;
#endif
Mmant motion;
int main(int argc, char *argv[]) {



signal(SIGCLD, SIG_IGN);
//signal(SIGINT,signhandler);
signal(SIGUSR1, signhandler);
char *files[] = {
              "/var/www/html/cam1/01-20041206192637m.mpg",
              "/var/www/html/cam1/01-20041206192637-12.jpg",
              "/var/www/html/cam1/01-20041206192637-15.jpg"};

			  
			  
#ifdef CLIENT // Modo CLIENTE

o_sock.SetAddr(6969, "127.0.0.1");
o_sock.InitSocket();
o_sock.Connect();
o_sock.Send("COMMAND TEST 1");
if ( ( o_sock.Send("DELETE") ) != 0)
	cout << "Error sending DELETE\n" << endl;
o_sock.Send("END");
#endif

#ifndef CLIENT // Modo SERVIDOR
o_sock.SetAddr(6969);
o_sock.InitSocket();
o_sock.AttachMmant(&motion);
o_sock.InitServer();
#endif
//motion.OpenDB(HOST,USER,PASS,DATABASE);

//motion.SetFiles(files,3);


//motion.SetEraseAll(1);
//motion.DeleteFiles();


return 1;
}

static void signhandler(int sig) {
	if (sig == SIGINT)
		motion.DeleteStop(sig);
	else if (sig == SIGUSR1)
		motion.CheckDelete(sig);
}
