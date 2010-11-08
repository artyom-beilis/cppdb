#include <postgresql/libpq-fe.h>
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
	namespace postgres {
		
		class result : public backend::result {
		public:
			result(PGresult *res) :
				res_(res),
				rows_(PQntuples(res)),
				cols_(PQnfields(res)),
				current_(-1)
			{
			}
			virtual ~result() 
			{
				PQclear(res_);
			}
			virtual next_row has_next()
			{
				if(current_ + 1 < rows_)
					return next_row_exists;
				else
					return last_row_reached; 

			}
			virtual bool next() 
			{
				current_ ++;
				if(current_ < rows_) {
					return true;
				}
				return false;
			}

			template<typename T>
			bool do_fetch(int col,T &v)
			{
				if(do_isnull(col))
					return false;
				std::istringstream ss;
				ss.imbue(std::locale::classic());
				ss.str(PQgetvalue(res_,current_,col));
				ss >> v;
				if(!ss || !ss.eof())
					throw bad_value_cast();
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
			virtual bool fetch(int col,float &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,double &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,long double &v)
			{
				return do_fetch(col,v);
			}
			virtual bool fetch(int col,std::string &v)
			{
				if(do_isnull(col))
					return false;
				v.assign(PQgetvalue(res_,current_,col),PQgetlength(res_,current_,col));
				return true;
			}
			virtual bool fetch(int col,std::ostream &v)
			{
				if(do_isnull(col))
					return false;
				v.write(PQgetvalue(res_,current_,col),PQgetlength(res_,current_,col));
				return true;
			}
			virtual bool fetch(int col,std::tm &v)
			{
				if(do_isnull(col))
					return false;
				v=parse_time(PQgetvalue(res_,current_,col));
				return true;
			}
			virtual bool is_null(int col)
			{
				return do_isnull(col);
			}
			virtual int cols() 
			{
				return cols_;
			}
			virtual int name_to_column(std::string const &n) 
			{
				return PQfnumber(res_,n.c_str());
			}
			virtual std::string column_to_name(int pos)
			{
				char const *name = PQfname(res_,pos);
				if(!name)
					return std::string();
				return name;
			}
		private:

			void check(int c)
			{
				if(c < 0 || c>= cols_)
					throw invalid_column();
			}
			bool do_isnull(int col)
			{
				check(col);
				return PQgetisnull(res_,current_,col);
			}
			PGresult *res_;
			int rows_;
			int cols_;
			int current_;
		};

		class statement : public backend::statement {
		public:
			statement(PGconn *conn,std::string const &src_query) :
				res_(0),
				conn_(conn),
				insert_id_(0),
				params_(0)
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());

				query_.reserve(src_query.size());
				bool inside_string=false;
				for(unsigned i=0;i<src_query.size();i++) {
					char c=src_query[i];
					if(c=='\'') {
						inside_string = !inside_string;
					}
					if(!inside_string && c=='?') {
						query_+='$';
						params_++;
						ss<<params_;
						query_+=ss.str();
						ss.str("");
					}
					else {
						query_+=c;
					}
				}
				reset();
			}
			virtual ~statement()
			{
				if(res_)
					PQclear(res_);
			}
			virtual void reset()
			{
				if(res_) {
					PQclear(res_);
					res_ = 0;
				}
				std::vector<std::string> vals(params_);
				std::vector<bool> flags(params_,false);
				params_values_.swap(vals);
				params_set_.swap(flags);
			}
			virtual void bind(int col,std::string const &v)
			{
				check(col);
				params_values_[col-1]=v;
				params_set_[col-1]=true;
			}
			virtual void bind(int col,char const *s)
			{
				check(col);
				params_values_[col-1]=s;
				params_set_[col-1]=true;
			}
			virtual void bind(int col,char const *b,char const *e)
			{
				check(col);
				params_values_[col-1].assign(b,e-b);
				params_set_[col-1]=true;
			}
			virtual void bind(int col,std::tm const &v) 
			{
				check(col);
				params_values_[col-1]=format_time(v);
				params_set_[col-1]=true;
			}
			virtual void bind(int col,std::istream const &in)
			{
				check(col);
				std::ostringstream ss;
				ss << in.rdbuf();
				params_values_[col-1]=ss.str();
				params_set_[col-1]=true;
			}
			
			template<typename T>
			void do_bind(int col,T v)
			{
				check(col);
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				if(!std::numeric_limits<T>::is_integer)
					ss << std::setprecision(std::numeric_limits<T>::digits10+1);
				ss << v;
				params_values_[col-1]=ss.str();
				params_set_[col-1]=true;
			}

			virtual void bind(int col,int v)
			{
				do_bind(col,v);
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
				do_bind(col,v);
			}
			virtual void bind(int col,long double v)
			{
				do_bind(col,v);
			}
			virtual void bind_null(int col)
			{
				check(col);
				params_set_[col-1]=false;
				std::string tmp;
				params_values_[col-1].swap(tmp);
			}

			void real_query()
			{
				char const * const *pvalues = 0;
				int *plengths = 0;
				std::vector<char const *> values;
				std::vector<int> lengths;
				if(params_>0) {
					values.resize(params_,0);
					lengths.resize(params_,0);
					for(unsigned i=0;i<params_;i++) {
						if(params_set_[i]) {
							values[i]=params_values_[i].c_str();
							lengths[i]=params_values_[i].size();
						}
					}
					pvalues=&values.front();
					plengths=&lengths.front();
				}
				if(res_) {
					PQclear(res_);
					res_ = 0;
				}
				res_ = PQexecParams(
					conn_,
					query_.c_str(),
					params_,
					0, // param types
					pvalues,
					plengths,
					0, // format - text
					0 // result format - text
					);
			}

			virtual result *query() 
			{
				insert_id_ = 0;
				real_query();
				switch(PQresultStatus(res_)){
				case PGRES_TUPLES_OK:
					{
						result *ptr = new result(res_);
						res_ = 0;
						return ptr;
					}
					break;
				case PGRES_COMMAND_OK:
					throw cppdb_error("Statement used instread of query");
					break;
				default:
					throw cppdb_error(PQresultErrorMessage(res_));
				}
			}
			virtual void exec() 
			{
				real_query();
				switch(PQresultStatus(res_)){
				case PGRES_TUPLES_OK:
					insert_id_ = 0;
					throw cppdb_error("Query used instread of statement");
					break;
				case PGRES_COMMAND_OK:
					read_insert_id();
					break;
				default:
					insert_id_ = 0;
					throw cppdb_error(PQresultErrorMessage(res_));
				}

			}
			void read_insert_id()
			{
				if(insert_id_ == 0)
					return;
				PGresult *res = 0;
				try {
					char const * const param_ptr = sequence_.c_str();
					res = PQexecParams(	conn_,
								"SELECT currval($1)",
								1, // 1 param
								0, // types
								&param_ptr, // param values
								0, // lengths
								0, // formats
								0 // string format
								);
					if(PQresultStatus(res) != PGRES_TUPLES_OK) {
						throw cppdb_error(PQresultErrorMessage(res));
					}
					char const *val = PQgetvalue(res,0,0);
					if(!val || *val==0)
						throw cppdb_error("Failed to get sequecne id");
					*insert_id_ = atoll(val);
					
				}
				catch(...) {
					insert_id_ = 0;
					if(res) PQclear(res);
					throw;
				}
				insert_id_ = 0;
				PQclear(res);
			}
			virtual unsigned long long affected() 
			{
				if(res_) {
					char const *s=PQcmdTuples(res_);
					if(!s || !*s)
						return 0;
					return atoll(s);
				}
				return 0;
			}
			virtual void bind_sequence(long long &value,std::string const &sequence)
			{
				insert_id_ = &value;
				sequence_ = sequence;
			}
		private:
			void check(int col)
			{
				if(col < 1 || col > int(params_))
					throw invalid_placeholder();
			}
			PGresult *res_;
			PGconn *conn_;
			long long *insert_id_;
			std::string sequence_;
			std::string query_;
			unsigned params_;
			std::vector<std::string> params_values_;
			std::vector<bool> params_set_;
		};

		class connection : public backend::connection {
		public:
			void do_simple_exec(char const *s)
			{
				PGresult *r=PQexec(conn_,s);
				try {
					
				}
				catch(...) {
					PQclear(r);
					throw;
				}
				PQclear(r);
			}
			virtual void begin()
			{
				do_simple_exec("begin");
			}
			virtual void commit() 
			{
				do_simple_exec("commit");
			}
			virtual void rollback()
			{
				try {
					do_simple_exec("rollback");
				}
				catch(...) {}
			}
			virtual statement *prepare(std::string const &s)
			{
				return new statement(conn_,s);
			}
			std::string do_escape(char const *b,size_t length)
			{
				std::vector<char> buf(2*length+1);
				size_t len = PQescapeStringConn(conn_,&buf.front(),b,length,0);
				return std::string(&buf.front(),len);
			}
			virtual std::string escape(std::string const &s)
			{
				return do_escape(s.c_str(),s.size());
			}
			virtual std::string escape(char const *s)
			{
				return do_escape(s,strlen(s));
			}
			virtual std::string escape(char const *b,char const *e) 
			{
				return do_escape(b,e-b);
			}
			connection(std::string const &conn_str)
			{
				conn_ = PQconnectdb(conn_str.c_str());	
				if(!conn_) {
					throw cppdb_error("postgresql::connection failed to create connection object");
				}
				if(PQstatus(conn_)!=CONNECTION_OK) {
					std::string message;
					try { message = PQerrorMessage(conn_); } catch(...) {}
					PQfinish(conn_);
					throw cppdb_error("postgresql::connection failed:" + message);
				}
			}
			virtual ~connection()
			{
				PQfinish(conn_);
			}
		private:
			PGconn *conn_;
		};

		class driver : public backend::driver {
		public:
			virtual ~driver() {}
			virtual std::string name() 
			{
				return "postgres";
			}
			virtual connection *open(std::string const &connection_string)
			{
				return new connection(connection_string);
			}
		};

	} // backend
} // cppdb


extern "C" {
	cppdb::backend::driver *cppdb_postgres_get_driver()
	{
		static cppdb::postgres::driver drv;
		return &drv;
	}
}
