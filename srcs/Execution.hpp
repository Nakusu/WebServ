
#ifndef EXECUTION_H
#define EXECUTION_H

#include "Header.hpp"
#include "VirtualServer.hpp"
#include "Request.hpp"

class Execution
{
	public:
		Execution(void){
			this->vserv = NULL;
			this->req = NULL;
			this->header = NULL;
		}
		Execution(ServerWeb *serv, VirtualServer *vserv, Request *req, HeaderRequest *header){
			this->serv = serv;
			this->vserv = vserv;
			this->req = req;
			this->header = header;
		}
		Execution(Execution const & rhs){
			operator=(rhs);
		}
		virtual ~Execution(void){

		}
		Execution &						operator=(Execution const & rhs){
			if (this != &rhs){
				this->vserv = rhs.vserv;
				this->req = rhs.req;
			}
			return (*this);

		}

		/***************************************************
		******************    Redirect    ******************
		***************************************************/
		std::string						getRoot(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("root",this->req->getUri(), 1, this->vserv->get_root());
			if (redir.empty())
				return (this->serv->get_root());
			return (redir[0]);
		}
		bool							getAutoIndex(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("autoindex",this->req->getUri(), 1, redir);
			if (redir.empty())
				return (this->vserv->get_autoIndex());
			else if (redir[0] == "on")
				return (1);
			else
				return (0);
		}
		int								needRedirection(void){
			if (this->vserv->folderIsOpenable(getRoot() + this->req->getUri())) {
				std::string uri = this->req->getUri();
				if (uri.rfind('/') == uri.size() - 1){
					return (0);
				}
				else{
					uri.push_back('/');
					this->header->updateContent("HTTP/1.1", "301 Moved Permanently");
					this->header->updateContent("Content-Type", "text/html");
					this->header->updateContent("Location", uri);
					this->header->sendHeader(this->req);
					return (1);
				}
			}
			return (0);
		}
		int								index(void){
			//If it's a folder
			if (this->vserv->folderIsOpenable(getRoot() + this->req->getUri())) {
				std::string					autoindex;
				std::vector<std::string>	files;
				std::vector<std::string>	vec;
				size_t						index;
				
				this->header->updateContent("Content-Type", "text/html");

				vec = this->vserv->findOption("index", this->req->getUri(), 0, this->vserv->get_index());
				files = this->listFilesInFolder(this->req->getUri());

				for (size_t i = 0; i < vec.size(); i++){
					if ((index = searchInVec(vec[i], files)) != -1){ //Compare index with files in Folder
						this->req->setUri(this->req->getUri() + files[index]); //Return new URI with the index
						return (0);
					}
				}
				autoindex = "<h1>Index of " + std::string(this->req->getUri()) + "</h1><hr><pre>";
				//Search if AutoIndex is on
				if (this->getAutoIndex()){
					autoindex += "<a href=\"../\"> ../</a><br/>";
					for (size_t j = 0; j < files.size(); j++){
						autoindex += "<a href=\"" + std::string(this->req->getUri()) + files[j] +"\">" + files[j] + "</a><br/>";
					}
					autoindex += "</pre><hr>";
					this->header->sendHeader(this->req);
					this->req->sendPacket(autoindex.c_str());
				}
				else{
					this->header->updateContent("HTTP/1.1", "403");
					this->header->sendHeader(this->req);
					this->req->sendPacket("Interaction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
				}
				return (1);
			}
			return (0);
		}
		void							error404(void){
			std::string redir;
			std::vector<std::string> vec = this->vserv->findOption("error_page", this->req->getUri(), 1, this->vserv->get_errorPages());
		
			this->header->updateContent("HTTP/1.1", "404 Not Found");
			this->header->updateContent("Content-Type", "text/html");
			this->header->sendHeader(this->req);
			if (searchInVec("404", vec) || searchInVec("404", this->vserv->get_errorPages()))
				req->sendPacket("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
			else {
				redir = this->getRoot() + vec[vec.size()];
			}
		}
		int								fileIsOpenable(std::string file){
			std::ifstream opfile;
			std::string tmp = this->getRoot() + file;
  			opfile.open(tmp.data());
			if (!opfile.is_open())
				return (0);
			opfile.close();
			return (1);
		}

		/***************************************************
		*****************    OpenFiles    ******************
		***************************************************/
		int								text(void){
			if ((this->req->getExtension() == "css" || this->req->getExtension() == "html") && this->openFile(this->req->getUri(), this->req)) {
				return (1);
			}
			return (0);
		}
		int								openBinary(std::string file){
			std::ifstream		opfile;
			char 				*content = new char[4096];
			std::string tmp = this->getRoot() + file;
			memset(content,0,4096);
  			opfile.open(tmp.data());
			  if (!opfile.is_open())
			  	return (0);
			req->sendPacket("HTTP/1.1 200\n\n");
			while (!opfile.eof()) {
				opfile.read(content, 4096); 
				req->sendPacket(content, 4096);
			}
			opfile.close();
			return (1);
		}
		int								binaryFile(void){
			if (openBinary(this->req->getUri()))
				return (1);
			return (0);
		}
		int								openFile(std::string file, Request *req){
			std::ifstream opfile;
			std::string content;
			std::string tmp = this->getRoot() + file;
  			opfile.open(tmp.data());
			if (!opfile.is_open())
				return (0);
			req->sendPacket("HTTP/1.1 200\n\n");
			while (std::getline(opfile, content))
				req->sendPacket(content.c_str());
			opfile.close();
			return (1);
		}
		std::vector<std::string>		listFilesInFolder(std::string repos){
			struct dirent				*entry;
			DIR							*folder;
			std::vector<std::string>	ret;

			folder = opendir((this->getRoot() + repos).c_str());
			while (folder && (entry = readdir(folder))) {
				if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
					ret.push_back(entry->d_name);
				} 
			}
			return (ret);
		}
	private:
		ServerWeb *			serv;
		VirtualServer *		vserv;
		Request * 			req;
		HeaderRequest *		header;
};
#endif
