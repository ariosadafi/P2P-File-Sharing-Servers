/*
 * main.c
 *
 *  Created on: Jun 26, 2013
 *      Author: ario
 */



#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"



#define LISTENQ 15
#define MAXBUFL 255
#define MAXSERL 50



char* SERVERCONF;

char *prog_name;




int search_local(int connfd,char* filename)
{
	FILE* fp = fopen(filename, "r");

	if (fp!=NULL)
	{
		write(connfd,"+OK\n", strlen("+OK\n"));

		char str[MAXBUFL+2];
		size_t len;
		uint32_t flen = 0;
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		flen = htonl(flen);
		write(connfd,&flen,sizeof(uint32_t));

		while((len = fread(str, 1, MAXBUFL, fp)) > 0)
		{
			
			write(connfd,str,len);
		}

		fclose(fp);


		return 1;
	}
	else
	{

		return 0;
	}


return 0;


}


int search_remote(int connfd,char* filename)
{

	FILE* config = fopen(SERVERCONF,"r");
	char server[MAXSERL+2];

	if (config == NULL)
		return 0;

	int i = 0, serverfd;
	while (fgets (server , MAXSERL , config))
	{
		struct sockaddr_in servaddr;
		long AddrSocket;
		char IP[40], Port[10];



		while (server[i++]!='\n');
		server[i-1]='\0';
		sscanf(server, "%s %s", IP, Port);

		AddrSocket=inet_addr(IP);
		serverfd = Socket(AF_INET, SOCK_STREAM, 0);
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(atoi(Port));
		servaddr.sin_addr.s_addr = AddrSocket;//htonl (INADDR_ANY);

		if (connect(serverfd, (SA*) &servaddr, sizeof(servaddr)) != 0)
			continue;

		char com[MAXBUFL+2];
		ssize_t nread = 0;

		strcpy(com,"GET");
		strcat(com,filename);
		strcat(com,"\r\n");
		int comlen = strlen(com)*sizeof(char);
		write(serverfd,com,comlen);
		nread = read(serverfd, com, strlen("+OK\n"));

		if ( nread == 0 )
		{
			printf("no response");
			close(serverfd);
				continue;
		}
		if (com[0]=='-' && com[1]=='E' && com[2]=='R')
		{
			close(serverfd);
			continue;
		}

		if (com[0]=='+' && com[1]=='O' && com[2]=='K')
		{
			write(connfd,com,nread);


			uint32_t len = 0;
			nread = read(serverfd,&len,sizeof(uint32_t));
			write(connfd,&len,sizeof(uint32_t));
			if (nread !=0)
			{
				len = ntohl(len);
				int totalread = 0;
				nread = read(serverfd,com,MAXBUFL);
				totalread += nread;
				write(connfd,com,nread);
				while (totalread < len)
				{
					nread = read(serverfd,com,MAXBUFL);
					totalread += nread;
					write(connfd,com,nread);
				}

			}

			write(serverfd,"QUIT\n", strlen("QUIT\n")*sizeof(char));
			close(serverfd);
			fclose(config);
			return 1;
		}

		close(serverfd);



	}

	fclose(config);
	return 0;
}


int start(int connfd)
{
	char buf[MAXBUFL+2]; /* +1 to make room for \r \n */
	int nread;

	while (1) {

		if ((nread = read(connfd, buf, MAXBUFL)) == 0 )
			return 0;

		int i =0;
		while(buf[i++] != '\n');

		if (i<5)
			return 1;

		buf[i-2] = '\0';

		if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T')
		{
			char* file_name = &buf[3];
			if (!search_local(connfd,file_name))
			{
				if(!search_remote(connfd,file_name))
				{
					write(connfd,"-ERR\n", strlen("-ERR\n"));
					close(connfd);
					break;
				}

			}


		}
		else if (buf[0]=='Q' && buf[1]=='U' && buf[2] == 'I' && buf[3] == 'T')
		{
			close(connfd);
			break;
		}
		else
		{
			close(connfd);
			break;
		}
	}

	return 0;
}



int main(int argc, char *argv[])
{

	int listenfd, connfd, err=0;
	short port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	if(argc < 3)
	{
		printf("Usage: ./prog_name [portnumber] [configuration file]");
		return 0;
	}
	port=atoi(argv[1]);
	SERVERCONF = argv[2];

	/* create socket */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);


	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);

	Bind(listenfd, (SA*) &servaddr, sizeof(servaddr));


	Listen(listenfd, LISTENQ);

	int pid;
	signal(SIGCHLD,SIG_IGN);
	while (1)
	{
		connfd = Accept (listenfd, (SA*) &cliaddr, &cliaddrlen);
	   if ((pid = fork()) == -1)
		{
			close(connfd);
			continue;
		}
		else if(pid > 0)
		{
			close(connfd);
			continue;
		}
		else if(pid == 0)
		{
			err = start(connfd);
			close (connfd);
		}
	}



	return 0;
}

