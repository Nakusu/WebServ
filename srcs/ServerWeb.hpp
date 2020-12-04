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
			this->_root = "public";
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
		std::string														get_root(void){
			return (this->_root);
		}
		VirtualServer*													getVS(int i){
			return (this->_VServs[i]);
		}
		size_t															getVSsize(void){
			return (this->_VServs.size());
		}
		int																getFdmax(void){
			return (this->_fdmax);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void															setAllFDSET_fdmax(void){
			for (size_t i = 0; i < this->_VServs.size(); i++){
				FD_SET(this->_VServs[i]->getFd() , &this->_readfds);
				if (this->_fdmax < this->_VServs[i]->getFd())
					this->_fdmax = this->_VServs[i]->getFd();
			}
		}

		/***************************************************
		*******************    WAIT   **********************
		***************************************************/
		int																waitForSelect(void){
			int activity;

			activity = select(this->_fdmax + 1, &this->_readfds , NULL , NULL , NULL);
			if ((activity < 0) && (errno != EINTR))  
				printf("select error");   
			return (activity);
		}
		int																verifFdFDISSET(int fd){
			return (FD_ISSET(fd, &this->_readfds));
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
		void															splitConfsVServ(void){
			//Transform the file in Vectors<VirtualServer> who contain is own conf in a Vector<String>
			std::vector<std::string> Conf;

			for (unsigned int i = 0; i < this->_file.size(); i++)
			{
				unsigned int cpt = 0;
				if ((this->_file[i].find("server") != SIZE_MAX))
				{
					Conf.push_back(this->_file[i]);
					i = (this->_file[i].find("{") != SIZE_MAX) ? i + 1 : i + 2;
					cpt++;
					while (cpt != 0 && i < this->_file.size()){
						if (this->_file[i].find('{') != SIZE_MAX)
							cpt++;
						if (this->_file[i].find('}') != SIZE_MAX)
							cpt--;
						Conf.push_back(this->_file[i]);
						i++;
					}
					i--;
				}
				this->_VServ_confs.push_back(Conf);
				Conf.clear();
			}
		}

		/***************************************************
		******************    OTHERS   *********************
		***************************************************/
		void															fileToVectorAndClean(std::ifstream *ifs){
			 std::string  line;
			while (std::getline(*ifs, line)){
				line = cleanSpaces(line);
				std::cout << "CHECK LINE [" << line << "]" << std::endl;
				if (!line.empty())
					this->_file.push_back(line);
			}
			(*ifs).close();
		}
		void															createVServs(void){
			for (size_t i = 0; i < this->_VServ_confs.size(); i++){
				VirtualServer *vserv = new VirtualServer(this->_VServ_confs[i], this->_root);
				this->_VServs.push_back(vserv);
			}
		}
		void															clearFd(void){
			FD_ZERO(&this->_readfds);   
		}

	private:
		std::vector<VirtualServer*> 									_VServs;
		std::string														_root;
		int																_fdmax;
		std::map<std::string, std::string>								_conf;
		std::vector<std::string> 										_file;
		std::vector<std::vector<std::string> >							_VServ_confs;
		fd_set		 													_readfds;
};

#endif
