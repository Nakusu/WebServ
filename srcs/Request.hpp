#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"

class Request {
	public:
		Request() {
			this->_socket = 0;
			this->_uri = NULL;
			this->_typecontent = "";
		}
		Request(int socket) {
			this->_socket = socket;
			this->_buffer[recv(this->_socket, &this->_buffer, sizeof(this->_buffer), 0)] = 0;
			this->_type = this->_buffer[0];
			parsing_map();
			parsing_mime();
			this->find_uri();
			this->find_typecontent();
		}

		virtual ~Request() {
			close(this->_socket);
			return ; 
		}
		char				*get_uri() const {
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
		int					get_socket() const {
			return (this->_socket);
		}
		void				set_socket(int socket) {
			this->_socket = socket;
		}
		void				send_packet(const char *content) {
			send(this->_socket, content, strlen(content), 0);
		}
		void				send_packet(const char *content, size_t len) {
			send(this->_socket, content, len, 0);
		}

		void				find_uri(void) {
			this->_uri = NULL;
			std::string string(this->_buffer);
			this->_uri = strndup(&this->_buffer[string.find("/")], (string.find("HTTP") - 5));
		}

		void				find_typecontent(void) {
			this->_typecontent = "";
			this->_typecontent = this->_map["Accept"];
			std::cout << RED << this->_typecontent << RESET << std::endl;
			// std::string findExtension = this->_uri;
			// if (static_cast<std::string>(this->_uri).find(".") != SIZE_MAX)
			// 	this->_typecontent = &this->_uri[findExtension.find(".") + 1];
			
		}
		void				parsing_map(){
			std::string 			pars = _buffer;
			std::istringstream 		buff(pars);

			getline(buff, pars);
			std::string key = "First";
			std::string value = pars;
			this->_map[key] = value;

			while (getline(buff, pars)){
				if (pars.find(":") != SIZE_MAX){
					int start = 0;
					int endkey = pars.find_first_of(":",0);
					key = pars.substr(start, endkey - start);
					value = pars.substr(endkey + 1, pars.size());
					this->_map[key] = value;
				}
			}
			for (std::map<std::string, std::string>::iterator it=this->_map.begin(); it != this->_map.end(); ++it){
				std::cout << GREEN << "key = " << it->first << std::endl << BLUE << " value = " << it->second << std::endl;
			}
		}
		void				parsing_mime(){
			std::ifstream			file("srcs/mime.types");
			std::string				line;

			while (getline(file, line)){
				size_t end = line.find_first_of("\t",0);
				std::string key = line.substr(0, end);
				// std::cout << BLUE << key << RESET << std::endl;
				size_t start = line.find_first_not_of("\t",end);
				std::list<std::string> value;
				while ((end = line.find_first_of(" ",start)) != SIZE_MAX){
					value.push_back(line.substr(start, end));
					// std::cout << GREEN << line.substr(start, end) << RESET << std::endl;
					start = end + 1;
				}
				this->_map_mime[key] = value;
			}
			file.close();
		}

	private:
		int													_socket;
		char												_buffer[1025];
		char												*_uri;
		char												_type;
		std::string											_typecontent;
		std::map<std::string, std::string>					_map;
		std::map<std::string, std::list<std::string> >		_map_mime;
};

#endif