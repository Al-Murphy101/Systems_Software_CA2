#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define LENGTH 512

int main(int argc, char *argv[]) { 

	//expect 2 arguments, 
	//first arg - program name 
	//seconds arg - file to be transferred
	if(argc != 3) {
		printf("Invalid number of arguments, usage: ./client <file-name> <destination>\n");
		exit(1);
	} 

	
	int SID;
	struct sockaddr_in server;
	char clientMessage[500];
	char serverMessage[500];	
	//char *filename = "index.html";
	char *filename = argv[1];
	printf("File name passed from argument: %s\n", filename);
	char *destination = argv[2]; 

	//if the user did not enter manufacturing or distribution in argv[2]
	if( !(strcmp(destination, "manufacturing") == 0 
			||
		strcmp(destination, "distribution") == 0 ) 
	  ) 	{ 
			printf("invalid destination folder, must be distribution or manufacturing\n");
			exit(1);
		}


	//create socket
	SID = socket(AF_INET, SOCK_STREAM, 0);

	if(SID == -1) { 
		printf("error creating socket\n");
	} else { 
		printf("socket created\n");
	}

	server.sin_port = htons (8801);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	

	//connect to server
	if(connect(SID, (struct  sockaddr*)&server, sizeof(server)) < 0 ) { 
		printf("connect failed. Error\n");
		return 1;
	}

	printf("Connected to the server ok\n");

	
	//get the id of the user 
	uid_t usr_id = getuid();
	uid_t converted_usr_id = htonl(usr_id);
	printf("sending id of the user to the server\n");


	//send the id of the user to the server
	if( write(SID, &converted_usr_id, sizeof(converted_usr_id)) < 0) {
		printf("send failed");
	     	return 1;	
	}



	//send the destination directory to the server
	if( write(SID, destination, strlen(destination)) < 0) { 
		printf("send failed\n");
       		return 1;
	}


	 
	if( recv(SID, serverMessage, strlen("destinationReceived"), 0) < 0)
	{
		printf("recv() error\n");
		return 1;
	}



	//send init transfer message to the server
	if( send(SID, "initTransfer", strlen("initTransfer"), 0) < 0)
	{
		printf("send failed\n");
		return 1;
	}


	int len;

	//receive reply from the server
	if( (len = recv(SID, serverMessage, strlen("filename"), 0)) < 0)
	{ 
		printf("IO error\n");
	}

	serverMessage[len] = '\0';
	printf("server sent %s\n", serverMessage);	


	//send file name to the server	
	if(strcmp(serverMessage, "filename") == 0 ) { 
		printf("Sending file %s\n", filename);
		
		if( send(SID, filename, strlen(filename), 0) < 0 ) { 
			printf("send failed\n");
			return 1;
		}

	} 

	memset(serverMessage, 0, 500);
	
	//receive reply from the server
	if( recv(SID, serverMessage, 500, 0) < 0) { 
		printf("IO error\n");
	}

	printf("Server sent %s of length %d\n", serverMessage, strlen(serverMessage) );

	if( strcmp(serverMessage, "begin") == 0 ) {
		printf("sending file %s\n", filename);

		char *fs_path = "/home/alex/Desktop/ca2/client_files/";
		char *fs_name = (char * ) malloc( 1 + strlen(fs_path) + strlen(filename) );
		strcpy(fs_name, fs_path);
		strcat(fs_name, filename);

		char sdbuf[LENGTH];
		printf("Client sending %s to server ...\n", fs_name);
		FILE *fs = fopen(fs_name, "r");
		
		if(fs == NULL) { 
			printf("Error: %s, file not found\n", fs_name);
			return 1;
		}

		bzero(sdbuf, LENGTH);
		int fs_block_sz, i = 0;

		while( (fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0 ) {
			printf("Data sent %d = %d", i , fs_block_sz);
			if( send(SID, sdbuf, fs_block_sz, 0) < 0) { 
				fprintf(stderr, "Error: failed to send file %s, errorno: %d\n", fs_name, errno);
				exit(1);
			}
			
			bzero(sdbuf, LENGTH);
			++i;
		}
	}

	close(SID);
	return 0;
}



