
#include "Header.hpp" 
#include "Server.hpp" 
#include "Request.hpp" 
#include "Execution.hpp" 

int main(int argc, char **argv)   
{   
	Server *serv = new Server;
           
    //a message  
    std::string message; 
         
    //accept the incoming connection  
    int addrlen = sizeof(serv->get_address());
	struct sockaddr_in address = serv->get_address();

    puts("Waiting for connections ...");   
    
	std::string defaultConf = "srcs/default.conf";
	
	// Verification des arguments d'entrée && lecture du fichier
	if (argc > 1)
		defaultConf = std::string(argv[1]) + "/" + defaultConf;
	std::ifstream	ifs(defaultConf.c_str());
	if (ifs.fail())
	{
		std::cerr << "Reading Error" << std::endl;
		return (0);
	}

	// Recupération du fichier default.conf dans un container vector
	std::string  line;
	std::vector<std::string> file;
	while (std::getline(ifs, line))
	{
		line = (line.find_first_not_of("\t ") != SIZE_MAX) ? line.substr(line.find_first_not_of("\t "), line.size()) : line;
		if (!line.empty())
			file.push_back(line);
	}
	ifs.close();
	serv->set_file(file);
	serv->parsingServerText();
	serv->parsingListen();
	serv->parsingServerNames();
	serv->parsingRoot();
	serv->parsingIndex();
	serv->parsingLocations();
	serv->parsingAutoIndex();


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
			if (exec.redirectToFolder()){
				if (!exec.index() && !exec.text(req->get_uri()) && !exec.binary_file(req->get_uri()))
					exec.redir_404();
			}
			delete req;
        }
    }     
    return 0;   
}   