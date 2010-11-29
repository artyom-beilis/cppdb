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
	namespace postgresql {
		
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
				unsigned char *val=(unsigned char*)PQgetvalue(res_,current_,col);
				size_t len = 0;
				unsigned char *buf=PQunescapeBytea(val,&len);
				if(!buf) {
					throw bad_value_cast();
				}
				try {
					v.write((char *)buf,len);
				}catch(...) {
					PQfreemem(buf);
					throw;
				}
				PQfreemem(buf);
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
			
			typedef enum {
				null_param,
				text_param,
				binary_param
			} param_type;

			statement(PGconn *conn,std::string const &src_query,unsigned long long prepared_id) :
				res_(0),
				conn_(conn),
				orig_query_(src_query),
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

				if(prepared_id > 0) {

					ss.str("");
					ss<<"cppdb_psqlstmt_" << prepared_id;
					prepared_id_ = ss.str();

					PGresult *r=PQprepare(conn_,prepared_id_.c_str(),query_.c_str(),0,0);
					if(!r)
						throw std::bad_alloc();
					if(PQresultStatus(r)==PGRES_COMMAND_OK) {
						PQclear(r);
					}
					else {
						std::string e = "unknown error";
						try { e = PQresultErrorMessage(r); }catch(...){}
						PQclear(r);
						throw cppdb_error("cppdb::postgresql::statement " + e);
					}
				}
			}
			virtual ~statement()
			{
				try {
					if(res_) {
						PQclear(res_);
						res_ = 0;
					}
					if(!prepared_id_.empty()) {
						std::string stmt = "DEALLOCATE " + prepared_id_;
						res_ = PQexec(conn_,stmt.c_str());
						if(res_)  {
							PQclear(res_);
							res_ = 0;
						}
					}
				}
				catch(...) 
				{
				}
			}
			virtual void reset()
			{
				if(res_) {
					PQclear(res_);
					res_ = 0;
				}
				std::vector<std::string> vals(params_);
				std::vector<param_type> flags(params_,null_param);
				params_values_.swap(vals);
				params_set_.swap(flags);
			}
			virtual void bind(int col,std::string const &v)
			{
				check(col);
				// FIXME
				params_values_[col-1]=v;
				params_set_[col-1]=text_param;
			}
			virtual void bind(int col,char const *s)
			{
				check(col);
				params_values_[col-1]=s;
				params_set_[col-1]=text_param;
			}
			virtual void bind(int col,char const *b,char const *e)
			{
				check(col);
				params_values_[col-1].assign(b,e-b);
				params_set_[col-1]=text_param;
			}
			virtual void bind(int col,std::tm const &v) 
			{
				check(col);
				params_values_[col-1]=format_time(v);
				params_set_[col-1]=text_param;
			}
			virtual void bind(int col,std::istream const &in)
			{
				check(col);
				std::ostringstream ss;
				ss << in.rdbuf();
				params_values_[col-1]=ss.str();
				params_set_[col-1]=binary_param;
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
				params_set_[col-1]=text_param;
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
				params_set_[col-1]=null_param;
				std::string tmp;
				params_values_[col-1].swap(tmp);
			}

			void real_query()
			{
				char const * const *pvalues = 0;
				int *plengths = 0;
				int *pformats = 0;
				std::vector<char const *> values;
				std::vector<int> lengths;
				std::vector<int> formats;
				if(params_>0) {
					values.resize(params_,0);
					lengths.resize(params_,0);
					formats.resize(params_,0);
					for(unsigned i=0;i<params_;i++) {
						if(params_set_[i]!=null_param) {
							values[i]=params_values_[i].c_str();
							lengths[i]=params_values_[i].size();
							if(params_set_[i]==binary_param) {
								formats[i]=1;
							}
						}
					}
					pvalues=&values.front();
					plengths=&lengths.front();
					pformats=&formats.front();
				}
				if(res_) {
					PQclear(res_);
					res_ = 0;
				}
				if(prepared_id_.empty()) {
					res_ = PQexecParams(
						conn_,
						query_.c_str(),
						params_,
						0, // param types
						pvalues,
						plengths,
						pformats, // format - text
						0 // result format - text
						);
				}
				else {
					res_ = PQexecPrepared(
						conn_,
						prepared_id_.c_str(),
						params_,
						pvalues,
						plengths,
						pformats, // format - text
						0 // result format - text
						);
				}
			}

			virtual result *query() 
			{
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
					throw cppdb_error("Query used instread of statement");
					break;
				case PGRES_COMMAND_OK:
					break;
				default:
					throw cppdb_error(PQresultErrorMessage(res_));
				}

			}
			virtual long long sequence_last(std::string const &sequence)
			{
				PGresult *res = 0;
				long long rowid;
				try {
					char const * const param_ptr = sequence.c_str();
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
					rowid = atoll(val);
					
				}
				catch(...) {
					if(res) PQclear(res);
					throw;
				}
				PQclear(res);
				return rowid;
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
			virtual std::string const &sql_query()
			{
				return orig_query_;
			}
		private:
			void check(int col)
			{
				if(col < 1 || col > int(params_))
					throw invalid_placeholder();
			}
			PGresult *res_;
			PGconn *conn_;
			std::string query_;
			std::string orig_query_;
			unsigned params_;
			std::vector<std::string> params_values_;
			std::vector<param_type> params_set_;
			std::string prepared_id_;
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
			virtual statement *prepare_statement(std::string const &q)
			{
				return new statement(conn_,q,++prepared_id_);
			}
			virtual statement *create_statement(std::string const &q)
			{
				return new statement(conn_,q,0);
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
			std::string pq_string(connection_info const &ci)
			{
				std::map<std::string,std::string>::const_iterator p;
				std::string pq_str;
				for(p=ci.properties.begin();p!=ci.properties.end();p++) {
					if(p->first.empty() || p->first[0]=='@')
						continue;
					pq_str+=p->first;
					pq_str+="='";
					pq_str+=escape_for_conn(p->second);
					pq_str+="' ";
				}
				return pq_str;
			}
			std::string escape_for_conn(std::string const &v)
			{
				std::string res;
				res.reserve(v.size());
				for(unsigned i=0;i<v.size();i++) {
					if(v[i]=='\\')
						res+="\\\\";
					else if(v[i]=='\'')
						res+="\\\'";
					else
						res+=v[i];
				}
				return res;
			}
			connection(connection_info const &ci) :
				backend::connection(ci),
				conn_(0),
				prepared_id_(0)
			{
				std::string pq=pq_string(ci);
				conn_ = PQconnectdb(pq.c_str());	
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
			virtual std::string driver()
			{
				return "postgresql";
			}
			virtual std::string engine()
			{
				return "postgresql";
			}
		private:
			PGconn *conn_;
			unsigned long long prepared_id_;
		};


	} // backend
} // cppdb


extern "C" {
	cppdb::backend::connection *cppdb_postgresql_get_connection(cppdb::connection_info const &cs)
	{
		return new cppdb::postgresql::connection(cs);
	}
}
