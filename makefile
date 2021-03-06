all: libomegle demos python-bindings


libomegle: lib/libomegle.so

lib/libomegle.so: src/Connection.o src/BufferedSocket.o src/Socket.o
	g++ -shared -Wl,-soname,libomegle.so src/Connection.o src/BufferedSocket.o src/Socket.o -fno-rtti -o $@

src/Connection.o: src/Connection.cpp include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	g++ -fpic -c src/Connection.cpp -Wall -Iinclude/Omegle -fno-rtti -o $@

src/BufferedSocket.o: src/BufferedSocket.cpp include/Omegle/BufferedSocket.h include/Omegle/Socket.h include/Omegle/Error.h
	g++ -fpic -c src/BufferedSocket.cpp -Wall -Iinclude/Omegle -fno-rtti -o $@

src/Socket.o: src/Socket.cpp include/Omegle/Socket.h include/Omegle/Error.h
	g++ -fpic -c src/Socket.cpp -Wall -Iinclude/Omegle -fno-rtti -o $@


demos: demos/basic-chatbot demos/omegle-cli-client
 
demos/omegle-cli-client: lib/libomegle.so demos/omegle-cli-client.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	g++ demos/omegle-cli-client.cpp -Iinclude -Llib -lomegle -Wall -fno-rtti -Wl,-rpath,../lib -o $@

demos/basic-chatbot: lib/libomegle.so demos/basic-chatbot.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	g++ demos/basic-chatbot.cpp -Iinclude -Llib -lomegle -Wall -fno-rtti -Wl,-rpath,../lib -o $@


python-bindings: python-bindings/_omegle.so

python-bindings/omegle.py python-bindings/src/omegle_wrap.cpp: python-bindings/src/omegle.i include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	swig -c++ -python -outdir python-bindings -o python-bindings/src/omegle_wrap.cpp python-bindings/src/omegle.i

python-bindings/src/omegle_wrap.o: python-bindings/src/omegle_wrap.cpp
	g++ -fpic -c python-bindings/src/omegle_wrap.cpp -Wall -I/usr/include/python2.7/ -fno-rtti -o $@
 
python-bindings/_omegle.so: python-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o
	g++ -shared -Wl,-soname,_omegle.so -Wall python-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o -o $@

install: lib/libomegle.so
	mkdir -p /usr/local/include && mkdir -p /usr/local/lib && cp lib/libomegle.so /usr/local/lib/ && cp -R include/* /usr/local/include/

clean:
	rm -f demos/omegle-cli-client demos/basic-chatbot src/*.o lib/libomegle.so python-bindings/_omegle.so python-bindings/omegle.py python-bindings/src/omegle_wrap.cpp python-bindings/src/omegle_wrap.o
