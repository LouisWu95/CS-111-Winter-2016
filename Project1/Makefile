CC = gcc
CFLAGS = -g -Wall -Wextra -Wno-unused -Werror -std=c11
LAB = 1
DISTDIR = lab1-TianyanWuRyanLi

all: simpsh

TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))

SIMPSH_SOURCES = \
  simpsh.c 
SIMPSH_OBJECTS = $(subst .c,.o,$(SIMPSH_SOURCES))

DIST_SOURCES = \
  $(SIMPSH_SOURCES) Makefile README catch.h option.h verbose.h execline1 execline2 execline3 output1.txt output2.txt error.txt check-dist $(TESTS)

simpsh: $(SIMPSH_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(SIMPSH_OBJECTS)

#alloc.o: alloc.h
#execute-command.o main.o print-command.o read-command.o: command.h
#execute-command.o print-command.o read-command.o: command-internals.h

dist: $(DISTDIR).tar.gz

$(DISTDIR).tar.gz: $(DIST_SOURCES) check-dist
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(DIST_SOURCES)
	./check-dist $(DISTDIR)
	mv $@.tmp $@

check: $(TEST_BASES)

$(TEST_BASES): simpsh
	./$@.sh

clean:
	rm -fr *.o *~ *.bak *.tar.gz core *.core *.tmp simpsh $(DISTDIR)

.PHONY: all dist check $(TEST_BASES) clean
