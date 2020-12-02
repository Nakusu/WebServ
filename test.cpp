#include "srcs/Header.hpp"



int main(int argc, char const *argv[])
{
	std::string toto;
	std::vector<std::string> vec;

	toto = "coucou vous ca va ?";
	split(toto, ' ', &vec);
	
	return 0;
}
