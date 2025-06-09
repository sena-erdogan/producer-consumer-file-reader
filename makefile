all:
	gcc -Wall -g -o hw4 hw4.c -pthread

clean:
	rm -rf *.o hw4
