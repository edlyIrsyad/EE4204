/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd, struct sockaddr *addr, uint addrlen, int batch_size, int du_size);                                                        // transmitting and receiving function

int main(int argc, char **argv)
{
	int sockfd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in cli_addr;
	int batch_size, du_size;

	batch_size = BATCHSIZE;
	if (argc >= 2) {
		batch_size = atoi(argv[1]);
	}

	du_size = DATALEN;
	if (argc >= 3) {
		du_size = atoi(argv[2]);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}

	do {
		str_ser(sockfd, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in), batch_size, du_size);                                          //receive packet and response
	} while (SERVER_LOOP);

	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, uint addrlen, int batch_size, int du_size)
{
	char buf[BUFSIZE];
	FILE *fp;
	// char recvs[DATALEN];
	struct pack_so pack;
	struct ack_so ack;
	int n = 0;
	long lseek=0, du=0;
	int num_packs = 0;
	int recv_packs = 0;

	printf("receiving data!\n");

	while(num_packs == 0 || recv_packs < num_packs)
	{
		if ((n= recvfrom(sockfd, &pack, sizeof(struct pack_so), 0, addr, &addrlen))==-1)                                   //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
		if (pack.data[pack.len-1] == '\0')									//if it is the end of the file
		{
			num_packs = pack.num + 1;
			// lseek = num_packs * DATALEN + pack.len;
			pack.len--;
		}
		memcpy((buf+(pack.num * du_size)), pack.data, pack.len);
		recv_packs++;
		lseek += pack.len;

		if (++du != batch_size) {
			continue;
		}

		ack.num = 1;
		ack.len = 0;
		if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1)
		{
				printf("send error!");								//send the ack
				exit(1);
		}
		if (DEBUG) printf("ACK sent\n");
		du = 0;
	}
	ack.num = 1;
	ack.len = 0;
	if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1)
	{
			printf("send error!");								//send the ack
			exit(1);
	}
	if (DEBUG) printf("Final ACK sent\n");
	if ((fp = fopen ("receive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
