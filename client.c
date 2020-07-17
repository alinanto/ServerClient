#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "protocolFT.h"

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	//Setting up socket to connect to Server
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	if (argc < 3)
	{
		error("Please specify server ip and port pair!\n");
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if (sockfd < 0)
	{
		error("ERROR opening socket");
	}

	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		error("ERROR, no such host\n");
	}
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR connnecting");
	}
	char filePath[256];
	bzero(filePath,256);
	printf("please enter the file path: ");
	scanf("%s",filePath);
	bzero(buffer,256);
	printf("please enter the output file name to be saved: ");
	scanf("%s",buffer);
	n = write(sockfd, filePath, strlen(filePath));
	if (n < 0)
	{
		error("ERROR writing file name to socket");
	}
	//Request sent to server for File Transfer......

	//From here on handling with file Transfer.......
	struct Packet packet;
	bzero(&packet,sizeof(packet));
	FILE *file;
	file=fopen(buffer,"wb");
	if(file == NULL)
	{
		error("Failed to open output file!\n");
	}
	unsigned long int filePoint = 0;
	unsigned char clientCrc[2];
	char acknowledgment;
	while(1)   // File Transfer loop
	{
		n=recv(sockfd,&packet,sizeof(packet),0);
		if (n < 0)
		{
			error("ERROR receiving file from server\n");
		}
		clientCrc[0] = 0;
		clientCrc[1] = 0;
		crcgenerator(packet.dataArray,packet.head.dataSize,clientCrc);
		if((packet.crc[0] == clientCrc[0]) && (packet.crc[1] == clientCrc[1]))
		{
			acknowledgment = 's';
			n = write(sockfd, &acknowledgment, 1);
			if (n < 0)
			{
				error("ERROR writing positive acknwldg to server\n");
			}
			filePoint += DATA_ARRAY_SIZE;
			if (filePoint >= packet.head.fileSize)
			{
				int extraBytes = packet.head.fileSize + DATA_ARRAY_SIZE - filePoint;
				if(extraBytes!=packet.head.dataSize)
				{
					printf("Unexpected end of file while parsing\n");
					error("End of file from server does not match with client side\n");
				}
				fwrite(packet.dataArray,extraBytes,1,file);
				filePoint = packet.head.fileSize;
				printf("Percentage received : 100.00%%     \n");
				break;
			}
			fwrite(packet.dataArray,DATA_ARRAY_SIZE,1,file);
		}
		else
		{
			printf("data integrity failed.\n");
			acknowledgment = 'f';
			n = write(sockfd, &acknowledgment,1);
			if (n < 0)
			{
				error("ERROR writing negative acknwldg to server\n");
			}
		}
		printf("Percentage received : %.2f%%\r",
		(float)(filePoint * 100)/ packet.head.fileSize );
	}
	printf("File received successfully.\nTotal file size : %d\n",filePoint);
	printf("Saving file as \"%s\"\n",buffer);
	fclose(file);
	bzero(buffer,256);
	n = read(sockfd, buffer, 256);
	if (n < 0)
	{
		error("ERROR reading end of file msg from server\n");
	}
	printf("End of file message from server : %s\n",buffer);
	printf("Program ends here.\n");
	scanf("\n");
	return 0;

}
