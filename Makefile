CC=gcc
LD=gcc
CFLAGS=-g -Wall 
CPPFLAGS=-I. -I/home/cs437/exercises/ex3/include
SP_LIBRARY_DIR=/home/cs437/exercises/ex3

all: sp_user class_user mcast

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

sp_user:  $(SP_LIBRARY_DIR)/libspread-core.a user.o
	$(LD) -o $@ user.o  $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a

class_user:  $(SP_LIBRARY_DIR)/libspread-core.a class_user.o
	$(LD) -o $@ class_user.o $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a

mcast:  $(SP_LIBRARY_DIR)/libspread-core.a mcast.o
	$(LD) -o $@ mcast.o $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a

clean:
	rm -f *.o sp_user class_user
	rm mcast


