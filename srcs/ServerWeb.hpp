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
		void															setAllFDSET_fdmax(void){
			for (size_t i = 0; i < this->_VServs.size(); i++){
				FD_SET(this->_VServs[i]->get_fd() , &this->_readfds);
				if (this->_fdmax < this->_VServs[i]->get_fd())
					this->_fdmax = this->_VServs[i]->get_fd();
				for (size_t j = 0; j < this->_VServs[i]->get_fd(); j++){
					this->_VServs[i]->get_fdClients(j);
				}
			}
		}

		/***************************************************
		*******************    WAIT   **********************
		***************************************************/
		int																waitForSelect(void){
			int activity;
			struct timeval			timeout;

			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
			activity = select(this->_fdmax + 1, &this->_readfds , NULL , NULL , &timeout);
			if ((activity < 0) && (errno != EINTR))  
				printf("select error");   
			return (activity);
		}
		int																verifFdFDISSET(int fd){
			return (FD_ISSET(fd, &this->_write) && FD_ISSET(fd, &this->_readfds));
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
		void															createVServs(void){
			//Find configuration and create VSERV
			std::vector<std::string> Conf;

			for (unsigned int i = 0; i < this->_file.size(); i++){
				unsigned int cpt = 0;
				if ((this->_file[i].find("server") != SIZE_MAX)){
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
				VirtualServer *vserv = new VirtualServer(Conf);
				this->_VServs.push_back(vserv);
				Conf.clear();
			}
		}

		/***************************************************
		******************    OTHERS   *********************
		***************************************************/
		void															fileToVectorAndClean(std::ifstream *ifs){
			 std::string  line;
			while (std::getline(*ifs, line)){
				line = cleanLine(line);
				if (!line.empty())
					this->_file.push_back(line);
			}
			(*ifs).close();
		}
		void															clearFd(void){
			FD_ZERO(&this->_readfds);
			FD_ZERO(&this->_write);
		}

	private:
		std::vector<std::string> 										_file;
		std::vector<VirtualServer*> 									_VServs;
		int																_fdmax;
		fd_set		 													_readfds;
		fd_set		 													_write;
};

#endif
