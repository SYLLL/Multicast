#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <time.h>
#include <errno.h>

#define PORT	     10330
#define MAX_MESS_LEN 1216
#define WINDOW_SIZE  64

typedef struct
{
    int type;
    int machineindex;
    int packetindex;
    int randomnumber;
    char additional[1200];
} packet;

