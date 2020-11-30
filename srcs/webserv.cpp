
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
         
    int addrlen = sizeof(serv->get_address());
	struct sockaddr_in address = serv->get_address();

    puts("Waiting for connections ...");   
    
	defaultConf = checkArgs(argc, argv, &defaultConf, serv);
	serv->parsingVrServ();
	serv->CreateVServs();

    while(TRUE)   
    {   
		serv->clear_fd();
		serv->set_fd();
        //Le server attends un nouvelle activité (une requete)
      	serv->wait_select();  
        //Si une requete est envoyé au serv->get_fd()
        if (serv->wait_request()){   
			Request *req = new Request(accept(serv->get_fd(), (struct sockaddr *)&address, (socklen_t *)&addrlen));
			HeaderRequest *header = new HeaderRequest();
			std::cout << YELLOW << req->get_typecontent() << RESET << std::endl;
			Execution exec = Execution(serv->getVS(0), req, header);
			
			if (exec.redirectToFolder()){
				if (!exec.index() && !exec.text(req->get_uri()) && !exec.binary_file(req->get_uri()))
					exec.redir_404(req->get_uri());
			}
			delete req;
			delete header;
        }
    }     
    return 0;   
}   