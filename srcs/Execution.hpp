
#ifndef EXECUTION_H
#define EXECUTION_H

#include "Header.hpp"
#include "Server.hpp"
#include "Request.hpp"

class Execution
{
	public:
		Execution(void){
			this->serv = NULL;
			this->req = NULL;
		}
		Execution(Server *serv, Request *req){
			this->serv = serv;
			this->req = req;
		}
		Execution(Execution const & rhs){
			operator=(rhs);
		}
		virtual ~Execution(void){

		}
		Execution &						operator=( Execution const & rhs){
			if (this != &rhs){
				this->serv = rhs.serv;
				this->req = rhs.req;
			}
			return (*this);

		}
		int								index(void){
			if (this->serv->check_repo(this->req->get_uri())) {
				std::vector<std::string> files;
				std::string autoindex = "HTTP/1.1 403\r\nContent-Type: text/html\n\n<h1>Index of : " + std::string(this->req->get_uri()) + "</h1>";
				files = this->serv->get_fileInFolder(this->req->get_uri()); // RECUPERATION DES FICHIERS DANS LE FOLDER
				for (size_t i = 0; i < this->serv->get_index_size(); i++){
					for (size_t j = 0; j < files.size(); j++){
						if (this->serv->get_index(i).compare(files[j].c_str())){
							this->req->set_uri(std::string(this->req->get_uri()) + "/" + files[j]);
							return (1);
						}
					}
				}
				if (this->serv->get_AutoIndex(this->req->get_uri())){
					for (size_t j = 0; j < files.size(); j++){
						autoindex += "<a href=\"" + std::string(this->req->get_uri()) + files[j] +"\">" + files[j] + "</a></br>";
					}
					this->req->send_packet(autoindex.c_str());
				}
				else
					this->req->send_packet("HTTP/1.1 403\r\nContent-Type: text/html\n\nInteraction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
				return (1);
			}
			return (0);
		}
		int								text(char *uri){
			if ((this->req->get_extension() == "css" || this->req->get_extension() == "html") && this->serv->open_file(uri, this->req)) {
				std::cout << "YES OPENFILE" << std::endl;
				return (1);
			}
			return (0);
		}
		int								binary_file(char *uri){
			if (this->serv->open_Binary(uri, this->req)) {
				std::cout << "YES OPENFILE BINARY" << std::endl;
				return (1);
			}
			return (0);
		}
		void							redir_404(void){
			 req->send_packet("HTTP/1.1 404\r\nContent-Type: text/html\n\n<html><head><link rel=\"stylesheet\" href=\"style.css\"></head><h1>Page introuvable</h1></html>");
		}

	private:
		Server*				serv;
		Request* 			req;
};
#endif
