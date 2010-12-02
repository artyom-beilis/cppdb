#ifndef CPPDB_POOL_H
#define CPPDB_POOL_H

#include "defs.h"
#include "ref_ptr.h"
#include "mutex.h"
#include "utils.h"
#include <memory>
#include <list>


namespace cppdb {
	class connection_info;
	namespace backend {
		class connection;
	}
	
	class CPPDB_API pool : public ref_counted {
		pool();
		pool(pool const &);
		void operator=(pool const &);
		pool(connection_info const &ci);
	public:
		static ref_ptr<pool> create(std::string const &connection_string);
		static ref_ptr<pool> create(connection_info const &ci);

		~pool();

		ref_ptr<backend::connection> open();
		void gc();
		void put(backend::connection *c_in);

	private:
		ref_ptr<backend::connection> get();
		void init(connection_info const &ci);

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
