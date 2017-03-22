#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <string.h>

inline std::string string_basename(std::string path)
{
	const char* base_name = basename(path.c_str());
	return std::string(base_name);
}

#endif
