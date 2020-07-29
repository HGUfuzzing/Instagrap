all : instagrapd.c submitter.c
	gcc instagrapd.c -o instagrapd
	gcc submitter.c -o submitter

submitter : submitter.c
	gcc submitter.c -o submitter

instagrapd : instagrapd.c
	gcc instagrapd.c -o instagrapd

clean :
	rm -f instagrapd
	rm -f submitter