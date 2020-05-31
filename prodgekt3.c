// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h>
#include <pthread.h>

#define PORT 8080
#define BUFSIZE 4096
void *handle_connektion(void *);

int main(int argc, char const *argv[]){
    int server_fd;
    int new_client_socket; 
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0){
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while (1){
        printf("\nWaiting for new connection...\n");
        if ((new_client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            perror("In accept");
            continue;
        }
	pthread_t thread;
	pthread_create(&thread, NULL, handle_connektion, (void*) new_client_socket);
    }

    return 0;
}

void *handle_connektion(void *ptr){
    char buffer[BUFSIZE] = { 0 };
    ssize_t bytes_read;
    int msg_size = 0;
    int client_socket = (int)ptr;

    while ((bytes_read = read(client_socket, buffer + msg_size, sizeof(buffer) - msg_size - 1)) > 0) {
        msg_size += bytes_read;
        if (msg_size > BUFSIZE - 1 || buffer[msg_size - 1] == '\n')break;
    }

    printf("%s\n", buffer);

    int fd = open("info.html", O_RDONLY);

    if (fd < 0) {
        perror("open");
        return NULL;
    }

    struct stat buf;
    stat("info.html", &buf);

    char http_header[300] = {0}; 
    sprintf(http_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", (int)buf.st_size);
    write(client_socket, http_header, strlen(http_header));
    printf("%s\n", http_header);
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0){
        printf("%s", buffer);
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    close(fd);
    return NULL;
}
