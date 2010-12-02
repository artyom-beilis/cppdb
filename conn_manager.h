#ifndef CPPDB_CONN_MANAGER_H
#define CPPDB_CONN_MANAGER_H

#include "defs.h"
#include "ref_ptr.h"
#include "mutex.h"
#include <map>
#include <string>
#include <memory>

namespace cppdb {
	class pool;
	class connection_info;
	namespace backend {
		class connection;
	}

	class CPPDB_API connections_manager {
		connections_manager();
		~connections_manager();
		connections_manager(connections_manager const &);
		void operator = (connections_manager const &);
	public:
		static connections_manager &instance();
		ref_ptr<backend::connection> open(std::string const &cs);
		ref_ptr<backend::connection> open(connection_info const &ci);
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
