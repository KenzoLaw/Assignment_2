/*tcpechoserver.c*/
#include"inet.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#define BUFSIZE 512
 
int main(){
    int sockfd,new_sockfd,clilen;
    char buffer[BUFSIZE+1];
    char buff_str[BUFSIZE+1];
    struct sockaddr_in serv_addr, cli_addr;
    struct sockaddr_in their_addr;
    struct stat st;
    int buff_int;
    char path[100];
    char path2[100];
    char tmp[100];
    int mid;
    key_t key;
    DIR *filename;
    struct dirent *ent;
 
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0){
        perror("Server: socket() error\n");
        exit(1);}
 
    printf("\nEcho application Demo\n"); 
 
    bzero( (char *) &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("10.0.2.15");
    serv_addr.sin_port = htons(SERV_TCP_PORT);
 
    //Creating a message queue
    key = ftok(".", 'z');
    if((mid = msgget(key, IPC_CREAT | 0660))<0){
        printf("Error Creating Message Queue\n");
        exit(-1);
    }
                      
    //Display Message Queue and Server ID
    printf("Message Queue ID: %d\n", mid);
    printf("Server ID: %ld\n", (long)getpid()); 
 
    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        perror("Server: bind() error\n");
        printf("\nWaiting for connection... [bind]\n");
     
        listen(sockfd,5);
 
    for(;;){
        clilen = sizeof(cli_addr);
        new_sockfd=accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
 
        if(fork() == 0){
            close(sockfd);
         
            if(new_sockfd > 0)
            printf("\nServer: Get connection from client %s\n",inet_ntoa(cli_addr.sin_addr));
 
            do{
                recv(new_sockfd,buffer, BUFSIZE,0);
                printf("\nReceived message [%s] from CLIENT\n", buffer);
                buff_int = atoi(buffer);
                switch(buff_int){
                case 1:
                    strcpy(buff_str,"Please insert your folder name.");
                    send(new_sockfd, buff_str, BUFSIZE,0);
                    recv(new_sockfd, buffer, BUFSIZE,0);
                    strcpy(path, ("/home/kenzo/%s",buffer));
                    if(stat(path, &st) == -1) {
                        mkdir(path, 0700);
                        strcpy(buff_str,"Folder had successfully created!\n");
                        send(new_sockfd, buff_str, BUFSIZE,0);
                        exit(0);
                    }
                    else
                        strcpy(buff_str,"Sorry, The folder already existed!\n");
                        send(new_sockfd, buff_str, BUFSIZE,0);
                        exit(0);
                 
                case 2:
                    strcpy(buff_str,"What directory do you want to delete?");
                    send(new_sockfd, buff_str, BUFSIZE,0);
                    recv(new_sockfd, buffer, BUFSIZE,0);
                    strcpy(path, ("/home/kenzo/%s",buffer));
                    if(stat(path, &st) == -1) {
                        strcpy(buff_str,"The folder doesn't exist!\n");
                        send(new_sockfd, buff_str, BUFSIZE,0);
                        exit(0);
                    } 
                    else
                        rmdir(path);
                        strcpy(buff_str,"Directory had been sucessfully deleted!\n");
                        send(new_sockfd, buff_str, BUFSIZE,0);
                        exit(0);
                 
                case 3:
                    filename = opendir ("/home/kenzo/Desktop/Server");
                    if (filename != NULL)
                    {
                        while ((ent = readdir (filename))!=NULL) {
                            strcat(buff_str,ent->d_name);
                            strcat(buff_str,"\n");
                    }
                    closedir (filename);
                    send(new_sockfd, buff_str, BUFSIZE,0);
                    }
                    int success = 0;
                    while(success == 0)
                    {
                        recv(new_sockfd, buffer, BUFSIZE,0);   //request filename from client
                        strcpy(path,"/home/kenzo/Desktop/Server/");
 
                        /*Send file to Client*/
                        char* fs_name = strcat(path,buffer);
                            printf("[Server] Sending %s to the Client...", fs_name);
                            FILE *fs = fopen(fs_name, "r");
                            if(fs == NULL)
                            {
                            fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
                                exit(1);
                            }
 
                            bzero(buffer, BUFSIZE); 
                            int fs_block_sz; 
                            while((fs_block_sz = fread(buffer, sizeof(char), BUFSIZE, fs))>0)
                            {
                            if(send(new_sockfd, buffer, fs_block_sz, 0) < 0)
                            {
                                fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
                                exit(1);
                            }
                            bzero(buffer, BUFSIZE);
                            }
                            printf("Ok sent to client!\n");
                            success = 1;
                            while(waitpid(-1, NULL, WNOHANG) > 0);
                    }
                    break;  
                case 4:
                    /*Receive File from Client */
                    printf("Server: Receiveing file from Client and saving it as receive_client_file.txt...\n");
                    char* fr_name = "/home/kenzo/Desktop/Server/receive_client_file.txt";
                    FILE *fr = fopen(fr_name, "a");
                    if(fr == NULL)
                        printf("File %s Cannot be opened file on server.\n", fr_name);
                    else
                    {
                        bzero(buffer, BUFSIZE); 
                        int fr_block_sz = 0;
                        while((fr_block_sz = recv(new_sockfd, buffer, BUFSIZE, 0)) > 0) 
                        {
                            int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
                            if(write_sz < fr_block_sz)
                            {
                            error("File write failed on server.\n");
                            }
                            bzero(buffer, BUFSIZE);
                            if (fr_block_sz == 0 || fr_block_sz != 512) 
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
                            exit(1);
                            }
                    }
                        printf("File has successfully received from client!\n");
                        fclose(fr); 
                        break;
                    }       
                printf("\nSending back messsage to CLIENT\n");
                send(new_sockfd, buffer, BUFSIZE,0);
                }
            }while(strcmp(buffer,"/q"));
             
            exit(0);
        }
        close(new_sockfd);
    }
    close(sockfd);
}
