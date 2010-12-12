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

	///
	/// \brief parse a string as time value.
	/// 
	/// Used by backend implementations;
	///
	CPPDB_API std::tm parse_time(char const *value);
	///
	/// \brief format a string as time value.
	/// 
	/// Used by backend implementations;
	///
	CPPDB_API std::string format_time(std::tm const &v);
	///
	/// \brief parse a string as time value.
	/// 
	/// Used by backend implementations;
	///
	CPPDB_API std::tm parse_time(std::string const &v);

	///
	/// \brief Parse a connection string \a cs into driver name \a driver_name and list of properties \a props
	///
	/// The connection string format is following:
	///
	/// \verbatim  driver:[key=value;]*  \endverbatim 
	///
	/// Where value can be either a sequence of characters (white space is trimmed) or it may be a general
	/// sequence encloded in a single quitation marks were double quote is used for insering a single quote value.
	///
	/// Key values starting with \@ are reserved to be used as special cppdb  keys
	/// For example:
	///
	/// \verbatim   mysql:username= root;password = 'asdf''5764dg';database=test;@use_prepared=off' \endverbatim 
	///
	/// Where driver is "mysql", username is "root", password is "asdf'5764dg", database is "test" and
	/// special value "@use_prepared" is off - internal cppdb option.
	CPPDB_API void parse_connection_string(	std::string const &cs,
						std::string &driver_name,
						std::map<std::string,std::string> &props);

	///
	/// \brief Class that represents parsed connection string
	///
	class CPPDB_API connection_info {
	public:
		///
		/// The original connection string
		///
		std::string connection_string;
		///
		/// The driver name
		///
		std::string driver;
		///
		/// Type that represent key, values set
		///
		typedef std::map<std::string,std::string> properties_type;
		///
		/// The std::map of key value properties.
		///
		properties_type properties;
		
		///
		/// Get property \a prop, returning \a default_value if not defined.
		///
		std::string get(std::string const &prop,std::string const &default_value=std::string()) const;
		///
		/// Get numeric value for property \a prop, returning \a default_value if not defined. 
		/// If the value is not a number, throws cppdb_error.
		///
		int get(std::string const &prop,int default_value) const;
	
		///
		/// Default constructor - empty info
		///	
		connection_info()
		{
		}
		///
		/// Create connection_info from the connection string parsing it.
		///
		explicit connection_info(std::string const &cs) :
			connection_string(cs)
		{
			parse_connection_string(cs,driver,properties);
		}

	};

}
#endif
