
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <unistd.h>
#include <iostream>
#include <string>
#define PORT 8080 
   
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    std::string hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        std::cout << "Socket creation error" << std::endl; 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){ 
        std::cout << "Invalid address/ Address not supported" << std::endl; 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
        std::cout << "Connection Failed" << std::endl; 
        return -1; 
    } 
    std::cout << "Hello message sent" << std::endl; 
    send(sock , hello.c_str() , hello.size() , 0 ); 
    while ((valread = read( sock , buffer, 1024))){
    	printf("%s\n",buffer ); 
	}

    send(sock , hello.c_str() , hello.size() , 0 ); 
    return 0; 
} 