#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"

class Client{
	public:
		Client(void){}
		Client(int fd){
			this->_fd = fd;
			this->_req = new Request(this->_fd);
		}
		Client(Client const &cpy){
			operator=(cpy);
		}
		Client &								operator=(Client const &cpy){
			if (this != &cpy){
			}
			return (*this);
		}
		virtual ~Client(void){
			std::cout << YELLOW << "Debut Suppression du client" << RESET << std::endl;
			delete this->_req;
			close(this->_fd);
			std::cout << YELLOW << "Fin Suppression du client" << RESET << std::endl;
		}

		int										get_fd(void){
			return (this->_fd);
		}
		Request *								get_req(void){
			return (this->_req);
		}
		void									new_req(void){
			delete this->_req;
			this->_req = new Request(this->_fd);
		}


private :
	int					_fd;
	Request *			_req;
};

#endif