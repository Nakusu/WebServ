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
			this->_authCredentials = "";
			this->_authType = "";
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
			this->parsingAuthorizations();
			this->setPathInfo();
		}

		virtual ~Request(){
			close(this->_socket);
			return ; 
		}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string								get_uri(void) const{
			return (this->_uri);
		}
		std::string								getContentLength(void) const{
			std::string notFounded = "0";
			if (this->_parsing.getMap()["Content-Length"].empty())
				return (notFounded);
			return (this->_parsing.getMap()["Content-Length"]);
		}
		char									*getBuffer(void){
			return (this->_buffer);
		}
		std::string								getTypeContent(void) const{ 
			return (this->_typeContent);
		}
		std::string								getExtension(void) const{
			return (this->_parsing.getExtension());
		}
		int										getSocket(void) const{
			return (this->_socket);
		}
		std::string								getContentMimes(void) const{
			return (this->_parsing.getMap().find("Content-Type") != this->_parsing.getMap().end() ? this->_parsing.getMap()["Content-Type"] : "");
		}
		std::string								getQueryString(void) const{
			return (this->_queryString);
		}
		ParsingRequest							get_Parsing(void) const {
			return (this->_parsing);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void				setQueryString(void){
			this->_queryString = (this->_uri.find("?") != SIZE_MAX) ? &this->_uri[this->_uri.find("?") + 1] : "";
		}
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

		void				parsingAuthorizations(void){
			std::string iss = this->_parsing.getMap()["Authorization"];
			iss = convertInSpaces(iss);
			iss = cleanSpaces(iss);
			std::vector<std::string> results = split(iss, " ");
			if (!results.empty()) {
				this->_authType = results[0];
				this->_authCredentials = results[1];
			}
		}
		void					setIPClient(char * pIPClient){
			this->_IPClient = (std::string)pIPClient;
		}

		void					setPathInfo(void) {
			std::string extension =  (this->_uri.find(".") != SIZE_MAX) ? &this->_uri[this->_uri.find(".")] : "";
			if (extension.empty())
				this->_pathInfo = "";
			else
				this->_pathInfo = (extension.find("/") != SIZE_MAX) ? &extension[extension.find("/") + 1] : "";
		}
		/***************************************************
		*******************	SEND   **********************
		***************************************************/
		void				sendPacket(std::string content){
			send(this->_socket, content.c_str(), strlen(content.c_str()), 0);
		}
		void				sendPacket(char *content, size_t len){
			send(this->_socket, content, len, 0);
		}

		/***************************************************
		*******************	FIND   **********************
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
		std::string				get_authType(void) const {
			return (this->_authType);
		}
		std::string				get_authCredential(void) const {
			return (this->_authCredentials);
		}
		std::string				get_host(void) const {
				return (this->_hostName);
			}
		std::string			get_port(void) const {
				return (this->_hostPort);
			}
		std::string			get_userAgent(void) const {
				return (this->_userAgent);
			}
		std::string			set_method() {
				std::string rep = "";
				for (int i = 0; (this->_buffer[i] && this->_buffer[i] != ' ') ; i++)
					rep += this->_buffer[i];
				return (rep);
			}
		std::string			get_method(void) const{ 
				return (this->_method);
			}
		std::string			get_IpClient(void) const {
				return (this->_IPClient);
			}
		std::string			get_PathInfo(void) const{
			return (this->_pathInfo);
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
		std::string											_authType;
		std::string											_authCredentials;
		std::string											_queryString;
		std::string 										_pathInfo;
};

#endif