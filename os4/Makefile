CC = gcc
PROGS = oss user
CMD = $(CC) $(FLAGS)

# hoare and my computer require a different flag for c99
FLAGS = --std=c99 -D_XOPEN_SOURCE
#FLAGS = --std=c99 -D_DEFAULT_SOURCE

all: $(PROGS)

queue.o: queue.c queue.h
	$(CMD) $< -c

control.o: control.c control.h
	$(CMD) $< -c

oss: oss.c queue.o control.o
	$(CMD) $^ -o oss

user: user.c control.o
	$(CMD) $^ -o user

clean:
	rm -f $(PROGS) *.o log.out
