CC = gcc
CFLAGS = -g -lpthread

all:addtest sltest

addtest: addtest.c
	$(CC) $(CFLAGS) addtest.c -o $@

sltest: sltest.c
	$(CC) $(CFLAGS) sltest.c -o $@


lab4_files = README Makefile addtest.c sltest.c answers.txt
lab4-TianyanwuRyanLi.tar.gz: $(lab4_files)
	tar -czf $@ $(lab4_files)

submission: lab4-TianyanwuRyanLi.tar.gz

clean:
	rm -f *.o *.tar.gz addtest sltest
