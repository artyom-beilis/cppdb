#include "frontend.h"
#include "conn_manager.h"

namespace cppdb {
	session::session()
	{
	}
	session::session(ref_ptr<backend::connection> conn) : 
		conn_(conn) 
	{
	}
	session::~session()
	{
	}
	session::session(std::string const &cs)
	{
		open(cs);
	}
	
	void session::open(std::string const &cs)
	{
		conn_ = connections_manager::instance().open(cs);
	}
	void session::close()
	{
		conn_.reset();
	}
} 
