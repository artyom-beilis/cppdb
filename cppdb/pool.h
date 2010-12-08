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
#ifndef CPPDB_POOL_H
#define CPPDB_POOL_H

#include <cppdb/defs.h>
#include <cppdb/ref_ptr.h>
#include <cppdb/mutex.h>
#include <cppdb/utils.h>
#include <memory>
#include <list>


namespace cppdb {
	class connection_info;
	namespace backend {
		class connection;
	}
	
	///
	/// \brief Connections pool, allows to handle multiple connections for specific connection string.
	///
	/// Note connections_manager provide more generic interface and hides pools inside it. So you 
	/// generally should not use this class directly.
	///
	/// Unlike connections_manager, it uses pool by default unless its size defined as 0.
	///
	/// All this class member functions are thread safe to use from several threads for the same object
	///
	class CPPDB_API pool : public ref_counted {
		pool();
		pool(pool const &);
		void operator=(pool const &);
		pool(connection_info const &ci);
	public:
		/// Create new pool for \a connection_string
		static ref_ptr<pool> create(std::string const &connection_string);
		/// Create new pool for a parsed connection string \a ci
		static ref_ptr<pool> create(connection_info const &ci);

		~pool();

		///
		/// Get a open a connection, it may be fetched either from pool or new one may be created
		///
		ref_ptr<backend::connection> open();
		///
		/// Collect connections that were not used for a long time (close them)
		///
		void gc();
		/// \cond INTERNAL
		void put(backend::connection *c_in);
		/// \endcond
	private:
		ref_ptr<backend::connection> get();

		struct data;
		std::auto_ptr<data> d;
		
		struct entry {
			entry() : last_used(0) {}
			ref_ptr<backend::connection> conn;
			time_t last_used;
		};

		typedef std::list<entry> pool_type;
		// non-mutable members
		
		size_t limit_;
		int life_time_;
		connection_info ci_;
		
		// mutex protected begin
		mutex lock_;
		size_t size_;
		pool_type pool_;
		// mutex protected end
		
	};
}


#endif
