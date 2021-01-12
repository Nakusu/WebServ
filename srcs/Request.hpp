#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Header.hpp"
#include "ParsingRequest.hpp"

class Request{
	public:
		Request(){
			this->_fd = 0;
			this->buffer = NULL;
			this->_CGI = 0;
			this->total = 0;
			this->_request = "";
			this->_uri = "";
			this->_typeContent = "";
			this->_authCredentials = "";
			this->_authType = "";
			this->_parsing = new ParsingRequest();
			this->headerSended = 0;
			this->pid = 0;
			this->status = 0;
			this->_time = 0;
			this->findend = 0;
			this->endHeader = 0;
		}
		Request(int fd, std::map<std::string, std::string> mineTypes){
			this->_fd = fd;
			this->buffer = NULL;
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
			this->headerSended = 0;
			this->pid = 0;
			this->status = 0;
			this->_time = 0;
			this->findend = 0;
			this->endHeader = 0;
		}
		virtual ~Request(){
			if (this->buffer != NULL)
				free(this->buffer);
			delete this->_parsing;
		}
		Request(Request const &cpy){
			operator=(cpy);
		}
		Request &											operator=( Request const &cpy){
			if (this != &cpy){
			}
			return (*this);
		}
		int													init(void){
			int 	size;

			if (this->_time == 0)
				time(&this->_time);
			this->buffer = (char *)calloc(sizeof(char), 9999999);
			size = recv(this->_fd, this->buffer, 9999999, MSG_DONTWAIT);
			this->total += size; 
			if (size == 0){
				free(this->buffer);
				this->buffer = NULL;
				return (-1);
			}
			if (size == -1){
				free(this->buffer);
				this->buffer = NULL;
				return (0);
			}
			this->_request += this->buffer;
			free(this->buffer);
			this->buffer = NULL;
			//Verification du header
			if (this->findend == 0){
				if ((this->endHeader = this->_request.find("\r\n\r\n")) != SIZE_MAX){
					this->findend = 1; 
					if (this->_request.find("Transfer-Encoding") != SIZE_MAX || this->_request.find("Length-Encoding") != SIZE_MAX)
						this->findend = 2;
				}
				else
					return (0);
			}
			if (this->findend == 2 && (this->endHeader == this->_request.size() - 4 || this->_request.compare(this->_request.size() - 4, 4, "\r\n\r\n") != 0))
				return (0);
			this->_requestBody = CleanBody(this->_request);
			this->_requestHeader = this->_request.substr(0, this->_request.find("\r\n\r\n") + 4);
			this->_method = this->set_method();
			this->_parsing->parsingMap((char *)this->_requestHeader.c_str());
			this->_parsing->parsingMime();
			this->_parsing->parseGet();
			this->_extension = this->_parsing->getExtension();
			this->_datas = "";
			this->findAcceptLanguage();
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

		int													get_CGI(void) const{
			return (this->_CGI);
		}
		std::string											get_uri(void) const{
			return (this->_uri);
		}
		std::string											getContentLength(void) const{
			std::string notFounded = "0";
			if (this->_parsing->getMap()["Content-Length"].empty())
				return (notFounded);
			return (this->_parsing->getMap()["Content-Length"]);
		}
		std::string											getTypeContent(void) const{ 
			return (this->_typeContent);
		}
		std::string											getExtension(void) const{
			return (this->_extension);
		}
		int													getfd(void) const{
			return (this->_fd);
		}
		std::string											getContentMimes(void) const{
			return (this->_parsing->getMap().find("Content-Type") != this->_parsing->getMap().end() ? this->_parsing->getMap()["Content-Type"] : "");
		}
		std::string											getQueryString(void) const{
			return (this->_queryString);
		}
		ParsingRequest *									get_Parsing(void) const{
			return (this->_parsing);
		}
		std::string											get_authType(void) const{
			return (this->_authType);
		}
		std::string											get_authCredential(void) const{
			return (this->_authCredentials);
		}
		std::string											get_host(void) const{
			return (this->_hostName);
		}
		std::string											get_port(void) const{
			return (this->_hostPort);
		}
		std::string											get_userAgent(void) const{
			return (this->_userAgent);
		}
		std::string											get_requestHeader(void) const{
			return (this->_requestHeader);
		}
		std::string											get_requestBody(void) const{
			return (this->_requestBody);
		}
		std::string											set_method(void){
			char *tmp = (char *)this->_requestHeader.c_str();
			std::string rep = "";
			for (int i = 0; (tmp[i] &&tmp[i] != ' ') ; i++)
				rep += tmp[i];
			return (rep);
		}
		std::string											get_method(void) const{ 
			return (this->_method);
		}
		std::string											get_IpClient(void) const{
			return (this->_IPClient);
		}
		std::string											get_PathInfo(void) const{
			return (this->_pathInfo);
		}
		std::string											get_url(void) const{
			return (("http://" + this->get_host() + ":" + this->get_port() + this->get_uri()));
		}
		std::string											get_datas(void) const{
			return (this->_datas);
		}
		int													get_headerSended(void){
			return (this->headerSended);
		}
		pid_t												get_PID(void){
			return (this->pid);
		}
		int *												get_Status(void){
			return (&this->status);
		}
		size_t												getSize(void) const{
			return (this->_content.size());
		}
		std::map<std::string, std::string>					getContent(void) const{
			return (this->_content);
		}
		std::string											getContent(std::string key){
			return (this->_content[key]);
		}
		void												getDatas(void) {
			this->_datas = this->_requestBody;
		}
		char *												getBuffer(void) {
			return (this->buffer);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void												setQueryString(void){
			this->_queryString = (this->_uri.find("?") != SIZE_MAX) ? &this->_uri[this->_uri.find("?") + 1] : "";
		}
		void												setCGI(int i){
			this->_CGI = i;
		}
		void												setfd(int fd){
			this->_fd = fd;
		}
		void												setUri(std::string uri){
			this->_uri = uri;
		}
		void												setheaderSended(int i){
			this->headerSended = i;
		}
		void												setPID(pid_t i){
			this->pid = i;
		}
		void												setStatus(int i){
			this->status = i;
		}
		void												setIPClient(char * pIPClient){
			this->_IPClient = (std::string)pIPClient;
		}
		void												setPathInfo(void){
			this->_extension =  (this->_uri.find(".") != SIZE_MAX) ? &this->_uri[this->_uri.find(".")] : "";
			if (this->_extension.empty())
				this->_pathInfo = "";
			else
				this->_pathInfo = (this->_extension.find("/") != SIZE_MAX) ? &this->_extension[this->_extension.find("/") + 1] : "";
		}

		/***************************************************
		*******************    SEND   **********************
		***************************************************/
		void												sendPacket(std::string content){
			send(this->_fd, content.c_str(), content.size(), MSG_CONFIRM);
		}
		void												sendPacket(char *content, size_t len){
			send(this->_fd, content, len, MSG_CONFIRM);
		}

		/***************************************************
		********************    FIND   *********************
		***************************************************/
		void												findUri(void){
			this->_uri = "";
			std::vector<std::string> lineUri = split(this->_requestHeader, " \t");
			this->_uri = lineUri[1];
			this->_uri = cleanLine(this->_uri);
		}
		void												findTypeContent(void){
			this->_typeContent = "";
			this->_typeContent = this->_parsing->getMap()["Accept"];
		}
		void												findAcceptLanguage(void){
			std::string cleanedLanguages = cleanLine(this->_parsing->getMap()["Accept-Language"]);
			std::vector<std::vector<std::string> > all;
			std::vector<std::string> firstParsing = split(cleanedLanguages, ",");
			size_t max;
			
			// Récupération de toute les langues et de leur priorite dans un vecteur
			for (size_t i = 0; i < firstParsing.size(); i++) {
				all.push_back(split(firstParsing[i], ";"));
				if (all[i].size() > 1) {
					if (all[i][1].find("=") != SIZE_MAX)
						all[i][1] = &all[i][1][all[i][1].find("=") + 1];
					else
						all[i][1] = "0";
				}
				else
						all[i].push_back("1");
			}
			// Tri des langue par ordre de priorite
			for (size_t i = 0; i < all.size() - 1; i++)
			{
				max = i;
				for (size_t j = i + 1; j < all.size(); j++)
					if (!all[j].empty() && !all[max].empty() && !all[max][1].empty() && !all[j][1].empty() && std::atof(all[j][1].c_str()) > std::atof(all[max][1].c_str()))
						max = j;
				if (max != i)
					std::swap(all[i], all[max]);
			}
			for (size_t k = 0; k < all.size(); k++){
				this->_acceptLanguage.push_back(all[k][0]);
			}
		}

		/***************************************************
		******************    Parsing   ********************
		***************************************************/
		void												parsingMetasVars(void){
			this->_hostName = this->_parsing->getMap()["Host"].substr(0, this->_parsing->getMap()["Host"].find_first_of(":"));
			this->_hostPort = &this->_parsing->getMap()["Host"][this->_parsing->getMap()["Host"].find_first_of(":") + 1];
			this->_userAgent = this->_parsing->getMap()["User-Agent"];
		}
		void												parsingAuthorizations(void){
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
		void												addContent(std::string key, std::string content){
			this->_content.insert(std::pair<std::string, std::string>(key, (content + "\r\n")));
			this->_size += 1;
		}
		void												updateContent(std::string key, std::string content){
			if (this->_content.find(key) == this->_content.end()) {
				this->addContent(key, content);
				return ;
			}
			this->_content[key] = (content + "\r\n");
		}
		void												sendHeader(){
			std::string										rep;
			rep = "HTTP/1.1 " + this->_content["HTTP/1.1"];
			for (std::map<std::string, std::string>::iterator i = this->_content.begin(); i != this->_content.end(); i++) {
				if (i->first != "HTTP/1.1")
					rep += i->first + ": " + i->second;
			}
			rep.erase(rep.size() - 1);
			rep.erase(rep.size() - 1);
			rep += "\n\n";
			sendPacket(rep.c_str());
		}
		void												basicAuthentificate(std::string realm){
			this->basicHeaderFormat();
			this->updateContent("HTTP/1.1", "401 Unauthorized");
			this->updateContent("www-Authenticate", std::string("Basic " + realm));
		}
		void												basicHeaderFormat(){
			this->addContent("Host", (get_host() + ":" + get_port()));
			this->updateContent("Content-Location", get_uri());
			this->addContent("Server", "webserv");
			this->addContent("Date", getTime());
			this->updateContent("Content-Type", "text/html");
			if (this->_mimeTypes[getExtension()] != "")
				this->updateContent("Content-Type", this->_mimeTypes[getExtension()]);
			this->updateContent("Accept-Charset", "utf-8");
		}
		void												Error405HeaderFormat(std::string allowMethods){
			this->basicHeaderFormat();
			this->updateContent("HTTP/1.1", "405 Method Not Allowed");
			this->updateContent("Content-Type", "text/html");
			this->addContent("Allow", allowMethods);
		}
		void												RedirectionHeaderFormat(std::string uri){
			this->basicHeaderFormat();
			this->updateContent("HTTP/1.1", "301 Moved Permanently");
			this->updateContent("Content-Type", "text/html");
			this->updateContent("Location", uri);
			this->updateContent("Retry-After", "1");
			this->updateContent("Connection", "keep-alive");
		}
		void												sendForCGI(void){
			std::string buff = "";
			char line[2048];
			int ret;
			std::string tmp_out = "./tmp/tmp_out_" + NumberToString(this->_fd) + ".txt";
			// std::string tmp_in = "./tmp/tmp_in_" + NumberToString(this->_fd) + ".txt";
			// remove(tmp_in.c_str());
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
			remove(tmp_out.c_str());
			close(fd);
		}

		void		updateURI(std::string fullPath)
		{
			std::string language = "";

			if (folderIsOpenable(fullPath))
				return;
			for (size_t i = 0; i < this->_acceptLanguage.size(); i++)
			{
				if (fileIsOpenable((fullPath + "." + this->_acceptLanguage[i]))) {
					this->_uri = this->_uri + "." + this->_acceptLanguage[i];
					language = "." + this->_acceptLanguage[i];
					return ;
				}
			}
			if (!fileIsOpenable(fullPath + language) && !language.empty())
			{
				this->basicHeaderFormat();
				this->updateContent("HTTP/1.1", "406 Not Accetpable");
				this->updateContent("Content-Length", "0");
				this->sendHeader();
			}
		}


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
		time_t												_time;
		size_t												findend;
		size_t												endHeader;
		std::vector<std::string>							_acceptLanguage;
		char *												buffer;
};

#endif
