/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newparsing.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cbertola <cbertola@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/24 11:54:00 by user42            #+#    #+#             */
/*   Updated: 2020/11/24 17:36:01 by cbertola         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iterator>
#include <sstream>

void	parsingServerText(std::vector<std::string> &file, std::vector<std::string> &serverText)
{
	for (unsigned int i = 0; i < file.size(); i++)
	{
		if (file[i].find("server ") != SIZE_MAX || file[i].find("server{") != SIZE_MAX)
		{	
			unsigned int j = i + 1;
			unsigned int brackets = 1;
			serverText.push_back(file[i]);
			while (brackets != 0 && j < file.size())
			{
				if (file[j].find("{") != SIZE_MAX)
					brackets++;
				else if (file[j].find("}") != SIZE_MAX)
					brackets--;
					serverText.push_back(file[j]);
				j++;
			}
		}
		return ;
	}
}

void	parsingListen(std::vector<std::string> &serverText, std::vector<std::string> &listen)
{
	for (unsigned int i = 0; i < serverText.size(); i++)
		if (serverText[i].find("listen ") != SIZE_MAX)
			listen.push_back(serverText[i].substr(7, serverText[i].size() - 8));
}


void	parsingServerNames(std::vector<std::string> &serverText, std::vector<std::string> &serverNames)
{
	for (unsigned int i = 0; i < serverText.size(); i++)
	{
		if (serverText[i].find("server_name ") != SIZE_MAX)
		{
			std::istringstream iss(serverText[i]);
			std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
			results.erase(results.begin());
			serverNames = results;
			serverNames.back().pop_back();
		}
	}
}

void	parsingRoot(std::vector<std::string> &serverText, std::string &root)
{
	for (unsigned int i = 0; i < serverText.size(); i++)
	{
		if (serverText[i].find("root ") != SIZE_MAX)
		{
			root = serverText[i].substr(5, serverText[i].size() - 6);
			return ;
		}
	}
}

void	parsingIndex(std::vector<std::string> &serverText, std::vector<std::string>	&index)
{
	for (unsigned int i = 0; i < serverText.size(); i++)
	{
		if (serverText[i].find("index ") == 0)
		{
			std::istringstream iss(serverText[i]);
			std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
			results.erase(results.begin());
			index = results;
			index.back().pop_back();
		}
	}
}


void	parsingAutoIndex(std::vector<std::string> &serverText, bool &autoIndex)
{
	for (unsigned int i = 0; i < serverText.size(); i++)
	{
		if (serverText[i].find("autoIndex ") != SIZE_MAX)
		{
			if (serverText[i].find("off") != SIZE_MAX)
				autoIndex = false;
			else
				autoIndex = true;
			return ;
		}
	}
}


void	parsingLocations(std::vector<std::string> &serverText, std::vector<std::map<std::string, std::string>>	&locations)
{
	std::map<std::string, std::string> value;
	for (unsigned int i = 0; i < serverText.size(); i++)
	{
		if (serverText[i].find("location") != SIZE_MAX)
		{
			unsigned int j = i + 1;
			value["key"] = serverText[i].substr(8, serverText[i].size() - 9);
			while (serverText[j].find("}") == SIZE_MAX && j < serverText.size())
			{
				std::istringstream iss(serverText[j]);
				std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
				value[results[0]] = &serverText[j][results[0].size() + 1];
				value[results[0]].pop_back();
				j++;
			}
			locations.push_back(value);
			value.clear();
		}
	}
	std::cout << "[" << locations[0]["autoindex"] << "]" << std::endl;
}


int main(int argc, char **argv)
{


	//Creation des variables
	std::vector<std::string>						serverText;
	std::vector<std::string>						listen;
	std::vector<std::string>						serverNames;
	std::string										root;
	std::vector<std::string>						index;
	bool											autoIndex;
	std::vector<std::map<std::string, std::string>>	locations;


	//Parsing et récupération de chaque variable
	parsingServerText(file, serverText);
	parsingListen(serverText, listen);
	parsingServerNames(serverText, serverNames);
	parsingRoot(serverText, root);
	parsingIndex(serverText, index);
	parsingLocations(serverText, locations);

	return (0);
}
