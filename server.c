/***************************************************************************
	@authors       Kacper Knuth
	@file          server.c
*******************************************************************************/

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

#define SIZE 1024
#define BUF_TO_ARG_SIZE 128
#define BUFSIZE 518

pthread_mutex_t mutex;
int clients[20];
int n=0;

const char *commands_f[] = {
	"/send",
	"/download",
};
const int count_f = sizeof(commands_f) / sizeof(commands_f[0]);

char **tokenize(char *line, char *delim);
void *recvmg(void *client_sock);
int conection();
void sendtoall(char *msg, int curr);
char *get_user_name();
int write_f(char *arguments[], int sock);
int send_f(char* arguments[], int sock);

int main(int argc, char** argv)
{
	conection();
	
	return 0;
}

char **tokenize(char *line, char *delim){
	
	int buffor_max = BUF_TO_ARG_SIZE, count_arg = 0;
	char *argument;
	char **arguments = malloc(buffor_max * sizeof(char*));
	argument = strtok(line, delim);
	while (argument != NULL)
	{
		arguments[count_arg] = argument;
		count_arg++;
		argument = strtok(NULL, delim);
		if (count_arg >= buffor_max)
		{
			buffor_max += BUF_TO_ARG_SIZE;
			arguments = realloc(arguments, buffor_max * sizeof(char*));
			if (!arguments)
			{
				printf("Allocation error.");
			}
		}
	}
	arguments[count_arg] = NULL;
	return arguments;
}

void *recvmg(void *client_sock){
	int (*server_f[])(char**, int) = {write_f, send_f};
	int sock = *((int *)client_sock);
	char msg[500];
    char exit_msg[200];
	int len;
	char *send_msg = malloc(sizeof(char)*500);
	char *username;
	while((len = recv(sock,msg,500,0)) > 0) {
        int i = 0, execute = 1;
		msg[len] = '\0';
        strcpy(send_msg,msg);
	    char **arguments = tokenize(msg, " \n");
        username = arguments[0];
	    while (i != count_f)
	    {
		    
		    if (strcmp(arguments[1], commands_f[i]) == 0)
		    {
			    (*server_f[i])(arguments, sock);
			    execute = 0;
		    }
		    i++;
	    }
	    if(execute)
	    {
		    sendtoall(send_msg,sock);
	    }
        printf("%s\n", send_msg);
        send_msg[0] = '\0';
        free(arguments);
	}	
    strcpy(exit_msg,username);
    strcat(exit_msg," has disconnected.\n");
    sendtoall(exit_msg, sock);
}


int conection()
{
	struct sockaddr_in ServerIp;
	pthread_t recvt;
	int sock=0 , Client_sock=0;; 
	
    sock = socket( AF_INET , SOCK_STREAM, 0 );
    if (sock == -1)
	{
		printf("Could not create socket");
	}
    puts("Socket created");

    ServerIp.sin_family = AF_INET;
	ServerIp.sin_port = htons(8888);
	ServerIp.sin_addr.s_addr = INADDR_ANY;
	
	if( bind(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp)) < 0)
    {
		perror("Bind failed. Error");
        return 1;
    }
	else
		puts("Bind done");
		
	if( listen( sock ,20 ) == -1 )
    {
		perror("Listening failed. Error");
        return 1;
    }
    puts("Waiting for incoming connections...");
		
	while(1){
		if( (Client_sock = accept(sock, (struct sockaddr *)NULL,NULL)) < 0 )
			puts("Accept failed");
        puts("Connection accepted");
		pthread_mutex_lock(&mutex);
		clients[n]= Client_sock;
		n++;
		// creating a thread for each client 
		pthread_create(&recvt,NULL,(void *)recvmg,&Client_sock);
		pthread_mutex_unlock(&mutex);
	}
	return 0; 
}

void sendtoall(char *msg, int curr){
	int i;
	pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) 
    {
			if(send(clients[i], msg, strlen(msg), 0) < 0) {
				printf("Sending failure.");
				continue;
		}
	}
	pthread_mutex_unlock(&mutex);
}

void sendtoone(char *msg, int curr){
	pthread_mutex_lock(&mutex);
	if(send(curr, msg, strlen(msg), 0) < 0) 
		printf("Sending failure.");
	pthread_mutex_unlock(&mutex);
}

int write_f(char *arguments[], int sock){
	char filename[200];
    ssize_t len;
    char buffer[BUFSIZ];
    int file_size;
    FILE *received_file;
    int remain_data = 0;
	char **path = tokenize(arguments[2], "/");
	char *send_msg = malloc(sizeof(char)*500);
	int size_path = 0;
	while(path[size_path] != NULL) 
	{
		size_path++;	
	}
	strcpy(filename, "recv_");
	strcat(filename,path[size_path - 1]);
	strcpy(send_msg,"User ");
	strcat(send_msg,arguments[0]);
	strcat(send_msg," has send ");
	strcat(send_msg,path[size_path - 1]);
    strcat(send_msg,"\n");
    /* Receiving file size */
    file_size = atoi(arguments[3]);
    fprintf(stdout, "\nFile size : %d\n", file_size);

    received_file = fopen(filename, "w");
    if (received_file == NULL)
    {
            fprintf(stderr, "Failed to open file --> %s\n", strerror(errno));
            sendtoone("Failed to create file on the server side.\n", sock);
            return 1;
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(sock, buffer, BUFSIZ, 0)) > 0))
    {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
    }
    fclose(received_file);

	sendtoall(send_msg,sock);
	free(path);
	return 0;
}

int send_f(char* arguments[], int sock){
	int leng;
	char path[512];
    ssize_t len;
    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;
    
	stpcpy(path, arguments[2]);
    fd = open(path, O_RDONLY);
    
    if (fd == -1)
    {
        fprintf(stderr, "Error opening file --> %s\n", strerror(errno));
        sendtoone(strerror(errno), sock);
        return 1;
    }
	else 
	{
		
        /* Get file stats */
        if (fstat(fd, &file_stat) < 0)
        {
                fprintf(stderr, "Error fstat --> %s\n", strerror(errno));
				sendtoone(strerror(errno), sock);
                return 1;
        }

        fprintf(stdout, "File Size: \n%ld bytes\n", file_stat.st_size);
        
        sprintf(file_size, "%ld", file_stat.st_size);
		sendtoone(file_size, sock);

        offset = 0;
        remain_data = file_stat.st_size;
        /* Sending file data */
        while (((sent_bytes = sendfile(sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
        {
                remain_data -= sent_bytes;
        }
        return 0;
    }
}