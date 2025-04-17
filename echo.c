#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8190
#define BUFFER_SIZE 1024

//handle communication
void* handleConnection(int* sock_fd_ptr){
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Handling connection on %d\n", sock_fd);
    while(1){
        //handle a message
        bytes_read = read(sock_fd, buffer, sizeof(buffer)-1);
        if(bytes_read <= 0){
            if(bytes_read == 0){
                printf("Connection is now closed\n");
            } else{
                printf("read error");
            }
            break;
        }

        buffer[bytes_read] = '\0';
        printf("Message recieved: %s\n", buffer);

        //message is echoed back to the client in WSL
        if(write(sock_fd, buffer, bytes_read) < 0){
            printf("echo write error");
            break;
        }
    }

    printf("done with connection %d\n", sock_fd);
    close(sock_fd);
    return NULL;

}

int main(int argc, char* argv[])
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("socket failed");
        return 1;
    }

    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(PORT);

    printf("Binding to port %d\n", PORT);

    int returnval;
    
    returnval= bind(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address));

    if(returnval< 0){
        perror("bind");
        return 1;
    }

    if(listen(socket_fd, 10) < 0){
        perror("listen");
        return 1;
    }
    //accpets new connections in a loop
    while(1){
        pthread_t thread;
        int* client_fd_buf = malloc(sizeof(int));
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        *client_fd_buf = accept(
            socket_fd, (struct sockaddr*)&client_address, &client_address_len);

        if(*client_fd_buf < 0){
            perror("accept");
            free(client_fd_buf);
            continue;
        }

        printf("Accepted connection on %d\n", *client_fd_buf);

        pthread_create(&thread, NULL, (void* (*) (void*))handleConnection,(void*)client_fd_buf);
    }

    return 0;
}