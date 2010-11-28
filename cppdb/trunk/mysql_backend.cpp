#include <mysql/mysql.h>

namespace cppdb {

namespace mysql_backend {	

class cppdb_myerror : public cppdb_error 
{
	cppdb_error(std::string const &str) : 
		cppdb_myerror("cppdb::mysql::" + str);
	{
	}
};

class result : public backend::result {
public:

	///
	/// Check if the next row in the result exists. If the DB engine can't perform
	/// this check without loosing data for current row, it should return next_row_unknown.
	///
	virtual next_row has_next() = 0;
	///
	/// Move to next row. Should be called before first access to any of members. If no rows remain
	/// return false, otherwise return true
	///
	virtual bool next() = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,short &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned short &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,int &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,long &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned long &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,long long &v) = 0;
	///
	/// Fetch an integer value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to integer or its range is not supported by the integer type.
	///
	virtual bool fetch(int col,unsigned long long &v) = 0;
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,float &v) = 0;
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,double &v) = 0;
	///
	/// Fetch a floating point value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
	/// can't be converted to floating point value.
	///
	virtual bool fetch(int col,long double &v) = 0;
	///
	/// Fetch a string value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
	/// text value (as formatting integer, floating point value or date-time as string).
	///
	virtual bool fetch(int col,std::string &v) = 0;
	///
	/// Fetch a BLOB value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
	/// BLOB value as text (as formatting integer, floating point value or date-time as string).
	///
	virtual bool fetch(int col,std::ostream &v) = 0;
	///
	/// Fetch a date-time value for column \a col starting from 0.
	/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
	///
	/// Should throw invalid_column() \a col value is invalid. If the data can't be converted
	/// to date-time it should throw bad_value_cast()
	///
	virtual bool fetch(int col,std::tm &v) = 0;
	///
	/// Check if the column \a col is NULL starting from 0, should throw invalid_column() if the index out of range
	///
	virtual bool is_null(int col) = 0;
	///
	/// Return the number of columns in the result. Should be valid even without calling next() first time.
	///
	virtual int cols() = 0;
	///
	/// Return the number of columns by its name. Return -1 if the name is invalid
	/// Should be able to work even without calling next() first time.
	///
	virtual int name_to_column(std::string const &) = 0;
	///
	/// Return the column name for column index starting from 0.
	/// Should throw invalid_column() if the index out of range
	/// Should be able to work even without calling next() first time.
	///
	virtual std::string column_to_name(int) = 0;

	// End of API
	
	result();
	virtual ~result();
private:
	struct data;
	std::auto_ptr<data> d;
};

class statements_cache;

class statement : public ref_counted {
public:
	// Begin of API
	///
	/// Reset the prepared statement to initial state as before the operation. It is
	/// called by front-end each time before new query() or exec() are called.
	///
	virtual void reset() = 0;
	///
	/// Get the query the statement works with. Return it as is, used as key for statement
	/// caching 
	///
	virtual std::string const &sql_query() = 0;

	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::string const &) = 0;
	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,char const *s) = 0;
	///
	/// Bind a text value to column \a col (starting from 1). You may assume
	/// that the reference remains valid until real call of query() or exec()
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,char const *b,char const *e) = 0;
	///
	/// Bind a date-time value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::tm const &) = 0;
	///
	/// Bind a BLOB value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,std::istream const &) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,int v) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned v) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,long v) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned long v) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,long long v) = 0;
	///
	/// Bind an integer value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	/// May throw bad_value_cast() if the value out of supported range by the DB. 
	///
	virtual void bind(int col,unsigned long long v) = 0;
	///
	/// Bind a floating point value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,double v) = 0;
	///
	/// Bind a floating point value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind(int col,long double v) = 0;
	///
	/// Bind a NULL value to column \a col (starting from 1).
	///
	/// Should throw invalid_placeholder() if the value of col is out of range. May
	/// ignore if it is impossible to know whether the placeholder exists without special
	/// support from back-end.
	///
	virtual void bind_null(int col) = 0;
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
	///
	/// Return SQL Query result, MAY throw cppdb_error if the statement is not a query
	///
	virtual result *query() = 0;
	///
	/// Execute a statement, MAY throw cppdb_error if the statement returns results.
	///
	virtual void exec() = 0;
	// End of API

	// Caching support
	
	statement(std::string const &q,MYSQL *conn) :
		query_(q),
		stmt_(0),
		params_count_(0),
	{
		stmt_ = mysql_stmt_init(conn);
		try {
			if(!stmt_) {
				throw cppdb_myerror(" Failed to create a statement");
			}
			if(!mysql_stmt_prepare(stmt_,q.c_str(),q.size())) {
				throw cppdb_myerror(mysql_stmt_error(stmt_));
			}
			params_count_ = mysql_stmt_param_count(stmt_);
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
	void reset()
	{
		params_.resize(0);
		params_.resize(params_count_);
		bind_.resize(0);
		bind_.resize(params_count_,MYSQL_BIND());
		for(int i=0;i<params_count_;i++) 
			bind_[i].is_null_= &is_null_[i];

	}

private:
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
		void set(char const *b,char const *e,bool blob)
		{
			length = e - b;
			buffer = b;
			is_blob = blob;
		}
		void set_str(std::string const &s)
		{
			value = s;
			buffer = value.c_str();
			length = value.size();
		}
		void set(std::tm const &t)
		{
			set_str(format_time(t));
		}
		void set(std::istream &in)
		{
			std::istringstream ss;
			ss << in.rdbuf();
			set_str(ss.str());
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

	std::vector<MYSQL_BIND> bind_;
	std::string query_;
	MYSQL_STMT *stmt_;
	int params_count_;
};

class connection;

class connection : public backend::connection {
public:
	connection(connection_info const &info) : 
		conn_(0)
	{
		conn_ = mysql_init(0);
		if(!conn_) {
			throw cppdb_error("cppdb::mysql filed to create connection");
		}
		std::string host = ci_.get("host","");
		char const *phost = host.empty() ? 0 : host.c_str();
		std::string user = ci_.get("user","");
		char const *puser = user.empty() ? 0 : user.c_str();
		std::string password = ci_.get("password","");
		char const *ppassword = password.empty() ? 0 : password.c_str();
		std::string database = ci_.get("database","");
		char const *pdatabase = database.empty() ? 0 : database.c_str();
		int port = ci_.get("port",0);
		std::string unix_socket = ci_.get("unix_socket","");
		char const *punix_socket = unix_socket.empty() ? 0 : unix_socket.c_str();
		
		if(!mysql_real_connect(conn_,)) {
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
		if(!mysql_real_query(conn_,"BEGIN",s.size())) {
			throw cppdb_myerror(mysql_error(conn_));
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
		catch(...) {}
	}
	///
	/// Create a prepared statement \a q. May throw if preparation had failed.
	/// Should never return null value.
	///
	virtual statement *real_prepare(std::string const &q)
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

