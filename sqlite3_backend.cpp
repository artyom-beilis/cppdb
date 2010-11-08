#include <sqlite3.h>

#include "backend.h"
#include "errors.h"
#include "utils.h"

#include <sstream>
#include <limits>
#include <iomanip>
#include <map>
#include <stdlib.h>
#include <string.h>

namespace cppdb {
	namespace sqlite3_backend {
		
		class result : public backend::result {
		public:
			result(sqlite3_stmt *st,sqlite3 *conn) : 
				st_(st),
				conn_(conn),
				cols_(-1)
			{
			}
			virtual ~result() 
			{
				st_ = 0;
			}
			virtual next_row has_next()
			{
				return next_row_unknown;
			}
			virtual bool next() 
			{
				int r = sqlite3_step(st_);
				if(r==SQLITE_DONE)
					return false;
				if(r!=SQLITE_ROW) {
					throw cppdb_error(std::string("sqlite3:") + sqlite3_errmsg(conn_));
				}
				if(cols_==-1)
					cols_=sqlite3_column_count(st_);
				return true;
			}
			template<typename T>
			bool do_fetch(int col,T &v)
			{
				if(do_is_null(col))
					return false;
				if(sqlite3_column_type(st_,col)==SQLITE_NULL)
					return false;
				else if(sizeof(T) < sizeof(int)) {
					int rv = sqlite3_column_int(st_,col);
					if(	rv < int(std::numeric_limits<T>::min())
						|| rv >int(std::numeric_limits<T>::max()))
					{
						throw bad_value_cast();
					}
					v=static_cast<T>(rv);
				}
				if(sizeof(T) == sizeof(int) && std::numeric_limits<T>::is_signed) {
					int rv = sqlite3_column_int(st_,col);
					v=static_cast<T>(rv);
				}
				else {
					sqlite3_int64 rv = sqlite3_column_int64(st_,col);
					if(rv < 0 && std::numeric_limits<T>::is_signed==false)
						throw bad_value_cast();
					v=static_cast<T>(rv);
				}
				return true;
			}
	
			virtual bool fetch(int col,short &v) 
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,unsigned short &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,int &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,unsigned &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,long &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,unsigned long &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,long long &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,unsigned long long &v)
			{
				return do_fetch(col,v);
			}
			template<typename T>
			bool do_real_fetch(int col,T &v)
			{
				if(do_is_null(col))
					return false;
				v=static_cast<T>(sqlite3_column_double(st_,col));
				return true;
			}
			virtual bool fetch(int col,float &v) 
			{
				return do_real_fetch(col,v);
			}
			virtual bool fetch(int col,double &v)
			{
				return do_real_fetch(col,v);
			}
			virtual bool fetch(int col,long double &v)
			{
				return do_real_fetch(col,v);
			}
			virtual bool fetch(int col,std::string &v)
			{
				if(do_is_null(col))
					return false;
				char const *txt = (char const *)sqlite3_column_text(st_,col);
				int size = sqlite3_column_bytes(st_,col);
				v.assign(txt,size);
				return true;
			}
			virtual bool fetch(int col,std::ostream &v)
			{
				if(do_is_null(col))
					return false;
				char const *txt = (char const *)sqlite3_column_text(st_,col);
				int size = sqlite3_column_bytes(st_,col);
				v.write(txt,size);
				return true;
			}
			virtual bool fetch(int col,std::tm &v)
			{
				if(do_is_null(col))
					return false;
				v=parse_time((char const *)(sqlite3_column_text(st_,col)));
				return true;
			}
			virtual bool is_null(int col)
			{
				return do_is_null(col);
			}
			virtual int cols() 
			{
				return cols_;
			}
			virtual int name_to_column(std::string const &n)
			{
				if(!column_names_prepared_) {
					for(int i=0;i<cols_;i++) {
						char const *name = sqlite3_column_name(st_,i);
						if(!name) {
							throw std::bad_alloc();
						}
						column_names_[name]=i;
					}
					column_names_prepared_ = true;
				}
				std::map<std::string,int>::const_iterator p=column_names_.find(n);
				if(p==column_names_.end())
					return -1;
				return p->second;
			}
			virtual std::string column_to_name(int col)
			{
				check(col);
				char const *name = sqlite3_column_name(st_,col);
				if(!name) {
					throw std::bad_alloc();
				}
				return name;
			}
		private:
			bool do_is_null(int col)
			{
				check(col);
				return sqlite3_column_type(st_,col)==SQLITE_NULL;
			}
			void check(int col)
			{
				if(col < 0 || col >= cols_)
					throw invalid_column();
			}
			sqlite3_stmt *st_;
			sqlite3 *conn_;
			std::map<std::string,int> column_names_;
			bool column_names_prepared_;
			int cols_;
		};

		class statement : public backend::statement {
		public:
			virtual void reset()
			{
				reset_stat();
				sqlite3_clear_bindings(st_);
				rowid_ = 0;
			}
			void reset_stat()
			{
				if(!reset_) {
					sqlite3_reset(st_);
					reset_=true;
				}
			}
			virtual void bind(int col,std::string const &v) 
			{
				reset_stat();
				check_bind(sqlite3_bind_text(st_,col,v.c_str(),v.size(),SQLITE_STATIC));
			}
			virtual void bind(int col,char const *s)
			{
				reset_stat();
				check_bind(sqlite3_bind_text(st_,col,s,-1,SQLITE_STATIC));
			}
			virtual void bind(int col,char const *b,char const *e) 
			{
				reset_stat();
				check_bind(sqlite3_bind_text(st_,col,b,e-b,SQLITE_STATIC));
			}
			virtual void bind(int col,std::tm const &v)
			{
				reset_stat();
				std::string tmp = format_time(v);
				check_bind(sqlite3_bind_text(st_,col,tmp.c_str(),tmp.size(),SQLITE_TRANSIENT));
			}
			virtual void bind(int col,std::istream const &v) 
			{
				reset_stat();
				// TODO Fix me
				std::ostringstream ss;
				ss<<v.rdbuf();
				std::string tmp = ss.str();
				check_bind(sqlite3_bind_text(st_,col,tmp.c_str(),tmp.size(),SQLITE_TRANSIENT));
			}
			virtual void bind(int col,int v) 
			{
				reset_stat();
				check_bind(sqlite3_bind_int(st_,col,v));
			}
			template<typename IntType>
			void do_bind(int col,IntType value)
			{
				reset_stat();
				int r;
				if(sizeof(value) > sizeof(int) || (long long)(value) > std::numeric_limits<int>::max())
					r = sqlite3_bind_int64(st_,col,static_cast<sqlite3_int64>(value));
				else
					r = sqlite3_bind_int(st_,col,static_cast<int>(value));
				check_bind(r);
			}
			virtual void bind(int col,unsigned v) 
			{
				do_bind(col,v);
			}
			virtual void bind(int col,long v)
			{
				do_bind(col,v);
			}
			virtual void bind(int col,unsigned long v)
			{
				do_bind(col,v);
			}
			virtual void bind(int col,long long v)
			{
				do_bind(col,v);
			}
			virtual void bind(int col,unsigned long long v)
			{
				do_bind(col,v);
			}
			virtual void bind(int col,double v)
			{
				reset_stat();
				check_bind(sqlite3_bind_double(st_,col,v));
			}
			virtual void bind(int col,long double v) 
			{
				reset_stat();
				check_bind(sqlite3_bind_double(st_,col,static_cast<double>(v)));
			}
			virtual void bind_null(int col)
			{
				reset_stat();
				check_bind(sqlite3_bind_null(st_,col));
			}
			virtual void bind_sequence(long long &value,std::string const & /*sequence*/)
			{
				rowid_ = &value;
			}
			virtual result *query()
			{
				if(rowid_) {
					rowid_ = 0;
				}
				reset_stat();
				reset_ = false;
				return new result(st_,conn_);
			}
			virtual void exec()
			{
				reset_stat();
				reset_ = false;
				int r = sqlite3_step(st_);
				if(r!=SQLITE_DONE) {
					rowid_ = 0;
					if(r==SQLITE_ROW) {
						throw cppdb_error("Using exec with query!");
					}
					else 
						check_bind(r);
				}
				if(rowid_) {
					*rowid_ = sqlite3_last_insert_rowid(conn_);
					rowid_ = 0;
				}
			}
			virtual unsigned long long affected()
			{
				return sqlite3_changes(conn_);
			}
			statement(std::string const &query,sqlite3 *conn) :
				rowid_(0),
				st_(0),
				conn_(conn),
				reset_(true)
			{
				if(sqlite3_prepare_v2(conn_,query.c_str(),query.size(),&st_,0)!=SQLITE_OK)
					throw cppdb_error(sqlite3_errmsg(conn_));
			}
			~statement()
			{
				sqlite3_finalize(st_);
			}

		private:
			void check_bind(int v)
			{
				if(v==SQLITE_RANGE) {
					throw invalid_placeholder(); 
				}
				if(v!=SQLITE_OK) {
					throw cppdb_error(sqlite3_errmsg(conn_));
				}
			}
			long long *rowid_;
			sqlite3_stmt *st_;
			sqlite3 *conn_;
			bool reset_;
		};
		class connection : public backend::connection {
		public:
			connection(std::string const &dbname) :
				conn_(0)
			{
				if(sqlite3_open(dbname.c_str(),&conn_)!=SQLITE_OK) {
					if(conn_ == 0) {
						throw cppdb_error("sqlite3:failed to create db object");
					}
					std::string error_message;
					try { error_message = sqlite3_errmsg(conn_); }catch(...){}
					sqlite3_close(conn_);
					conn_ = 0;
					throw cppdb_error("sqlite3:Failed to open connection:" + error_message);
				}
			}
			virtual ~connection() 
			{
				sqlite3_close(conn_);
			}
			virtual void begin()
			{
				fast_exec("begin");	
			}
			virtual void commit() 
			{
				fast_exec("commit");
			}
			virtual void rollback()
			{
				fast_exec("rollback");
			}
			virtual statement *prepare(std::string const &s)
			{
				return new statement(s,conn_);
			}
			virtual std::string escape(std::string const &s)
			{
				return escape(s.c_str(),s.c_str()+s.size());
			}
			virtual std::string escape(char const *s)
			{
				return escape(s,s+strlen(s));
			}
			virtual std::string escape(char const *b,char const *e)
			{
				std::string result;
				result.reserve(e-b);
				for(;b!=e;b++) {
					char c=*b;
					if(c=='\'')
						result+="''";
					else
						result+=c;
				}
				return result;
			}
		private:
			void fast_exec(char const *query)
			{
				if(sqlite3_exec(conn_,query,0,0,0)!=SQLITE_OK) {
					throw cppdb_error(std::string("sqlite3:") + sqlite3_errmsg(conn_));
				}
			}

			sqlite3 *conn_;
		};
		class driver : public backend::driver {
		public:
			virtual ~driver() {}
			virtual std::string name() { return "sqlite3"; }
			virtual connection *open(std::string const &connection_string) 
			{
				return new connection(connection_string);
			}
		};

	} // sqlite3_backend
} // cppdb

extern "C" {
	cppdb::backend::driver *cppdb_sqlite3_get_driver()
	{
		static cppdb::sqlite3_backend::driver drv;
		return &drv;
	}
}
