#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include<strings.h>
#include<string.h>
#include "protocolFT.h"


int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr, cli_addr;
	if(argc<2)
	{
		fprintf(stderr, "ERROR,no port provided.\n");
		exit(1);
	}int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		fprintf(stderr, "ERROR,opening socket.\n");
		exit(1);
	}bzero((char *)&serv_addr, sizeof(serv_addr));
	int portno;
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "ERROR,binding socket.\n");
		exit(1);
	}
	printf("listening for the client........\n");
	listen(sockfd,5);
	int clilen;
	clilen = sizeof(cli_addr);	
	int newsockfd;
	newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
	if(newsockfd < 0)
	{	
		fprintf(stderr, "ERROR,new socket.\n");
		exit(1);
	}	
	char buffer[256];
	bzero(buffer, 256);
	int n;
	n = read(newsockfd, buffer, 255);
	if(n < 0)
	{
		fprintf(stderr, "ERROR,reading from socket.\n");
		exit(1);	
	}
	printf("File path received from client: \"%s\"\n",buffer);
	FILE *ptrFile;
	ptrFile = fopen(buffer,"rb");
	if(!ptrFile)
	{
		fprintf(stderr, "ERROR,opening file.\n");
		exit(1);	
	}
	printf("file opening success.\n");
	//finding size of file
	unsigned long int fileSize;
	fseek(ptrFile,0,SEEK_END);
	fileSize = ftell(ptrFile);
	rewind(ptrFile);
	printf("size of file = %d\n",fileSize);
	unsigned char *fileArray;	
	//calloc automatically sets all bytes to zero after allocating
	fileArray = (unsigned char*) calloc(fileSize , sizeof(char));
	if(!fileArray)
	{
		printf("memory allocation of fileArray failed!!!\n");
		exit(1);
	}
	unsigned int readSize;
	readSize = fread(fileArray, sizeof(char), fileSize ,ptrFile);
	if(readSize != fileSize)
	{
		printf("error in reading of the %s file\n", buffer);
		printf("read size = %d\n", readSize);
		exit(1);
	}
	else printf("Reading success!!!\n");
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
		packet.head.sendNo = repeatNo;
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
		char acknowledgment;
		do
		{
			send(newsockfd,&packet,sizeof(packet),0);
			printf("Percentage sent : %.2f%%\r",(float)(i * 100)/ fileSize );
			n = read(newsockfd, &acknowledgment, 1);
			if(n < 0)
			{
				fprintf(stderr, "ERROR,reading packet send acknowledge from socket.\n");
				exit(1);	
			}
			repeatNo++;
			packet.head.sendNo = repeatNo;
		}while(acknowledgment == 'f');

	}
	printf("Percentage sent : 100.00%%    \n");
	bzero(buffer, 256);
	strcpy(buffer,"End of file transmission.");
	n = write(newsockfd, buffer, strlen(buffer));
	if (n < 0)
	{
		fprintf(stderr, "ERROR, sending end of file msg to client.\n");
		exit(1);	
	}	
	printf("Program ends here.\n");
	scanf("\n");	
	return 0;
}

















