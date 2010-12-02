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
#ifndef CPPDB_ERRORS_H
#define CPPDB_ERRORS_H
#include <stdexcept>

namespace cppdb {
	class cppdb_error : public std::runtime_error {
	public:
		cppdb_error(std::string const &v) : std::runtime_error(v) {}
	};

	class bad_value_cast : public cppdb_error {
	public:
		bad_value_cast() : cppdb_error("cppdb::bad_value_cast can't convert data")
		{
		}
	};

	class null_value_fetch : public cppdb_error {
	public:
		null_value_fetch() : cppdb_error("cppdb::null_value_fetch attempt fetch null column")
		{
		}
	};
	class empty_row_access : public cppdb_error {
	public:
		empty_row_access() : cppdb_error("cppdb::empty_row_access attempt to fetch from empty column")
		{
		}
	};

	class invalid_column : public cppdb_error {
	public:
		invalid_column() : cppdb_error("cppdb::invalid_column attempt access to invalid column")
		{
		}
	};
	class invalid_placeholder : public cppdb_error {
	public:
		invalid_placeholder() : cppdb_error("cppdb::invalid_placeholder attempt bind to invalid placeholder")
		{
		}
	};
	class multiple_rows_query : public cppdb_error {
	public:
		multiple_rows_query() : cppdb_error(	"cppdb::multiple_rows_query "
							"multiple rows result for a single row request")
		{
		}
	};

	class not_supported_by_backend : public cppdb_error {
	public:
		not_supported_by_backend(std::string const &e) :
			cppdb_error(e)
		{
		}
	};

}

#endif
