// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8000
#define StSize 10000
char b[1000];
int pipeerror = 0;
void pipehandler(int signum)
{
	printf("The connection is lost .Server might be down \n");
	exit(EXIT_FAILURE);
}

#define int long long int

void progress(int untilnow, int length)
{
	sprintf(b,"\rProgress = %0.3f",((float)(100*(untilnow)))/(float)(length));
	write(1,&b,sizeof(b));
}
signed main(int argc, char const *argv[])
{
	signal(SIGPIPE,pipehandler); // To handle the situations whenever the server exit abruptly
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[10000] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
	// which is meant to be, and rest is defined below

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Converts an IP address in numbers-and-dots notation into either a 
	// struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	if(argc == 1)
	{
		printf("No files are provided :(\n");
	}
	/*send(sock , hello , strlen(hello) , 0 );  // send the message.
	  printf("Hello message sent\n");
	  valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
	  printf("%s\n",buffer);*/
	for(int i = 1;i<argc;i++) // To loop through all the files
	{
		memset(buffer , '\0', StSize);
		int fdwrite;
		printf("\nThe file to be downloaded from server is %s\n",argv[i]);
		//	printf("arg is %s\n",argv[i]);
		if(send(sock, argv[i], strlen(argv[i]), 0) < 0) // Sending the file name to server
		{
			printf("FOUND\n");
		}
		memset(buffer,'\0',StSize);
		int filesize = 0,recieved_status = 0;
		//printf("SUSPECT!\n");
		memset(buffer, '\0', StSize);
		recieved_status = recv(sock, buffer, sizeof(buffer),0); // In response clients gets the file size

		//printf("SUSPECT2\n");
		if(recieved_status > 0)
		{
			if(strcmp(buffer, "ABORT") == 0) // If the file is not present in the server 
			{
				printf("%s : THE FILE is not able to be transfered\n", argv[i]);
				continue;
			}
			else // It sends the size of the file
			{
				//		    printf("befire atoi is %s\n",buffer);
				filesize = atoll(buffer);
				/*
				   printf("file size is %lld\n",filesize);
				   printf("file name is %s\n",argv[i]);*/
				fdwrite = open(argv[i],O_WRONLY | O_CREAT | O_TRUNC,0777); // Open the file in client so that we save the downloaded file
				if(fdwrite<0) // If the file in client side has no write permission
				{
					printf("%s : Unable to open the file. File doesn't have sufficient permissions\n",argv[i]);
					// printf("The File is ");
					memset(buffer,'\0',StSize);
					strcpy(buffer, "ABORT");

					send(sock, buffer, strlen(buffer), 0);
					memset(buffer, '\0',StSize);
					recv(sock, buffer, StSize,0);
					continue;
				}
				else
				{
					memset(buffer,'\0',StSize);
					strcpy(buffer, "START");
					send(sock, buffer, strlen(buffer), 0);
				}
			}
		}
		else // If the server exits abruptly
		{
			printf("Junk was received. Information might got corrupted\n");     
			memset(buffer,'\0',StSize);
			strcpy(buffer, "ABORT");
			send(sock, buffer, strlen(buffer), 0);
			continue;
		}
		int rbsize = 0;
		int untilnow = 0;

		memset(buffer,'\0',StSize);
		if(filesize>0)
		{		
			printf("%s : File sharing has started\n", argv[i]);
			rbsize = recv(sock, buffer , StSize,0);
		}
		// Start receiving the file
		//printf("BUFFER IS %s rbsize is %lld\n", buffer,rbsize);
		while(rbsize>0)
		{
			//	   printf("ENTERED with rbsoze %lld\n",rbsize);
			int written = write(fdwrite, buffer, rbsize); // write the received content to the destination file
			progress(untilnow, filesize);  // print progress
	
		
			untilnow += rbsize;
			if(written < rbsize) // if the write fails 
			{
				memset(buffer,'\0',StSize);
	                        if(send(sock, "STOPPAY", sizeof("STOPPAY"), 0)<=0) // If any error happens
				{
					break;
				}

				printf("%s : FILE WAS CORRUPTED\n", argv[i]);
				break;
			}
		        memset(buffer,'\0',StSize);
                        send(sock, "NICE", sizeof("NICE"), 0); // Send acknowledgement

			if(untilnow >= filesize) // Check whether the file is completely obtained
			{
				//		  printf("CULPRIT %lld\n", untilnow);
				sprintf( b,"\rProgress = %0.3f",((float)(100)) );

				write(1,&b,sizeof(b));
				printf("\n%s : File is received\n",argv[i]);
				memset(buffer, '\0', StSize); 
				break;
			}
			memset(buffer, '\0', StSize);   
			rbsize = recv(sock, buffer , StSize,0);
			//	   printf("rbsize is %lld \n",rbsize);
		}
		if(recv(sock, buffer, StSize, 0)<=0) // if server exits abruptly
                {
                    	printf("\nThe connection is lost .Server might be down \n");
			exit(EXIT_FAILURE);
		}


	}
	if(argc>1)
	{
		printf("CLient is disconnecting\n");
	}
	send(sock,"EXITMANEXITMAN",sizeof("EXITMANEXITMAN"),0); // send acknowledgement that client workis done

	return 0;
}
