CC=gcc
OBJS=md5.o test.o
test:$(OBJS)
	$(CC) -o test $(OBJS)
md5.o:md5.h md5.c
.PHONY:clean
clean:
	rm -f *.exe $(OBJS)
