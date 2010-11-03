#ifndef CPPDB_UTIL_H
#define CPPDB_UTIL_H
#include <string>
#include <ctime>

namespace cppdb {
	std::tm parse_time(char const *value);
	std::string format_time(std::tm const &v);
	inline std::tm parse_time(std::string const &v)
	{
		return parse_time(v.c_str());
	}
}
#endif
