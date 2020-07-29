all : instagrapd.c submitter.c socket_file_io.o socket_file_io.h
	gcc instagrapd.c socket_file_io.o -o instagrapd
	gcc submitter.c socket_file_io.o -o submitter

instagrapd : instagrapd.c socket_file_io.o socket_file_io.h
	gcc instagrapd.c socket_file_io.o -o instagrapd

submitter : submitter.c socket_file_io.o socket_file_io.h
	gcc submitter.c socket_file_io.o -o submitter

socket_file_io.o : socket_file_io.c socket_file_io.h
	gcc -c socket_file_io.c

clean :
	rm -f instagrapd
	rm -f submitter
	rm -f socket_file_io.o