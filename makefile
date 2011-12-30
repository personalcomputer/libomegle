all: libomegle demos python-bindings perl-bindings


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
 
demos/omegle-cli-client: libomegle demos/omegle-cli-client.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	g++ demos/omegle-cli-client.cpp -Iinclude -Llib -pthread -lomegle -Wall -std=c++0x  -fno-rtti -Wl,-rpath,../lib -o $@

demos/basic-chatbot: libomegle demos/basic-chatbot.cpp include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	g++ demos/basic-chatbot.cpp -Iinclude -Llib -lomegle -Wall -fno-rtti -Wl,-rpath,../lib -o $@


python-bindings: python-bindings/_omegle.so

python-bindings/omegle.py python-bindings/src/omegle_wrap.cpp: python-bindings/src/omegle.i include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	swig -c++ -python -outdir python-bindings -o python-bindings/src/omegle_wrap.cpp python-bindings/src/omegle.i

python-bindings/src/omegle_wrap.o: python-bindings/src/omegle_wrap.cpp
	g++ -fpic -c python-bindings/src/omegle_wrap.cpp -Wall -I/usr/include/python2.7/ -fno-rtti -o $@
 
python-bindings/_omegle.so: python-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o
	g++ -shared -Wl,-soname,_omegle.so -Wall python-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o -o $@


perl-bindings: perl-bindings/_omegle.so

perl-bindings/omegle.py perl-bindings/src/omegle_wrap.cpp: perl-bindings/src/omegle.i include/Omegle.h include/Omegle/Connection.h include/Omegle/Error.h include/Omegle/BufferedSocket.h include/Omegle/Socket.h
	swig -c++ -perl -outdir perl-bindings -o perl-bindings/src/omegle_wrap.cpp perl-bindings/src/omegle.i

perl-bindings/src/omegle_wrap.o: perl-bindings/src/omegle_wrap.cpp
	g++ -fpic -c perl-bindings/src/omegle_wrap.cpp -Wall -I/usr/lib/perl/5.12.4/CORE `perl -MConfig -e 'print join(" ", @Config{qw(ccflags optimize cccdlflags)}, "-I$Config{archlib}/CORE")'` -fno-rtti -o $@
 
perl-bindings/_omegle.so: perl-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o
	g++ -shared -Wl,-soname,_omegle.so -Wall `perl -MConfig -e 'print $Config{lddlflags}'` perl-bindings/src/omegle_wrap.o src/Connection.o src/BufferedSocket.o src/Socket.o -o $@


clean:
	rm -f demos/omegle-cli-client demos/basic-chatbot src/*.o lib/libomegle.so python-bindings/_omegle.so python-bindings/omegle.py python-bindings/src/omegle_wrap.cpp python-bindings/src/omegle_wrap.o
