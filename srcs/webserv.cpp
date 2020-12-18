
#include "Header.hpp"
#include "Client.hpp"
#include "ServerWeb.hpp" 
#include "VirtualServer.hpp" 
#include "Request.hpp" 
#include "HeaderRequest.hpp" 
#include "Execution.hpp" 

int			checkArgs(int argc, char **argv, std::string *defaultConf, ServerWeb *serv){
	*defaultConf = "srcs/default.conf";

	if (argc > 1)
		*defaultConf = std::string(argv[1]);
	std::ifstream	ifs((*defaultConf).c_str());
	if (ifs.fail()){
		std::cerr << "Reading Error" << std::endl;
		return (0);
	}
	serv->fileToVectorAndClean(&ifs);
	return (1);
}

void		Exec(ServerWeb *serv, Request *req, int i, char **env){
	HeaderRequest *header = new HeaderRequest();
	Execution exec = Execution(serv, serv->getVS(i), req, header, env);
	if (!exec.checkMethod())
		exec.searchError405();
	if (!exec.needRedirection() && exec.checkMethod()){
		if (!exec.searchIndex() && !exec.initCGI() && !exec.binaryFile())
			exec.searchError404();	
	}
	serv->getVS(i)->setHistory((req->get_IpClient() + req->get_userAgent()), req->get_url());
	delete header;
	
}

int			main(int argc, char **argv, char **env)
{   
	ServerWeb *serv = new ServerWeb;
	std::string message; 
	std::string defaultConf;
	int			nb_activity;
	int 		fdClient;
	Client 		*client;


	defaultConf = checkArgs(argc, argv, &defaultConf, serv);
	serv->createVServs();
	
	puts("Waiting for connections ...");
	while(TRUE)
	{
		serv->clearFd();
		serv->setAllFDSET_fdmax();
		//Le server attends un nouvelle activité (une requete)
		nb_activity = serv->waitForSelect();
		//Si une requete est envoyé au serv->get_fd()
		// std::cout << "test de passage" << std::endl;
		for (size_t i = 0; i < serv->getVSsize() && nb_activity; i++){
		
			//Si le virtualServer a recu une requete :
			// if(FD_ISSET(STDIN_FILENO, serv->get_readfds())){
			// 		char * _buffer = (char *)calloc(sizeof(char), 30);
			// 		_buffer[read(STDIN_FILENO, _buffer, 30)] = 0;

			// 		std::cout << _buffer << std::endl;
			// 		std::cout << strcmp(_buffer, "quit")  << std::endl;
			// 		if (strcmp(_buffer, "quit") == 0)
			// 			return (1);
			// }

			//Check les sockets master
			if (FD_ISSET(serv->getVS(i)->get_fd(), serv->get_readfds()) && nb_activity){
				std::cout << GREEN << "SERVER" << RESET << std::endl;
				int addrlen = sizeof(serv->getVS(i)->get_address());
				struct sockaddr_in * IPClient = serv->getVS(i)->get_address();
				fdClient = accept(serv->getVS(i)->get_fd(), (struct sockaddr *)IPClient, (socklen_t *)&addrlen);
				std::cout << YELLOW << "Creation du fd = " << fdClient << RESET << std::endl;
				client = new Client(fdClient, IPClient);
				serv->getVS(i)->setClient(client);
				if (client->get_req()->init()){
					Exec(serv, client->get_req(), i, env);
					client->new_req();
				}
				nb_activity--;
			}

			//check les clients
			for (size_t j = 0; j < serv->getVS(i)->get_clients().size() && nb_activity; j++){
				client = serv->getVS(i)->get_client(j);
				if (serv->verifFdFDISSET(client->get_fd())){
					std::cout << GREEN << "CLIENT" << RESET << std::endl;
					std::cout << BLUE << "Lecture du fd = " << fdClient << RESET << std::endl;
					int ret;
					ret = client->get_req()->init();
					if (ret){
						Exec(serv, client->get_req(), i, env);
						client->new_req();
					}
					else if (ret == -1){
						serv->getVS(i)->delClient(client);
						delete client;
					}
					nb_activity--;
				}
			}

		}
	}
	return 0;
}