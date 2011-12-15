all: lib/libomegle.so demos

lib/libomegle.so: src/Connection.o src/BufferedSocket.o
	g++ -shared -Wl,-rpath,lib -Wl,-soname,libomegle.so src/Connection.o src/BufferedSocket.o -o lib/libomegle.so

src/Connection.o: src/Connection.cpp include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h
	g++ -c src/Connection.cpp -Wall -Iinclude/Omegle -o src/Connection.o

src/BufferedSocket.o: src/BufferedSocket.cpp include/Omegle/BufferedSocket.h include/Omegle/Error.h
	g++ -c src/BufferedSocket.cpp -Wall -Iinclude/Omegle -o src/BufferedSocket.o

demos: demos/basic-chatbot demos/omegle-cli-client

demos/omegle-cli-client: demos/omegle-cli-client.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h
	g++ demos/omegle-cli-client.cpp -Iinclude -Llib -pthread -lomegle -Wall -std=c++0x -Wl,-rpath,../lib -o demos/omegle-cli-client

demos/basic-chatbot: demos/basic-chatbot.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h
	g++ demos/basic-chatbot.cpp -Iinclude -Llib -pthread -lomegle -Wall -std=c++0x -Wl,-rpath,../lib -o demos/basic-chatbot

clean:
	rm -f demos/omegle-cli-client demos/basic-chatbot src/*.o lib/libomegle.so
