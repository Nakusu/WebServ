#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string> 
#include <iostream> 
#define PORT 8080 
int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_link, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    std::string hello = "HTTP/1.1 500\r\nContent-Type: text/html\n\n<html><h1>Hello from server</h1></html>"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
       
    // Help in reuse of address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){ 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Attach le socket sur le port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

	while (1){
		//ecoute sur le port


		if (listen(server_fd, 3) < 0){
			perror("listen"); 
			exit(EXIT_FAILURE); 
		}
		//creation connection client server
		if ((new_link = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}

		valread = read( new_link , buffer, 1024); 
		printf("%s\n",buffer ); 


		//creer requete reponse et envoyer sur les socket php ruby python


		if (send(new_link , hello.c_str() , hello.size() , 0 ) != -1) 
			printf("Hello message sent\n");
		close(new_link);
	}
    return 0; 
} 