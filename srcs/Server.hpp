#ifndef SERVER_HPP
#define SERVER_HPP

#include "./Header.hpp"
#include "Request.hpp"

class Server
{
	public:
		Server(void){
			parsing_conf();
			init_fd(AF_INET , SOCK_STREAM , 0);
			init_addr(AF_INET, INADDR_ANY, htons(PORT));
			init_link();
			init_listen(atoi(this->_conf["worker_processes"].c_str()));
			set_repos("public");
		}
		Server(Server const &){}
		virtual ~Server(void){}
		Server &														operator=( Server const &rhs){
			if (this != &rhs){
				this->_fd = rhs._fd;
				this->_address = rhs._address;
			}
			return (*this);
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
		int																get_fd(void){
			return (this->_fd);
		}
		void															clear_fd(void){
			FD_ZERO(&this->_readfds);   
		}
		void															set_fd(void){
			FD_SET(this->_fd, &this->_readfds);    
		}
		struct sockaddr_in												get_address(void){
			return (this->_address);
		}
		void															wait_select(void){
			int activity;

			activity = select(this->_fd + 1, &this->_readfds , NULL , NULL , NULL); 
			if ((activity < 0) && (errno!=EINTR)){   
				printf("select error");   
			}  
		}
		int																wait_request(void){
			return (FD_ISSET(this->_fd, &this->_readfds));
		}
		std::string														get_repos(void){
			return (this->_repos);
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

		void															parsing_conf(void){
			std::ifstream			file("srcs/server.conf");
			std::string				line;

			while (getline(file, line)){
				if (line.find(";") != __SIZE_MAX__ && (line.find("#") == __SIZE_MAX__)){
					int start = line.find_first_not_of(" \t",0);
					int endkey = line.find_first_of(" ",start);
					std::string key = line.substr(start, endkey - start);
					std::string value = line.substr(line.find_first_not_of(" ",endkey), line.find_first_of(";",start) - start);
					this->_conf[key] = value;
				}
			}
			file.close();
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
						std::cout << "FILE : " << entry->d_name << std::endl;
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
			for (size_t i = 0; i < this->_locations.size(); i++)
			{
				std::cout << RED << this->_autoIndex << RESET << std::endl;
				if ((strncmp((char *)this->_locations[i]["key"].c_str(), (char *)uri.c_str(), this->_locations[i]["key"].length()) == 0) && 
				!this->_locations[i]["autoindex"].empty()){
					if (this->_locations[i]["autoindex"] == "on")
						return (1);
					else if (this->_locations[i]["autoindex"] == "off")
						return (0);
				}
			}
			return (this->_autoIndex);
		}
		void															parsingServerText(void){
			for (unsigned int i = 0; i < this->_file.size(); i++){
				if (this->_file[i].find("server ") != SIZE_MAX || this->_file[i].find("server{") != SIZE_MAX){	
					unsigned int j = i + 1;
					unsigned int brackets = 1;
					this->_serverText.push_back(this->_file[i]);
					while (brackets != 0 && j < this->_file.size())
					{
						if (this->_file[j].find("{") != SIZE_MAX)
							brackets++;
						else if (this->_file[j].find("}") != SIZE_MAX)
							brackets--;
							this->_serverText.push_back(this->_file[j]);
						j++;
					}
				}
				return ;
			}
		}
		void															parsingListen(void){
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
				if (this->_serverText[i].find("listen ") != SIZE_MAX)
					this->_listen.push_back(this->_serverText[i].substr(7, this->_serverText[i].size() - 8));
		}
		void															parsingServerNames(void){
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
			{
				if (this->_serverText[i].find("server_name ") != SIZE_MAX)
				{
					std::istringstream iss(this->_serverText[i]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					results.erase(results.begin());
					this->_serverNames = results;
					this->_serverNames.back().pop_back();
				}
			}
		}
		void															parsingRoot(void){
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
			{
				if (this->_serverText[i].find("root ") != SIZE_MAX)
				{
					this->_root = this->_serverText[i].substr(5, this->_serverText[i].size() - 6);
					return ;
				}
			}
		}
		void															parsingIndex(void){
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
			{
				if (this->_serverText[i].find("index ") == 0)
				{
					std::istringstream iss(this->_serverText[i]);
					std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
					results.erase(results.begin());
					this->_index = results;
					this->_index.back().pop_back();
				}
			}
		}
		void															parsingAutoIndex(void){
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
			{
				if (this->_serverText[i].find("autoindex ") != SIZE_MAX)
				{
					if (this->_serverText[i].find("off") != SIZE_MAX)
						this->_autoIndex = false;
					else
						this->_autoIndex = true;
					return ;
				}
			}
		}
		void															parsingLocations(void){
			std::map<std::string, std::string> value;
			for (unsigned int i = 0; i < this->_serverText.size(); i++)
			{
				if (this->_serverText[i].find("location") != SIZE_MAX)
				{
					unsigned int j = i + 1;
					value["key"] = this->_serverText[i].substr(this->_serverText[i].find_first_not_of(" \t", 8), this->_serverText[i].size() - (this->_serverText[i].find_first_not_of(" \t", 8) + 2));
					while (this->_serverText[j].find("}") == SIZE_MAX && j < this->_serverText.size())
					{
						std::istringstream iss(this->_serverText[j]);
						std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
						value[results[0]] = &this->_serverText[j][results[0].size() + 1];
						value[results[0]].pop_back();
						j++;
					}
					this->_locations.push_back(value);
					value.clear();
				}
			}
			std::cout << "[" << this->_locations[0]["autoindex"] << "]" << std::endl;
		}
		void															set_file(std::vector<std::string> file){
			this->_file = file;
		}
	private:
		int 															_fd;
		struct sockaddr_in 												_address;
		fd_set 															_readfds;
		std::string														_repos;
		std::map<std::string, std::string>								_conf;

		std::vector<std::string>										_serverText;
		std::vector<std::string>										_listen;
		std::vector<std::string>										_serverNames;
		std::string														_root;
		std::vector<std::string>										_index;
		bool															_autoIndex;
		std::vector<std::map<std::string, std::string> >				_locations;
		std::vector<std::string> 										_file;
};

#endif
