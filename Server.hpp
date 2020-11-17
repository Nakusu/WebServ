
#ifndef SERVER_H
#define SERVER_H

#include "./Header.hpp"

class Server
{
	public:
		Server(void);
		Server(Server const &);
		virtual ~Server(void);
		Server &					operator=( Server const &);

		int							init_fd(int, int, int);
		struct sockaddr_in &		init_addr(int, in_addr_t, in_port_t);
		void						init_link(void);
		void						init_listen(int);
		int							get_fd(void);
		struct sockaddr_in 			get_address(void);
		

	private:
		int 						_fd;
		struct sockaddr_in 			_address;  
};

#endif
