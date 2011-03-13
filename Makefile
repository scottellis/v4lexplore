# 
# native make
#

CC = gcc
CFLAGS = -Wall

v4lexplore: v4lexplore.c
	$(CC) $(CFLAGS) v4lexplore.c -o v4lexplore


clean:
	rm -f v4lexplore
