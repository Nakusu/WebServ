/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cbertola <cbertola@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 10:09:23 by cbertola          #+#    #+#             */
/*   Updated: 2020/11/17 17:09:55 by cbertola         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() {
	init_fd(AF_INET , SOCK_STREAM , 0);
	init_addr(AF_INET, INADDR_ANY, htons(PORT));
	init_link();
	init_listen(3);
}

Server::Server(Server const & src) {
	Server::operator=(src);
}

Server::~Server() {}

Server &										Server::operator=( Server const & rhs) {
	if (this != &rhs)
	{
		this->_fd = rhs._fd;
		this->_address = rhs._address;
	}
	return (*this);
}

int												Server::init_fd(int domain, int type, int protocol){
    if( (this->_fd = socket(domain , type , protocol)) == 0){   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }
	return (this->_fd);
}

void											Server::init_link(void){
	if (this->_fd != 0){
		if(bind(this->_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0){   
       		perror("bind failed");   
        	exit(EXIT_FAILURE);   
		}
    }
}
struct sockaddr_in &							Server::init_addr(int family, in_addr_t s_addr, in_port_t port){
	this->_address.sin_family = family;   
	this->_address.sin_addr.s_addr = s_addr;   
   	this->_address.sin_port = port;
	return (this->_address); 
}

void											Server::init_listen(int number){
	if (listen(this->_fd, number) < 0){
        perror("listen");   
        exit(EXIT_FAILURE);   
    }       
}

int												Server::get_fd(void){
	return (this->_fd);
}

struct sockaddr_in 								Server::get_address(void){
	return (this->_address);
}