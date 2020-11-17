# define RESET   		"\033[0m"
# define BLACK   		"\033[30m"				/* Black */
# define RED     		"\033[31m"				/* Red */
# define GREEN   		"\033[32m"				/* Green */
# define YELLOW  		"\033[33m"				/* Yellow */
# define BLUE    		"\033[34m"				/* Blue */
# define MAGENTA 		"\033[35m"				/* Magenta */
# define CYAN    		"\033[36m"				/* Cyan */
# define WHITE   		"\033[37m"				/* White */
# define BOLDBLACK   	"\033[1m\033[30m"      /* Bold Black */
# define BOLDRED     	"\033[1m\033[31m"      /* Bold Red */
# define BOLDGREEN   	"\033[1m\033[32m"      /* Bold Green */
# define BOLDYELLOW  	"\033[1m\033[33m"      /* Bold Yellow */
# define BOLDBLUE    	"\033[1m\033[34m"      /* Bold Blue */
# define BOLDMAGENTA 	"\033[1m\033[35m"      /* Bold Magenta */
# define BOLDCYAN    	"\033[1m\033[36m"      /* Bold Cyan */
# define BOLDWHITE   	"\033[1m\033[37m"      /* Bold White */




//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include "Header.hpp" 
#include "Server.hpp" 

int main(void)   
{   
    int  addrlen, activity, fd;   
    int max_sd; 
	int opt = TRUE;
	Server *serv = new Server;
    //set of socket descriptors  
    fd_set readfds;
	struct sockaddr_in address; 
           
    //a message  
    std::string message; 
	message = "HTTP/1.1 200\r\nContent-Type: text/html\n\n<html><head><link rel=\"stylesheet\" href=\"style.css\"></head><h1>Hello from server</h1></html>";   
         
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(serv->get_fd(), SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }
     

         
    //accept the incoming connection  
    addrlen = sizeof(serv->get_address());   
    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(serv->get_fd(), &readfds);   
        max_sd = serv->get_fd();   
     
        //Le server attends un nouvelle activité (une requete)
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
       
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");   
        }   
             
        //Si une requete est envoyé au serv->get_fd()
        if (FD_ISSET(serv->get_fd(), &readfds))   
        {   
			//Creation d'une new socket (enfant)
			address = serv->get_address();
            if ((fd = accept(serv->get_fd(), (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands
			printf(GREEN);
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , fd , inet_ntoa(serv->get_address().sin_addr) , ntohs (serv->get_address().sin_port));   
			printf(RESET);
           
            //send new connection greeting message  
            send(fd, message.c_str(), message.size(), 0) ;

                 
            puts("Ici notre parsing et nos CGI\n");   
			
			printf(RED);
			printf("DISCONNECTED\n");
            printf(RESET);    
			close( fd );   
        }
    }     
    return 0;   
}   