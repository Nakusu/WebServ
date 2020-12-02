#include "Header.hpp"

std::vector<std::string>				split(std::string chaine, char delimiteur)
{
	std::vector<std::string> elements;
	std::stringstream ss(chaine);
	std::string sousChaine;
	while (getline(ss, sousChaine, delimiteur)){
		elements.push_back(sousChaine);
	}
	return (elements);
}

int					searchInVec(std::string find, std::vector<std::string> entry){
	for (size_t i = 0; i < entry.size(); i++){
		if (!find.compare(entry[i]))
			return (i);
	}
	return (-1);
}

std::string							fileToString(std::string file)
{
	std::string fileToString;
	std::string line;
std::ifstream	ifs(file.c_str());

	if (ifs.fail()){
		std::cerr << "Reading Error" << std::endl;
		return (fileToString);
	}
	while (std::getline(ifs, line)){
		fileToString.append(line);
	}
	(ifs).close();
	return (fileToString);
}

std::string							convertInSpaces(std::string line)
{
	std::string symbols = "\t\n\v\r\f";
	for (unsigned int i = 0; i < line.size(); i++)
	{
		for (unsigned int j = 0; j < symbols.size(); j++)
		{
			if (line[i] == symbols[j])
				line[i] = ' ';
		}
	}
	return (line);
}

std::string							cleanSpaces(std::string	&line)
{
	std::string lineCleaned = convertInSpaces(line);

	std::istringstream iss(lineCleaned);
	std::vector<std::string> res(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
	lineCleaned.clear();
	for (unsigned int i = 0; i < res.size(); i++){
		lineCleaned.append(res[i]);
		if (i < res.size() - 1)
			lineCleaned.append(" ");
	}
	return (lineCleaned);
}