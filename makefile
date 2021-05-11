OBJ = environmentServer.o robotClient.o display.o stop.o

all: main robot stop display
	gcc -o environmentServer environmentServer.o display.o -lX11 -lm -lpthread
	gcc -o robotClient robotClient.o -lm -lpthread
	gcc -o stop stop.o -lpthread

main: environmentServer.c simulator.h
	gcc -c environmentServer.c

robot: robotClient.c simulator.h 
	gcc -c robotClient.c

stop: stop.c simulator.h
	gcc -c stop.c

display: display.c simulator.h
	gcc -c display.c

clean:
	rm -f  $(OBJ) environmentServer robotClient stop
