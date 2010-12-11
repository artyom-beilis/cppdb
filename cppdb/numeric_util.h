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
#ifndef CPPDB_NUMERIC_UTIL_H
#define CPPDB_NUMERIC_UTIL_H

#include <cppdb/errors.h>
#include <string>
#include <sstream>
#include <limits>
#include <iomanip>

namespace cppdb {

	///
	/// Small utility functions for backends, accepts - source string and stringstream with imbued std::locale
	/// it tries to case the value to T in best possible way.
	///
	/// For floating point string it casts it to the nearest ineger
	///
	template<typename T>
	T parse_number(std::string const &s,std::istringstream &ss)
	{
		ss.clear();
		ss.str(s);
		if(s.find_first_of(".eEdD")!=std::string::npos) {
			long double v;
			ss >> v;
			if(ss.fail() || !std::ws(ss).eof())
				throw bad_value_cast();
			if(std::numeric_limits<T>::is_integer) {
				if(v > std::numeric_limits<T>::max() || v < std::numeric_limits<T>::min())
					throw bad_value_cast();
			}
			return static_cast<T>(v);
		}
		T v;
		ss >> v;
		if(ss.fail() || !std::ws(ss).eof()) 
			throw bad_value_cast();
		if(	std::numeric_limits<T>::is_integer 
			&& !std::numeric_limits<T>::is_signed 
			&& s.find('-') != std::string::npos 
			&& v!=0) 
		{
			throw bad_value_cast();
		}
		return v;
	}


}
#endif
