/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: user42 <user42@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/23 11:13:51 by user42            #+#    #+#             */
/*   Updated: 2020/11/23 18:19:22 by user42           ###   ########.fr       */
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

typedef std::map<std::string, std::vector<std::string>>  containerMap;
typedef std::vector<std::string>						 containerVector;

void	parseLocations(containerMap datas, std::vector<std::string> file)
{
	unsigned int cpt = 0;
	std::string location = "location";
	for (unsigned int i = 0; i < file.size(); i++)
	{
		if (file[i].find("location") != SIZE_MAX)
		{	
			unsigned int j = i + 1;
			unsigned int spaces = 0;
			while (file[i][spaces] == '\t')
				spaces++;
			spaces += strlen("location ");
			datas[location + std::to_string(cpt)].push_back(file[i].substr(spaces, file[i].rfind("{") - (spaces + 1)));
			while (file[j].find("}") == SIZE_MAX && j < file.size())
			{
				spaces = 0;
				while (file[j][spaces] == '\t')
					spaces++;
				datas[location + std::to_string(cpt)].push_back(file[j].substr(spaces, file[j].size() - (spaces + 1)));
				j++;
			}
			cpt++;
		}
	}

	//Affichage de toutes les locations
	std::map<std::string, std::vector<std::string>>::iterator it;
	for (it = datas.find("location0"); it != datas.end(); it++)
	{
		for (int i = 0; i < it->second.size(); i++)
			std::cout << "Index = " << i << " -> Value = [" <<  it->second[i] << "]" << std::endl;
		std::cout << "--------------------------------------" << std::endl;
	}
}

void	parseServerNames(containerMap datas, std::vector<std::string> file)
{
	for (unsigned int i = 0; i < file.size(); i++)
	{
		if (file[i].find("server_name") != SIZE_MAX)
		{
			unsigned int spaces = 0;
			while (file[i][spaces] == '\t')
				spaces++;
			spaces += strlen("server_name ");
			std::istringstream iss(&file[i][spaces]);
			std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
			datas["server_name"] = results;
			datas["server_name"].back().pop_back();
		}
	}
	//Affichage de tous les server_name
	for (unsigned int i = 0; i < datas["server_name"].size(); i++)
		std::cout << "server name -> [" << datas["server_name"][i] << "]" << std::endl;
}

int main(int argc, char **argv)
{
	std::string defaultConf = "default.conf";
	
	// Verification des arguments
	if (argc > 1)
		defaultConf = std::string(argv[1]) + "/" + defaultConf;
	std::ifstream	ifs(defaultConf.c_str());
	if (ifs.fail())
	{
		std::cerr << "Reading Error" << std::endl;
		return (0);
	}

	// Recupération des donnés dans un container vector
	std::string  line;
	std::vector<std::string> file;
	while (std::getline(ifs, line))
		file.push_back(line);
	ifs.close();

	//Creation du container map
	std::map<std::string, std::vector<std::string>> datas;
	parseLocations(datas, file);
	parseServerNames(datas, file);
	
	
	// Affichage des donnés
	/*for (int i = 0; i < file.size(); i++)
		std::cout << file[i] << std::endl;*/


	return (0);
}