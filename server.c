#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
extern int errno;
#define PORT 8000
#define StSize 10000
void pipehandler(int signum)
{
	printf("Client exited \n");
}
#define int long long int
char b[1000];
void progress(int untilnow, int length)//  to print the progress
{
        sprintf(b,"\rProgress = %0.3f",((float)(100*(untilnow)))/(float)(length));
        write(1,&b,sizeof(b));
}

signed main(int argc, char const *argv[])
{
	signal(SIGPIPE, pipehandler);  // To handle when the client exit abruptly
	int server_fd, new_socket, valread;
	struct sockaddr_in address;  
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[10000] = {0};
	char *hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// This is to lose the pesky "Address already in use" error message
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
	address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
	address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
				sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Port bind is done. You want to wait for incoming connections and handle them in some way.
	// The process is two step: first you listen(), then you accept()
	if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		printf("\nWaiting for client\n");
		int file ;
		// returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		printf("Client connected\n");
		while(1)
		{
		
			//printf("FOR REMAINING\n");
			char filename[StSize];
			//printf("Buffer before memset is %s\n",buffer);
			memset(buffer,'\0',StSize);
			//printf("Buffer after memset is %s\n", buffer);
			if(recv(new_socket, buffer,StSize, 0) <= 0) // Receive the filename from the client
			{
			
				break;
			}
			//printf("SUXPECT!!\n");

			if(buffer == NULL) // If NULL exit
			{
				printf("Disconnecting . Null is send by the client\n");
				memset(buffer,'\0', StSize);
				send(new_socket, "ABORT", sizeof("ABORT"), 0);
				break;
			}

			if(strcmp(buffer, "EXITMANEXITMAN")==0) // If the clients says that it work is completed
			{
				break;
			}
			strcpy(filename, buffer);
			printf("The file asked by the client is %s\n",filename);
			file = open(filename, O_RDONLY);
			if(file < 0) // Check if the file exists.if already exists we will overwrite
			{
				printf("%s : The Server is unable to send this file.\n",filename);
				memset(buffer,'\0', StSize);
				send(new_socket, "ABORT", sizeof("ABORT"), 0);
				continue;
			}
			//printf("file descriptor is %d\n",file);
			int length;
			//	length = ftell(file);
			struct stat st;
			stat(filename , &st);
			length = st.st_size; // obtaint the number of characters in the file
		//	printf("length is %lld\n",length);
			//	lseek(file , 0,SEEK_SET);
			int converted = length;
			memset(buffer,'\0',StSize);
			sprintf(buffer, "%lld", converted);
			if(send(new_socket, buffer, StSize,0)<=0)
			{
				printf("\nConnection broken abruptly\n");
				break;
			}
			memset(buffer, '\0', StSize); 
			if(recv(new_socket, buffer,StSize, 0)>0) // Receive whether the client is willing to receive or not
			{
				if(strcmp(buffer , "ABORT") == 0) // if there are permission problems in the client side
				{
					printf("%s : There may be problem at the client end\n", filename);
					if(send(new_socket, "FINE", sizeof("FINE"),0)<=0)
					{
						printf("\nConnection broken abruptly\n");
						break;
					}
					continue;
				}
				else if(strcmp(buffer , "START") == 0)
				{
					printf("%s : FILE SHARING STARTS\n", filename);
				}
				else
				{
					//printf("Buffer is %s\n",buffer);
				//	printf("wierd\n");
					continue;
				}
			}
			else
			{
				printf("Connection broken abruptly\n");
				perror("junk!! In recv : ");
				break;
			}

			memset(buffer,'\0',StSize);
			int untilnow = 0;
			int noofread = read(file,buffer, StSize); // read from the file
			while(noofread > 0)
			{
				untilnow += noofread;
			//	printf("noof read is %d\n", noofread);
				//printf("Socket is %d\n buffer is %s\n ", new_socket, buffer);
				if(send(new_socket, buffer, noofread,0) <= 0) // Send it to the client
				{
					perror("send ");  
					printf("%s : Transcation failed\n",filename);
					break;
				}
				progress(untilnow, length); // print progress
				memset(buffer,'\0',StSize);
				//printf("Done once\n");
				if(recv(new_socket, buffer, StSize, 0)<=0) // Receive acknowledgement
				{
					printf("Connection broken abruptly......\n");
					break;
				}
				if(strcmp(buffer,"STOPPAY")==0)
				{
					break;
				}
				//printf("buffer is %s \n ",buffer);
				memset(buffer, '\0', StSize);
				noofread = read(file,buffer, StSize);
				if(untilnow>=length)
				{
					sprintf(b,"\rProgress = %0.3f",((float)(100)) );
				        write(1,&b,sizeof(b));
					printf("\n%s : File transfer is over\n",filename);
					break;
				}
			}
			
			if(send(new_socket,"ENDED",strlen("ENDED"),0) <= 0)
			{
				printf("\n%s : SEND has failed . Aborting the transcation\n",filename);
				break;
			}
			//	send(new_socket, "Transcationended", sizeof("Transcationended"),0);


			/*valread = read(new_socket , buffer, 1024);  // read infromation received into the buffer
			  printf("%s\n",buffer);
			  send(new_socket , hello , strlen(hello) , 0 );  // use sendto() and recvfrom() for DGRAM
			  printf("Hello message sent\n");
			  */

			//close(new_socket);
		}

		printf("Client got disconnected\n");
	}
        printf("ENDED\n");
	
	return 0;
}
