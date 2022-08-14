#include "FingerprintSocket.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#define MAXLINE 4096
FingerprintSocket::FingerprintSocket(/* args */)
{
}

FingerprintSocket::~FingerprintSocket()
{

}

void FingerprintSocket::handleMessage(void *data, unsigned int len)
{

}

int FingerprintSocket::createSocket() {
    int  sfd, cfd;
    struct sockaddr_in  servaddr;
    char  buff[4096];
    int n;

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8421);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }

    if(listen(sfd, 10) == -1) {
        return 0;
    }

    while(1) {
        if((cfd = accept(sfd, (struct sockaddr*)NULL, NULL)) == -1) {
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }
        n = recv(cfd, buff, MAXLINE, 0);
        buff[n] = '\0';
        printf("recv msg from client: %s\n", buff);
        handleMessage(buff, sizeof(buff));
        write(cfd, buff, sizeof(buff));
        close(cfd);
    }
    close(sfd);
    return 0;
}

void FingerprintSocket::sendCmdToSocket() {
}