#ifndef BASE64_HPP
#define BASE64_HPP

#include "Header.hpp"

class base64
{
	public:
		base64() {}
		virtual ~base64() {}
		static std::string decode(const std::string &in)
		{

			std::string out;

			std::vector<int> T(256, -1);
			for (int i = 0; i < 64; i++)
				T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

			int val = 0, valb = -8;
			for (size_t i = 0; in[i]; i++) {
				if (T[in[i]] == -1)
					break;
				val = (val << 6) + T[in[i]];
				valb += 6;
				if (valb >= 0)
				{
					out.push_back(char((val >> valb) & 0xFF));
					valb -= 8;
				}
			}
			return out;
		}
};

#endif
