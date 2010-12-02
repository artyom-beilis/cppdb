///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPDB_UTIL_H
#define CPPDB_UTIL_H

#include <cppdb/defs.h>
#include <string>
#include <ctime>
#include <map>


namespace cppdb {

	CPPDB_API std::tm parse_time(char const *value);
	CPPDB_API std::string format_time(std::tm const &v);
	CPPDB_API std::tm parse_time(std::string const &v);

	CPPDB_API void parse_connection_string(	std::string const &cs,
						std::string &driver_name,
						std::map<std::string,std::string> &props);

	class CPPDB_API connection_info {
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
