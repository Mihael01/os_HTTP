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

char path[MAX_PATH + 1];

void *handle_connektion(void *);

int main(int argc, char const *argv[]){

    if( argc == 2 ) {
       printf("The argument supplied is %s\n", argv[1]);
    }
    else if( argc > 2 ) {
       printf("Too many arguments supplied.\n");
       return 1;
    }
    else {
       printf("One argument expected.\n");
       return 1;
    }

    if(realpath(argv[1], path) == NULL) {
    	printf("bad path %s\n", argv[1]);
        return 1;
    }

    int server_fd;
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("In socket");
        return 1;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("In bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0){
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    int address_len = sizeof(address);
    int new_client_socket; 
    while (1){
        printf("\nWaiting for new connection...\n");
        if ((new_client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&address_len)) < 0){
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

 
    DIR *dir = opendir(path); 
  
    if (dir == NULL) 
    { 
        printf("Could not open current directory" ); 
        return NULL; 
    } 
  
    struct dirent *dir_ent;
    while ((dir_ent = readdir(dir)) != NULL) {
        printf("%s\n", dir_ent->d_name);
        if (dir_ent->d_type == DT_DIR)
            continue;

        char content_type[300] = { 0 };

        if (strstr(".html", dir_ent->d_name)){
            content_type = "text/html";
        }
        else if (strstr(".js", dir_ent->d_name)){
            content_type = "text/javascript";
        }
        else if (strstr(".css", dir_ent->d_name)){
            content_type = "text/css";
        }
        else if (strstr(".jpeg", dir_ent->d_name)){
            content_type = "image/jpeg";
        }
        else if (strstr(".jpg", dir_ent->d_name)){
            content_type = "image/jpeg";
        }
        else
        {
            continue;
        }

        int fd = open(dir_ent->d_name, O_RDONLY);

        if (fd < 0) {
            perror("open");
            return NULL;
        }
        struct stat buf = { 0 };
        stat(dir_ent->d_name, &buf);

        char http_header[300] = { 0 };

        sprintf(http_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", content_type, (int)buf.st_size);
        write(client_socket, http_header, strlen(http_header));
        printf("%s\n", http_header);
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            printf("%s", buffer);
            write(client_socket, buffer, bytes_read);
        }

        close(fd);
    }
  
    closedir(dir);     
          
    close(client_socket);

    return NULL;
}
