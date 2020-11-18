
#include "Header.hpp" 
#include "Server.hpp" 
#include "Request.hpp" 

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
        serv->set_repos("public");

        //Le server attends un nouvelle activité (une requete)
      	serv->wait_select();  
        //Si une requete est envoyé au serv->get_fd()
        if (serv->wait_request()){   
			Request *req = new Request(accept(serv->get_fd(), (struct sockaddr *)&address, (socklen_t *)&addrlen));
			//std::cout << " URI : " << req->get_uri() << " TYPE : " << req->get_type() << std::endl;
			//printf("New connection , socket fd is %d , ip is : %s , port : %d\n\n %s\n" , serv->get_fd() , inet_ntoa(address.sin_addr) , ntohs(address.sin_port), req->get_buffer());
            if (strcmp(req->get_uri(), "/")) {
                std::string fcontent = serv->open_file(req->get_uri());
                if (fcontent.empty())
                    message = "HTTP/1.1 404\r\nContent-Type: text/html\n\n<html><head><link rel=\"stylesheet\" href=\"style.css\"></head><h1>Page introuvable</h1></html>";
                else {
                    message = "HTTP/1.1 200\r\nContent-Type: " + req->get_typecontent() + "\n\n";
                    message += fcontent;
                }
            }
			req->send_packet(message.c_str());

			delete req;
        }
    }     
    return 0;   
}   