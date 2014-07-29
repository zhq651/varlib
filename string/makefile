CC=gcc
OBJS=example.o strbuf.o
example:$(OBJS)
	$(CC) -o example $(OBJS)
example.o:example.c strbuf.h strbuf.c
strbuf.o:strbuf.h strbuf.c
.PHONY:clean
clean:
	rm -f *.exe $(OBJS)
