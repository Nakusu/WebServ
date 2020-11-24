
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
			std::cout << YELLOW << req->get_typecontent() << RESET << std::endl;
		    if (strcmp(req->get_uri(), "/")) {
                if (serv->check_repo(req->get_uri())) {
                    serv->get_fileInFolder(req->get_uri()); // RECUPERATION DES FICHIERS DANS LE FOLDER
                    // VOIR SI UN FICHIER DU FOLDER EST UN INDEX, SI C'EST LE CAS IL FAUT L'AFFICHER
                    // SINON VOIR SI L'AUTOINDEX EST ON ET AFFICHER UN AUTOINDEX SI C'EST LE CAS
                    req->send_packet("HTTP/1.1 403\r\nContent-Type: text/html\n\nInteraction interdite..."); // SI IL N'Y A PAS D'INDEX DE BASE ET QUE L'AUTOINDEX EST SUR OFF
                } else if ((req->get_extension() == "css" || req->get_extension() == "html") && serv->open_file(req->get_uri(), req)) {
                    std::cout << "YES OPENFILE" << std::endl;
                } else if (serv->open_Binary(req->get_uri(), req)) {
                    std::cout << "YES OPENFILE BINARY" << std::endl;
                } else {
                    req->send_packet("HTTP/1.1 404\r\nContent-Type: text/html\n\n<html><head><link rel=\"stylesheet\" href=\"style.css\"></head><h1>Page introuvable</h1></html>");
                }
            } else 
                req->send_packet("HTTP/1.1 404\r\nContent-Type: text/html\n\n<html><head><link rel=\"stylesheet\" href=\"style.css\"></head><h1>Page introuvable</h1></html>");
			delete req;
        }
    }     
    return 0;   
}   