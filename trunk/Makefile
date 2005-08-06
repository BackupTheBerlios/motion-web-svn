all: client servidor

mantmot: mmant.cpp main.cpp sock.cpp
	g++ -DWITH_PTHREAD -O3 -fomit-frame-pointer -march=pentium4 -mtune=pentium4 sock.cpp mmant.cpp main.cpp -o mmant -lmysqlcppapi-1.9 -pthread
	
mantmot-debug: mmant.cpp main.cpp sock.cpp
	g++ -g mmant.cpp main.cpp sock.cpp -o mmant -lmysqlcppapi-1.9 -pthread

client: mmant.cpp main.cpp sock.cpp
	g++ -g -DDEBUG -DCLIENT  mmant.cpp main.cpp sock.cpp -o client -lmysqlcppapi-1.9 -pthread
	
servidor: mmant.cpp main.cpp sock.cpp 
	g++ -g  -DDEBUG -DWITH_PTHREAD  mmant.cpp main.cpp sock.cpp -o servidor -lmysqlcppapi-1.9 -pthread	

clean:
	rm -f mmant
	rm -f servidor
	rm -f client
	rm -f *~
