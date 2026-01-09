CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11

OBJS=main.o graph.o topo.o schedule.o

scheduler: $(OBJS)
	$(CC) $(CFLAGS) -o scheduler $(OBJS)

main.o: main.c graph.h topo.h schedule.h
	$(CC) $(CFLAGS) -c main.c

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c

topo.o: topo.c topo.h graph.h
	$(CC) $(CFLAGS) -c topo.c

schedule.o: schedule.c schedule.h topo.h graph.h
	$(CC) $(CFLAGS) -c schedule.c

clean:
	rm -f *.o scheduler tests

test: tests.o graph.o topo.o schedule.o
	$(CC) $(CFLAGS) -o tests tests.o graph.o topo.o schedule.o
	./tests

tests.o: tests.c graph.h topo.h schedule.h
	$(CC) $(CFLAGS) -c tests.c
