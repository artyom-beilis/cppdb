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
#ifndef CPPDB_CONN_MANAGER_H
#define CPPDB_CONN_MANAGER_H

#include <cppdb/defs.h>
#include <cppdb/ref_ptr.h>
#include <cppdb/mutex.h>
#include <map>
#include <string>
#include <memory>

namespace cppdb {
	class pool;
	class connection_info;
	namespace backend {
		class connection;
	}

	///
	/// \brief This class is the major gateway to new connections
	///
	/// It handles connection pools and forwards request to the drivers.
	///
	/// This class member functions are thread safe
	///
	class CPPDB_API connections_manager {
		connections_manager();
		~connections_manager();
		connections_manager(connections_manager const &);
		void operator = (connections_manager const &);
	public:
		///
		/// Get a singleton instance of the class
		///
		static connections_manager &instance();
		///
		/// Create a new connection using connection string \a cs
		///
		ref_ptr<backend::connection> open(std::string const &cs);
		///
		/// Create a new connection using parsed connection string \a ci
		///
		ref_ptr<backend::connection> open(connection_info const &ci);
		///
		/// Collect all connections that were not used for long time and close them.
		///
		void gc();
	private:
		struct data;
		std::auto_ptr<data> d;

		mutex lock_;
		typedef std::map<std::string,ref_ptr<pool> > connections_type;
		connections_type connections_;
	};
} // cppdb


#endif
