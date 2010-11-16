#ifndef CPPDB_UTIL_H
#define CPPDB_UTIL_H
#include <string>
#include <ctime>
#include <map>

namespace cppdb {
	std::tm parse_time(char const *value);
	std::string format_time(std::tm const &v);
	inline std::tm parse_time(std::string const &v)
	{
		return parse_time(v.c_str());
	}
	void parse_connection_string(	std::string const &cs,
					std::string &driver_name,
					std::map<std::string,std::string> &props);
}
#endif
