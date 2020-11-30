#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include "./Header.hpp"
#include "Request.hpp"

class VirtualServer
{
	public:
		VirtualServer(void){
			init_addr(AF_INET, INADDR_ANY, htons(PORT));
		}
		VirtualServer(std::vector<std::string> file, std::string repos){
			this->_file = file;
			this->_repos = repos;
			this->parsing();
			init_addr(AF_INET, INADDR_ANY, htons(atoi(this->_listen[0].c_str())));
			init_fd(AF_INET , SOCK_STREAM , 0);
			init_link();
			init_listen(4);
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

		struct sockaddr_in												init_addr(int family, in_addr_t s_addr, in_port_t port){
			this->_address.sin_family = family;   
			this->_address.sin_addr.s_addr = s_addr;   
   			this->_address.sin_port = port;
			return (this->_address); 

		}
		void															init_link(void){
			if (this->_fd != 0){
				if(bind(this->_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0){   
       				perror("bind failed");   
        			exit(EXIT_FAILURE);   
				}
 		   }
		}
		void															init_listen(int number){
			if (listen(this->_fd, number) < 0){
      			perror("listen");   
     			exit(EXIT_FAILURE);
			}   
		}
		int																init_fd(int domain, int type, int protocol){
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
		struct sockaddr_in												*get_address(void){
			return (&this->_address);
		}
		int																get_fd(void){
			return (this->_fd);
		}
		
		
		





		int																open_file(std::string file, Request *req) {
			std::ifstream opfile;
			std::string content;
			std::string tmp = this->_repos + file;
  			opfile.open(tmp.data());
			if (!opfile.is_open())
				return (0);
			req->send_packet("HTTP/1.1 200\n\n");
			while (std::getline(opfile, content))
				req->send_packet(content.c_str());
			opfile.close();
			return (1);
		}
		int																try_open_file(std::string file) {
			std::ifstream opfile;
			std::string tmp = this->_repos + file;
  			opfile.open(tmp.data());
			if (!opfile.is_open())
				return (0);
			opfile.close();
			return (1);
		}
		int																open_Binary(std::string file, Request *req) {
			std::ifstream		opfile;
			char 				*content = new char[4096];
			std::string tmp = this->_repos + file;
			memset(content,0,4096);
  			opfile.open(tmp.data());
			  if (!opfile.is_open())
			  	return (0);
			req->send_packet("HTTP/1.1 200\n\n");
			while (!opfile.eof()) {
				opfile.read(content, 4096); 
				req->send_packet(content, 4096);
			}
			opfile.close();
			return (1);
		}
		std::vector<std::string>										get_fileInFolder(std::string repos) {
			struct dirent				*entry;
			DIR							*folder;
			std::vector<std::string>	ret;

			folder = opendir((this->_repos + repos).c_str());
			if (folder) {
				while ((entry = readdir(folder))) {
					if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
						ret.push_back(entry->d_name);
					} 
				}
			}
			this->_repos = this->_repos.substr(this->_repos.find_first_not_of("\n "), this->_repos.size());
			return (ret);
		}
		size_t															get_index_size(){
			return (this->_index.size());
		}
		std::string														get_index(size_t i){
			return(this->_index[i]);
		}
		int																get_AutoIndex(std::string uri){
			// (void)uri;
			for (size_t i = 0; i < this->_locations.size(); i++)
			{
				if(uri.compare(this->_locations[i]["key"][0]) == 0	&& !this->_locations[i]["autoindex"][0].empty()){
					if (this->_locations[i]["autoindex"][0] == "on")
						return (1);
					else if (this->_locations[i]["autoindex"][0] == "off")
						return (0);
				}
			}
			return (this->_autoIndex);
		}
		std::string														get_repos(void){
			return (this->_repos);
		}
		void                      										set_repos(std::string repos){
            std::ifstream	folder(repos.c_str());
            if(folder.good() && this->check_repo(repos))
                this->_repos = repos;
            else
                std::cout << "REPO NOT FOUND" << repos << std::endl;
				// ERROR DE REPO BLOCK
        }
		bool															check_repo(std::string repos) {
			DIR		*folder = opendir((repos).c_str());
			bool	ret = false;
            if(folder) {
				closedir(folder);
                ret = true;
			}
            return (ret);
		}


		/***************************************************
		******************    Parsing    *******************
		***************************************************/
		void															parsing(void){
			this->parsingServerToVector();
			this->parsingListen();
			this->parsingServerNames();
			this->parsingRepos();
			this->parsingIndex();
			this->parsingLocations();
			this->parsingAutoIndex();
			this->parsingRedirGbl();
		}
		void															parsingListen(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
				if (this->_virtualserver[i].find("listen ") != SIZE_MAX){
					this->_listen.push_back(this->_virtualserver[i].substr(7, this->_virtualserver[i].size() - 8));
				}
		}
		void															parsingRepos(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("root ") != SIZE_MAX){
					this->_repos = this->_virtualserver[i].substr(5, this->_virtualserver[i].size() - 6);
					return ;
				}

			}
		}
		void															parsingServerToVector(void){
			for (unsigned int i = 0; i < this->_file.size(); i++){
				if (this->_file[i].find("server ") != SIZE_MAX || this->_file[i].find("server{") != SIZE_MAX){	
					unsigned int j = i + 1;
					unsigned int brackets = 1;
					this->_virtualserver.push_back(this->_file[i]);
					while (brackets != 0 && j < this->_file.size())
					{
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
		void															parsingServerNames(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("server_name ") != SIZE_MAX)
				{
					std::istringstream iss(this->_virtualserver[i]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					results.erase(results.begin());
					this->_serverNames = results;
					this->_serverNames.back().pop_back();
				}
			}
		}
		void															parsingIndex(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("index ") == 0)
				{
					std::istringstream iss(this->_virtualserver[i]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					results.erase(results.begin());
					this->_index = results;
					this->_index.back().pop_back();
				}
			}
		}
		void															parsingAutoIndex(void){
			for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
			{
				if (this->_virtualserver[i].find("autoindex ") != SIZE_MAX)
				{
					if (this->_virtualserver[i].find("off") != SIZE_MAX)
						this->_autoIndex = false;
					else
						this->_autoIndex = true;
					return ;
				}
			}
		}
		void															parsingLocations(void){
		std::map<std::string, std::vector<std::string>> value;
		for (unsigned int i = 0; i < this->_virtualserver.size(); i++)
		{
			if (this->_virtualserver[i].find("location") != SIZE_MAX)
			{
				std::istringstream qss(this->_virtualserver[i]);
				std::vector<std::string> res(std::istream_iterator<std::string>{qss}, std::istream_iterator<std::string>());
				value["key"].push_back(res[1]);
				unsigned int j = (this->_virtualserver[i].find("{") != SIZE_MAX) ? i + 1 : i + 2;
				while (value["key"][0].find_last_not_of(" \t") != value["key"][0].size() -1 && value["key"][0].find_first_not_of(" \t") != SIZE_MAX)
						value["key"][0].pop_back();
				while (this->_virtualserver[j].find("}") == SIZE_MAX && j < this->_virtualserver.size())
				{
					std::istringstream iss(this->_virtualserver[j]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					value[results[0]].push_back(&this->_virtualserver[j][results[0].size() + 1]);
					value[results[0]][value[results[0]].size() - 1].pop_back();
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
				if (this->_virtualserver[i].find("rewrite") != SIZE_MAX && (cpt == 1))
                    this->_errorPages.push_back(this->_virtualserver[i].substr(7, this->_virtualserver[i].size() - 8));
			}
		}

		std::map<std::string, std::string>								recupErrorByKeyLocations (std::vector<std::string>	&locations){
			std::map<std::string, std::string> errorsMap;

			if (!locations.empty())
			{
				for (unsigned int i = 0; i < locations.size(); i++)
				{
					std::istringstream iss(locations[i]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					for (unsigned int j = 0; j < results.size() - 1; j++)
					{
						if (atoi(results[j].c_str()) > 99 && atoi(results[j].c_str()) < 600 && errorsMap[results[j]].empty())
							errorsMap[results[j]] = results[results.size() - 1];
					}
				}
			}
			return (errorsMap);
		}
		std::vector<size_t>												findLocations(std::string uri){
			std::vector<size_t> index;
			while (uri.find('/') != SIZE_MAX){
				for (size_t i = 0; i < this->_locations.size(); i++){
					if (this->_locations[i]["key"][0] == uri){
						index.push_back(i);
					}
				}
				uri.pop_back();
				uri = (uri.rfind('/') != SIZE_MAX) ? uri.substr(0,uri.rfind('/') + 1) : uri ;
			}
			return (index);
		}
		std::string														findRedirection(std::string key , std::string option, std::string uri){
			std::vector<size_t> tab;

			tab = findLocations(uri); //recherche des locations en rapport avec uri (du plus profond jusqu'a /) ex : index[0]:/var/toto/ index[1]:/var/ index[2]:/
	        if (!tab.empty()){ // si pas de location pour cette uri on va recherche dans le global
				for (size_t i = 0; i < tab.size(); i++){
					if (!this->_locations[tab[i]][option].empty()){ //Si on trouve l'option que l'on recherche ex : error_page ou rewrite
						if (key.empty()){ //Si on ne connait pas la key que l'on recheche (exemple pour rewrite on recherche la correspondance d'un rewrite avec notre uri)
							for (size_t j = 0; j < this->_locations[tab[i]][option].size(); j++){
								//On split notre string pour rechercher si l'uri est egale a notre split[0] et si oui on redirige vers split[1]
								std::string uricut = uri;
								std::istringstream iss(this->_locations[tab[i]][option][j]);
								std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
								//On decoupe l'uri pour rechercher des chemins de plus plus large jusqu'a /
								while (uricut.find('/') != SIZE_MAX){
									if (results[0] == uricut)
										return (results[1]);
									uricut = (uricut.rfind('/') != SIZE_MAX) ? uricut.substr(0,uricut.rfind('/')) : uricut ;
								}
							}
						}
						//Si on trouve une correspondance avec notre clé dans notre location
						if (!recupErrorByKeyLocations(this->_locations[tab[i]][option])[key].empty())
							return (recupErrorByKeyLocations(_locations[tab[i]][option])[key]);
					}
				}
			} // Sinon on recherche dans le global
            if (option == "error_page"){
				if (!recupErrorByKeyLocations(this->_errorPages)[key].empty())
					return (recupErrorByKeyLocations(this->_errorPages)[key]);
			}
			else if (option == "rewrite"){
				if (!recupErrorByKeyLocations(this->_rewrite)[key].empty())
					return (recupErrorByKeyLocations(this->_rewrite)[key]);
			}
			return ("error");
        }
		void															set_file(std::vector<std::string> file){
			this->_file = file;
		}

	private:
		int 															_fd;
		bool															_autoIndex;
		std::vector<std::string>										_virtualserver;
		struct sockaddr_in 												_address;
	std::vector<std::string>											_listen;
		std::vector<std::string>										_serverNames;
		std::vector<std::string>										_index;
		std::vector<std::map<std::string, std::vector<std::string>>> 	_locations;
		std::vector<std::string>										_errorPages;
		std::vector<std::string> 										_rewrite;
		std::vector<std::string> 										_file;
		std::string 													_repos;


};

#endif
