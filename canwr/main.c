#include "main.h"

/* Add function definitions here */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>


#ifdef __cplusplus    // If used by C++ code,

extern "C" {          // we need to export the C interface
#endif
         
/*
// gcc -c -Wall -Werror -fpic ./canwr.c -DUNIX
// gcc -shared -o libcanwr.so canwr.o
*/

//------------------------------------------------------------------------------------------------
int		cansock = -1;
//------------------------------------------------------------------------------------------------
extern	int can_init(int filter[], int filtermask[], int num_filters) {
	        int nbytes;
	        struct sockaddr_can addr;
	        struct can_frame frame;
	        struct ifreq ifr;
	
	        char *ifname = "can0";
	
			system("sudo ip link set can0 up type can bitrate 500000");
			sleep(3);
			
	        if((cansock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
	                perror("Error while opening socket");
	                return -1;
	        }
	
	        strcpy(ifr.ifr_name, ifname);
	        ioctl(cansock, SIOCGIFINDEX, &ifr);
	        addr.can_family  = AF_CAN;
	        addr.can_ifindex = ifr.ifr_ifindex;
	        
			struct can_filter rfilter[num_filters];
			int i;
			for(i=0; i<num_filters; i++) {
				rfilter[i].can_id   = filter[i];
				rfilter[i].can_mask = filtermask[i];
			}
		
			setsockopt(cansock, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
		
	        if(bind(cansock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	                perror("Error in socket bind");
	                return -2;
	        }
	        fcntl(cansock, F_SETFL, O_RDWR );					// nisem ziher, da je potrebno...
	        fcntl(cansock, F_SETFL, O_NONBLOCK); 			// ce ni na koncu, read blokira :/
	        return 0;
}
//------------------------------------------------------------------------------------------------
extern	int can_write(int id, char data[], int n) {
	        struct can_frame frame;
		
	        frame.can_id  = id;
	        frame.can_dlc = n;
	        memcpy(frame.data,data,n);
	
	        return write(cansock, &frame, sizeof(frame));
}
//------------------------------------------------------------------------------------------------
extern	int can_read(char data[], int len) {
			struct can_frame frame;
			int n,bytes=0;
			while(bytes < len-sizeof(frame)) {
				n=read(cansock,&frame, sizeof(frame));
				if(n <= 0) 
					return bytes;
				memcpy(&data[bytes],&frame,sizeof(frame));
				bytes += sizeof(frame);
			}
			return bytes;
}
//------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
