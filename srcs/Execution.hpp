
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
			this->file = 1;
		}
		Execution(ServerWeb *serv, VirtualServer *vserv, Request *req, HeaderRequest *header, char **envs){
			this->serv = serv;
			this->vserv = vserv;
			this->req = req;
			this->header = header;
			this->_envs = envs;
			this->file = 1;
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
		std::string									getFullPath(void){
			return (this->vserv->findRoot(this->req->get_uri()) + this->req->get_uri());
		}
		std::string									getFullPath(std::string path){
			return (this->vserv->findRoot(path) + path);
		}
		std::string									get_fullPath(void){
			return (this->_fullPath);
		}
		std::string									findFullPath(void){
			std::vector<size_t> index = this->vserv->findLocationsAndSublocations(this->req->get_uri());
			for (size_t i = 0; i < index.size(); i++){
				if (!this->vserv->get_locations()[index[i]]["root"].empty())
					return (this->_fullPath = replaceStrStart(this->req->get_uri(), this->vserv->get_locations()[index[i]]["key"][0], this->vserv->get_locations()[index[i]]["root"][0]));
			}
			return (this->vserv->get_root() + this->req->get_uri());
		}
		std::string									findFullPath(std::string path){
			std::vector<size_t> index = this->vserv->findLocationsAndSublocations(path);
			if (!index.empty()){
				for (size_t i = 0; i < index.size(); i++){
					if (!this->vserv->get_locations()[index[i]]["root"].empty())
						return (this->_fullPath = replaceStr(path, this->vserv->get_locations()[index[i]]["key"][0], this->vserv->get_locations()[index[i]]["root"][0]));
				}
			}
			return (this->vserv->get_root() + path);
		}

		/***************************************************
		*******************    SEARCH    *******************
		***************************************************/
		int											searchIndex(void){
			//If it's a folder
			if (this->file == 1)
				return (0);
			if (folderIsOpenable(this->findFullPath())){
				std::string					autoindex = "";
				std::vector<std::string>	files;
				std::vector<std::string>	vec;
				size_t						index;

				this->header->updateContent("Content-Type", "text/html");
				vec = this->vserv->findIndex(this->req->get_uri());
				files = listFilesInFolder(this->findFullPath());
				for (size_t i = 0; i < vec.size(); i++){
					if ((index = searchInVec(vec[i], files)) != -1){//Compare index with files in Folder
						this->req->setUri(this->req->get_uri() + files[index]); //Return new URI with the index
						this->req->setPathInfo();
						return (0);
					}
				}
				//Search if AutoIndex is on
				if (this->vserv->findAutoIndex(this->req->get_uri())){
					
						autoindex = "<html><head><title>AutoIndex</title></head><body>";
						autoindex += "<h1>Index of " + this->req->get_uri() + "</h1><hr><pre>";
						autoindex += "<a href=\"../\"> ../</a><br/>";
						for (size_t j = 0; j < files.size(); j++)
							autoindex += "<a href=\"" + this->req->get_uri() + files[j] +"\">" + files[j] + "</a><br/>";
						autoindex += "</pre><hr>";
						autoindex += "</body></html>";
					this->header->basicHeaderFormat(this->req);
					this->header->basicHistory(this->vserv, this->req);
					this->header->updateContent("Content-Length", NumberToString(autoindex.size()));
					this->header->sendHeader(this->req);
					this->req->sendPacket(autoindex);
				}
				else{
					this->header->updateContent("HTTP/1.1", "403");
					this->header->basicHistory(this->vserv, this->req);
					this->header->updateContent("Content-Length", "24");
					this->header->sendHeader(this->req);
					this->req->sendPacket("Interaction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
				}
				return (1);
			}
			return (0);
		}
		void										searchError404(void){
			std::string redir = this->vserv->findErrorPage(this->req->get_uri(), "404");

			this->header->updateContent("HTTP/1.1", "404 Not Found");
			this->header->updateContent("Content-Type", "text/html");
			this->header->basicHistory(this->vserv, this->req);
			if (redir.empty()){
				this->header->updateContent("Content-Length", "159");
				this->header->sendHeader(this->req);
				req->sendPacket("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
				
			}
			else{
				this->header->sendHeader(this->req);
				req->sendPacket(fileToString(redir));
			}
		}
		void										searchError405(void){
			std::string redir = this->vserv->findErrorPage(this->req->get_uri(), "405");
		
			this->header->Error405HeaderFormat(this->req, this->getAllowMethods());
			this->header->basicHistory(this->vserv, this->req);
			if (redir.empty()){
				this->header->updateContent("Content-Length", "177");
				this->header->sendHeader(this->req);
				if (this->req->get_method() != "HEAD")
					req->sendPacket("<html><head><title>405 Method Not Allowed</title></head><body bgcolor=\"white\"><center><h1>405 Method Not Allowed</h1></center><hr><center>Les Poldters Server Web</center></html>");
			}
			else{
				this->header->sendHeader(this->req);
				if (this->req->get_method() != "HEAD")
					req->sendPacket(fileToString(redir));
			} 
		}

		/***************************************************
		*****************    OpenFiles    ******************
		***************************************************/
		int											openText(void){
			std::string textExtensions[68] = {".appcache", ".ics", ".ifb", ".css", ".csv", ".html", ".htm", ".n3", ".txt", ".text",
			"conf", ".def", ".list", ".log", ".in", ".dsc", ".rtx", ".sgml", ".sgm", ".tsv",
			"t", ".tr", ".roff", ".man", ".me", ".ms", ".ttl", ".uri", ".uris", ".urls",
			"vcard", ".curl", ".dcurl", ".scurl", ".mcurl", ".sub", ".fly", ".flx", ".gv",
			"3dml", ".spot", ".jad", ".wml", ".wmls", ".s", ".asm", ".c", ".cc", ".cxx",
			"cpp", ".h", ".hh", ".hpp", ".dic", ".f", ".for", ".f77", ".f90", ".java",
			"opml", ".p", ".pas", ".nfo", ".etx", ".sfv", ".uu", ".vcs", ".vcf"};
			for (size_t i = 0; i < 68; i++)
				if (textExtensions[i] == this->req->getExtension() && this->binaryFile())
					return (1);
			return (0);
		}
		int											binaryFile(void){
			std::ifstream		opfile;

			std::string tmp = this->findFullPath();
  			opfile.open(tmp.data(), std::ios::binary | std::ios::in);
			if (!opfile.is_open())
				return (0);
			long long unsigned int size_file = getSizeFileBits(tmp);
			char 				*content = (char *)calloc(sizeof(char), size_file + 1);
			this->header->basicHeaderFormat(this->req);
			this->header->basicHistory(this->vserv, this->req);
			this->header->updateContent("Content-Length", NumberToString(size_file));
			if (this->req->get_method() == "HEAD")
				this->header->updateContent("Content-Length", "0");
			this->header->sendHeader(this->req);
			if (this->req->get_method() != "HEAD") {
				opfile.read(content, size_file);
				req->sendPacket(content, size_file);
			}
			opfile.close();
			return (1);
		}

		/***************************************************
		********************    CGI    *********************
		***************************************************/

 		std::map<std::string, std::string>			setMetaCGI(std::string script_name){
			std::map<std::string, std::string> args;
			if (this->req->get_Parsing()->getMap().size() > 0) {
				std::map<std::string, std::string> tmpmap = this->req->get_Parsing()->getMap();
				std::map<std::string, std::string>::iterator it = tmpmap.begin();

				while (it != tmpmap.end() && it->first == "\n\r") { // Deuxieme condition a vérifier
					if (it->first != "First" && !it->first.empty())
						args.insert(std::make_pair(("HTTP_" + it->first), it->second));
					it++;
				}
			}
			args["AUTH_TYPE"] = req->get_authType();
			args["SERVER_SOFTWARE"] = "webserv";
			args["SERVER_PROTOCOL"] = "HTTP/1.1";
			if (req->getContentMimes() == "" && this->openText())
				args["CONTENT_TYPE"] = "text/plain";
			else if (req->getContentMimes() == "" && !this->openText())
				args["CONTENT_TYPE"] = "application/octet-stream";
			else
				args["CONTENT_TYPE"] = req->getContentMimes();
			args["CONTENT_LENGTH"] = req->getContentLength();
			if (req->getQueryString() != "")
				args["QUERY_STRING"] = req->getQueryString();
			else if (req->get_datas() != "")
				args["QUERY_STRING"] = req->get_datas();
			else
				args["QUERY_STRING"];
			args["SERVER_NAME"] = this->req->get_host();
			args["SERVER_PORT"] = this->req->get_port();
			args["REQUEST_URI"] = this->req->get_uri();
			args["SCRIPT_NAME"] = script_name;
			args["REMOTE_ADDR"] = this->req->get_IpClient();
			args["REQUEST_METHOD"] = this->req->get_method();
			args["GATEWAY_INTERFACE"] = "CGI/1.1";
			args["REMOTE_USER"] = this->req->get_authCredential();
			args["REMOTE_IDENT"] = this->req->get_authCredential();
			args["PATH_INFO"] = this->req->get_uri();
			args["PATH_TRANSLATED"] = "./" + this->vserv->get_root() + this->req->get_uri();
			return (args);
		}
		char **										swapMaptoChar(std::map<std::string, std::string> args){
			char	**tmpargs = (char**)malloc(sizeof(char*) * (args.size() + 1));
			size_t	i = 0;
			tmpargs[args.size()] = 0;

			for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
				tmpargs[i++] = strdup((it->first + "=" + it->second).c_str());
			tmpargs[i] = 0;
			return (tmpargs);
		}
		void										processCGI(std::string cgi_path, char **args){
			int  pfd[2];
			int  pid;
			char **env = mergeArrays(args, this->_envs, 0);
			int status;

			char **tmp = (char**)malloc(sizeof(char*) * 1);
			tmp[0] = strdup(cgi_path.c_str());

			this->header->basicHeaderFormat(this->req);
			this->header->sendHeader(this->req);
			if (pipe(pfd) == -1)
				return ; // error gestion
			if ((pid = fork()) < 0)
				return ; // error gestion
			if (pid == 0) {
				close(pfd[1]);
				pfd[0] = open(std::string(this->vserv->get_root() + this->req->get_uri()).c_str(), O_RDONLY);
				dup2(pfd[0], 0); // ici en entrée mettre le body
				dup2(this->req->getfd(), 1);
				errno = 0;
				if (execve(cgi_path.c_str(), tmp, env) == -1){
					std::cerr << "Error with CGI: " << strerror(errno) << std::endl;
					exit(1);
				}
			}
			else {
				close(pfd[0]);
				waitpid(pid, &status,0);
			}
			free(tmp[0]);
			free(tmp);
		}
		int											initCGI(void){
			std::string extension = (this->req->getExtension().find(".", 0) != SIZE_MAX) ? this->req->getExtension() : "." + this->req->getExtension();
			std::string path = this->vserv->findCGI(this->req->get_uri(), extension, this->req->get_method());
			std::cout << "CHECK PATH " << path << std::endl;
			if (path != "bad_method" && path != "no_cgi"){
				std::cout << "JUST BEFORE CGI" << std::endl;
				if (fileIsOpenable(path)){
					std::map<std::string, std::string> args = setMetaCGI(path);
					char **tmpargs = swapMaptoChar(args);
					processCGI(path, tmpargs);
					
					for (size_t i = 0; tmpargs[i]; i++){
						free(tmpargs[i]);
					}
					free(tmpargs);
					
					return (1);
				}
			}
			return (0);
		}
		int											doPut(void) {
			if (this->req->get_method() == "PUT") {
				std::string path = this->get_fullPath();
				std::string filename = getfilename(this->req->get_uri);
				std::cout << "CHECK FULL PATH " << path << std::endl;
				if (fileIsOpenable(path)) {
					// UPDATE FILE
				} else {

				}
				return (1);
			}
			return (0);
		}
		/***************************************************
		*****************    Operation    ******************
		***************************************************/
		int											needRedirection(void){

			this->_fullPath = this->findFullPath();
			if (fileIsOpenable(this->_fullPath) && !folderIsOpenable(this->_fullPath))
				return (0);
			this->file = 0;
			if (this->_fullPath.rfind("/", 0) != this->_fullPath .size() - 1)
				this->_fullPath = this->findFullPath(this->req->get_uri() + "/");
			if (folderIsOpenable(this->_fullPath)) {
				std::string uri = this->req->get_uri();
				if (uri.rfind('/') == uri.size() - 1)
					return (0);
				else{
					uri.push_back('/');
					this->header->RedirectionHeaderFormat(this->req, uri);
					this->header->basicHistory(this->vserv, this->req);
					this->header->updateContent("Content-Length", "0");
					this->header->sendHeader(this->req);
					return (1);
				}
			}
			return (0);
		}
		std::string									getAllowMethods(void){
			std::string					ret;
			std::vector<std::string>	methods = this->vserv->findMethod(this->req->get_uri());

			for (size_t i = 0; i < methods.size(); i++)
				ret = " " + methods[i] + ",";
			return (ret);
		}
		bool										checkMethod(void){
			if (this->vserv->findCGI(this->req->get_uri(), this->req->getExtension(), this->req->get_method()) == "bad_method")
				return (false);
			return (this->vserv->findMethod(this->req->get_uri(), this->req->get_method()));
		}

	private:
		ServerWeb *			serv;
		VirtualServer *		vserv;
		Request * 			req;
		HeaderRequest *		header;
		std::string 		_fullPath;
		char **				_envs;
		bool				file;
};
#endif

