#ifndef PARSING_REQUEST_HPP
#define PARSING_REQUEST_HPP

class Parsing_request {
	public:
		Parsing_request() {
			this->_extension = "";
			this->_file = "";
			this->_path = "";
		}
		virtual ~Parsing_request() {}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string											get_path(void) const{
			return (this->_path);
		}
		std::string											get_file(void) const{
			return (this->_file);
		}
		std::string											get_extension(void) const{
			return (this->_extension);
		}
		std::map<std::string, std::string>					get_map(void) const{
			return (this->_map);
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
		void												parse_get(void){
			this->_path = "";

			this->_path = &this->_map["First"][this->_map["First"].find(" ") + 1];
			this->_path = (this->_path.find("?") != SIZE_MAX) ? this->_path.substr(0, this->_path.find("?") - 1)
			: this->_path.substr(0, this->_path.find("HTTP") - 1);
			this->_file = (this->_path.rfind("/") != SIZE_MAX) ? &this->_path[this->_path.rfind("/") + 1] : this->_path;
			this->_extension = (this->_file.rfind(".") != SIZE_MAX) ? &this->_file[this->_file.rfind(".") + 1] : "";
		}
		void												parsing_mime(void){
			std::ifstream			file("srcs/mime.types");
			std::string				line;

			while (getline(file, line)){
				size_t end = line.find_first_of("\t",0);
				std::string key = line.substr(0, end);
				size_t start = line.find_first_not_of("\t",end);
				std::list<std::string> value;
				while ((end = line.find_first_of(" ",start)) != SIZE_MAX){
					value.push_back(line.substr(start, end));
					start = end + 1;
				}
				this->_map_mime[key] = value;
			}
			file.close();
		}
		void												parsing_map(char *_buffer){
			std::string 			pars = _buffer;
			std::istringstream 		buff(pars);

			getline(buff, pars);
			std::string key = "First";
			std::string value = pars;
			this->_map[key] = value;

			while (getline(buff, pars)){
				if (pars.find(":") != SIZE_MAX){
					int start = 0;
					int endkey = pars.find_first_of(":",0);
					key = pars.substr(start, endkey - start);
					value = pars.substr(endkey + 1, pars.size());
					this->_map[key] = value;
				}
			}
		}

	private:
		std::string											_path;
		std::string											_file;
		std::string											_extension;
		std::map<std::string, std::string>					_map;
		std::map<std::string, std::list<std::string> >		_map_mime;
};

#endif