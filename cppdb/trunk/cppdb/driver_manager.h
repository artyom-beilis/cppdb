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
#ifndef CPPDB_DRIVER_MANAGER_H
#define CPPDB_DRIVER_MANAGER_H

#include <cppdb/defs.h>
#include <cppdb/ref_ptr.h>
#include <cppdb/mutex.h>
#include <map>
#include <string>
#include <vector>

namespace cppdb {
	namespace backend {
		class connection;
		class driver;
	}
	class connection_info;	

	///
	/// \brief this class is used to handle all drivers, loading them, unloading them etc.
	///
	/// All its member functions are thread safe
	///
	class CPPDB_API driver_manager {
	public:
		///
		/// Get the singleton instance of the class
		///
		static driver_manager &instance();
		///
		/// Install new driver \a drv named \a name to the manager.
		///
		void install_driver(std::string const &name,ref_ptr<backend::driver> drv);
		///
		/// Unload all drivers that have no more open connections.
		///
		void collect_unused();

		///
		/// Add a path were the driver should search for loadable modules
		///
		void add_search_path(std::string const &);
		///
		/// Clear previously added a paths 
		///
		void clear_search_paths();
		///
		/// Search the library under default directory (i.e. empty path prefix) or not, default is true
		///
		void use_default_search_path(bool v);
		///
		/// Create a new connection object using parsed connection string \a ci
		///
		backend::connection *connect(connection_info const &ci);
		///
		/// Create a new connection object using connection string \a connectoin_string
		///
		backend::connection *connect(std::string const &connectoin_string);

	private:
		driver_manager(driver_manager const &);
		void operator=(driver_manager const &);
		~driver_manager();
		driver_manager();
		
		ref_ptr<backend::driver> load_driver(connection_info const &ci);

		typedef std::map<std::string,ref_ptr<backend::driver> > drivers_type;
		std::vector<std::string> search_paths_;
		bool no_default_directory_; 
		drivers_type drivers_;
		mutex lock_;
	};
}

#endif
