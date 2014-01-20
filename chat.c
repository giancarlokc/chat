#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 512  //Max length of buffer
#define SIZE_SERVER 100
#define SIZE_NAME 1000

char SERVER[SIZE_SERVER];
int PORT_IN, PORT_OUT, ID;
char name[SIZE_NAME];

/* In case of error call this functions */
void error(char *s){
	printf("Error: %s\n", s);
}

/* HELP */
void help(){
	printf("Usage: ./chat [ID] [ADRESS] [PORT_IN] [PORT_OUT]\n");
}

/* Check arguments */
int getArgs(int argc, char **argv){
	if(argc > 1 && !strcmp(argv[1],"help")){
		help();
		return 0;
	}
	if(argc != 5)
		return 0;
	PORT_IN = atoi(argv[3]);
	PORT_OUT = atoi(argv[4]);
	strcpy(SERVER, argv[2]);
	ID = atoi(argv[1]);
	return 1;
}

/* Setup the socket config */
int setupSocket(int *s, struct sockaddr_in *si_other){
	// Create UDP socket
	if ((*s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		error("create_socket");
		return 0;
	}

	// Prepare the sockaddr_in structure
	memset((char *) si_other, 0, sizeof(*si_other));
	si_other->sin_family = AF_INET;
	si_other->sin_port = htons(PORT_OUT);
    
	if (inet_aton(SERVER , &si_other->sin_addr) == 0) {
		error("init_socket");
		return 0;
	}
	return 1;
}

/* mount the package to be sent */
void mountPackage(char *s, int dest, char *name){
	int i = 0, s_name = strlen(name), aux;
	char mesg[BUFLEN];

	int n_mesg = strlen(s);
	strcpy(mesg, s);
	memset(s, '\0', sizeof(char)*BUFLEN);
	s[0] = ID + '0';
	s[1] = dest + '0';
	strcpy(&(s[2]),name);
	strcpy(&(s[12]),mesg);
	s[2 + strlen(name)] = '\0';
	s[12 + n_mesg - 1] = '\0';
}

/* This will handle connection for each client */
void *connection_handler(void *socket_desc){
	struct sockaddr_in si_me, si_other, si_next;
     
	int s, i, slen = sizeof(si_other) , recv_len;
	int s_next, slen_next = sizeof(si_next);
	char buf[BUFLEN];

	setupSocket(&s_next, &si_next);
     
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		error("socket");
	}
     
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
     
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT_IN);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
	//bind socket to PORT_OUT
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
		error("bind");
	}
     
	//keep listening for data
	while(1){
		fflush(stdout);
         
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1){
			error("recvfrom()");
		}
         
		//print details of the client/peer and the data received
//		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
//		printf("Data: %s\n",buf);
//		printf("FROM: %c\n",buf[0]);
//		printf("TO: %c\n",buf[1]);
//		printf("NAME: %s\n",&(buf[2]));
//		printf("MESG: %s\n",&(buf[12]));
		if(buf[1] - '0' != ID){
			printf("\n%s says: %s\n" , &(buf[2]), &(buf[12]));
			printf("%s says: ", name);
			if(sendto(s_next, buf, 100 , 0 , (struct sockaddr *) &si_next, slen_next) == -1)
				return;
		}
	}
 
	close(s);
	return;
}

int main(int argc, char **argv){
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other), *new_sock;
	char buf[BUFLEN];
	char message[BUFLEN];
 
	if(!getArgs(argc, argv)){
		error("get_args");
		return 1;
	}

	if(!setupSocket(&s, &si_other)){
		error("setup_socket");
		return 1;
	}

	// Start the thread that handles the incoming messages */
	pthread_t sniffer_thread;
	if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0){
		error("pthread_create");
		return 1;
   	}

	// get name
	printf("Your name: ");
	fgets(name, SIZE_NAME, stdin);
	name[strlen(name) - 1] = '\0';

	while(!strcmp(message,"!exit\n") != 1){ // main loop

		printf("%s says: ", name);
		fgets(message, BUFLEN, stdin);

		if(message[0] == '!'){ 	// case message is a command
			
		} else { 		// case normal message
			//send the message
			mountPackage(message, ID, name);
//			printf("Sending package: %s\n",message);
			if (sendto(s, message, 100 , 0 , (struct sockaddr *) &si_other, slen) == -1)
				return 1;
		}
	}

	close(s);
	return 0;
}
