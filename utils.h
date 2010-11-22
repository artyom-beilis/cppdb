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

	class connection_info {
	public:
		std::string connection_string;
		std::string driver;
		typedef std::map<std::string,std::string> properties_type;
		properties_type properties;
		
		std::string get(std::string const &prop,std::string const &default_value=std::string()) const;
		int get(std::string const &prop,int default_value) const;
		
		connection_info()
		{
		}
		explicit connection_info(std::string const &cs) :
			connection_string(cs)
		{
			parse_connection_string(cs,driver,properties);
		}

	};
}
#endif
