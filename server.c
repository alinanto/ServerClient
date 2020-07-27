#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include "protocolFT.h"

int main(int argc, char *argv[])
{
	PERROR(argc < 2,"No port specified!\nExpected one argument.\n")
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	PERROR(sockfd < 0,"Failed to create socket.\n")
	bzero((char *)&serv_addr, sizeof(serv_addr));
	int portno;
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	PERROR(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 , "Failed to bind socket.\n")
	listen(sockfd,5);
	printf("listening for the client........\n");
	int clilen;
	clilen = sizeof(cli_addr);
	int newsockfd;
	newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
	PERROR(newsockfd < 0 , "Failed to accept connection.\n")
	//Connected to client

	//getting file path from client and opening file
	char buffer[256];
	FILE *ptrFile;
	bzero(buffer, 256);
	PERROR(read(newsockfd, buffer, 255)<0,"Error receiving file path from client")
	printf("File path received from client: \"%s\"\n",buffer);
	ptrFile = fopen(buffer,"rb");
	char acknowledgment;
	while(ptrFile==NULL)
	{
		acknowledgment = FILE_OPEN_FAILED;
		PERROR(write(newsockfd,&acknowledgment,1)<0,"Failed to send file prompt message to client.\n")
		bzero(buffer, 256);
		PERROR(read(newsockfd, buffer, 255)<0,"Error receiving file path from client")
		printf("File path received from client: \"%s\"\n",buffer);
		ptrFile = fopen(buffer,"rb");
	}
	acknowledgment = FILE_READY;
	PERROR(write(newsockfd,&acknowledgment,1)<0,"Failed to send acknowledgment to client.\n")
	printf("File opened.\n");

	//finding size of file
	unsigned long int fileSize;
	fseek(ptrFile,0,SEEK_END);
	fileSize = ftell(ptrFile);
	rewind(ptrFile);
	printf("Size of file = %d\n",fileSize);

	//Loading file to dynamic array
	unsigned char *fileArray;
	fileArray = (unsigned char*) calloc(fileSize , sizeof(char));
	PERROR(!fileArray,"Failed to allocate memory for loading file!\n")
	unsigned int readSize;
	readSize = fread(fileArray, sizeof(char), fileSize ,ptrFile);
	PERROR(readSize != fileSize, "Failed to read from file!\n")
	printf("Reading success.\n");
	fclose(ptrFile);
	//File loading completed

	//Starting to send file to client
	unsigned long int i=0,j=0;
	int packetNo = 1;
	int repeatNo = 0;
	struct Packet packet;

	//making each packets...
	for(i = 0; i < fileSize; i += DATA_ARRAY_SIZE )
	{
		packet.head.fileSize = fileSize;
		packet.head.sequenceNo = packetNo;
		packet.head.repeatNo = repeatNo;
		packet.head.dataSize = DATA_ARRAY_SIZE;
		for(j = 0; j < DATA_ARRAY_SIZE ; j++)
		{
			if(i+j<fileSize)
			{
				packet.dataArray[j] = fileArray[i+j];
			}
			else if(i+j == fileSize)
			{
				packet.dataArray[j] = 0;
				packet.head.dataSize = j;
			}
			else
			{
				packet.dataArray[j] = 0;
			}
		}
		packet.crc[1] = 0;
		packet.crc[0] = 0;
		crcgenerator(packet.dataArray,packet.head.dataSize,packet.crc);
		packetNo++;

		repeatNo = 0;
		acknowledgment = CRC_MISMATCH;
		do
		{
			PERROR(acknowledgment != CRC_MISMATCH,"Conflicting message from client.\n")
			send(newsockfd,&packet,sizeof(packet),0);
			printf("Percentage sent : %.2f%%\r",(float)(i * 100)/ fileSize );
			PERROR(read(newsockfd, &acknowledgment, 1)<0,"ERROR,reading packet send acknowledge from socket.\n")
			repeatNo++;
			packet.head.repeatNo = repeatNo;
		}while(acknowledgment != PACKET_GOOD );
	}
	printf("Percentage sent : 100.00%%    \n");
	acknowledgment = END_OF_FILE;
	PERROR(write(newsockfd,&acknowledgment,1)<0,"ERROR, sending end of file msg to client.\n")
	printf("Program ends here.\n");
	scanf("\n");
	return 0;
}
