#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>

//informations inside header

#define DATA_ARRAY_SIZE 1024  // 256 bytes of data in one packet

struct Header
{
	unsigned long int fileSize;
	unsigned long int dataSize;
	unsigned long int sequenceNo;
	unsigned int sendNo;
};

struct Packet 
{
		struct Header head;
		char dataArray[DATA_ARRAY_SIZE];
		unsigned char crc[2];
};

//crc function
//this is only converting 1 byte, so put all array in a for loop for get crc
void crcgenerator(unsigned char *byte, unsigned long int size,unsigned char *crc)
{
	unsigned int D[16];
	int i,j,k;
	for(int i=0;i<16;i++)
	{
		D[i] = 0;
	}
	for ( i = 0; i < size; i++)
	{	//converting byte to bits
		int bit[8];
		
		for ( j = 7; j >= 0; j--) //i =7 becoz ,8 bit are creating from 1 byte
		{
		    bit[j] = (byte[i] >> j) & 1;
		  
		}
	 

		int feedback=0;
		
		for(k =0; k < 8; k++)
		{
			//printf("%d",bit[i]);
			feedback = bit[k] ^ D[15];
			D[15] = D[14] ^ feedback;
			D[14] = D[13];
			D[13] = D[12];
			D[12] = D[11];
			D[11] = D[10] ^ feedback;
			D[10] = D[9];
			D[9] = D[8];
			D[8] = D[7];
			D[7] = D[6];
			D[6] = D[5];
			D[5] = D[4];
			D[4] = D[3] ^ feedback;
			D[3] = D[2];
			D[2] = D[1];
			D[1] = D[0];
			D[0] = feedback;
		}
		
	}
	
		
	
	/*     //for testing
	for(i=15;i>=0;i--)
	{
		printf("%d",D[i]);
	}
	printf("\n");
	
	*/
	
	unsigned int checksum=0;
	for(i = 7; i >= 0; i--)
	{
		checksum += D[i]*pow(2,i);
	}
	crc[0] = checksum;
	checksum = 0;
	for(i = 15; i >= 8; i--)
	{
		checksum += D[i]*pow(2,i-8);
	}
	crc[1] = checksum;
}

