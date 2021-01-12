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
			this->_nbActivity = 0;
		}
		ServerWeb(ServerWeb const &rhs){
			operator=(rhs);
		}
		virtual ~ServerWeb(void){}
		ServerWeb &														operator=(ServerWeb const &rhs){
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
		int																get_nbActivity(void){
			return (this->_nbActivity);
		}
		fd_set *														get_readfds(void){
			return (&this->_readfds);
		}
		fd_set *														get_writefds(void){
			return (&this->_writefds);
		}
		std::map<std::string, std::string>								get_mimesTypes(void){
			return (this->_mimesTypes);
		}
		void															getContentType(void){
			std::string line;
			std::ifstream	ifs("srcs/mime.types");
			std::vector<std::string> res;
			if (ifs.fail()){
				std::cerr << "Reading Error" << std::endl;
				return;
			}
			while (std::getline(ifs, line)){
				line = cleanLine(line);
				res = split(line, " ");
				this->_mimesTypes[res[1]] = res[0];
			}
			(ifs).close();
		}
		std::map<std::string, std::string>								get_MimesTypes(void){
			return (this->_mimesTypes);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void															setAllFDSET_fdmax(void){
			
			FD_SET(STDIN_FILENO, &this->_readfds);
			for (size_t i = 0; i < this->_VServs.size(); i++){
				FD_SET(this->_VServs[i]->get_fd(), &this->_readfds);
				FD_SET(this->_VServs[i]->get_fd() , &this->_writefds);
				if (this->_fdmax < this->_VServs[i]->get_fd())
					this->_fdmax = this->_VServs[i]->get_fd();
				for (size_t j = 0; j < this->_VServs[i]->get_clients().size(); j++){
					if (!this->_VServs[i]->get_client(j)->CGIIsRunning()){
						FD_SET(this->_VServs[i]->get_client(j)->get_fd(), &this->_readfds);
						FD_SET(this->_VServs[i]->get_client(j)->get_fd(), &this->_writefds);
						if (this->_fdmax < this->_VServs[i]->get_client(j)->get_fd())
							this->_fdmax = this->_VServs[i]->get_client(j)->get_fd();
					}
				}
			}
		}

		/***************************************************
		*******************    WAIT   **********************
		***************************************************/
		void															waitForSelect(void){
			struct timeval			timeout;

			timeout.tv_sec = 0;
			timeout.tv_usec = 5000;
			// this->_writefds = this->_readfds;
			this->_nbActivity = 1;
			while ((this->_nbActivity = select(this->_fdmax + 1, &this->_readfds , &this->_writefds , NULL , &timeout)) == -1){
			}
			if ((this->_nbActivity < 0) && (errno != EINTR))  
				printf("select error");
		}
		int																verifFdFDISSET(int fd){
			// << YELLOW << "on test fd : " << fd << " result wr = " << FD_ISSET(fd, &this->_writefds) << FD_ISSET(fd, &this->_readfds) << std::endl;
			return (FD_ISSET(fd, &this->_readfds) && FD_ISSET(fd, &this->_writefds));
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
		void															createVServs(void){
			//Find configuration and create VSERV
			std::vector<std::string>	Conf;

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
			FD_ZERO(&this->_writefds);
			this->_fdmax = 0;
		}
		int																checkEndCGI(void){
				int stillRunning = 0;
			for (size_t i = 0; i < this->_VServs.size(); i++){
				for (size_t j = 0; j < this->_VServs[i]->get_clients().size(); j++){
					if (this->_VServs[i]->get_client(j)->CGIIsRunning()){
						stillRunning++;
						if (waitpid(this->_VServs[i]->get_client(j)->get_req()->get_PID(), this->_VServs[i]->get_client(j)->get_req()->get_Status(), WNOHANG) == this->_VServs[i]->get_client(j)->get_req()->get_PID()){
							stillRunning--;
							this->_VServs[i]->get_client(j)->get_req()->sendForCGI();
							this->_VServs[i]->get_client(j)->get_req()->setCGI(0);
							this->_VServs[i]->get_client(j)->new_req();
						}
					}
				}
			}
			return (stillRunning);
		}
		void															dec_nbActivity(void){
			this->_nbActivity--;
		}
		void															delLastVS(void){
			this->_VServs.pop_back();
		}


	private:
		std::vector<std::string> 										_file;
		std::vector<VirtualServer*> 									_VServs;
		int																_fdmax;
		int																_nbActivity;
		fd_set		 													_readfds;
		fd_set		 													_writefds;
		std::map<std::string, std::string>								_mimesTypes;
};

#endif
