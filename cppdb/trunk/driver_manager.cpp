#include "driver_manager.h"
#include "shared_object.h"
#include "backend.h"
#include "utils.h"

#include <vector>
#include <list>

extern "C" {
	#ifdef CPPDB_WITH_SQLITE3 
	cppdb::backend::connection *cppdb_sqlite3_get_connection(std::string const &cs);
	#endif
	#ifdef CPPDB_WITH_PQ 
	cppdb::backend::connection *cppdb_postgres_get_connection(std::string const &cs);
	#endif
}


namespace cppdb {

	extern "C" {
		typedef cppdb::backend::connection *(*connect_function_type)(std::string const &connection_string);
	}

	class static_driver : public backend::driver {
	public:
		static_driver(connect_function_type c) : connect_(c)
		{
		}
		virtual bool in_use()
		{
			return true;
		}
		virtual backend::connection *open(std::string const &cs)
		{
			return connect_(cs);
		}
	private:
		connect_function_type connect_;
	};

	class so_driver : public backend::loadable_driver {
	public:
		so_driver(std::string const &name,std::vector<std::string> const &so_list) :
			connect_(0)
		{
			std::string symbol_name = "cppdb_" + name + "_get_connection";
			for(unsigned i=0;i<so_list.size();i++) {
				so_ = shared_object::open(so_list[i]);
				if(!so_) {
					so_->safe_resolve(symbol_name,connect_);
					break;
				}
			}
			if(!so_ || !connect_) {
				throw cppdb_error("cppdb::driver failed to load driver " + name + " - no module found");
			}
		}
		virtual bool in_use()
		{
			return use_count() == 1;
		}
		virtual backend::connection *open(std::string const &cs)
		{
			return connect_(cs);
		}
	private:
		connect_function_type connect_;
		ref_ptr<shared_object> so_;
	};

	backend::connection *driver_manager::connect(std::string const &str)
	{
		size_t sep = str.find(':');
		std::string driver_name=str.substr(0,sep);
		ref_ptr<backend::driver> drv_ptr;
		drivers_type::iterator p;
		{ // get driver
			mutex::guard lock(lock_);
			p=drivers_.find(driver_name);
			if(p!=drivers_.end()) {
				drv_ptr = p->second;
			}
			else {
				drv_ptr = load_driver(str);
				drivers_[driver_name] = drv_ptr;	
			}
		}
		return drv_ptr->connect(str);
	}
	void driver_manager::collect_unused()
	{
		std::list<ref_ptr<backend::driver> > garbage;
		{
			mutex::guard lock(lock_);
			drivers_type::iterator p=drivers_.begin(),tmp;
			while(p!=drivers_.end()) {
				if(!p->second->in_use()) {
					garbage.push_back(p->second);
					tmp=p;
					++p;
					drivers_.erase(tmp);
				}
				else {
					++p;
				}
			}
		}
		garbage.clear();
	}

	ref_ptr<backend::driver> driver_manager::load_driver(std::string const &connection_string)
	{
		std::string name;
		std::map<std::string,std::string>::iterator p;
		std::map<std::string,std::string> properties;
		parse_connection_string(connection_string,name,properties);
		std::vector<std::string> so_names;
		if((p=properties.find("cppdb_module"))==properties.end()) {
			so_names.push_back(p->second);
		}
		else {
			std::string so_name1 = "libcppdb_" + name + ".so";
			std::string so_name2 = "libcppdb_" + name + ".so.0";
			for(unsigned i=0;i<search_paths_.size();i++) {
				so_names.push_back(search_paths_[i]+"/" + so_name1);
				so_names.push_back(search_paths_[i]+"/" + so_name2);
			}
			if(!no_default_directory_) {
				so_names.push_back(so_name1);
				so_names.push_back(so_name2);
			}
		}
		ref_ptr<backend::driver> drv=new so_driver(name,so_names);
		return drv;
	}

	void driver_manager::install_driver(std::string const &name,ref_ptr<backend::driver> drv)
	{
		if(!drv) {
			throw cppdb_error("cppdb::driver_manager::install_driver: Can't install empty driver");
		}
		mutex::guard lock(lock_);
		drivers_[name]=drv;
	}

	driver_manager::driver_manager() : 
		no_default_directory_(false)
	{
	}
	driver_manager::~driver_manager()
	{
	}
	
	void driver_manager::add_search_path(std::string const &p)
	{
		mutex::guard l(lock_);
		search_paths_.push_back(p);
	}
	void driver_manager::clear_search_paths()
	{
		mutex::guard l(lock_);
		search_paths_.clear();
	}
	void driver_manager::use_default_search_path(bool v)
	{
		mutex::guard l(lock_);
		no_default_directory_ = !v;
	}
	driver_manager &driver_manager::instance()
	{
		static driver_manager instance;
		return instance;
	}
	namespace {
		struct initializer {
			initializer() { 
				driver_manager::instance(); 
				#ifdef CPPDB_WITH_SQLITE3 
				driver_manager::instance().install_driver(
					"sqlite3",new static_driver(cppdb_sqlite3_get_connection)
				);
				#endif
				#ifdef CPPDB_WITH_PQ 
				driver_manager::instance().install_driver(
					"postgres",new static_driver(cppdb_postgres_get_connection)
				);
				#endif
			}
			
		} init;
	}
} // cppdb
