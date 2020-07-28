all : instagrapd.c worker.c
	gcc instagrapd.c -o instagrapd
	gcc worker.c -o worker
	
server : instagrapd.c
	gcc instagrapd.c -o instagrapd

client : worker.c
	gcc worker.c -o worker

clean :
	rm -f instagrapd
	rm -f worker