#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<sys/types.h>
#include<sys/select.h>
#include<readline/readline.h>
#include<readline/history.h>


#define M_GREEN "\033[0;32;1m"
#define M_WHITE "\033[0;37;1m"
#define M_BLUE "\033[0;34;1m"
#define M_RED "\033[0;31;1m"
#define M_WHITE_NB "\033[0;37m"




int main(){

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFd == -1){
        printf("Error creating the client socket.\n");
        exit(0);
    }

    char ipAddr[64];
    int port;
    system("clear");
    printf("**********RSH CLIENT INITIALIZED**********\n");
    printf("Please enter the IP address of the server: ");
    scanf("%s", ipAddr);
    printf("\nPlease enter the port of the server: ");
    scanf("%d", &port); 

    struct sockaddr_in ServerDetails;
    ServerDetails.sin_family = AF_INET;
    ServerDetails.sin_addr.s_addr = inet_addr(ipAddr);
    ServerDetails.sin_port = htons(port);

    printf("\nEstablishing connection to RSH server at %s:%d\n",ipAddr, port);
    int connection = connect(socketFd, (struct sockaddr*) &ServerDetails, sizeof(ServerDetails));

    if(connection == -1){
        printf("Error connecting to the server\n");
        exit(0);
    }
    char response[5000];
    recv(socketFd, &response, 5000, 0);
    printf("%s", response);

    usleep(50000);
    bzero(response, 5000);
    recv(socketFd, &response, 5000, 0);
    printf("%s", response);


    while(1){
        char *toSend = NULL;
        toSend = readline("╰✗ " M_WHITE_NB);
        add_history(toSend);
        send(socketFd, toSend, 256, 0);

        if(strcmp(toSend, "exit") == 0){
            exit(0);
        }

        int ret_val=1;
        struct timeval tv;
        tv.tv_sec=0;
        tv.tv_usec=20000;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(socketFd, &fds);
        while(ret_val != 0 && ret_val != -1){
            ret_val = select(1, &fds, NULL, NULL, &tv);
            bzero(response, 5000);
            recv(socketFd, &response, 5000, 0);
            printf("%s",response);
        }



        free(toSend);

    }
    close(socketFd);
}
