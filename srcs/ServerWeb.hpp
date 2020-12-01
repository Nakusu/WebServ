#ifndef SERVERWEB_HPP
#define SERVERWEB_HPP

#include "./Header.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"

class ServerWeb
{
	public:
		ServerWeb(void){
			this->_fdmax = 0;
			parsing_conf();
			set_repos("public");
		}
		ServerWeb(ServerWeb const &rhs){
			operator=(rhs);
		}
		virtual ~ServerWeb(void){}
		ServerWeb &														operator=( ServerWeb const &rhs){
			if (this != &rhs){
			}
			return (*this);
		}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string														get_repos(void){
			return (this->_repos);
		}
		VirtualServer*													getVS(int i){
			return (this->_VServs[i]);
		}
		size_t															getVSsize(void){
			return (this->_VServs.size());
		}
		int																get_fdmax(void){
			return (this->_fdmax);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void					  										set_repos(std::string repos){
			std::ifstream	folder(repos.c_str());
			if(folder.good() && this->check_repo(repos))
				this->_repos = repos;
			else
				std::cout << "REPO NOT FOUND" << repos << std::endl;
				// ERROR DE REPO BLOCK
		}
		void															set_fd(void){
			for (size_t i = 0; i < this->_VServs.size(); i++){
				FD_SET(this->_VServs[i]->get_fd() , &this->_readfds);
				if (this->_fdmax < this->_VServs[i]->get_fd())
					this->_fdmax = this->_VServs[i]->get_fd();
			}
			
		}

		/***************************************************
		*******************    WAIT   **********************
		***************************************************/
		int																wait_select(void){
			int activity;

			activity = select(this->_fdmax + 1, &this->_readfds , NULL , NULL , NULL);
			if ((activity < 0) && (errno!=EINTR)){   
				printf("select error");   
			}
			return (activity);
		}
		int																wait_request(int fd){
			return (FD_ISSET(fd, &this->_readfds));
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
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
		void															parsingVrServ(void){
			std::vector<std::string> Serv;

			for (unsigned int i = 0; i < this->_file.size(); i++)
			{
				unsigned int cpt = 0;
				if ((this->_file[i].find("server") != SIZE_MAX))
				{
					Serv.push_back(this->_file[i]);
					i = (this->_file[i].find("{") != SIZE_MAX) ? i + 1 : i + 2;
					cpt++;
					while (cpt != 0 && i < this->_file.size()){
						if (this->_file[i].find('{') != SIZE_MAX)
							cpt++;
						if (this->_file[i].find('}') != SIZE_MAX)
							cpt--;
						Serv.push_back(this->_file[i]);
						i++;
					}
					i--;
				}
				this->_separateVrServ.push_back(Serv);
				Serv.clear();
			}
		}

		/***************************************************
		******************    OTHERS   *********************
		***************************************************/
		bool															check_repo(std::string repos){
			DIR		*folder = opendir((repos).c_str());
			bool	ret = false;
			if(folder) {
				closedir(folder);
				ret = true;
			}
			return (ret);
		}
		void															OpenDefault(std::ifstream	*ifs){
			std::string  line;
			while (std::getline(*ifs, line)){
				line = (line.find_first_not_of("\t ") != SIZE_MAX) ? line.substr(line.find_first_not_of("\t "), line.size()) : line;
				line = (line.find_last_not_of("\t ") != SIZE_MAX) ? line.substr(0, line.find_last_not_of("\t ") + 1) : line;
				if (!line.empty())
					this->_file.push_back(line);
			}
			(*ifs).close();
		}
		void															CreateVServs(void){
			for (size_t i = 0; i < this->_separateVrServ.size(); i++){
				VirtualServer *vserv = new VirtualServer(this->_separateVrServ[i], this->_repos);
				this->_VServs.push_back(vserv);
			}
		}
		void															clear_fd(void){
			FD_ZERO(&this->_readfds);   
		}

	private:
		std::vector<VirtualServer*> 									_VServs;
		std::string														_repos;
		int																_fdmax;
		fd_set		 													_readfds;
		std::map<std::string, std::string>								_conf;
		std::vector<std::string> 										_file;
		std::vector<std::vector<std::string>>						   _separateVrServ;

};

#endif
