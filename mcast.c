#include "sp.h"
#include <sys/types.h>
#include "net_include.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define int32u unsigned int
#define BUF_SIZE MAX_MASS_LEN - 4*sizeof(int)

static  char    User[80];
static  char    Spread_name[80];
static  char    *group = "ppap"; 
static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;
static  int     To_exit = 0;

#define MAX_MESSLEN     1300   //102400
#define MAX_VSSETS      10
#define MAX_MEMBERS     10

int mcast(int num_of_packets, int machine_index, int num_of_machines);
static void Bye();

int main(int argc, char const *argv[]) {

    int ret;
    int num_of_messages;
    int num_of_processes;
    int process_index;
    sprintf( User, "heirenwenhao" );
    sprintf( Spread_name, "4803");
     
    if (argc != 4) {
        printf("Usage: mcast <num_of_messages><process_index><number_of_processes>\n");
        return 1;
    }

    //get command inputs
    num_of_messages = (int)strtol(argv[1],(char **)NULL, 10);
    process_index = (int)strtol(argv[2],(char **)NULL, 10);
    num_of_processes = (int)strtol(argv[3],(char **)NULL, 10);
    if (num_of_processes > 10) {
	 printf("Usage: mcast <num_of_messages><process_index><number_of_processes>\n");
        return 1;
    }

    mcast(num_of_messages, process_index, num_of_processes);
}


int mcast(int num_of_messages, int process_index, int num_of_processes)
{    
    char              mess[MAX_MESSLEN];
    int		      currentMemberNum = 0;
    int               ret;
    int               service_type;
    int               mess_type;
    membership_info   memb_info;
    int               endian_mismatch;
    int               num_groups;
    char              target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
    char              sender[MAX_GROUP_NAME];
    int               mver, miver, pver;
    char              outputfile[80];
    packet            *currpacket;
    int               currentpacketsize;
    int               message_index = 0;
    int               burstsize = 600;
    //the flag is set to 1 if this process finishes sending
    int               all_sent = 0;
    //the flag decreases by one when it receives all packets from a process
    //it becomes zero when received all packets from all machines
    int               all_received = num_of_processes;
    sp_time	      start, end, diff;

    FILE              *fd;
    if (!SP_version( &mver, &miver, &pver))
    {         
        printf("main: Illegal variables passed to SP_version()\n");
        Bye();
    }
        
    printf("Spread library version is %d.%d.%d\n", mver, miver, pver);

    ret = SP_connect( Spread_name, User, 0, 1, &Mbox, Private_group);
    
    if( ret != ACCEPT_SESSION )
    {
        SP_error( ret );
        Bye();
    }
    printf("User: connected to %s with private group %s\n", Spread_name, Private_group );
    
    ret = SP_join(Mbox, group);
    if( ret < 0 ) SP_error( ret );
    printf("Joined %s\n", group);

    //wait for all members to join
    while (currentMemberNum < num_of_processes) {
        service_type = 0;
	ret = SP_receive(Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(mess), mess); 
	//if service type is regular member
	if (Is_membership_mess (service_type)) {
            ret = SP_get_memb_info( mess, service_type, &memb_info);
            if (ret < 0) {
                printf("membership message body invalid.\n");
                SP_error(ret);
                exit(1);
            } 
            if (Is_reg_memb_mess(service_type)) {
                printf("Now %s has %d members, where I am member: %d\n", sender, num_groups, mess_type);
                //get number of current processes joined the group
                currentMemberNum = num_groups;
            }           
        }
    }
    
    start = E_get_time();  

    //open the output file
    sprintf(outputfile, "%d.out",process_index);
    if ((fd = fopen(outputfile, "wb+")) == NULL) {
	perror("fopen");
        exit(0);
    }
    printf("now all members have joined, we can start the process.\n");

    //malloc only once
    currentpacketsize = sizeof(packet);
    currpacket = malloc(currentpacketsize);
       
    //first multicast whether itself sends 0 packets
    if(num_of_messages == 0) {
        //indicating that do not expect messages from this machine
	currpacket->type = 4;
    } else {
        //it sends messages
	currpacket->type = 3;
    }
    ret = SP_multicast( Mbox, AGREED_MESS, "ppap", 0, currentpacketsize, currpacket);
    if( ret < 0 ) {
        printf("ret < 0\n");
        SP_error( ret );
        Bye();
    }
    //collecting  information from all other machines
    int numS = num_of_processes;
    int numsent = 0;
    while (numS > 0) {
        service_type = 0;
            ret = SP_receive(Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(packet), currpacket);
            if (ret < 0) {
                SP_error(ret);
                exit(1);
            } else {
		if (currpacket->type == 3) {
		    numsent ++;
                }
	    }
	    numS--;
    }
    all_received = numsent;

    //for machines not sending anything
    if(num_of_messages == 0) {
       
         while (all_received != 0) {
           //receive
            int stoprecv = all_received; //currently, it still need to receive from all_received this many machines
            while (stoprecv != 0) { //for this round
                service_type = 0;
                ret = SP_receive(Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(packet), currpacket);
                if (ret < 0) {
                    SP_error(ret);
                    exit(1);
                } else {
                    int machine_index = currpacket->machineindex;
                    if (currpacket->type == 0) {  //finishing mark for this round
		        stoprecv--;
                    }
                    if (currpacket->type == 1) {  //finishing mark for all messages
                        all_received--;
                        stoprecv--;
                    }
                    fprintf(fd, "%2d, %8d, %8d\n", currpacket->machineindex, currpacket->packetindex, currpacket->randomnumber);
	        }
            }//while still need to receive some
        } //while still has job

    } else {  //machine that both sends and receives
        while (all_sent == 0 || all_received != 0) {  //all_sent=1 when all required are sent
            //send burst
            int i = message_index;
            for (i = message_index; i< message_index + burstsize; i++) {
                //break if no more sending
                if(all_sent == 1) {
                    break;
                }    
                if(i == num_of_messages-1) {  //last message
                    all_sent = 1;
                }
	        currpacket->type = -1; //normal
	        if (i == num_of_messages-1) {
		    currpacket->type = 1; //meaning sent all for all rounds
	        } else if(i == message_index+burstsize-1) {
		    currpacket->type = 0; //meaning sent all for this round
	        }       
                currpacket->machineindex = process_index;
                int r = ( rand() % 1000000) + 1;
                currpacket->randomnumber = r;
                currpacket->packetindex = i;
                ret = SP_multicast( Mbox, AGREED_MESS, "ppap", 0, currentpacketsize, currpacket);
                if( ret < 0 ) {
                    printf("ret < 0\n");
		    SP_error( ret );
                    Bye();
                }
            }
            message_index = i;
            //receive
            int stoprecv = all_received;
            while (stoprecv != 0) {
                service_type = 0;
                ret = SP_receive(Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(packet), currpacket);
                if (ret < 0) {
                    SP_error(ret);
                    exit(1);
                } else {
                //store the received packet info into received array
                //    int machine_index = currpacket->machineindex;
                    if (currpacket->type == 0) { //has received burstsize packets for this round
		        stoprecv--;
                    }
                    if (currpacket->type == 1) { //has received all packets from one machine
                        all_received--;
                        stoprecv--;
                    }
                    //write to file
                    fprintf(fd, "%2d, %8d, %8d\n", currpacket->machineindex, currpacket->packetindex, currpacket->randomnumber);
	        }
            }//while still need to receive some
        } //while still has job
    }
    //mark: after else
    end = E_get_time();
    diff = E_sub_time(end, start);
    printf("Duration = %lu seconds, %lu microseconds\n", diff.sec, diff.usec);
    fclose(fd);
    free(currpacket);
    ret = SP_leave( Mbox, group);
    if( ret < 0 ) SP_error( ret );
    printf("Left %s\n", group);
    return 0;    
}//mcast

static  void	Bye()
{
	To_exit = 1;

	printf("\nBye.\n");

	SP_disconnect( Mbox );

	exit( 0 );

}
