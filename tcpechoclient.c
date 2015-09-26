/*tcpechoclient.c*/
#include"inet.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#define BUFSIZE 256
 
int main(int argc, char *argv[]){
    int sockfd; //create socket
    char buffer[BUFSIZE+1];
    struct sockaddr_in serv_addr; //client will connect on this
    int mid;
    key_t key;
    char buff_str[BUFSIZE+1];
    int buff_int;
    char path[100];
    DIR *filename;
    struct dirent *ent;
    sigset_t set1, set2;
    sigfillset (&set1); 
    sigfillset (&set2);
    sigdelset (&set2, SIGINT); 
    sigdelset (&set2, SIGQUIT);
    sigprocmask(SIG_SETMASK, &set1, (void *)0);
 
    if(argc <= 1){
        printf("How to use: %s remoteIPaddress [example: ./client 127.0.0.1]\n", argv[0]);
        exit(1);}
    bzero( (char *) &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_TCP_PORT);
    inet_pton (AF_INET, argv[1], &serv_addr.sin_addr);
     
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0){
        perror("Client: socket() error\n");
        exit(1);}
 
    if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Client: connect() error\n");
        exit(1);}
 
    //Aquiring Message Queue ID
    key = ftok(".", 'z');
    mid = msgget(key, 0);
      
    //Display Message Queue and Client ID
    printf("Message Queue ID: %d\n", mid);
    printf("Client ID: %ld\n", (long)getpid());
 
    printf("\nServer: Welcome...What do you want to do? \n[type enter to continue][type /q to quit]\n");
    printf("1.Create directory\n"
           "2.Delete directory\n"
           "3.Download file from server\n"
           "4.Send file to server\n"
           "Client: ");
    do{
        gets(buffer);
        send(sockfd, buffer, BUFSIZE, 0);
        buff_int = atoi(buffer);
        switch(buff_int){
        case 1:
            bzero(buffer, sizeof(buffer));
            recv(sockfd, buffer, BUFSIZE, 0);
            printf("Server: %s\n",buffer);
            bzero(buffer, sizeof(buffer));
            gets(buffer);
            send(sockfd, buffer, BUFSIZE, 0);
            recv(sockfd, buffer, BUFSIZE, 0);
            break;
        case 2:
            bzero(buffer, sizeof(buffer));
            recv(sockfd, buffer, BUFSIZE, 0);
            printf("Server: %s\n",buffer);
            break;
        case 3:
            bzero(buffer, sizeof(buffer));
            recv(sockfd, buffer, BUFSIZE, 0);
            printf("Server: %s\n",buffer);
            printf("Choose the file that you want to download.\n");
            gets(buffer);
            send(sockfd, buffer, BUFSIZE, 0);
            bzero(buffer, sizeof(buffer));
            recv(sockfd, buffer, BUFSIZE, 0);
 
            /*Receive File from Server*/
            printf("Client: Receiveing file from Server and saving it as receive_server_file.txt...\n");
            char* fr_name = "/home/kenzo/Desktop/Client/receive_server_file.txt";
            FILE *fr = fopen(fr_name, "a");
            if(fr == NULL)
                printf("File %s Cannot be opened.\n", fr_name);
            else
            {
                bzero(buffer, BUFSIZE); 
                int fr_block_sz = 0;
                while((fr_block_sz = recv(sockfd, buffer, BUFSIZE, 0)) > 0)
                {
                    int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
                if(write_sz < fr_block_sz)
                    {
                    error("File write failed.\n");
                }
                    bzero(buffer, BUFSIZE);
                    if (fr_block_sz == 0 || fr_block_sz != 256 ) 
                    {
                        break;
                    }
                }
                if(fr_block_sz < 0)
                {
                    if (errno == EAGAIN)
                    {
                        printf("recv() timed out.\n");
                    }
                    else
                    {
                        fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                    }
                }
                printf("File had successfully received from server!\n");
                fclose(fr);
            }
            break;
        case 4:
            printf("Which file do you want to send to server?");
            filename = opendir ("/home/kenzo/Desktop/Client");
            if (filename != NULL)
            {
                while ((ent = readdir (filename))!=NULL) {
                    strcat(buffer,ent->d_name);
                    strcat(buffer,"\n");
                }
            closedir (filename);
            puts(buffer);
            }
 
            gets(buffer);
            strcpy(path,"/home/kenzo/Desktop/Client/");
            char* fs_name = strcat(path,buffer);
            printf("Client: Sending %s to the Server...\n ", fs_name);
            FILE *fs = fopen(fs_name, "r");
            if(fs == NULL)
            {
                printf("ERROR: File %s not found.\n", fs_name);
                exit(1);
            }
 
            bzero(buffer, BUFSIZE); 
            int fs_block_sz; 
            while((fs_block_sz = fread(buffer, sizeof(char), BUFSIZE, fs)) > 0)
            {
                if(send(sockfd, buffer, fs_block_sz, 0) < 0)
                {
                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                }
                bzero(buffer, BUFSIZE);
            }
            printf("File %s from Client has successfully Sent!\n", fs_name);
        }
    }while(strcmp(buffer, "/q"));
    sigprocmask (SIG_UNBLOCK, &set1, (void *)0);
    close(sockfd);
}
