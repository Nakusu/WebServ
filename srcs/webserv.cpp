
#include "Header.hpp"
#include "Client.hpp"
#include "ServerWeb.hpp" 
#include "VirtualServer.hpp" 
#include "Request.hpp" 
#include "Execution.hpp" 

ServerWeb *serv = new ServerWeb;

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
	if (!folderIsOpenable("./tmp")) {
		std::cerr << "Folder Error : no ./tmp folder in this repository" << std::endl;
		return (0);
	}
	return (1);
}

void		Exec(ServerWeb *serv, Client *client, int i, char **env){
	Request *req = client->get_req();
	client->setHistory(NumberToString(client->get_fd()), client->get_req()->get_uri());
	Execution exec = Execution(serv, serv->getVS(i), req, env);
	std::string Method = req->get_method();
	std::cout << "HOSTS COMP " << req->get_host() << " TO " << serv->getVS(i)->get_serverNames() << std::endl;
	if (serv->getVS(i)->get_serverNames().find(req->get_host()) == SIZE_MAX) {
		req->basicHeaderFormat();
		req->updateContent("HTTP/1.1", "400 Bad Request Error");
		req->updateContent("Content-Length", "0");
		req->sendHeader();
		std::cout << "400 Bad Request Error" << std::endl;
		return ;
	}
	if (!exec.needRedirection() && !exec.doTrace() && !exec.doOptions() && !exec.doAuthenticate() && !exec.checkMethod() && !exec.doPost() && !exec.doDelete() && !exec.doPut() && !exec.searchIndex() && !exec.initCGI() && !exec.binaryFile())
		exec.searchError404();
	if (!client->CGIIsRunning()){
		client->new_req();
	}
}

void		closeServ(int code){
	(void)code;

	serv->clearFd();
	std::cout << std::endl << RED << "Wait end of process still runnings..." << RESET << std::endl;
	while (serv->checkEndCGI() != 0){}
	std::cout << RED << "Closing serveurs..." << RESET << std::endl;
	int i;
	int j;
	while ((i = serv->getVSsize() - 1) != -1){
		while ((j = serv->getVS(i)->get_clients().size() - 1) != -1){
			delete serv->getVS(i)->get_client(j);
			serv->getVS(i)->delLastClient();
		}
		delete (serv->getVS(i));
		serv->delLastVS();
	}
	delete (serv);
	exit(0);
}
int			main(int argc, char **argv, char **env)
{   
	std::string message; 
	std::string defaultConf;
	int 		fdClient;
	Client 		*client;


	if (!checkArgs(argc, argv, &defaultConf, serv))
		return (-1);
	serv->createVServs();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, &closeServ);
	
	std::cout << GREEN << "Server is Running..." << RESET << std::endl;
	while(TRUE)
	{
		serv->clearFd();
		serv->setAllFDSET_fdmax();
		serv->waitForSelect();
		for (size_t i = 0; i < serv->getVSsize() && serv->get_nbActivity(); i++){
			//Check les sockets master
			if (FD_ISSET(serv->getVS(i)->get_fd(), serv->get_readfds()) && serv->get_nbActivity()){
				int addrlen = sizeof(serv->getVS(i)->get_address());
				struct sockaddr_in * AddrVS = serv->getVS(i)->get_address();
				fdClient = accept(serv->getVS(i)->get_fd(), (struct sockaddr *)AddrVS, (socklen_t *)&addrlen);

				client = new Client(fdClient, serv->get_mimesTypes());
				serv->getVS(i)->setClient(client);
				int ret = client->get_req()->init();
				if (ret > 0)
					Exec(serv, client, i, env);
				else if (ret == -1 || client->get_req()->get_error() == -1){
					serv->getVS(i)->delClient(client);
					delete client;
				}
				serv->dec_nbActivity();
				serv->checkEndCGI();
			}
			for (size_t j = 0; j < serv->getVS(i)->get_clients().size() && serv->get_nbActivity(); j++){
				client = serv->getVS(i)->get_client(j);
				if (serv->verifFdFDISSET(client->get_fd())){
					int ret = client->get_req()->init();
					if (ret > 0)
						Exec(serv, client, i, env);
					else if (ret == -1){
						serv->getVS(i)->delClient(client);
						delete client;
					}
					serv->dec_nbActivity();
					serv->checkEndCGI();
				}
			}
		}
		serv->checkEndCGI();
	}
	return 0;
}