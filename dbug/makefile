OBJS=example.o dbug.o
example:$(OBJS)
	gcc -o example $(OBJS)
example.o:dbug.h
	gcc -c example.c
dbug.o:dbug.h
	gcc -c dbug.c
.PHONY:clean
clean:
	rm -f *.exe $(OBJS)
