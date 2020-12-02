#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"
#include "ParsingRequest.hpp"

class Request {
	public:
		Request(){
			this->_socket = 0;
			this->_uri = "";
			this->_typeContent = "";
		}
		Request(int socket){
			this->_socket = socket;
			this->_buffer[recv(this->_socket, &this->_buffer, sizeof(this->_buffer), 0)] = 0;
			this->_type = this->_buffer[0];
			this->_parsing.parsingMap(this->_buffer);
			this->_parsing.parsingMime();
			this->_parsing.parseGet();
			this->findUri();
			this->findTypeContent();
		}

		virtual ~Request(){
			close(this->_socket);
			return ; 
		}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string			getUri(void) const{
			return (this->_uri);
		}
		char				*getBuffer(void){
			return (this->_buffer);
		}
		char				getType(void) const{ 
			return (this->_type);
		}
		std::string			getTypeContent(void) const{ 
			return (this->_typeContent);
		}
		std::string			getExtension(void) const{
			return (this->_parsing.getExtension());
		}
		int					getSocket(void) const{
			return (this->_socket);
		}


		/***************************************************
		********************    SET   **********************
		***************************************************/
		void				setSocket(int socket){
			this->_socket = socket;
		}
		void				setUri(std::string uri){
			this->_uri = uri;
		}

		/***************************************************
		*******************    SEND   **********************
		***************************************************/
		void				sendPacket(std::string content){
			send(this->_socket, content.c_str(), strlen(content.c_str()), 0);
		}
		void				sendPacket(char *content, size_t len){
			send(this->_socket, content, len, 0);
		}

		/***************************************************
		*******************    FIND   **********************
		***************************************************/
		void				findUri(void){
			this->_uri = "";
			std::string string(this->_buffer);
			this->_uri = strndup(&this->_buffer[string.find("/")], (string.find("HTTP") - 5));
		}
		void				findTypeContent(void){
			this->_typeContent = "";
			this->_typeContent = this->_parsing.getMap()["Accept"];
		}

	private:
		int													_socket;
		char												_buffer[1025];
		char												_type;
		std::string											_uri;
		std::string											_typeContent;
		ParsingRequest										_parsing;
};

#endif