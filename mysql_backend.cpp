#include <mysql/mysql.h>

#include "backend.h"
#include "errors.h"
#include "utils.h"
#include <sstream>
#include <vector>
#include <limits>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#include <iostream>

namespace cppdb {

namespace mysql_backend {	

class cppdb_myerror : public cppdb_error 
{
public:
	cppdb_myerror(std::string const &str) : 
		cppdb_error("cppdb::mysql::" + str)
	{
	}
};

class result : public backend::result {
	struct bind_data {
		bind_data() :
			ptr(0),
			length(0),
			is_null(0),
			error(0)
		{
			memset(&buf,0,sizeof(buf));
		}
		char buf[128];
		std::vector<char> vbuf;
		char *ptr;
		unsigned long length;
		my_bool is_null;
		my_bool error;
	};
public:

	///
	/// Check if the next row in the result exists. If the DB engine can't perform
	/// this check without loosing data for current row, it should return next_row_unknown.
	///
	virtual next_row has_next() 
	{
		if(current_row_ >= mysql_stmt_num_rows(stmt_))
			return last_row_reached;
		else
			return next_row_exists;
	}
	///
	/// Move to next row. Should be called before first access to any of members. If no rows remain
	/// return false, otherwise return true
	///
	virtual bool next() 
	{
		current_row_ ++;
		reset();
		if(cols_ > 0) {
			if(mysql_stmt_bind_result(stmt_,&bind_[0])) {
				throw cppdb_myerror(mysql_stmt_error(stmt_));
			}
		}
		int r = mysql_stmt_fetch(stmt_);
		if(r==MYSQL_NO_DATA) { 
			return false;
		}
		if(r==MYSQL_DATA_TRUNCATED) {
			for(int i=0;i<cols_;i++) {
				if(bind_data_[i].error && !bind_data_[i].is_null && bind_data_[i].length >= sizeof(bind_data_[i].buf)) {
					bind_data_[i].vbuf.resize(bind_data_[i].length);
					MYSQL_BIND b=MYSQL_BIND();
					bind_[i].buffer = &bind_data_[i].vbuf.front();
					bind_[i].buffer_length = bind_data_[i].length;
					if(mysql_stmt_fetch_column(stmt_,&bind_[i],i,0)) {
						throw cppdb_myerror(mysql_stmt_error(stmt_));
					}
					bind_data_[i].ptr = &bind_data_[i].vbuf.front();
				}
			}
		}
		return true;
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	bind_data &at(int col)
	{
		if(col < 0 || col >= cols_)
			throw invalid_column();
		if(bind_data_.empty())
			throw cppdb_myerror("Attempt to access data without fetching it first");
		return bind_data_.at(col);
	}

	template<typename T>
	bool do_fetch(int col,T &v)
	{
		bind_data &d=at(col);
		if(d.is_null)
			return false;

		std::istringstream ss;
		ss.imbue(std::locale::classic());
		ss.str(std::string(d.ptr,d.length));
		ss >> v;
		if(!ss || !ss.eof())
			throw bad_value_cast();
		return true;
	}
	virtual bool fetch(int col,short &v) 
	{
		return do_fetch(col,v);;
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned short &v) 
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,int &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned &v) 
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,long &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned long &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,long long &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned long long &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,float &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,double &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,long double &v)
	{
		return do_fetch(col,v);
	}
	///
	/// Fetch a string value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
	/// text value (as formatting integer, floating point value or date-time as string).
	///
	virtual bool fetch(int col,std::string &v)
	{
		bind_data &d=at(col);
		if(d.is_null)
			return false;
		v.assign(d.ptr,d.length);
		return true;
	}
	///
	/// Fetch a BLOB value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
	/// BLOB value as text (as formatting integer, floating point value or date-time as string).
	///
	virtual bool fetch(int col,std::ostream &v)
	{
		bind_data &d=at(col);
		if(d.is_null)
			return false;
		v.write(d.ptr,d.length);
		return true;
	}
	///
	/// Fetch a date-time value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid. If the data can't be converted
	/// to date-time it should throw bad_value_cast()
	///
	virtual bool fetch(int col,std::tm &v) 
	{
		std::string tmp;
		if(!fetch(col,tmp))
			return false;
		v = parse_time(tmp);
		return true;
	}
	///
	/// Check if the column \a col is NULL starting from 0, should throw invalid_column() if the index out of range
	///
	virtual bool is_null(int col) 
	{
		return at(col).is_null;
	}
	///
	/// Return the number of columns in the result. Should be valid even without calling next() first time.
	///
	virtual int cols() 
	{
		return cols_;
	}
	virtual std::string column_to_name(int col) 
	{
		if(col < 0 || col >=cols_)
			throw invalid_column();
		MYSQL_FIELD *flds=mysql_fetch_fields(meta_);
		if(!flds) {
			throw cppdb_myerror("Internal error empty fileds");
		}
		return flds[col].name;
	}
	virtual int name_to_column(std::string const &name) 
	{
		MYSQL_FIELD *flds=mysql_fetch_fields(meta_);
		if(!flds) {
			throw cppdb_myerror("Internal error empty fileds");
		}
		for(int i=0;i<cols_;i++)
			if(name == flds[i].name)
				return i;
		return -1;
	}

	// End of API
	
	result(MYSQL_STMT *stmt) : 
		stmt_(stmt), current_row_(0),meta_(0)
	{
		cols_ = mysql_stmt_field_count(stmt_);
		if(mysql_stmt_store_result(stmt_)) {
			throw cppdb_myerror(mysql_stmt_error(stmt_));
		}
		meta_ = mysql_stmt_result_metadata(stmt_);
		if(!meta_) {
			throw cppdb_myerror("Seems that the query does not produce any result");
		}
	}
	~result()
	{
		mysql_free_result(meta_);
	}
	void reset()
	{
		bind_.resize(0);
		bind_data_.resize(0);
		bind_.resize(cols_,MYSQL_BIND());
		bind_data_.resize(cols_,bind_data());
		for(int i=0;i<cols_;i++) {
			bind_[i].buffer_type = MYSQL_TYPE_STRING;
			bind_[i].buffer = bind_data_[i].buf;
			bind_[i].buffer_length = sizeof(bind_data_[i].buf);
			bind_[i].length = &bind_data_[i].length;
			bind_[i].is_null = &bind_data_[i].is_null;
			bind_[i].error = &bind_data_[i].error;
			bind_data_[i].ptr = bind_data_[i].buf;
		}
	}
private:
	int cols_;
	MYSQL_STMT *stmt_;
	unsigned current_row_;
	MYSQL_RES *meta_;
	std::vector<MYSQL_BIND> bind_;
	std::vector<bind_data> bind_data_;
};

class statement : public backend::statement {
	struct param {
		my_bool is_null;
		bool is_blob;
		unsigned long length;
		std::string value;
		void *buffer;
		param() : 
			is_null(1),
			is_blob(false),
			length(0),
			buffer(0)
		{
		}
		template<typename T>
		void set(T v)
		{
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			if(!std::numeric_limits<T>::is_integer)
				ss << std::setprecision(std::numeric_limits<T>::digits10+1);
			ss << v;
			set_str(ss.str());
		}
		void set(char const *b,char const *e,bool blob=false)
		{
			length = e - b;
			buffer = const_cast<char *>(b);
			is_blob = blob;
			is_null = 0;
		}
		void set_str(std::string const &s)
		{
			value = s;
			buffer = const_cast<char *>(value.c_str());
			length = value.size();
			is_null = 0;
		}
		void set(std::tm const &t)
		{
			set_str(format_time(t));
		}
		void bind_it(MYSQL_BIND *b) 
		{
			b->is_null = &is_null;
			if(!is_null) {
				b->buffer_type = is_blob ? MYSQL_TYPE_BLOB : MYSQL_TYPE_STRING;
				b->buffer = buffer;
				b->buffer_length = length;
				b->length = &length;
			}
			else {
				b->buffer_type = MYSQL_TYPE_NULL;
			}
		}
	};

public:
	// Begin of API
	///
	/// Get the query the statement works with. Return it as is, used as key for statement
	/// caching 
	///
	virtual std::string const &sql_query() 
	{
		return query_;
	}

	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::string const &s)
	{
		at(col).set(s.c_str(),s.c_str()+s.size());
	}
	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,char const *s)
	{
		at(col).set(s,s+strlen(s));
	}
	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,char const *b,char const *e) 
	{
		at(col).set(b,e);
	}
	///
	/// Bind a date-time value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::tm const &v) 
	{
		at(col).set(v);
	}
	///
	/// Bind a BLOB value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::istream const &v)
	{
		std::ostringstream ss;
		ss << v.rdbuf();
		at(col).set_str(ss.str());
		at(col).is_blob = true;
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,int v)
	{
		at(col).set(v);
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned v)
	{
		at(col).set(v);
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,long v)
	{
		at(col).set(v);
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned long v)
	{
		at(col).set(v);
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,long long v)
	{
		at(col).set(v);
	}
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned long long v)
	{
		at(col).set(v);
	}
	///
	/// Bind a floating point value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,double v) 
	{
		at(col).set(v);
	}
	///
	/// Bind a floating point value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,long double v)
	{
		at(col).set(v);
	}
	///
	/// Bind a NULL value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind_null(int col)
	{
		at(col)=param();
	}
	///
	/// Fetch the last sequence generated for last inserted row. May use sequence as parameter
	/// if the database uses sequences, should ignore the parameter \a sequence if the last
	/// id is fetched without parameter.
	///
	/// Should be called after exec() for insert statement, otherwise the behavior is undefined.
	///
	/// MUST throw not_supported_by_backend() if such option is not supported by the DB engine.
	///
	virtual long long sequence_last(std::string const &/*sequence*/) 
	{
		return mysql_stmt_insert_id(stmt_);
	}
	///
	/// Return the number of affected rows by last statement.
	///
	/// Should be called after exec(), otherwise behavior is undefined.
	///
	virtual unsigned long long affected()
	{
		return mysql_stmt_affected_rows(stmt_);
	}
	
	void bind_all()
	{
		if(!params_.empty()) {
			for(unsigned i=0;i<params_.size();i++)
				params_[i].bind_it(&bind_[i]);
			if(mysql_stmt_bind_param(stmt_,&bind_.front())) {
				throw cppdb_myerror(mysql_stmt_error(stmt_));
			}
		}
	}

	///
	/// Return SQL Query result, MAY throw cppdb_error if the statement is not a query
	///
	virtual result *query() 
	{
		bind_all();
		if(mysql_stmt_execute(stmt_)) {
			throw cppdb_myerror(mysql_stmt_error(stmt_));
		}
		return new result(stmt_);
	}
	///
	/// Execute a statement, MAY throw cppdb_error if the statement returns results.
	///
	virtual void exec() 
	{
		bind_all();
		if(mysql_stmt_execute(stmt_)) {
			throw cppdb_myerror(mysql_stmt_error(stmt_));
		}
		if(mysql_stmt_store_result(stmt_)) {
			throw cppdb_myerror(mysql_stmt_error(stmt_));
		}
		MYSQL_RES *r=mysql_stmt_result_metadata(stmt_);
		if(r) {
			mysql_free_result(r);
			throw cppdb_myerror("Calling exec() on query!");
		}
	}
	// End of API

	// Caching support
	
	statement(std::string const &q,MYSQL *conn) :
		query_(q),
		stmt_(0),
		params_count_(0)
	{
		stmt_ = mysql_stmt_init(conn);
		try {
			if(!stmt_) {
				throw cppdb_myerror(" Failed to create a statement");
			}
			if(mysql_stmt_prepare(stmt_,q.c_str(),q.size())) {
				throw cppdb_myerror(mysql_stmt_error(stmt_));
			}
			params_count_ = mysql_stmt_param_count(stmt_);
			reset_data();
		}
		catch(...) {
			if(stmt_)
				mysql_stmt_close(stmt_);
			throw;
		}
	}
	virtual ~statement()
	{
		mysql_stmt_close(stmt_);
	}
	void reset_data()
	{
		params_.resize(0);
		params_.resize(params_count_);
		bind_.resize(0);
		bind_.resize(params_count_,MYSQL_BIND());
	}
	virtual void reset()
	{
		reset_data();
		mysql_stmt_reset(stmt_);
	}

private:
	param &at(int col)
	{
		if(col < 1 || col > params_count_)
			throw invalid_placeholder();
		return params_[col-1];
	}

	std::vector<param> params_;
	std::vector<MYSQL_BIND> bind_;
	std::string query_;
	MYSQL_STMT *stmt_;
	int params_count_;
};

class connection;

class connection : public backend::connection {
public:
	connection(connection_info const &ci) : 
		backend::connection(ci),
		conn_(0)
	{
		conn_ = mysql_init(0);
		if(!conn_) {
			throw cppdb_error("cppdb::mysql filed to create connection");
		}
		std::string host = ci.get("host","");
		char const *phost = host.empty() ? 0 : host.c_str();
		std::string user = ci.get("user","");
		char const *puser = user.empty() ? 0 : user.c_str();
		std::string password = ci.get("password","");
		char const *ppassword = password.empty() ? 0 : password.c_str();
		std::string database = ci.get("database","");
		char const *pdatabase = database.empty() ? 0 : database.c_str();
		int port = ci.get("port",0);
		std::string unix_socket = ci.get("unix_socket","");
		char const *punix_socket = unix_socket.empty() ? 0 : unix_socket.c_str();
		
		if(!mysql_real_connect(conn_,phost,puser,ppassword,pdatabase,port,punix_socket,0)) {
			std::string err="unknown";
			try { err = mysql_error(conn_); }catch(...){}
			mysql_close(conn_);
			throw cppdb_myerror(err);
		}

	}
	~connection()
	{
		mysql_close(conn_);
	}
	// API 
	
	void exec(std::string const &s) 
	{
		if(mysql_real_query(conn_,s.c_str(),s.size())) {
			throw cppdb_myerror(mysql_error(conn_));
		}
	}

	///
	/// Start new isolated transaction. Would not be called
	/// withing other transaction on current connection.
	///
	virtual void begin() 
	{
		exec("BEGIN");
	}
	///
	/// Commit the transaction, you may assume that is called after begin()
	/// was called.
	///
	virtual void commit() 
	{
		exec("COMMIT");
	}
	///
	/// Rollback the transaction. MUST never throw!!!
	///
	virtual void rollback() 
	{
		try {
			exec("ROLLBACK");
		}
		catch(...) {
		}
	}
	///
	/// Create a prepared statement \a q. May throw if preparation had failed.
	/// Should never return null value.
	///
	virtual statement *prepare_statement(std::string const &q)
	{
		return new statement(q,conn_);
	}
	virtual statement *create_statement(std::string const &q)
	{
		return new statement(q,conn_);
	}
	///
	/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
	///
	virtual std::string escape(std::string const &s) 
	{
		return escape(s.c_str(),s.c_str()+s.size());
	}
	///
	/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
	///
	virtual std::string escape(char const *s)
	{
		return escape(s,s+strlen(s));
	}
	///
	/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
	///
	virtual std::string escape(char const *b,char const *e) 
	{
		std::vector<char> buf(2*(e-b)+1);
		size_t len = mysql_real_escape_string(conn_,&buf.front(),b,e-b);
		std::string result;
		result.assign(&buf.front(),len);
		return result;
	}
	///
	/// Get the name of the driver, for example sqlite3, odbc
	///
	virtual std::string driver() 
	{
		return "mysql";
	}
	///
	/// Get the name of the SQL Server, for example sqlite3, mssql, oracle, differs from driver() when
	/// the backend supports multiple databases like odbc backend.
	///
	virtual std::string engine()
	{
		return "mysql";
	}

	// API
	
private:
	connection_info ci_;
	MYSQL *conn_;
};


} // backend
} // cppdb

extern "C" {
	cppdb::backend::connection *cppdb_mysql_get_connection(cppdb::connection_info const &cs)
	{
		return new cppdb::mysql_backend::connection(cs);
	}
}
