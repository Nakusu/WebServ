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
			this->_method = this->set_method();
			this->_parsing.parsingMap(this->_buffer);
			this->_parsing.parsingMime();
			this->_parsing.parseGet();
			this->findUri();
			this->findTypeContent();
			this->parsingMetasVars();
		}

		virtual ~Request(){
			close(this->_socket);
			return ; 
		}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string			get_uri(void) const{
			return (this->_uri);
		}
		char				*getBuffer(void){
			return (this->_buffer);
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

		void				parsingMetasVars(void){
			this->_hostName = this->_parsing.getMap()["Host"].substr(0, this->_parsing.getMap()["Host"].find_first_of(":"));
			this->_hostPort = &this->_parsing.getMap()["Host"][this->_parsing.getMap()["Host"].find_first_of(":") + 1];
			this->_userAgent = this->_parsing.getMap()["User-Agent"];
		}
 
		void				setIPClient(char * pIPClient){
			this->_IPClient = (std::string)pIPClient;
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
		std::string			get_host(void) const{
			return (this->_hostName);
		}
		std::string			get_port(void) const{
			return (this->_hostPort);
		}
		std::string			get_userAgent(void) const{
			return (this->_userAgent);
		}
		std::string			set_method(){
			std::string rep = "";
			for (int i = 0; this->_buffer[i] != ' '; i++)
				rep += this->_buffer[i];
			return (rep);
		}
		std::string			get_method(void) const{ 
			return (this->_method);
		}
		std::string			get_IPClient(void) const{
			return (this->_IPClient);
		}

	private:
		int													_socket;
		char												_buffer[1025];
		std::string											_uri;
		std::string											_typeContent;
		ParsingRequest										_parsing;
		std::string											_method;
		std::string											_hostName;
		std::string											_hostPort;
		std::string											_IPClient;
		std::string											_userAgent;
};

#endif