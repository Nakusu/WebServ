#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"
#include "Parsing_request.hpp"

class Request {
	public:
		Request() {
			this->_socket = 0;
			this->_uri = "";
			this->_typecontent = "";
		}
		Request(int socket) {
			this->_socket = socket;
			this->_buffer[recv(this->_socket, &this->_buffer, sizeof(this->_buffer), 0)] = 0;
			this->_type = this->_buffer[0];
			this->_parsing.parsing_map(this->_buffer);
			this->_parsing.parsing_mime();
			this->_parsing.parse_get();
			this->find_uri();
			this->find_typecontent();
		}

		virtual ~Request() {
			close(this->_socket);
			return ; 
		}
		std::string			get_uri() const {
			return (this->_uri);
		}
		char				*get_buffer(){
			return (this->_buffer);
		}
		char				get_type() const { 
			return (this->_type);
		}
		std::string			get_typecontent() const { 
			return (this->_typecontent);
		}
		std::string			get_extension() const {
			return (this->_parsing.get_extension());
		}
		int					get_socket() const {
			return (this->_socket);
		}
		void				set_socket(int socket) {
			this->_socket = socket;
		}
		void				set_uri(std::string uri) {
			this->_uri = uri;
		}
		void				send_packet(const char *content) {
			send(this->_socket, content, strlen(content), 0);
		}
		void				send_packet(const char *content, size_t len) {
			send(this->_socket, content, len, 0);
		}

		void				find_uri(void) {
			this->_uri = "";
			std::string string(this->_buffer);
			this->_uri = strndup(&this->_buffer[string.find("/")], (string.find("HTTP") - 5));
		}

		void				find_typecontent(void) {
			this->_typecontent = "";
			this->_typecontent = this->_parsing.get_map()["Accept"];
		}

	private:
		int													_socket;
		char												_buffer[1025];
		char												_type;
		std::string											_uri;
		std::string											_typecontent;
		Parsing_request										_parsing;
};

#endif