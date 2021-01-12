
#ifndef EXECUTION_H
#define EXECUTION_H

#include "Header.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"

class Execution
{
	public:
		Execution(void){
			this->vserv = NULL;
			this->req = NULL;
			this->req = NULL;
			this->file = 1;
		}
		Execution(ServerWeb *serv, VirtualServer *vserv, Request *req, char **envs){
			this->serv = serv;
			this->vserv = vserv;
			this->req = req;
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
						return (this->_fullPath = replaceStr(path, this->vserv->get_locations()[index[i]]["key"][0], this->vserv->get_locations()[index[i]]["root"][0] + '/'));
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

				this->req->updateContent("Content-Type", "text/html");
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
					this->req->basicHeaderFormat();
					// this->req->basicHistory(this->vserv);
					this->req->updateContent("Content-Length", NumberToString(autoindex.size()));
					this->req->sendHeader();
					this->req->sendPacket(autoindex);
				}
				else
					searchError404();
				return (1);
			}
			return (0);
		}
		void										searchError404(void){
			std::string redir = this->vserv->findErrorPage(this->req->get_uri(), "404");

			this->req->updateContent("HTTP/1.1", "404 Not Found");
			this->req->updateContent("Content-Type", "text/html");
			// this->req->basicHistory(this->vserv);
			if (redir.empty()){
				this->req->updateContent("Content-Length", "159");
				this->req->sendHeader();
				req->sendPacket("<html><head><title>404 Not Found</title></head><body bgcolor=\"white\"><center><h1>404 Not Found</h1></center><hr><center>Les Poldters Server Web</center></html>");
				
			}
			else{
				this->req->sendHeader();
				req->sendPacket(fileToString(redir));
			}
		}
		void										searchError405(void){
			std::string redir = this->vserv->findErrorPage(this->req->get_uri(), "405");
		
			this->req->Error405HeaderFormat(this->getAllowMethods());
			// this->req->basicHistory(this->vserv);
			if (redir.empty()){
				this->req->updateContent("Content-Length", "177");
				this->req->sendHeader();
				if (this->req->get_method() != "HEAD")
					req->sendPacket("<html><head><title>405 Method Not Allowed</title></head><body bgcolor=\"white\"><center><h1>405 Method Not Allowed</h1></center><hr><center>Les Poldters Server Web</center></html>");
			}
			else{
				this->req->sendHeader();
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
			char *content = (char *)calloc(sizeof(char), size_file + 1);
			this->req->basicHeaderFormat();
			//this->req->basicHistory(this->vserv);
			this->req->updateContent("Content-Length", NumberToString(size_file));
			if (this->req->get_method() == "HEAD")
				this->req->updateContent("Content-Length", "0");
			this->req->sendHeader();
			if (this->req->get_method() != "HEAD") {
				opfile.read(content, size_file);
				req->sendPacket(std::string(content));
			}
			opfile.close();
			free(content);

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

				while (it != tmpmap.end()) { // Deuxieme condition a vérifier
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
			if (this->req->get_method() == "POST")
				args["CONTENT_LENGTH"] = NumberToString(this->req->get_datas().size()) ;
			if (req->getQueryString() != "")
				args["QUERY_STRING"] = req->getQueryString();
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
		void										CreateTmpRequestCGI(std::string tmp_in){
			std::ofstream in (tmp_in.c_str());
			if (this->req->get_method() == "POST")
				in << this->req->get_datas();
			std::ifstream fichier (this->get_fullPath().c_str());
			std::string contenu;
			while (getline(fichier, contenu))
				in << contenu;
			fichier.close();
			in.close();
		}
		void										processCGI(std::string cgi_path, char **args){
			int  pfd[2];
			char **env = mergeArrays(args, this->_envs, 0);

			char **tmp = (char**)malloc(sizeof(char*) * 3);
			tmp[0] = strdup(cgi_path.c_str());
			tmp[1] = NULL;
			tmp[2] = NULL;
			



			std::string tmp_in = "./tmp/tmp_in_" + NumberToString(this->req->getfd()) + ".txt";
			std::string tmp_out = "./tmp/tmp_out_" + NumberToString(this->req->getfd()) + ".txt";
			this->req->setCGI(1);

			if (pipe(pfd) == -1)
				return ; // error pipe
			pid_t pid = fork();
				
			if (pid < 0)
				return ; // error fork
			if (pid == 0) { // in the fork child
				close(pfd[1]);
				CreateTmpRequestCGI(tmp_in);
				//Open and send request to EXEC
				pfd[0]= open(tmp_in.c_str(), O_CREAT | O_RDONLY, 0777);
				dup2(pfd[0], 0); // ici en entrée mettre le body
				//Create a receive request in EXEC
				close(pfd[0]);
				remove(tmp_in.c_str());
				int tmp_fd = open(tmp_out.c_str(), O_CREAT | O_WRONLY, 0777);
				dup2(tmp_fd, 1);
				errno = 0;
				if (execve(cgi_path.c_str(), tmp, env) == -1){
					std::cerr << "Error with CGI: " << strerror(errno) << std::endl;
					exit(1);
				}
				// close(pfd[0]);
				close(tmp_fd);
			}
			else {
				close(pfd[0]);
				this->req->setPID(pid);
			}
			free(tmp[0]);
			free(tmp);
			free(env);
		}
		int											initCGI(void){
			std::string extension = (this->req->getExtension().find(".", 0) != SIZE_MAX) ? this->req->getExtension() : "." + this->req->getExtension();
			std::string path = this->vserv->findCGI(this->req->get_uri(), extension, this->req->get_method());
			if (path != "bad_method" && path != "no_cgi"){
				if (fileIsOpenable(path)){
					std::map<std::string, std::string> args = setMetaCGI(path);
					char **tmpargs = swapMaptoChar(args);
					processCGI(path, tmpargs);
					for (size_t i = 0; tmpargs[i]; i++)
						free(tmpargs[i]);
					free(tmpargs);
					return (1);
				}
			}
			return (0);
		}
		int											doAuthenticate(void) {
			std::vector<std::string> global;
			std::vector<std::string> option = this->vserv->findOption("Authenticate", this->req->get_uri(), global);
			
			if (!option.empty() && option.size() == 4) {
				if (!this->req->get_authType().empty() && !this->req->get_authCredential().empty()) {
					if (this->req->get_authType() == option[0]) {
						std::string tmp = decode64(this->req->get_authCredential());
						std::vector<std::string> elements = split(tmp, ":");
						if (elements.size() == 2 && elements[0] == option[2] && elements[1] == option[3]) {
							return (0);
						}
					}
				}
				this->req->basicAuthentificate(option[0], option[1]);
				this->req->updateContent("Content-Length", "80");
				this->req->sendHeader();
				this->req->sendPacket("<html><head><title>Unauthorized</title></head><body><h1>Unauthorized</h1></body>");
				return (1);
			}
			return (0);
		}
		int											doPut(void) {
			if (this->req->get_method() == "PUT") {
				std::string path = this->get_fullPath();
				std::string newFileName = (path[path.length() - 1] == '/') ? path.substr(0, path.length() - 1) : path;
				std::ofstream	newFile(newFileName.c_str());
				std::string newFileContent = this->req->get_requestBody();

				if (newFile.fail())
					return (0);
				newFile << newFileContent;
				std::string root = this->vserv->get_root();
				std::string headerLoc = (path.find(root) != SIZE_MAX) ? &path[root.length()] : path;
				if (!newFileContent.empty())
					this->req->updateContent("HTTP/1.1", "201 Created");
				else
					this->req->updateContent("HTTP/1.1", "204 No Content");
				this->req->updateContent("Content-Location", headerLoc);
				this->req->updateContent("Content-Length", "0");
				this->req->sendHeader();
				newFile.close();
				return (1);
			}
			return (0);
		}
		int											doPost(void) {
			if (this->req->get_method() == "POST") {
				this->req->getDatas();
				this->req->basicHeaderFormat();
				this->req->updateContent("HTTP/1.1", "200 OK");

				std::string maxbody = this->vserv->findOption("maxBody", this->req->get_uri(), this->vserv->get_maxBody());
				if (!maxbody.empty() && std::strtoul(maxbody.c_str(), NULL, 10) < this->req->get_datas().size()){
					this->req->updateContent("HTTP/1.1", "413 Request Entity Too Large");
					this->req->updateContent("Content-Length", "0");
					this->req->sendHeader();
					return (1);
				}
				else
					this->req->updateContent("Content-Length", NumberToString(this->req->get_datas().size()));
				this->req->sendHeader();
				this->req->setheaderSended(1);
				if (initCGI() == 0)
					this->req->sendPacket(this->req->get_requestBody());
				return (1);
			}
			return (0);
		}

		int											doDelete(void) {
			if (this->req->get_method() == "DELETE") {
				std::string path = this->get_fullPath();
				std::string newFileName = (path[path.length() - 1] == '/') ? path.substr(0, path.length() - 1) : path;
				std::string root = this->vserv->get_root();
				std::string headerLoc = (path.find(root) != SIZE_MAX) ? &path[root.length()] : path;

				if (!std::remove(newFileName .c_str())) {
					this->req->updateContent("HTTP/1.1", "200 OK");
					this->req->updateContent("Content-Location", headerLoc);
					this->req->updateContent("Content-Length", "48");
					this->req->sendHeader();
					this->req->sendPacket("<html><body><h1>File deleted.</h1></body></html>");
					return (1);
				}
				this->req->updateContent("HTTP/1.1", "204 No Content");
				this->req->updateContent("Content-Location", headerLoc);
				this->req->updateContent("Content-Length", "0");
				this->req->sendHeader();
				return (1);
			}
			return (0);
		}

		int											doOptions(void) {
			
			if (this->req->get_method() == "OPTIONS") {
				if (this->req->get_url() == "*")
					this->req->updateContent("Allow", "GET, POST, PUT, CONNECT, OPTIONS, HEAD, TRACE");
				else
					this->req->updateContent("Allow", getAllowMethods());
				this->req->updateContent("Content-Length", "0");
				this->req->sendHeader();
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
			if (this->_fullPath.rfind("/") != this->_fullPath .size() - 1)
				this->_fullPath = this->findFullPath(this->req->get_uri() + "/");
			if (folderIsOpenable(this->_fullPath)) {
				std::string uri = this->req->get_uri();
				if (uri.rfind('/') == uri.size() - 1)
					return (0);
				else{
					uri.push_back('/');
					this->req->setUri(uri);
					return (0);
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
			if (!fileIsOpenable(this->get_fullPath()) && this->req->get_method() != "POST" && this->req->get_method() != "PUT"){
				this->searchError404();
				return 1;
			}
			if (this->vserv->findCGI(this->req->get_uri(), this->req->getExtension(), this->req->get_method()) == "bad_method" ||
			!this->vserv->findMethod(this->req->get_uri(), this->req->get_method())){
				this->searchError405();
				return 1;
			}
			return 0;
		}

	private:
		ServerWeb *			serv;
		VirtualServer *		vserv;
		Request * 			req;
		std::string 		_fullPath;
		char **				_envs;
		bool				file;

};
#endif

