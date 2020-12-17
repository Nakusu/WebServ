
#include "Header.hpp"
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
	serv->getVS(i)->setHistory(req->getfd(), req->get_url());
	delete req;
	delete header;
	
}

int			main(int argc, char **argv, char **env)
{   
	ServerWeb *serv = new ServerWeb;
	std::string message; 
	std::string defaultConf;
	int			nb_activity;
	int 		fdClient; 


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
		for (size_t i = 0; i < serv->getVSsize() && nb_activity; i++){
			//Si le virtualServer a recu une requete :
				std::cout << RED << "NUMERO " << i << RESET << std::endl;
				std::cout << RED << "fd " << serv->getVS(i)->get_fd() << RESET << std::endl;
			if (serv->verifFdFDISSET(serv->getVS(i)->get_fd())){
				std::cout << RED << "ici" << RESET << std::endl;
				int addrlen = sizeof(serv->getVS(i)->get_address());
				struct sockaddr_in * IPClient = serv->getVS(i)->get_address();
				fdClient = accept(serv->getVS(i)->get_fd(), (struct sockaddr *)IPClient, (socklen_t *)&addrlen);
				serv->getVS(i)->set_fdClients(fdClient);
				Request *req = new Request(fdClient);
				if (req->init()){
					req->setIPClient(inet_ntoa(IPClient->sin_addr));
					Exec(serv, req, i, env);
				}
				else{
					std::cout << "ON CLOSE LE " << fdClient << " dans la part 1" << std::endl;
					delete req;
					close(fdClient);
					serv->getVS(i)->del_fdClients(fdClient);
				}
				nb_activity--;
			}
			//Si l'un des clients du virtualServer a recu une requete :
			for (size_t j = 0; j < serv->getVS(i)->get_fdClients().size() && nb_activity; j++){
				std::cout << GREEN << "LA" << RESET << std::endl;
				fdClient = serv->getVS(i)->get_fdClients(j);
				if (serv->verifFdFDISSET(fdClient)){
					Request *req = new Request(serv->verifFdFDISSET(fdClient));
					if (req->init())
						Exec(serv, req, i, env);
					else{
						std::cout << "ON CLOSE LE " << fdClient << std::endl;
						delete req;
						close(fdClient);
						serv->getVS(i)->del_fdClients(fdClient);
					}
					nb_activity--;
				}
			}
		}
	}
	return 0;
}