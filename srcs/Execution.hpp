
#ifndef EXECUTION_H
#define EXECUTION_H

#include "Header.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"

class Execution
{
	public:
		Execution(void){
			this->serv = NULL;
			this->req = NULL;
			this->header = NULL;
		}
		Execution(VirtualServer *serv, Request *req, HeaderRequest *header){
			this->serv = serv;
			this->req = req;
			this->header = header;
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
		int								redirectToFolder(void){
			if (this->serv->check_repo(this->serv->get_repos() + this->req->get_uri())) {
				std::string uri = this->req->get_uri();
				if (uri.rfind('/') == uri.size() - 1){
					return (1);
				}
				else{
					uri.push_back('/');
					this->header->update_content("HTTP/1.1", "301 Moved Permanently");
					this->header->update_content("Content-Type", "text/html");
					this->header->update_content("Location", uri);
					this->header->send_header(this->req);
					return (0);
				}
			}
			return (1);
		}
		int								index(void){
			if (this->serv->check_repo(this->serv->get_repos() + this->req->get_uri())) {
				std::vector<std::string> files;
				this->header->update_content("Content-Type", "text/html");
				std::string autoindex = "<h1>Index of " + std::string(this->req->get_uri()) + "</h1><hr><pre>";
				files = this->serv->get_fileInFolder(this->req->get_uri()); // RECUPERATION DES FICHIERS DANS LE FOLDER
				for (size_t i = 0; i < this->serv->get_index_size(); i++){
					for (size_t j = 0; j < files.size(); j++){
						if (!this->serv->get_index(i).compare(files[j].c_str())){
							this->req->set_uri(std::string(this->req->get_uri()) + files[j]);
							return (0);
						}
					}
				}
				if (this->serv->get_AutoIndex(this->req->get_uri())){
						autoindex += "<a href=\"../\"> ../</a><br/>";
					for (size_t j = 0; j < files.size(); j++){
						autoindex += "<a href=\"" + std::string(this->req->get_uri()) + files[j] +"\">" + files[j] + "</a><br/>";
					}
						autoindex += "</pre><hr>";
					this->header->send_header(this->req);
					this->req->send_packet(autoindex.c_str());
				}
				else{
					this->header->update_content("HTTP/1.1", "403");
					this->header->send_header(this->req);
					this->req->send_packet("Interaction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
				}
				return (1);
			}
			return (0);
		}
		int								text(char *uri){
			if ((this->req->get_extension() == "css" || this->req->get_extension() == "html") && this->serv->open_file(uri, this->req)) {
				std::cout << "OPENFILE CSS HTML" << std::endl;
				return (1);
			}
			return (0);
		}
		int								binary_file(char *uri){
			if (this->serv->open_Binary(uri, this->req)) {
				std::cout << "OPENFILE BINARY" << std::endl;
				return (1);
			}
			return (0);
		}
		void							redir_404(std::string uri){
			std::cout << RED << uri << RESET << std::endl;
			std::string redir;
			if ((redir = this->serv->findRedirection("404", "error_page", uri)) == "error" || this->serv->try_open_file(redir) == 0){
				std::cout << RED << redir << RESET << std::endl;
				this->header->update_content("HTTP/1.1", "404 Not Found");
				this->header->update_content("Content-Type", "text/html");
				this->header->send_header(this->req);
				
				req->send_packet("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
			}
			else {
					std::cout << YELLOW << redir << RESET << std::endl;
					this->header->update_content("HTTP/1.1", "301 Moved Permanently");
					this->header->update_content("Content-Type", "text/html");
					this->header->update_content("Location", redir);
					this->header->send_header(this->req);
			}
		}

		

	private:
		VirtualServer*		serv;
		Request* 			req;
		HeaderRequest*		header;
};
#endif
