CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11

OBJS=main.o graph.o topo_sort.o schedule.o

scheduler: $(OBJS)
	$(CC) $(CFLAGS) -o scheduler $(OBJS)

main.o: main.c graph.h topo_sort.h schedule.h
	$(CC) $(CFLAGS) -c main.c

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c

topo_sort.o: topo_sort.c topo_sort.h graph.h
	$(CC) $(CFLAGS) -c topo_sort.c

schedule.o: schedule.c schedule.h topo_sort.h graph.h
	$(CC) $(CFLAGS) -c schedule.c

clean:
	rm -f *.o scheduler
