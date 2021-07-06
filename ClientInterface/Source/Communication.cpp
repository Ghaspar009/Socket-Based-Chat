#include "Communication.h"
#include"myWidget.h"
#include"ui_myWidget.h"
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include<qdebug.h>
#include<algorithm>

#define SIZE 1024
#define BUF_TO_ARG_SIZE 128
#define BUFSIZE 518



char * toChar(QString in)
{
    QByteArray a; a.append(in);
    return a.data();
}
namespace Core {
Communication::Communication(myWidget *wid):widget(wid)
{

}

void* Communication::recvmg(){
    char msg[500];
    int len;
    // client thread always ready to receive message
    while((len = recv(my_socket,msg,500,0)) > 0) {
        msg[len-1] = '\0';
        puts(msg);
        widget->ui->Chat->append(QString(msg));
    }
}

int Communication::conection(char *user_name){
    int sock, len;
    char greetings[100];
    struct sockaddr_in ServerIp;
    sock = socket( AF_INET, SOCK_STREAM,0);
    strcpy(greetings, user_name);
    strcat(greetings, " has join to the chat.\n");

    if (sock == -1)
    {
        widget->ui->Chat->append(QString("Could not create socket."));
        printf("Could not create socket.");
    }
    widget->ui->Chat->append(QString("Socket created."));
    puts("Socket created.");

    ServerIp.sin_port = htons(8888);
    ServerIp.sin_family= AF_INET;
    ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
    if( (connect( sock ,(struct sockaddr *)&ServerIp,sizeof(ServerIp))) == -1 )
    {
        widget->ui->Chat->append(QString("Connection to socket filed."));
        puts("Connection to socket failed.");
    }
    widget->ui->Chat->append(QString("Connected to socket."));
    puts("Connected to socket.");
    pthread_create(&recvt,NULL,[](void* ptr) -> void* {static_cast<Communication*>(ptr)->recvmg(); return (void*)nullptr;}, this);
    len = write(sock,greetings,strlen(greetings));
            if(len < 0){
                widget->ui->Chat->append(QString("Greetings not sent."));
                puts("Greetings not sent.");
            }
    my_socket = sock;
    return sock;
}

int Communication::send_msg(char *msg, int sock){
    int len;
    len = write(sock,msg,strlen(msg));
            if(len < 0){
                widget->ui->Chat->append(QString("Message not sent."));
                printf("Message not sent.");
            }
    return len;
}

int Communication::send_f(char *arguments, int sock, char* user_name){
    int leng;
    char send_msg[500];
    char path[512];
    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;

    strcpy(send_msg,user_name);
    strcat(send_msg," ");
    strcat(send_msg, "/send");
    strcat(send_msg," ");
    strcat(send_msg, arguments);
    stpcpy(path, arguments);
    fd = open(path, O_RDONLY);

    if (fd == -1)
    {
        widget->ui->Chat->append(QString("Error opening file."));
        fprintf(stderr, "Error opening file --> %s\n", strerror(errno));
        return 1;
    }
    else
    {

        /* Get file stats */
        if (fstat(fd, &file_stat) < 0)
        {
                fprintf(stderr, "Error fstat --> %s\n", strerror(errno));

                return 1;
        }

        //fprintf(stdout, "File Size: \n%ld bytes\n", file_stat.st_size);

        sprintf(file_size, "%ld", file_stat.st_size);
        strcat(send_msg, " ");
        strcat(send_msg, file_size);
        leng = write(sock,send_msg,strlen(send_msg));
        if(leng < 0)
        {
            widget->ui->Chat->append(QString("Command not sent."));
            printf("Command not sent.");
            return 1;
        }

        offset = 0;
        remain_data = file_stat.st_size;
        /* Sending file data */
        while (((sent_bytes = sendfile(sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
        {
                remain_data -= sent_bytes;
        }
        widget->ui->Chat->append(QString("File send successfully."));
        return 0;
    }
}

int Communication::download_f(char *arguments, int sock, char* user_name){

    char tmp_filename[200];
    ssize_t len;
    char* buffer =  new char[BUFSIZ];
    int file_size;
    FILE *received_file;
    int remain_data = 0, leng;   
    char *send_msg = (char*)malloc(sizeof(char)*500);
 /*   char path[strlen(arguments)];
    strcpy(path,arguments);
    int j = 0, i;
    int f_len = strlen(path);
    for(i = f_len - 1; i >= 0; i--){
        if(path[i] == '/')
            break;
        tmp_filename[j] = path[i];
        j++;
    }
    tmp_filename[0] == '\0';
    j = 0;
    i = strlen(tmp_filename) - 1;
    char filename[i], c_filename[i + 11];
    while(tmp_filename[i] != '\0'){
        filename[j] = tmp_filename[i];
        j++;
        i--;
    }

    strcpy(c_filename, "downloaded_");
    strcat(c_filename, "filename"); */
    strcpy(send_msg,user_name);
    strcat(send_msg," ");
    strcat(send_msg, "/download");
    strcat(send_msg," ");
    strcat(send_msg, arguments);

    //int cancel = pthread_cancel(recvt);
    leng = write(sock,send_msg,strlen(send_msg));
    if(leng < 0)
    {
            printf("Command not sent.");
            widget->ui->Chat->append(QString("Command not sent."));
            return 1;
    }


    recv(sock, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);
    bzero(buffer, BUFSIZ);
    fprintf(stdout, "\nFile size : %d\n", file_size);
    received_file = fopen("c_filename", "wb");
    if (received_file == NULL)
    {
            fprintf(stderr, "Failed to open file --> %s\n", strerror(errno));
            widget->ui->Chat->append(QString("Failed to create a file."));
            return 1;
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(sock, buffer, BUFSIZ, 0)) > 0))
    {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
            bzero(buffer, BUFSIZ);
    }
    fclose(received_file);
    //pthread_create(&recvt,NULL, [](void* ptr){static_cast<Communication*>(ptr)->recvmg(); return (void*)nullptr;}, this);
    widget->ui->Chat->append(QString("File downloaded successfully."));
    return 0;
}
} //end namespace
