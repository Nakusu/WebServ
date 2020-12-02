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


