#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"
#include "ParsingRequest.hpp"

class Request{
	public:
		Request(){
			this->_fd = 0;
			this->_CGI = 0;
			this->total = 0;
			this->_request = "";
			this->_uri = "";
			this->_typeContent = "";
			this->_authCredentials = "";
			this->_authType = "";
			this->_parsing = new ParsingRequest();
		}
		Request(int fd, std::map<std::string, std::string> mineTypes){
			this->_fd = fd;
			this->_CGI = 0;
			this->total = 0;
			this->_request = "";
			this->_uri = "";
			this->_typeContent = "";
			this->_authCredentials = "";
			this->_authType = "";
			this->_parsing = new ParsingRequest();
			this->addContent("HTTP/1.1", "200 OK");
			this->_mimeTypes = mineTypes;
		}
		virtual ~Request(){
			delete this->_parsing;
		}
		Request(Request const &cpy){
			operator=(cpy);
		}
		Request &								operator=( Request const &cpy){
			if (this != &cpy){
			}
			return (*this);
		}
		int										init(void){
			int 	size;
			char*	buffer;

			buffer = (char *)calloc(sizeof(char), 9999999);
			size = recv(this->_fd, buffer, 9999999, MSG_DONTWAIT);
			this->total += size; 

			if (size == 0)
				return (-1);
			// if (size == -1)
			// 	return(-1);
			this->_request += buffer;
			free(buffer);
			if (this->_request.find("\r\n\r\n") == SIZE_MAX)
				return (0);
			if (this->_request.rfind("\r\n\r\n") < 15)
				return (0);
			if (this->_request.find("Transfer-Encoding") != SIZE_MAX && this->_request.rfind("\r\n\r\n") == this->_request.find("\r\n\r\n"))
				return (0);
			if (this->_request.find("Content-Length") != SIZE_MAX && this->_request.rfind("\r\n\r\n") == this->_request.find("\r\n\r\n"))
				return (0);

			this->_requestBody = CleanBody(this->_request);
			this->_requestHeader = this->_request.substr(0, this->_request.find("\r\n\r\n") + 4);
			// std::cout << "--------------REQUEST HEADER--------------" << std::endl << this->_requestHeader << std::endl;
			// std::cout << "-------------- END REQUEST HEADER--------------" << std::endl << std::endl;
			this->_method = this->set_method();
			this->_parsing->parsingMap((char *)this->_requestHeader.c_str());
			this->_parsing->parsingMime();
			this->_parsing->parseGet();
			this->_extension = this->_parsing->getExtension();
			this->_datas = "";
			this->findUri();
			this->findTypeContent();
			this->parsingMetasVars();
			this->parsingAuthorizations();
			this->setPathInfo();
			return (1);
		}
		/***************************************************
		********************    GET   **********************
		***************************************************/
		int										get_CGI(void) const{
			return (this->_CGI);
		}
		std::string								get_uri(void) const{
			return (this->_uri);
		}
		std::string								getContentLength(void) const{
			std::string notFounded = "0";
			if (this->_parsing->getMap()["Content-Length"].empty())
				return (notFounded);
			return (this->_parsing->getMap()["Content-Length"]);
		}
		std::string								getTypeContent(void) const{ 
			return (this->_typeContent);
		}
		std::string								getExtension(void) const{
			return (this->_extension);
		}
		int										getfd(void) const{
			return (this->_fd);
		}
		std::string								getContentMimes(void) const{
			return (this->_parsing->getMap().find("Content-Type") != this->_parsing->getMap().end() ? this->_parsing->getMap()["Content-Type"] : "");
		}
		std::string								getQueryString(void) const{
			return (this->_queryString);
		}
		ParsingRequest *						get_Parsing(void) const{
			return (this->_parsing);
		}
		std::string								get_authType(void) const{
			return (this->_authType);
		}
		std::string								get_authCredential(void) const{
			return (this->_authCredentials);
		}
		std::string								get_host(void) const{
			return (this->_hostName);
		}
		std::string								get_port(void) const{
			return (this->_hostPort);
		}
		std::string								get_userAgent(void) const{
			return (this->_userAgent);
		}
		std::string								get_requestHeader(void) const{
			return (this->_requestHeader);
		}
		std::string								get_requestBody(void) const{
			return (this->_requestBody);
		}
		std::string								set_method(void){
			char *tmp = (char *)this->_requestHeader.c_str();
			std::string rep = "";
			for (int i = 0; (tmp[i] &&tmp[i] != ' ') ; i++)
				rep += tmp[i];
			return (rep);
		}
		std::string								get_method(void) const{ 
			return (this->_method);
		}
		std::string								get_IpClient(void) const{
			return (this->_IPClient);
		}
		std::string								get_PathInfo(void) const{
			return (this->_pathInfo);
		}
		std::string								get_url(void) const{
			return (("http://" + this->get_host() + ":" + this->get_port() + this->get_uri()));
		}
		std::string								get_datas(void) const{
			return (this->_datas);
		}
		int										get_headerSended(void){
			return (this->headerSended);
		}
		pid_t									get_PID(void){
			return (this->pid);
		}
		int *									get_Status(void){
			return (&this->status);
		}
		size_t									getSize(void) const{
			return (this->_content.size());
		}
		std::map<std::string, std::string>		getContent(void) const{
			return (this->_content);
		}
		std::string								getContent(std::string key){
			return (this->_content[key]);
		}
		// std::string								getDatas(void) {
		// 	std::string							tmpbuffer = this->_request;
		// 	std::string							ret;
		// 	size_t								lock = 0;
		// 	int									j = 0;

		// 	tmpbuffer = &tmpbuffer[(tmpbuffer.find("\n\r") + 3)];
		// 	for (size_t i = 0; i < tmpbuffer.size(); i++) {
		// 		if (tmpbuffer[i] == '=')
		// 			lock = 1;
		// 		else if (tmpbuffer[i] == '&')
		// 			lock = 0;
		// 		if (lock == 1)
		// 			j++;
		// 		if (j < atoi(this->getContentLength().c_str()) && lock == 1)
		// 			ret += tmpbuffer[i];
		// 	}
		// 	return (ret);
		// }

		void									getDatas(void) {
			this->_datas = this->_requestBody;
		}
		/***************************************************
		********************    SET   **********************
		***************************************************/
		void									setQueryString(void){
			this->_queryString = (this->_uri.find("?") != SIZE_MAX) ? &this->_uri[this->_uri.find("?") + 1] : "";
		}
		void									setCGI(int i){
			this->_CGI = i;
		}
		void									setfd(int fd){
			this->_fd = fd;
		}
		void									setUri(std::string uri){
			this->_uri = uri;
		}
		void									setheaderSended(int i){
			this->headerSended = i;
		}
		void									setPID(pid_t i){
			this->pid = i;
		}
		void									setStatus(int i){
			this->status = i;
		}
		void									setIPClient(char * pIPClient){
			this->_IPClient = (std::string)pIPClient;
		}
		void									setPathInfo(void){
			this->_extension =  (this->_uri.find(".") != SIZE_MAX) ? &this->_uri[this->_uri.find(".")] : "";
			if (this->_extension.empty())
				this->_pathInfo = "";
			else
				this->_pathInfo = (this->_extension.find("/") != SIZE_MAX) ? &this->_extension[this->_extension.find("/") + 1] : "";
		}
		/***************************************************
		*******************    SEND   **********************
		***************************************************/
		void									sendPacket(std::string content){
			// std::cout << "SIZE ========= " << content.size() << std::endl;
			send(this->_fd, content.c_str(), content.size(), MSG_CONFIRM);
		}
		void									sendPacket(char *content, size_t len){
			send(this->_fd, content, len, MSG_CONFIRM);
		}

		/***************************************************
		********************    FIND   *********************
		***************************************************/
		void									findUri(void){
			this->_uri = "";
			std::vector<std::string> lineUri = split(this->_requestHeader, " \t");
			this->_uri = lineUri[1];
			//this->_uri = this->_request.substr(this->_request.find("/"), (this->_request.find("HTTP") - 5));
			this->_uri = cleanLine(this->_uri);
		}
		void									findTypeContent(void){
			this->_typeContent = "";
			this->_typeContent = this->_parsing->getMap()["Accept"];
		}


		/***************************************************
		******************    Parsing   ********************
		***************************************************/
		void									parsingMetasVars(void){
			this->_hostName = this->_parsing->getMap()["Host"].substr(0, this->_parsing->getMap()["Host"].find_first_of(":"));
			this->_hostPort = &this->_parsing->getMap()["Host"][this->_parsing->getMap()["Host"].find_first_of(":") + 1];
			this->_userAgent = this->_parsing->getMap()["User-Agent"];
		}
		void									parsingAuthorizations(void){
			std::string iss = this->_parsing->getMap()["Authorization"];
			iss = convertInSpaces(iss);
			iss = cleanLine(iss);
			std::vector<std::string> results = split(iss, " ");
			if (!results.empty()){
				this->_authType = results[0];
				this->_authCredentials = results[1];
			}
		}

		/***************************************************
		*****************    Operations    *****************
		***************************************************/
		void									addContent(std::string key, std::string content){
			this->_content.insert(std::pair<std::string, std::string>(key, (content + "\r\n")));
			this->_size += 1;
		}
		void									updateContent(std::string key, std::string content){
			if (this->_content.find(key) == this->_content.end()) {
				this->addContent(key, content);
				return ;
			}
			this->_content[key] = (content + "\r\n");
		}
		void									sendHeader(){
			std::string										rep;
			rep = "HTTP/1.1 " + this->_content["HTTP/1.1"];
			for (std::map<std::string, std::string>::iterator i = this->_content.begin(); i != this->_content.end(); i++) {
				if (i->first != "HTTP/1.1")
					rep += i->first + ": " + i->second;
			}
			rep.erase(rep.size() - 1);
			rep.erase(rep.size() - 1);
			rep += "\n\n";

			// std::cout << "---------- REQUEST URI ------------    "<< req->get_uri() << std::endl;
			// std::cout << "---------- REPONSE HEADER----------" << std::endl << rep << std::endl;
			// std::cout << "---------- FIN REPONSE HEADER----------" << std::endl;
			sendPacket(rep.c_str());
		}
		void									basicHeaderFormat(){
			this->addContent("Host", (get_host() + ":" + get_port()));
			this->updateContent("Content-Location", get_uri());
			this->addContent("Server", "webserv");
			this->addContent("Date", getTime());
			this->updateContent("Content-Type", "text/html");
			if (this->_mimeTypes[getExtension()] != "")
				this->updateContent("Content-Type", this->_mimeTypes[getExtension()]);
			this->updateContent("Accept-Charset", "utf-8");
		}
		// void									basicHistory(VirtualServer *vserv){
		// 	if (vserv->get_history((get_IpClient() + get_userAgent())) != "")
		// 		this->updateContent("Referer", vserv->get_history((get_IpClient() + get_userAgent())));
		// 	if (!folderIsOpenable((vserv->findRoot(get_uri()))))
		// 		this->updateContent("Content-Length", NumberToString(getSizeFileBits(vserv->findRoot(get_uri()))));
		// }
		void									Error405HeaderFormat(std::string allowMethods){
			this->basicHeaderFormat();
			this->updateContent("HTTP/1.1", "405 Method Not Allowed");
			this->updateContent("Content-Type", "text/html");
			this->addContent("Allow", allowMethods);
		}
		void									RedirectionHeaderFormat(std::string uri){
			this->basicHeaderFormat();
			this->updateContent("HTTP/1.1", "301 Moved Permanently");
			this->updateContent("Content-Type", "text/html");
			this->updateContent("Location", uri);
			this->updateContent("Retry-After", "1");
			this->updateContent("Connection", "keep-alive");
		}
		void									sendForCGI(void){
			std::string buff = "";
			char line[2048];
			int ret;
			std::string tmp_out = "./tmp/tmp_out_" + NumberToString(this->_fd) + ".txt";
			int fd = open(tmp_out.c_str(), O_CREAT | O_RDONLY);

			while ((ret = read(fd, &line, 2046)) > 0){
				line[ret] = '\0';
				buff += std::string(line);
			}
			if (buff.find("\r\n\r\n") != SIZE_MAX)
				buff = &buff[buff.find("\r\n\r\n") + 4];
			if (this->headerSended == 0){
				basicHeaderFormat();
				updateContent("Content-Length", NumberToString(buff.size()));
				sendHeader();
			}
			sendPacket(buff);
			close(fd);
		}

		/***************************************************
		*********************    GET   *********************
		***************************************************/























private :
		int													_fd;
		int													_CGI;
		size_t												total;

		std::string											_request;
		std::string											_requestHeader;
		std::string											_requestBody;
		std::string											_uri;
		std::string											_typeContent;
		ParsingRequest *									_parsing;
		std::string											_method;
		std::string											_hostName;
		std::string											_hostPort;
		std::string											_IPClient;
		std::string											_userAgent;
		std::string											_authType;
		std::string											_authCredentials;
		std::string											_queryString;
		std::string 										_pathInfo;
		std::string											_extension;
		std::string											_datas;
		std::map<std::string, std::string> 					_mimesTypes;
		int													headerSended;
		pid_t												pid;
		int													status;
		std::map<std::string, std::string>					_content;
		size_t												_size;
		std::map<std::string, std::string>					_mimeTypes;
};

#endif
