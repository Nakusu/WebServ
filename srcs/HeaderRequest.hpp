#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#include "Header.hpp"
#include "Request.hpp"
class HeaderRequest {
	public:
		HeaderRequest() {
			this->_content.insert(std::pair<std::string, std::string>("HTTP/1.1", "200 OK"));
		}
		virtual	~HeaderRequest() {
			return ;
		}

		/***************************************************
		*****************    Operations    *****************
		***************************************************/
		void											addContent(std::string key, std::string content){
			this->_content.insert(std::pair<std::string, std::string>(key, (content + "\r\n")));
			this->_size += 1;
		}
		void											updateContent(std::string key, std::string content){
			if (this->_content.find(key) == this->_content.end()) {
				this->addContent(key, content);
				return ;
			}
			this->_content[key] = (content + "\r\n");
		}
		void											sendHeader(Request *req){
			std::string										rep;
			rep = "HTTP/1.1 " + this->_content["HTTP/1.1"];
			for (std::map<std::string, std::string>::iterator i = this->_content.begin(); i != this->_content.end(); i++) {
				if (i->first != "HTTP/1.1"){
					rep += i->first + ": " + i->second;
					std::cout << "CHECK REQUEST HEADER " << rep << std::endl;
				}
			}
			rep.erase(rep.size() - 1);
			rep.erase(rep.size() - 1);
			rep += "\n\n";
			req->sendPacket(rep.c_str());
		}

		/***************************************************
		*********************    GET   *********************
		***************************************************/
		size_t											getSize(void) const{
			return (this->_content.size());
		}
		std::map<std::string, std::string>				getContent(void) const{
			return (this->_content);
		}
		std::string										getContent(std::string key){
			return (this->_content[key]);
		}

	private:
		std::map<std::string, std::string>				_content;
		size_t											_size;
};

#endif