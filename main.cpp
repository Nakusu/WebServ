#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <list>
#include <iterator>
#include <utility>
#include <algorithm>

char										**swapMaptoChar(std::map<std::string, std::string> args){
	char	**tmpargs = (char**)malloc(sizeof(char*) * (args.size() + 1));
	size_t	i = 0;
	tmpargs[args.size()] = 0;

	for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
		tmpargs[i++] = strdup((it->first + "=" + it->second).c_str());
	tmpargs[i] = NULL;
	return (tmpargs);
}

std::map<std::string, std::string>			setMetaCGI(std::string script_name) {			
	std::map<std::string, std::string> args;
	/*args = this->req->get_Parsing().getMap();
	for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
	{
		std::cout <<  it->first << std::endl;
	}*/

	(void)script_name;
	args["AUTH_TYPE"] = "";
	args["SERVER_SOFTWARE"] = "webserv";
	args["SERVER_PROTOCOL"] = "HTTP/1.1";
	args["CONTENT_TYPE"] = "";
	args["CONTENT_LENGTH"] = "0";
			//if (req->getQueryString() != "")
			//	args["QUERY_STRING"] = req->getQueryString();
			//else
			//	args["QUERY_STRING"];
	args["SERVER_NAME"] = "localhost";
	args["SERVER_PORT"] = "8080";
	args["REQUEST_URI"] = "/php/test.php";
	args["SCRIPT_NAME"] = "/php/test.php";
			//args["REMOTE_ADDR"] = this->req->get_IpClient();
	args["REQUEST_METHOD"] = "GET";
	args["GATEWAY_INTERFACE"] = "CGI/1.1";
	args["REMOTE_USER"] = "";
	args["REMOTE_IDENT"] = "";
	args["PATH_INFO"] = "/php/test.php";
	args["PATH_TRANSLATED"] = "/home/user42/Bureau/webserv/public/php/test.php";

	for (std::map<std::string, std::string>::iterator it = args.begin(); it != args.end(); it++)
		std::cout << "KEY [" << it->first << "] VALUE [" << it->second << "]" << std::endl;
	return (args);
}

int			main() {
	std::map<std::string, std::string> args = setMetaCGI("test");
	std::cout << "METAS WAS ALL SET" << std::endl;
	char **tmpargs = swapMaptoChar(args);

	char **tmp = (char**)malloc(sizeof(char*) * 3);
	tmp[0] = strdup("./public/php/cgi/ubuntu_cgi_tester");
	tmp[1] = strdup(std::string("./public/php/test.php").c_str());
	tmp[2] = NULL;
	std::cout << "JUST BEFORE JOB" << std::endl;
	std::cout<< "RESULT OF EXECVE : " << execve("./public/php/cgi/ubuntu_cgi_tester", tmp, tmpargs) << std::endl;

}