#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>

class request {
	public:
		request() {
			this->_socket = 0;
			this->_uri = NULL;
		}
		request(int socket) {
			this->_socket = socket;
			this->_buffer[recv(this->_socket, &this->_buffer, sizeof(this->_buffer), 0)] = 0;
			this->_type = this->_buffer[0];
			std::string string(this->_buffer);
			this->_uri = strndup(&this->_buffer[string.find("/")], (string.find("H") - 5));
		}
		virtual ~request() {
			close(this->_socket);
			return ; 
		}
		char	*getUri() const {
			return (this->_uri);
		}
		char	getType() const { 
			return (this->_type);
		}
		int		getSocket() const {
			return (this->_socket);
		}
		void	setSocket(int socket) {
			this->_socket = socket;
		}
		void	sendPacket(char *content) {
			send(this->_socket, content, strlen(content), 0);
		}

	private:
		int		_socket;
		char	_buffer[1025];
		char	*_uri;
		char	_type;
};

#endif