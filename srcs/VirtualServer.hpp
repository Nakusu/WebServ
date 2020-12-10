#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include "./Header.hpp"
#include "Request.hpp"

class VirtualServer
{
	public:
		VirtualServer(void){
			initAddr(AF_INET, INADDR_ANY, htons(PORT));
		}
		VirtualServer(std::vector<std::string> file, std::string repos){
			this->_file = file;
			this->_root.push_back(repos);
			this->parsing();
			initAddr(AF_INET, INADDR_ANY, htons(atoi(this->_listen[0].c_str())));
			initFd(AF_INET , SOCK_STREAM , 0);
			initLink();
			initListen(4);
		}
		VirtualServer(VirtualServer const &rhs){
			operator=(rhs);
		}
		virtual ~VirtualServer(void){}
		VirtualServer &													operator=( VirtualServer const &rhs){
			if (this != &rhs){
			}
			return (*this);
		}

		class IncorrectMethodUsed : public std::exception
		{

			public:
				IncorrectMethodUsed( void ) {}
				IncorrectMethodUsed( IncorrectMethodUsed const & src ) { this->operator=(src); }
				virtual ~IncorrectMethodUsed( void ) throw() {}
				IncorrectMethodUsed &	operator=( IncorrectMethodUsed const & rhs ) { (void)rhs; return (*this); }
				virtual const char* what() const throw() { return ("Error : Incorrect Method Used."); }
		};

		/***************************************************
		*******************    Socket    *******************
		***************************************************/
		struct sockaddr_in												initAddr(int family, in_addr_t s_addr, in_port_t port){
			this->_address.sin_family = family;   
			this->_address.sin_addr.s_addr = s_addr;   
			this->_address.sin_port = port;
			return (this->_address); 
		}
		void															initLink(void){
			if (this->_fd != 0 && bind(this->_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0){
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
		}
		void															initListen(int number){
			if (listen(this->_fd, number) < 0){
				perror("listen");
				exit(EXIT_FAILURE);
			}
		}
		int																initFd(int domain, int type, int protocol){
			int opt = TRUE;

			if( (this->_fd = socket(domain , type , protocol)) == 0){
				perror("socket failed");
				exit(EXIT_FAILURE);
			}
			if( setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}
			return (this->_fd);
		}

		/***************************************************
		*****************    Operations    *****************
		***************************************************/
		std::vector<size_t>												findLocation(std::string uri){
			std::vector<size_t> index;

			for (size_t i = 0; i < this->_locations.size(); i++){
				if (this->_locations[i]["key"][0] == uri)
					index.push_back(i);
			}
			return (index);
		}
		std::vector<size_t>												findLocationsAndSublocations(std::string uri){
			//Search the locations and sublocations of the uri ex : index[0]:/var/toto/ index[1]:/var/ index[2]:/
			std::vector<size_t> index;

			while (uri.find('/') != SIZE_MAX){
				for (size_t i = 0; i < this->_locations.size(); i++){
					if (this->_locations[i]["key"][0] == uri)
						index.push_back(i);
				}
				uri = uri.erase(uri.size() - 1);
				uri = (uri.rfind('/') != SIZE_MAX) ? uri.substr(0,uri.rfind('/') + 1) : uri ;
			}
			return (index);
		}
		std::vector<std::string>										findOption(std::string option, std::string uri, int sub, std::vector<std::string> global){
			std::vector<size_t> tab;
			std::vector<std::string> result;

			if (sub)
				tab = findLocationsAndSublocations(uri);
			else
				tab = findLocation(uri);
			if (!tab.empty()){ // If there is a location
				for (size_t i = 0; i < tab.size(); i++){
					if (!this->_locations[tab[i]][option].empty()) //If we find the option, we split in vector
						result = this->_locations[tab[i]][option];
				}
			}
			if (result.empty())
				for (size_t i = 0; i < global.size(); i++)
					result.push_back(global[i]);
			return (result);
		}

		/***************************************************
		******************    GET/SET   ********************
		***************************************************/
		bool															get_autoIndex(void){
			return (this->_autoIndex);
		}
		std::vector<std::string>										get_errorPages(void){
			return (this->_errorPages);
		}
		std::vector<std::string>										get_index(void){
			return (this->_index);
		}
		size_t															get_indexSize(void){
			return (this->_index.size());
		}
		std::vector<std::string>										get_listen(void){
			return (this->_listen);
		}
		std::vector<std::string>										get_serverNames(void){
			return (this->_serverNames);
		}
		std::vector<std::string>										get_root(void){
			return (this->_root);
		}
		std::vector<std::map<std::string, std::vector<std::string> > >	get_locations(void){
			return (this->_locations);
		}
		std::string														getIndexByIndex(size_t i){
			return(this->_index[i]);
		}
		struct sockaddr_in *											getAddress(void){
			return (&this->_address);
		}
		std::string														getRoot(void){
			return (this->_root[0]);
		}
		int																getFd(void){
			return (this->_fd);
		}
		void															setFile(std::vector<std::string> file){
			this->_file = file;
		}
		std::vector<std::string>										get_method(void){
			return (this->_methods);
		}
		// std::vector<std::string>										get_cgi(void){
		// 	return (this->_cgi);
		// }

		/***************************************************
		******************    Parsing    *******************
		***************************************************/
		void															parsing(void){
			this->parsingServerToVector();
			this->parsingListen();
			this->parsingServerNames();
			this->parsingRoot();
			this->parsingIndex();
			this->parsingLocations();
			this->parsingAutoIndex();
			this->parsingRedirGbl();
			this->parsingMethods();
			this->parsingMaxBody();
		}
		void															parsingAutoIndex(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("autoindex") != SIZE_MAX)
				{
					if (this->_virtualserver[i].find("off") != SIZE_MAX)
						this->_autoIndex = false;
					else
						this->_autoIndex = true;
					return ;
				}
			}
		}
		void															parsingIndex(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("index") == 0)
				{
					std::string iss = this->_virtualserver[i];
					iss = convertInSpaces(iss);
					iss = cleanSpaces(iss);
					std::vector<std::string> results = split(iss, " ");
					results.erase(results.begin());
					this->_index = results;
					this->_index.back().erase(this->_index.back().size() - 1);
				}
			}
		}
		void															parsingListen(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++) {
				if (this->_virtualserver[i].find("listen") != SIZE_MAX){
					this->_listen.push_back(this->_virtualserver[i].substr(7, this->_virtualserver[i].size() - 8));
				}
			}
		}
		void															parsingLocations(void){
		std::map<std::string, std::vector<std::string> > value;
		for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
		{
			if (this->_virtualserver[i].find("location") != SIZE_MAX)
			{
				std::string qss = this->_virtualserver[i];
				qss = convertInSpaces(qss);
				qss = cleanSpaces(qss);
				std::vector<std::string> res = split(qss, " ");
				value["key"].push_back(res[1]);
				unsigned int j = (this->_virtualserver[i].find("{") != SIZE_MAX) ? i + 1 : i + 2;
				while (value["key"][0].find_last_not_of(" \t") != value["key"][0].size() -1 && value["key"][0].find_first_not_of(" \t") != SIZE_MAX)
						value["key"][0].erase(value["key"][0].size() - 1);
				while (this->_virtualserver[j].find("}") == SIZE_MAX && j < this->_virtualserver.size())
				{
					std::string iss = this->_virtualserver[j];
					iss = convertInSpaces(iss);
					iss = cleanSpaces(iss);
					std::vector<std::string> results = split(iss, " ");
					std::string key = results[0];
					results.erase(results.begin());
					if (key == "CGI"){
						key = results[0];
						results.erase(results.begin());
					}
					value[key] = results;
					//Delete the ';' at the end of the last
					value[key][value[key].size() - 1].erase(value[key][value[key].size() - 1].size() - 1);
					j++;
				}
				this->_locations.push_back(value);
				value.clear();
			}
		}
		}
		void 															parsingRedirGbl(void){
			unsigned int cpt = 0;
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_virtualserver[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_virtualserver[i].find("error_page") != SIZE_MAX && (cpt == 1))
					this->_errorPages.push_back(this->_virtualserver[i].substr(10, this->_virtualserver[i].size() - 11));
			}
		}
		void															parsingRoot(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("root") != SIZE_MAX){
					this->_root.push_back(this->_virtualserver[i].substr(5, this->_virtualserver[i].size() - 6));
					return ;
				}
			}
		}
		void															parsingServerNames(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("server_name") != SIZE_MAX){
					std::string iss = this->_virtualserver[i];
					iss = convertInSpaces(iss);
					iss = cleanSpaces(iss);
					std::vector<std::string> results = split(iss, " ");
					results.erase(results.begin());
					this->_serverNames = results;
					this->_serverNames.back().erase(this->_serverNames.back().size() - 1);
				}
			}
		}
		void															parsingServerToVector(void){
			for (unsigned int i = 0; i < this->_file.size(); i++){
				if (this->_file[i].find("server") != SIZE_MAX || this->_file[i].find("server{") != SIZE_MAX){
					unsigned int j = i + 1;
					unsigned int brackets = 1;
					this->_virtualserver.push_back(this->_file[i]);
					while (brackets != 0 && j < this->_file.size()){
						if (this->_file[j].find("{") != SIZE_MAX)
							brackets++;
						else if (this->_file[j].find("}") != SIZE_MAX)
							brackets--;
							this->_virtualserver.push_back(this->_file[j]);
						j++;
					}
				}
				return ;
			}
		}
		void																		parsingMethods(void){
			size_t cpt = 0;

			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_virtualserver[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_virtualserver[i].find("method") != SIZE_MAX && (cpt == 1)){
					this->_methods = split(this->_virtualserver[i], " ");
				}
			}
			if (!this->_methods.empty()){
				this->_methods[this->_methods.size() - 1].erase(this->_methods[this->_methods.size() - 1].size() - 1);
				this->_methods.erase(this->_methods.begin());
			}
		}
		void																		parsingMaxBody(void){
			size_t cpt = 0;

			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_virtualserver[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_virtualserver[i].find("maxBody") != SIZE_MAX && (cpt == 1)){
					this->_maxBody = split(this->_virtualserver[i], " ");
				}
			}
			if (!this->_maxBody.empty()){
				this->_maxBody[1].erase(this->_maxBody[1].size() - 1);
				this->_maxBody.erase(this->_maxBody.begin());
			}
		}
		void																		parsingCGI(void){
			size_t cpt = 0;
			std::vector<std::string> result;
			std::string key;

			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_virtualserver[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_virtualserver[i].find("CGI") != SIZE_MAX && (cpt == 1)){
					result = split(this->_virtualserver[i], " ");
					result.erase(result.begin());
					key = result[0];
					result.erase(result.begin());

				}
			}
			if (!this->_maxBody.empty()){
				this->_maxBody[1].erase(this->_maxBody[1].size() - 1);
				this->_maxBody.erase(this->_maxBody.begin());
			}
		}

		/***************************************************
		*******************    Verif    ********************
		***************************************************/

		void 																		verifications(void){
			//verifAllPathsInLocations();
			//verifAllMethods();
			//verifListen();
			//verifGblErrorPages();
			//verifLocationsErrorPages();
			//verifGblIndex();
			//verifLocationIndex();
			//verifGblRoot();
			//verifLocationRoot();
			//verifServerName();
			//verifLocationsPaths();
		}

		bool																		verifMethod(std::string method) {
			std::string validMethods[8] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};
			for (size_t i = 0; i < 8; i++)
				if (method == validMethods[i])
					return (true);
			return (false);
		}

		bool																		verifAllMethods(void) {
			for (size_t i = 0; i < this->_methods.size() - 1; i++) {
				if (!verifMethod(this->_methods[i])) {
					std::cerr << RED << "Error: Bad configuration of the global methods" << RESET << std::endl;
					return (false);
				}
			}
			for (size_t i = 0; i < this->_locations.size() - 1; i++) {
				if (!this->_locations[i]["method"].empty()) {
					if (!verifMethod(this->_locations[i]["method"][0])) { // A verifier
						std::cerr << RED << "Error: Bad configuration of method parameter in the location " << i + 1 << RESET << std::endl;
						return (false);
					}
				}
			}
			return (true);
		}

		bool																		verifListen(void){
			for (size_t i = 0; i < this->_listen.size(); i++) {	
				if (atoi(this->_listen[i].c_str()) < 0 || atoi(this->_listen[i].c_str()) > 65535) {
					std::cerr << RED << "Error: Bad configuration in listen parameter" << RESET << std::endl;
					return (false);
				}
			}
			return (true);
		}
		
		bool																		verifGblErrorPages(void){
			std::ifstream opfile;
			for (size_t i = 0; i < this->_errorPages.size() - 1; i++) {
				if (atoi(this->_errorPages[i].c_str()) < 100 || atoi(this->_errorPages[i].c_str()) > 599) {
					std::cerr << RED << "Error: Bad configuration of ports in global errorPages parameter" << RESET << std::endl;
					return (false);
				}
			}
			opfile.open(this->_errorPages[this->_errorPages.size() - 1].c_str());
			if (!opfile.is_open()) {
				std::cerr << RED << "Error: Bad configuration of path in global errorPages parameter" << RESET << std::endl;
				return (false);
			}
			opfile.close();
			return (true);
		}

		bool																		verifLocationsErrorPages(void){
			std::ifstream opfile;
			for (size_t i = 0; i < this->_locations.size() - 1; i++) {
				if (!this->_locations[i]["error_page"].empty()) {
					for (size_t j = 0; j < this->_locations[i]["error_page"].size() - 1; j++) {
						if (atoi(this->_locations[i]["error_page"][j].c_str()) < 100 || atoi(this->_locations[i]["error_page"][j].c_str()) > 599) {
							std::cerr << RED << "Error: Bad configuration of ports of errorPages parameter in the location" << i + 1 << RESET << std::endl;
							return (false);
						}
					}
					opfile.open((this->_locations[i]["error_page"][this->_locations[i]["error_page"].size() - 1] + this->_locations[i]["key"][0]).c_str());
					if (!opfile.is_open()) {
						std::cerr << RED << "Error: Bad configuration of path of errorPages parameter in the location" << i + 1 << RESET << std::endl;
						return (false);
					}
					opfile.close();
				}
			}
			return (true);
		}



		bool																		verifGblIndex(void) {
			std::ifstream opfile;
			for (size_t i = 0; i < this->_index.size(); i++) {
				opfile.open(this->_index[i].c_str());
				if (!opfile.is_open()) {
					std::cerr << RED << "Error: Bad configuration of path in global index parameter" << RESET << std::endl;
					return (false);	
				}
				opfile.close();
			}
			return (true);
		}

		bool																		verifLocationIndex(void) {
			std::ifstream opfile;
			for (size_t i = 0; i < this->_locations.size() - 1; i++) {
				if (!this->_locations[i]["index"].empty()) {
					for (size_t j = 0; j < this->_index.size(); j++) {
						opfile.open((this->_locations[i]["index"][j] + this->_locations[i]["key"][0]).c_str());
						if (!opfile.is_open()) {
							std::cerr << RED << "Error: Bad configuration of path in index parameter of location" << i + 1 << RESET << std::endl;
							return (false);	
						}
					opfile.close();
					}
				}
			}
			return (true);
		}

		bool																		verifGblRoot(void) {
			std::ifstream opfile;
			for (size_t i = 0; i < this->_root.size(); i++) {
				opfile.open(this->_root[i].c_str());
				if (!opfile.is_open()) {
					std::cerr << RED << "Error: Bad configuration of path in global root parameter" << RESET << std::endl;
					return (false);	
				}
				opfile.close();
			}
			return (true);
		}
		
		std::string                                                            getRootLocation(std::string path){
            std::vector<std::string> redir;
            redir = this->findOption("root", path, 0, this->get_root());
            if (redir.empty())
                return (this->getRoot());
            return (redir[0]);
        }

		bool																		verifLocationRoot(void) {
			std::ifstream opfile;
			for (size_t i = 0; i < this->_locations.size() - 1; i++) {
				if (!this->_locations[i]["root"].empty()) {
					for (size_t j = 0; j < this->_root.size(); j++) {
						opfile.open((getRootLocation(this->_locations[i]["key"][0]) + this->_locations[i]["key"][0]).c_str());
						if (!opfile.is_open()) {
							std::cerr << RED << "Error: Bad configuration of path in root parameter of location" << i + 1 << RESET << std::endl;
							return (false);	
						}
					opfile.close();
					}
				}
			}
			return (true);
		}

		bool																		verifServerName(void) {
			if (this->_serverNames.empty()) {
				std::cerr << RED << "Error: Bad configuration of server_name parameter" << RESET << std::endl;
				return (false);
			}
			return (true);
		}

		bool																		verifLocationsPaths(void) {
			std::ifstream opfile;
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (this->_locations[i]["key"].empty()) {
					std::cerr << RED << "Error: Path of location " << i + 1 << " is empty" << RESET << std::endl;
					return (false);
				}
				opfile.open(this->_locations[i]["key"][0].c_str());
				if (!opfile.is_open()) {
					std::cerr << RED << "Error: Bad configuration of path in location " << i + 1 << RESET << std::endl;
					return (false);	
				}
				opfile.close();
			}
			return (true);
		}

		bool																		verifAllPathsInLocations(void) {
			for (size_t i = 0; i < this->_locations.size(); i++) {
				std::map<std::string, std::vector<std::string> >::iterator it;
				for (it = this->_locations[i].begin(); it != this->_locations[i].end(); it++) {
					for (size_t i = 0; i < it->second.size(); i++) {
						if (it->first != "key" && it->second[i][it->second[i].size() - 1] == '/') {
							std::cerr << RED << "Error: Bad initialisation of path in a location " << RESET << std::endl;
							return (false);
						}
						else if (it->first == "key" && (it->second[0][0] != '/' || it->second[0][it->second[0].size() - 1] != '/'))
							return (false);
					}
				}
			}
			return (true);
		}

	private:
		int 															_fd;
		struct sockaddr_in 												_address;
		bool															_autoIndex;
		std::vector<std::string>										_errorPages;
		std::vector<std::string> 										_file;
		std::vector<std::string>										_index;
		std::vector<std::string>										_listen;
		std::vector<std::string>										_serverNames;
		std::vector<std::string> 										_root;
		std::vector<std::string>										_virtualserver;
		std::vector<std::map<std::string, std::vector<std::string> > >	_locations;
		std::vector<std::string>										_methods;
		std::vector<std::string>										_maxBody;
		std::map<std::string, std::vector<std::string> >				_CGI;

};

#endif
