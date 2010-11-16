#ifndef CPPDB_POOL_H
#define CPPDB_POOL_H
#include "backend.h"
#include "mutex.h"

namespace cppdb {

	class pool {
		pool(pool const &);
		void operator=(pool const &);
	public:
		pool(std::string const &connection_string) : 
			limit_(0),
			size_(0), 
			life_time_(0),
			connection_string_(connection_string)
		{
			std::map<std::string,std::string> properties;
			std::string driver;
			parse_connection_string(connection_string,driver,properties);
			std::map<std::string,std::string>::const_iterator p;
			p = properties.find("cppdb_pool");
			bool use_pool = true;
			int pool_size = 16;
			int idle_time = 600;
			if(p!=properties.end()) {
				if(p->second=="off") {
					return;
				}
				else if(p->second=="on") {
				}
				else {
					throw cppdb_error("cppdb::pool pool property should be either on or off");
				}
				p=properties.find("cppdb_pool_size");
				if(p!=properties.end()) {
					pool_size = atoi(p->second());
				}
				p=properties.find("cppdb_pool_max_idle");
				if(p!=properties.end()) {
					idle_time = atoi(p->second());
				}
			}
			if(use_pool && pool_size > 0) {
				init(pool_size,idle_time);
			}
		}

		void init(size_t limit,int life_time) 
		{
			limit_ = limit;
			size_ = 0;
			life_time_ = life_time;
		}
		
		~pool()
		{
			clear_pool(pool_);
		}

		backend::connection *open()
		{
			backend::connection *p = get();
			if(p)
				return p;
			return driver_manager::instance().connect(connection_string_);
		}

		// this is thread safe member function
		backend::connection *get()
		{
			if(limit_ == 0)
				return 0;
			std::auto_ptr<backend::connection> c;
			pool_type garbage;
			time_t now = time(0);
			{
				mutex::guard l(lock_);
				// Nothing there should throw so it is safe
				pool_type::iterator p = pool_.begin(),tmp;
				while(p!=pool_.end()) {
					if(p->last_used + life_time_ < now) {
						tmp=p;
						p++;
						garbage.splice(garbage.begin(),pool_,tmp);
						size_ --;
					}
					else {
						// all is sorted by time
						break;
					}
				}
				if(!pool_.empty()) {
					c.reset(pool_.back().conn);
					pool_.pop_back();
					size_ --;
				}
			}
			clear_pool(garbage);
			return c.release();
		}
		
		// this is thread safe member function
		void put(backend::connection *c_in)
		{
			std::auto_ptr<backend::connection> c(c_in);
			if(limit_ == 0)
				return;
			pool_type garbage;
			time_t now = time(0);
			{
				mutex::guard l(lock_);
				// under lock do all very fast
				if(c.get()) {
					pool_.push_back(entry());
					pool_.back().last_used = now;
					pool_.back().conn = c.release();
					size_ ++;
				}
				
				// Nothing there should throw so it is safe
				
				pool_type::iterator p = pool_.begin(),tmp;
				while(p!=pool_.end()) {
					if(p->last_used + life_time_ < now) {
						tmp=p;
						p++;
						garbage.splice(garbage.begin(),pool_,tmp);
						size_ --;
					}
					else {
						// all is sorted by time
						break;
					}
				}
				// can be at most 1 entry bigger then limit
				if(size_ > limit_) {
					garbage.splice(time_out.begin(),pool_.begin());
					size_--;
				}
			}
			clear_pool(garbage);
		}
		// this is thread safe member function
		void gc()
		{
			put(0);
		}
	private:
		static void clear_pool(pool_type &pool)
		{
			pool_type::iterator p;
			for(p=pool.begin();p!=pool.end();++p) 
			{
				delete p->conn;
				p->conn = 0;
			}
			pool.clear();
		}
		struct entry {
			entry() : conn(0),last_used(0) {}
			backend::connection *conn;
			time_t last_used;
		};
		
		// non-mutable members
		
		size_t limit_;
		int life_time_;
		
		// mutex protected begin
		mutex lock_;
		size_t size_;
		typedef std::list<entry> pool_type;
		pool_type pool_;
		// mutex protected end
	};
}


#endif
