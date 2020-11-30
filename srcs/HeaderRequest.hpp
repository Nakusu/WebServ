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
		void											add_content(std::string key, std::string content) {
			this->_content.insert(std::pair<std::string, std::string>(key, content));
			this->_size += 1;
		}
		void											update_content(std::string key, std::string content) {
			if (this->_content.find(key) == this->_content.end()) {
				this->add_content(key, content);
				return ;
			}
			this->_content[key] = content;
		}
		size_t											get_size() const {
			return (this->_content.size());
		}
		std::map<std::string, std::string>				get_content() const {
			return (this->_content);
		}
		std::string										get_content(std::string key) {
			return (this->_content[key]);
		}
		void											send_header(Request *req) {
			std::string										rep;
			rep = "HTTP/1.1 " + this->_content["HTTP/1.1"] + "\r\n";
			for (std::map<std::string, std::string>::iterator i = this->_content.begin(); i != this->_content.end(); i++) {
				if (i->first != "HTTP/1.1"){
					rep += i->first + ": " + i->second;
					rep += "\r\n";
				}
			}
			rep.pop_back();
			rep.pop_back();
			rep += "\n\n";
			req->send_packet(rep.c_str());
		}
	private:
		std::map<std::string, std::string>				_content;
		size_t											_size;
};

#endif