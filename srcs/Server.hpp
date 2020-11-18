
#ifndef SERVER_HPP
#define SERVER_HPP

#include "./Header.hpp"

class Server
{
	public:
		Server(void){
			parsing_conf();
			init_fd(AF_INET , SOCK_STREAM , 0);
			init_addr(AF_INET, INADDR_ANY, htons(PORT));
			init_link();
			init_listen(this->_conf.operator["worker_processes"]);
			set_repos("~");
		}
		Server(Server const &){}
		virtual ~Server(void){}
		Server &					operator=( Server const &rhs){
			if (this != &rhs){
				this->_fd = rhs._fd;
				this->_address = rhs._address;
			}
			return (*this);
		}

		int							init_fd(int domain, int type, int protocol){
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
		struct sockaddr_in			init_addr(int family, in_addr_t s_addr, in_port_t port){
			this->_address.sin_family = family;   
			this->_address.sin_addr.s_addr = s_addr;   
   			this->_address.sin_port = port;
			return (this->_address); 

		}
		void						init_link(void){
			if (this->_fd != 0){
				if(bind(this->_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0){   
       				perror("bind failed");   
        			exit(EXIT_FAILURE);   
				}
 		   }
		}
		void						init_listen(int number){
			if (listen(this->_fd, number) < 0){
      			perror("listen");   
     			exit(EXIT_FAILURE);
			}   
		}
		int							get_fd(void){
			return (this->_fd);
		}
		void						clear_fd(void){
			FD_ZERO(&this->_readfds);   
		}
		void						set_fd(void){
			FD_SET(this->_fd, &this->_readfds);    
		}
		struct sockaddr_in			get_address(void){
			return (this->_address);
		}
		void						wait_select(void){
			int activity;

			activity = select(this->_fd + 1, &this->_readfds , NULL , NULL , NULL); 
			if ((activity < 0) && (errno!=EINTR)){   
				printf("select error");   
			}  
		}
		int							wait_request(void){
			return (FD_ISSET(this->_fd, &this->_readfds));
		}
		std::string					get_repos(void){
			return (this->_repos);
		}
		std::string					open_file(std::string file) {
			std::ifstream opfile;
			std::string content;
			std::string reponse;
			std::string tmp = this->repos + file;
  			opfile.open(tmp.data());
			if (!opfile.is_open())
				return (NULL);
			while (std::getline(opfile, content))
				reponse += content;
			std:: cout << "REPONSE BEFORE " << reponse.c_str() << std::endl;
			return (reponse);
		}
		void						set_repos(std::string repos){
			std::ifstream folder(repos.c_str());
			if(folder.good())
				this->repos = repos;
			else
				std::cout << "REPO NOT FOUND" << repos << std::endl;
			// FAIRE L'ERROR DE LANCEMENT SI FOLDER NOT FOUND 
		void                        set_repos(std::string repos){
            std::ifstream folder(repos.c_str());
            if(folder.good())
                this->_repos = repos;
            else
                std::cout << "REPO NOT FOUND" << repos << std::endl;
            // FAIRE L'ERROR DE LANCEMENT SI FOLDER NOT FOUND 
        }
		void						parsing_conf(void){
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
			for (std::map<std::string, std::string>::iterator it=this->_conf.begin(); it != this->_conf.end(); ++it){
				std::cout << "key = " << it->first << " value = " << it->second << std::endl;
			}
		}

	private:
		int 									_fd;
		struct sockaddr_in 						_address;
		fd_set 									_readfds;
		std::string								_repos;
		std::map<std::string, std::string>		_conf;
};

#endif
