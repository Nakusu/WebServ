
#include "Header.hpp"
#include "ServerWeb.hpp" 
#include "VirtualServer.hpp" 
#include "Request.hpp" 
#include "HeaderRequest.hpp" 
#include "Execution.hpp" 

int			checkArgs(int argc, char **argv, std::string *defaultConf, ServerWeb *serv){
	*defaultConf = "srcs/default.conf";

	if (argc > 1)
		*defaultConf = std::string(argv[1]) + "/" + *defaultConf;
	std::ifstream	ifs((*defaultConf).c_str());
	if (ifs.fail()){
		std::cerr << "Reading Error" << std::endl;
		return (0);
	}
	serv->OpenDefault(&ifs);
	return (1);
}

int main(int argc, char **argv)   
{   
	ServerWeb *serv = new ServerWeb;
    std::string message; 
	std::string defaultConf;
	int			nb_activity;
         

    puts("Waiting for connections ...");   
    
	defaultConf = checkArgs(argc, argv, &defaultConf, serv);
	serv->parsingVrServ();
	serv->CreateVServs();
	
    while(TRUE)   
    {   
		serv->clear_fd();
		serv->set_fd();
        //Le server attends un nouvelle activité (une requete)
      	nb_activity = serv->wait_select();  
        //Si une requete est envoyé au serv->get_fd()
		for (size_t i = 0; i < serv->_VServs.size() && nb_activity; i++)
		{
			if (serv->wait_request(serv->_VServs[i]->get_fd())){
				int addrlen = sizeof(serv->_VServs[i]->get_address()); //virtual server
				Request *req = new Request(accept(serv->_VServs[i]->get_fd(), (struct sockaddr *)serv->_VServs[i]->get_address(), (socklen_t *)&addrlen));
				HeaderRequest *header = new HeaderRequest();
				Execution exec = Execution(serv->getVS(0), req, header);
				
				if (exec.redirectToFolder()){
					if (!exec.index() && !exec.text(req->get_uri()) && !exec.binary_file(req->get_uri()))
						exec.redir_404(req->get_uri());
				}
				delete req;
				delete header;
				nb_activity--;
			}
		}
    }     
    return 0;   
}   