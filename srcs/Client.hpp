#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"

class Client{
	public:
		Client(void){}
		Client(int fd, std::map<std::string, std::string> _mimesTypes){
			this->_fd = fd;
			this->_mimesTypes = _mimesTypes;
			this->_req = new Request(this->_fd, this->_mimesTypes);
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
			delete this->_req;
			close(this->_fd);
		}
		int										get_fd(void){
			return (this->_fd);
		}
		int										CGIIsRunning(void){
			return (this->_req->get_CGI());
		}
		Request *								get_req(void){
			return (this->_req);
		}
		void									new_req(void){
			delete this->_req;
			this->_req = new Request(this->_fd, this->_mimesTypes);
		}
		std::map<std::string, std::string>									get_history(void){
			return (this->_history);
		}
		std::string															get_history(std::string user){
			return (this->_history[user]);
		}
		void																setHistory(std::string user, std::string url){
			this->_history[user] = url;
		}
		void									basicHistory(Client *client){
			if (client->get_history((this->_req->get_IpClient() + this->_req->get_userAgent())) != "")
				this->_req->updateContent("Referer", client->get_history((this->_req->get_IpClient() + this->_req->get_userAgent())));
		}
private :
	int										_fd;
	Request *								_req;
	std::map<std::string, std::string>		_mimesTypes;
	std::map<std::string, std::string> 		_history;
};

#endif