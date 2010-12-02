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

	struct result::data {};

	result::result() :
		eof_(false),
		fetched_(false),
		current_col_(0)
	{
	}
	result::result(	ref_ptr<backend::result> res,
			ref_ptr<backend::statement> stat,
			ref_ptr<backend::connection> conn)
	: eof_(false),
	  fetched_(false),
	  current_col_(0),
	  res_(res),
	  stat_(stat),
	  conn_(conn)
	{
	}
	result::result(result const &other) :
		eof_(other.eof_),
		fetched_(other.fetched_),
		current_col_(other.current_col_),
		res_(other.res_),
		stat_(other.stat_),
		conn_(other.conn_)
	{
	}

	result const &result::operator=(result const &other)
	{
		eof_ = other.eof_;
		fetched_ = other.fetched_;
		current_col_ = other.current_col_;
		res_ = other.res_;
		stat_ = other.stat_;
		conn_ = other.conn_;
		return *this;
	}

	result::~result()
	{
		clear();
	}

	int result::cols()
	{
		return res_->cols();
	}

	bool result::next()
	{
		if(eof_)
			return false;
		fetched_=true;
		eof_ = res_->next()==false;
		current_col_ = 0;
		return !eof_;
	}
	
	int result::index(std::string const &n)
	{
		int c = res_->name_to_column(n);
		if(c<0)
			throw invalid_column();
		return c;
	}

	std::string result::name(int col)
	{
		if(col < 0 || col>= cols())
			throw invalid_column();
		return res_->column_to_name(col);
	}

	int result::find_column(std::string const &name)
	{
		int c = res_->name_to_column(name);
		if(c < 0)
			return -1;
		return c;
	}

	void result::rewind_column()
	{
		current_col_ = 0;
	}
	
	bool result::empty()
	{
		if(!res_)
			return true;
		return eof_ || !fetched_;
	}

	void result::clear()
	{
		eof_ = true;
		fetched_ = true;
		res_.reset();
		stat_.reset();
		conn_.reset();
	}

	void result::check()
	{
		if(empty())
			throw empty_row_access();
	}

	bool result::is_null(int col)
	{
		return res_->is_null(col);
	}
	bool result::is_null(std::string const &n)
	{
		return is_null(index(n));
	}

	
	bool result::fetch(int col,short &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,unsigned short &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,int &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,unsigned &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,long &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,unsigned long &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,long long &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,unsigned long long &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,float &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,double &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,long double &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,std::string &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,std::tm &v) { return res_->fetch(col,v); }
	bool result::fetch(int col,std::ostream &v) { return res_->fetch(col,v); }

	bool result::fetch(std::string const &n,short &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,unsigned short &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,int &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,unsigned &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,long &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,unsigned long &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,long long &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,unsigned long long &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,float &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,double &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,long double &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,std::string &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,std::tm &v) { return res_->fetch(index(n),v); }
	bool result::fetch(std::string const &n,std::ostream &v) { return res_->fetch(index(n),v); }

	bool result::fetch(short &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(unsigned short &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(int &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(unsigned &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(long &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(unsigned long &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(long long &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(unsigned long long &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(float &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(double &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(long double &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(std::string &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(std::tm &v) { return res_->fetch(current_col_++,v); }
	bool result::fetch(std::ostream &v) { return res_->fetch(current_col_++,v); }


}  // cppdb
