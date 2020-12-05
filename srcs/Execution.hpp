
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
		Execution(ServerWeb *serv, VirtualServer *vserv, Request *req, HeaderRequest *header, char **envs){
			this->serv = serv;
			this->vserv = vserv;
			this->req = req;
			this->header = header;
			this->_envs = envs;
		}
		Execution(Execution const & rhs){
			operator=(rhs);
		}
		virtual ~Execution(void){
		}
		Execution &									operator=(Execution const & rhs){
			if (this != &rhs){
				this->vserv = rhs.vserv;
				this->req = rhs.req;
			}
			return (*this);
		}

		/***************************************************
		*********************    GET    ********************
		***************************************************/
		std::string									getRoot(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("root", this->req->get_uri(), 1, this->vserv->get_root());
			if (redir.empty())
				return (this->serv->get_root());
			return (redir[0]);
		}
		bool										getAutoIndex(void){
			std::vector<std::string> redir;
			redir = this->vserv->findOption("autoindex",this->req->get_uri(), 1, redir);
			if (redir.empty())
				return (this->vserv->get_autoIndex());
			else if (redir[0] == "on")
				return (1);
			else
				return (0);
		}

		/***************************************************
		*******************    SEARCH    *******************
		***************************************************/
		int											searchIndex(void){
			//If it's a folder
			if (folderIsOpenable(getRoot() + this->req->get_uri())) {
				std::string					autoindex;
				std::vector<std::string>	files;
				std::vector<std::string>	vec;
				size_t						index;
				
				this->header->updateContent("Content-Type", "text/html");

				vec = this->vserv->findOption("index", this->req->get_uri(), 0, this->vserv->get_index());
				files = listFilesInFolder(this->getRoot() + this->req->get_uri());

				for (size_t i = 0; i < vec.size(); i++){
					if ((index = searchInVec(vec[i], files)) != -1){ //Compare index with files in Folder
						this->req->setUri(this->req->get_uri() + files[index]); //Return new URI with the index
						return (0);
					}
				}
				autoindex = "<h1>Index of " + std::string(this->req->get_uri()) + "</h1><hr><pre>";
				//Search if AutoIndex is on
				if (this->getAutoIndex()){
					autoindex += "<a href=\"../\"> ../</a><br/>";
					for (size_t j = 0; j < files.size(); j++){
						autoindex += "<a href=\"" + std::string(this->req->get_uri()) + files[j] +"\">" + files[j] + "</a><br/>";
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
		void										searchError404(void){
			std::string redir;
			std::vector<std::string> vec = this->vserv->findOption("error_page", this->req->get_uri(), 1, this->vserv->get_errorPages());
		
			this->header->updateContent("HTTP/1.1", "404 Not Found");
			this->header->updateContent("Content-Type", "text/html");
			this->header->sendHeader(this->req);
			redir = vec.empty() ? this->getRoot() : this->getRoot() + "/" + vec[vec.size() - 1];
			if ((searchInVec("404", vec) == -1 && searchInVec("404", this->vserv->get_errorPages()) == -1) ||
			!fileIsOpenable(redir))
				req->sendPacket("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
			else 
				req->sendPacket(fileToString(redir));
		}

		/***************************************************
		*****************    OpenFiles    ******************
		***************************************************/
		int											openText(void){
			std::string textExtensions[68] = {"appcache", "ics", "ifb", "css", "csv", "html", "htm", "n3", "txt", "text",
			"conf", "def", "list", "log", "in", "dsc", "rtx", "sgml", "sgm", "tsv",
			"t", "tr", "roff", "man", "me", "ms", "ttl", "uri", "uris", "urls",
			"vcard", "curl", "dcurl", "scurl", "mcurl", "sub", "fly", "flx", "gv",
			"3dml", "spot", "jad", "wml", "wmls", "s", "asm", "c", "cc", "cxx",
			"cpp", "h", "hh", "hpp", "dic", "f", "for", "f77", "f90", "java",
			"opml", "p", "pas", "nfo", "etx", "sfv", "uu", "vcs", "vcf"};
			for (size_t i = 0; i < 68; i++)
				if (textExtensions[i] == (std::string)this->req->getExtension() && this->openFile(this->req->get_uri(), this->req))
					return (1);
			return (0);
		}
		int											openBinary(std::string file){
			std::ifstream		opfile;
			char 				*content = new char[4096];
			std::string tmp = this->getRoot() + file;
			memset(content,0,4096);
  			opfile.open(tmp.data(), std::ios::binary | std::ios::in);
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
		int											binaryFile(void){
			if (openBinary(this->req->get_uri()))
				return (1);
			return (0);
		}
		int											openFile(std::string file, Request *req){
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

		/***************************************************
		********************    CGI    *********************
		***************************************************/
 		std::map<std::string, std::string>			setMetaCGI(std::string script_name) {
			std::map<std::string, std::string> args;
			args = this->req->get_Parsing().getMap();
		
			args["AUTH_TYPE"] = req->get_authType();
			args["SERVER_SOFTWARE"] = "POLDERSERV/HTTP1.1";
			args["SERVER_PROTOCOL"] = "HTTPT/1.1";
			if (req->getContentMimes() == "" && this->openText())
				args["CONTENT_TYPE"] = "text/plain";
			else if (req->getContentMimes() == "" && !this->openText())
				args["CONTENT_TYPE"] = "application/octet-stream";
			else
				args["CONTENT_TYPE"] = req->getContentMimes();
			args["CONTENT_LENGTH"] = req->getContentLength();
			args["QUERY_STRING"] = req->getQueryString();
			args["SERVER_NAME"] = this->req->get_host();
			args["SERVER_PORT"] = this->req->get_port();
			args["REQUEST_URI"] = this->req->get_uri();
			args["SCRIPT_NAME"] = script_name;
			args["REMOTE_ADDR"] = this->req->get_IpClient();
			args["REQUEST_METHOD"] = this->req->get_method();
			args["GATEWAY_INTERFACE"] = "CGI/1.1";
			args["REMOTE_USER"] = req->get_authCredential();
			args["REMOTE_IDENT"] = req->get_authCredential();
			args["PATH_INFO"] = req->get_PathInfo();
			args["PATH_TRANSLATED"] = (this->getRoot() + this->req->get_uri());
			return (args);
		}
		char										**swapMaptoChar(std::map<std::string, std::string> args){
			char	**tmpargs = (char**)malloc(sizeof(char*) * (args.size() + 1));
			size_t	i = 0;
			tmpargs[args.size()] = 0;

			for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
				tmpargs[i++] = strdup((it->first + "=" + it->second).c_str());
			return (tmpargs);
		}
		void										processCGI(std::string cgi_path, char **args) {
			int  pfd[2];
			int  pid;

   			if (pipe(pfd) == -1)
       			return ; // error gestion
			pfd[0] = 1;
			pfd[1] = this->vserv->getFd();
			dup2(pfd[0], 0);
			dup2(pfd[1], 1);
   			if ((pid = fork()) < 0)
				return ; // error gestion
			if (pid == 0) {
				if (execve(cgi_path.c_str(), args, this->_envs) == -1)
					return ;
			} else {
				close(pfd[0]);
				close(pfd[1]);
			}
		}
		int											initCGI(Request *req) {
			std::vector<size_t> indexs = this->vserv->findLocationsAndSublocations(this->req->get_uri());
			std::vector<std::map<std::string, std::vector<std::string> > > locations = this->vserv->get_locations();
			if (!indexs.empty() && !locations[indexs[0]]["cgiextension"].empty() && !locations[indexs[0]]["cgi_path"].empty() && req->getExtension() == &locations[indexs[0]]["cgiextension"][0][1]) {
				std::cout << "READY FOR DO WORK !" << std::endl;
				if (fileIsOpenable(locations[indexs[0]]["cgi_path"][0])) {
					std::map<std::string, std::string> args = setMetaCGI(locations[indexs[0]]["cgi_path"][0]);
					char **tmpargs = swapMaptoChar(args);
					processCGI(locations[indexs[0]]["cgi_path"][0], tmpargs);
				}
			}
			return (1);
		}

		/***************************************************
		*****************    Operation    ******************
		***************************************************/
		int											needRedirection(void){
			if (folderIsOpenable(getRoot() + this->req->get_uri())) {
				std::string uri = this->req->get_uri();
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

	private:
		ServerWeb *			serv;
		VirtualServer *		vserv;
		Request * 			req;
		HeaderRequest *		header;
		char		  **	_envs;
};
#endif
