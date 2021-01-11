
#include "Header.hpp"
#include "Client.hpp"
#include "ServerWeb.hpp" 
#include "VirtualServer.hpp" 
#include "Request.hpp" 
#include "Execution.hpp" 

int			checkArgs(int argc, char **argv, std::string *defaultConf, ServerWeb *serv){
	*defaultConf = "srcs/default.conf";

	if (argc > 1)
		*defaultConf = std::string(argv[1]);
	std::ifstream	ifs((*defaultConf).c_str());
	if (ifs.fail()){
		std::cerr << "Reading Error 1" << std::endl;
		return (0);
	}
	serv->fileToVectorAndClean(&ifs);
	ifs.close();
	return (1);
}

void		Exec(ServerWeb *serv, Client *client, int i, char **env){
	Request *req = client->get_req();
	Execution exec = Execution(serv, serv->getVS(i), req, env);
	std::string Method = req->get_method();
	if (!exec.needRedirection() && !exec.checkMethod() && !exec.doPost() && !exec.doDelete() && !exec.doPut() && !exec.searchIndex() && !exec.initCGI() && !exec.binaryFile())
		exec.searchError404();
	if (!client->CGIIsRunning()){
		client->new_req();
	}
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
		nb_activity = serv->waitForSelect();
		for (size_t i = 0; i < serv->getVSsize() && nb_activity; i++){
			//Check les sockets master
			if (FD_ISSET(serv->getVS(i)->get_fd(), serv->get_readfds()) && nb_activity){
				int addrlen = sizeof(serv->getVS(i)->get_address());
				struct sockaddr_in * AddrVS = serv->getVS(i)->get_address();
				fdClient = accept(serv->getVS(i)->get_fd(), (struct sockaddr *)AddrVS, (socklen_t *)&addrlen);

				client = new Client(fdClient, serv->get_mimesTypes());
				serv->getVS(i)->setClient(client);
				int ret = client->get_req()->init();
				if (ret > 0)
					Exec(serv, client, i, env);
				else if (ret == -1){
					serv->getVS(i)->delClient(client);
					delete client;
				}
				nb_activity--;
				serv->checkEndCGI();
			}
			for (size_t j = 0; j < serv->getVS(i)->get_clients().size() && nb_activity; j++){
				client = serv->getVS(i)->get_client(j);
				if (serv->verifFdFDISSET(client->get_fd())){
					int ret = client->get_req()->init();
					if (ret > 0)
						Exec(serv, client, i, env);
					else if (ret == -1){
						serv->getVS(i)->delClient(client);
						delete client;
					}
					nb_activity--;
					serv->checkEndCGI();
				}
			}
		}
		serv->checkEndCGI();
	}
	return 0;
}