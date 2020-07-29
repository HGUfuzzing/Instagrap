all : instagrapd.c worker.c socket_file_io.o socket_file_io.h
	gcc instagrapd.c socket_file_io.o -o instagrapd
	gcc worker.c socket_file_io.o -o worker

instagrapd : instagrapd.c socket_file_io.o socket_file_io.h
	gcc instagrapd.c socket_file_io.o -o instagrapd

worker : worker.c socket_file_io.o socket_file_io.h
	gcc worker.c socket_file_io.o -o worker

socket_file_io.o : socket_file_io.c socket_file_io.h
	gcc -c socket_file_io.c

clean :
	rm -f instagrapd
	rm -f worker
	rm -f socket_file_io.o