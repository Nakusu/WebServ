

#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <stdint.h>
#include <map>


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

#define TRUE   1  
#define FALSE  0  
#define PORT 8080  

#endif