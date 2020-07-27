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

int main(int argc, char *argv[])
{
	//Setting up socket to connect to Server
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	PERROR(argc < 3,"Please specify server ip and port pair!\nExpecting two arguments.\n")
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	PERROR(sockfd < 0,"ERROR opening socket!\n")
	server = gethostbyname(argv[1]);
	PERROR(server == NULL,"ERROR, no such host!\n")
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	PERROR(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0,"ERROR connnecting")
	char filePath[256];
	bzero(filePath,256);
	printf("please enter the file path: ");
	scanf("%s",filePath);
	bzero(buffer,256);
	printf("please enter the output file name to be saved: ");
	scanf("%s",buffer);
	PERROR(write(sockfd, filePath, strlen(filePath))<0,"ERROR writing file name to socket")
	char acknowledgment;
	PERROR(read(sockfd, &acknowledgment, 1)<0,"Error receiving acknowledgment from server")
	while(acknowledgment!=FILE_READY)
	{
		bzero(filePath,256);
		PERROR(acknowledgment!=FILE_OPEN_FAILED,"Conflicting message from server.\n")
		printf("Message from server: Failed to open file!\nPlease reenter the file path: ");
		scanf("%s",filePath);
		PERROR(write(sockfd, filePath, strlen(filePath))<0,"ERROR writing file name to socket")
		PERROR(read(sockfd, &acknowledgment, 1)<0,"Error receiving acknowledgment from server")
	}
	//Request sent to server for File Transfer......


	//From here on handling with file Transfer.......
	struct Packet packet;
	bzero(&packet,sizeof(packet));
	FILE *file;
	file=fopen(buffer,"wb");
	PERROR(file == NULL ,"Failed to open output file!\n" )
	unsigned long int filePoint = 0;
	unsigned char clientCrc[2];
	while(1)   // File Transfer loop
	{
		PERROR(read(sockfd,&packet,sizeof(packet))<0,"ERROR receiving file from server\n")
		clientCrc[0] = 0;
		clientCrc[1] = 0;
		crcgenerator(packet.dataArray,packet.head.dataSize,clientCrc);
		if((packet.crc[0] == clientCrc[0]) && (packet.crc[1] == clientCrc[1]))
		{
			acknowledgment = PACKET_GOOD;
			PERROR(write(sockfd, &acknowledgment, 1)<0,"ERROR writing positive acknwldg to server\n")
			filePoint += DATA_ARRAY_SIZE;
			if (filePoint >= packet.head.fileSize)
			{
				int extraBytes = packet.head.fileSize + DATA_ARRAY_SIZE - filePoint;
				PERROR(extraBytes!=packet.head.dataSize,"End of file from server does not match with client side\n")
				fwrite(packet.dataArray,extraBytes,1,file);
				filePoint = packet.head.fileSize;
				printf("Percentage received : 100.00%%     \n");
				break;
			}
			fwrite(packet.dataArray,DATA_ARRAY_SIZE,1,file);
		}
		else
		{
			printf("Data integrity failed!\n");
			acknowledgment = CRC_MISMATCH;
			PERROR(write(sockfd,&acknowledgment,1)<0,"ERROR writing negative acknwldg to server\n")
		}
		printf("Percentage received : %.2f%%\r",
		(float)(filePoint * 100)/ packet.head.fileSize );
	}
	printf("File received successfully.\nTotal file size : %d\n",filePoint);
	printf("Saving file as \"%s\"\n",buffer);
	fclose(file);
	bzero(buffer,256);
	PERROR(read(sockfd, buffer, 256)<0,"ERROR reading end of file msg from server\n")
	printf("End of file message from server. \n");
	printf("Program ends here.\n");
	scanf("\n");
	return 0;
}
