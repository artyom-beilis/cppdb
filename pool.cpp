#include "pool.h"
#include "backend.h"
#include "utils.h"
#include "driver_manager.h"

#include <stdlib.h>

namespace cppdb {

	struct pool::data {};

	pool::pool(std::string const &cs) :
		limit_(0),
		life_time_(0),
		size_(0)
	{
		connection_info inf(cs);
		init(cs);
	}
	pool::pool(connection_info const &cs) :
		limit_(0),
		life_time_(0),
		size_(0)
	{
		init(cs);
	}
		
	void pool::init(connection_info const &ci)
	{
		ci_ = ci;
		
		size_ = 0;
		limit_ = ci_.get("@pool_size",16);
		life_time_ = ci_.get("@pool_max_idle",600);
	}
	
	pool::~pool()
	{
	}

	ref_ptr<backend::connection> pool::open()
	{
		if(limit_ == 0)
			return driver_manager::instance().connect(ci_);

		ref_ptr<backend::connection> p = get();

		if(!p) {
			p=driver_manager::instance().connect(ci_);
		}
		p->recycle_pool(this);
		return p;
	}

	// this is thread safe member function
	ref_ptr<backend::connection> pool::get()
	{
		if(limit_ == 0)
			return 0;
		ref_ptr<backend::connection> c;
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
				c = pool_.back().conn;
				pool_.pop_back();
				size_ --;
			}
		}
		return c;
	}
	
	// this is thread safe member function
	void pool::put(backend::connection *c_in)
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
				garbage.splice(garbage.begin(),pool_,pool_.begin());
				size_--;
			}
		}
	}
	
	void pool::gc()
	{
		put(0);
	}

}



