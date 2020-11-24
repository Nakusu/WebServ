
#include "Header.hpp" 
#include "Server.hpp" 
#include "Request.hpp" 
#include "Execution.hpp" 

int main(void)   
{   
	Server *serv = new Server;
           
    //a message  
    std::string message; 
         
    //accept the incoming connection  
    int addrlen = sizeof(serv->get_address());
	struct sockaddr_in address = serv->get_address();

    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
		serv->clear_fd();
		serv->set_fd();
        // serv->set_repos("public");

        //Le server attends un nouvelle activité (une requete)
      	serv->wait_select();  
        //Si une requete est envoyé au serv->get_fd()
        if (serv->wait_request()){   
			Request *req = new Request(accept(serv->get_fd(), (struct sockaddr *)&address, (socklen_t *)&addrlen));
			std::cout << YELLOW << req->get_typecontent() << RESET << std::endl;
			Execution exec = Execution(serv, req);
			if (!exec.index() && !exec.text(req->get_uri()) && !exec.binary_file(req->get_uri()))
				exec.redir_404();
			delete req;
        }
    }     
    return 0;   
}   