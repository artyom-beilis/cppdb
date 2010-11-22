#include "conn_manager.h"
#include "backend.h"
#include "pool.h"
#include "driver_manager.h"

namespace cppdb {
	struct connections_manager::data{};
	connections_manager::connections_manager() {}
	connections_manager::~connections_manager() {}

	connections_manager &connections_manager::instance()
	{
		static connections_manager mgr;
		return mgr;
	}

	namespace { 
		struct init { 
			init() 
			{ connections_manager::instance(); }
		} initializer; 
	}

	ref_ptr<backend::connection> connections_manager::open(std::string const &cs)
	{
		connection_info ci(cs);
		return open(ci);
	}
	ref_ptr<backend::connection> connections_manager::open(connection_info const &ci)
	{
		if(ci.get("@pool_size",0)==0) {
			driver_manager::instance().connect(ci);
		}
		ref_ptr<pool> p;
		{
			mutex::guard l(lock_);
			ref_ptr<pool> &ref_p = connections_[ci.connection_string];
			if(!ref_p) {
				ref_p = pool::create(ci);
			}
			p=ref_p;
		}
		return p->open();
	}
	void connections_manager::gc()
	{
		std::vector<ref_ptr<pool> > pools_;
		pools_.reserve(100);
		{
			mutex::guard l(lock_);
			for(connections_type::iterator p=connections_.begin();p!=connections_.end();++p) {
				pools_.push_back(p->second);
			}
		}
		for(unsigned i=0;i<pools_.size();i++) {
			pools_[i]->gc();
		}
		pools_.clear();
		{
			mutex::guard l(lock_);
			for(connections_type::iterator p=connections_.begin();p!=connections_.end();) {
				if(p->second->use_count() == 1) {
					pools_.push_back(p->second);
					connections_type::iterator tmp = p;
					++p;
					connections_.erase(tmp);
				}
				else
					++p;
			}
		}
		pools_.clear();

	}
		

} // cppdb
