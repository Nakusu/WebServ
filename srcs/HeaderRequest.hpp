#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#include <iostream>
#include <map>
#include <string>
#include "Request.hpp"

class HeaderRequest {
	public:
		HeaderRequest() {
			this->_size = 0;
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
			return (this->_size);
		}
		std::map<std::string, std::string>				get_content() const {
			return (this->_content);
		}
		std::string										get_content(std::string key) {
			return (this->_content[key]);
		}
		void											send_header(Request req) {
			std::map<std::string, std::string>::iterator i = this->_content.begin();
			for (; i != this->_content.end(); i++) {
				req->send_packet((std::get<0>(i) + " " std::get<1>(i) + "\n\r").c_str());
			}
			req->send_packet(("\n\n").c_str());
		}
	private:
		std::map<std::string, std::string>				_content;
		size_t											_size;
};

#endif