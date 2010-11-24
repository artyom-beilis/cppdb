#include "backend.h"
#include "utils.h"
#include <list>
#include <vector>
#include <sstream>
#include <limits>
#include <iomanip>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32) || defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif
#include <sqlext.h>

namespace cppdb {

	class connection_info;
	class pool;

	namespace odbc_backend {

		void check_odbc_error(SQLRETURN error,SQLHANDLE h,SQLSMALLINT type)
		{
			if(SQL_SUCCEEDED(error))
				return;
			SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH + 2] = {0};
			SQLCHAR stat[SQL_SQLSTATE_SIZE + 1] = {0};
			SQLINTEGER err;
			SQLSMALLINT len;
			SQLGetDiagRec(type,h,1,stat,&err,msg,sizeof(msg),&len);
			throw cppdb_error("cppdb::odbc::Failed with error `" + std::string((char *)msg)+"'");
		}

		std::string widen(std::string const &s)
		{
			throw not_supported_by_backend("not supported for now");
		}

		std::string widen(char const *b,char const *e)
		{
			throw not_supported_by_backend("not supported for now");
		}

		std::string narrower(std::string const &wide)
		{
			throw not_supported_by_backend("not supported for now");
		}
		
		std::basic_string<SQLWCHAR> tosqlwide(std::string const &n)
		{
			throw not_supported_by_backend("not supported for now");
		}


		class result : public backend::result {
		public:
			typedef std::pair<bool,std::string> cell_type;
			typedef std::vector<cell_type> row_type;
			typedef std::list<row_type> rows_type;
			
			virtual next_row has_next()
			{
				rows_type::iterator p=current_;
				if(p == rows_.end() || ++p==rows_.end())
					return last_row_reached;
				else
					return next_row_exists;
			}
			virtual bool next() 
			{
				if(started_ == false) {
					current_ = rows_.begin();
					started_ = true;
				}
				else if(current_!=rows_.end()) {
					++current_;
				}
				return current_!=rows_.end();
			}
			template<typename T>
			bool do_fetch(int col,T &v)
			{
				if(at(col).first)
					return false;
				std::istringstream ss;
				ss.imbue(std::locale::classic());
				ss.str(at(col).second);
				ss >> v;
				if(!ss || !ss.eof()) {
					throw bad_value_cast();
				}
				return true;
			}
			virtual bool fetch(int col,short &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,unsigned short &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,int &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,unsigned &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,long &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,unsigned long &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,long long &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,unsigned long long &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,float &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,double &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,long double &v)
			{
				return  do_fetch(col,v);	
			}
			virtual bool fetch(int col,std::string &v)
			{
				if(at(col).first)
					return false;
				v=at(col).second;
				return true;
			}
			virtual bool fetch(int col,std::ostream &v) 
			{
				if(at(col).first)
					return false;
				v << at(col).second;
				return true;
			}
			virtual bool fetch(int col,std::tm &v)
			{
				if(at(col).first)
					return false;
				v = parse_time(at(col).second);
				return true;
			}
			virtual bool is_null(int col)
			{
				return at(col).first;
			}
			virtual int cols()
			{
				if(rows_.begin()!=rows_.end())
					return rows_.begin()->size();
				return 0;
			}
			virtual int name_to_column(std::string const &) 
			{
				throw not_supported_by_backend("not supported for now");
			}
			virtual std::string column_to_name(int) 
			{
				throw not_supported_by_backend("not supported for now");
			}
			
			result(rows_type &rows)
			{
				rows_.swap(rows);
				started_ = false;
				current_ = rows_.end();
			}
			cell_type &at(int col)
			{
				if(current_!=rows_.end() && col >= 0 && col <int(current_->size()))
					return current_->at(col);
				throw invalid_column();
			}
		private:
			bool started_;
			rows_type::iterator current_;
			rows_type rows_;
		};

		class statements_cache;

		class statement : public backend::statement {
			struct parameter {
				parameter() : 
					null(true),
					ctype(SQL_C_CHAR),
					sqltype(SQL_C_NUMERIC)
				{
				}
				void set(char const *b,char const *e,bool wide,int sqt)
				{
					if(!wide) {
						value.assign(b,e-b);
						null=false;
						ctype=SQL_C_CHAR;
						sqltype = sqt;
					}
					else {
						std::string tmp = widen(b,e);
						null=false;
						ctype=SQL_C_WCHAR;
						sqltype = sqt;
					}
				}
				void set(std::string &s,bool wide,int sqt)
				{
					if(!wide) {
						value.swap(s);
						null=false;
						ctype=SQL_C_CHAR;
						sqltype = sqt;
					}
					else {
						std::string tmp = widen(s);
						null=false;
						ctype=SQL_C_WCHAR;
						sqltype = sqt;
					}
				}
				void set(char const *b,char const *e,bool wide=false)
				{
					if(!wide) {
						set(b,e,false,SQL_LONGVARCHAR);
					}
					else {
						set(b,e,true,SQL_WLONGVARCHAR);
					}
				}
				void set(std::tm const &v,bool wide=false)
				{
					std::string s = format_time(v);
					set(s,wide,SQL_C_TIMESTAMP);
				}
				template<typename T>
				void set(T v,bool wide=false)
				{
					std::ostringstream ss;
					ss.imbue(std::locale::classic());
					if(!std::numeric_limits<T>::is_integer)
						ss << std::setprecision(std::numeric_limits<T>::digits10+1);
					ss << v;
					std::string sv=ss.str();
					if(std::numeric_limits<T>::is_integer)
						set(sv,wide,SQL_INTEGER);
					else
						set(sv,wide,SQL_DOUBLE);
				}
				void bind(int col,SQLHSTMT stmt)
				{
					int r;
					if(null) {
						SQLLEN len = SQL_NULL_DATA;
						r = SQLBindParameter(	stmt,
									col,
									SQL_PARAM_INPUT,
									SQL_C_CHAR, // C type C_CHAR or C_WCHAR
									SQL_NUMERIC, // for null
									value.size(), // COLUMNSIZE
									0, //  Presision
									0, // string
									0, // size
									&len);
					}
					else {
						r = SQLBindParameter(	stmt,
									col,
									SQL_PARAM_INPUT,
									ctype, // C type C_CHAR or C_WCHAR
									sqltype,
									value.size(), // COLUMNSIZE
									0, //  Presision
									(SQLCHAR*)value.c_str(), // string
									value.size(),
									0);
					}

					check_odbc_error(r,stmt,SQL_HANDLE_STMT);
				}

				std::string value;
				bool null;
				SQLSMALLINT ctype;
				SQLSMALLINT sqltype;
			};
		public:
			// Begin of API
			virtual void reset()
			{
				SQLFreeStmt(stmt_,SQL_UNBIND);
				params_.resize(0);
			}
			parameter &param_at(int col)
			{
				if(col < 0)
					throw invalid_placeholder();
				if(params_.size() < size_t(col))
					params_.resize(col);
				return params_[col - 1];
			}
			virtual std::string const &sql_query()
			{
				return query_;
			}
			virtual void bind(int col,std::string const &s)
			{
				bind(col,s.c_str(),s.c_str()+s.size());
			}
			virtual void bind(int col,char const *s)
			{
				bind(col,s,s+strlen(s));
			}
			virtual void bind(int col,char const *b,char const *e)
			{
				param_at(col).set(b,e,wide_);
			}
			virtual void bind(int col,std::tm const &s)
			{
				param_at(col).set(s,wide_);
			}
			virtual void bind(int col,std::istream const &in) 
			{
				std::ostringstream ss;
				ss << in.rdbuf();
				bind(col,ss.str());
			}
			template<typename T>
			void do_bind_num(int col,T v)
			{
				param_at(col).set(v,wide_);
			}
			virtual void bind(int col,int v) 
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,unsigned v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,long v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,unsigned long v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,long long v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,unsigned long long v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,double v)
			{
				do_bind_num(col,v);
			}
			virtual void bind(int col,long double v)
			{
				do_bind_num(col,v);
			}
			virtual void bind_null(int col)
			{
				param_at(col) = parameter();
			}
			void bind_all()
			{
				for(unsigned i=0;i<params_.size();i++) {
					params_[i].bind(i+1,stmt_);
				}

			}
			virtual long long sequence_last(std::string const &sequence) 
			{
				throw not_supported_by_backend("cppdb::odbc::sequence_last is not supported by odbc backend");
			}
			virtual unsigned long long affected() 
			{
				SQLLEN rows = 0;
				int r = SQLRowCount(stmt_,&rows);
				check_error(r);
				return rows;
			}
			virtual result *query()
			{
				bind_all();
				int r=SQLExecute(stmt_);
				check_error(r);
				char buf[1024];
				result::rows_type rows;
				result::row_type row;
				std::string value;
				bool is_null = false;
				while((r=SQLFetch(stmt_))==SQL_SUCCESS || r==SQL_SUCCESS_WITH_INFO) {
					row.reserve(100);
					for(int col=1;;col++) {
						SQLLEN len = 0;
						value.clear();
						is_null=false;
						int type = wide_ ? SQL_C_WCHAR : SQL_C_CHAR;
						int r = SQLGetData(stmt_,col,type,buf,sizeof(buf),&len);
						if(r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
							if(len == SQL_NULL_DATA) {
								is_null = true;
							}
							else if(len == SQL_NO_TOTAL) {
								for(;;) {
									if(!wide_)
										value.append(buf,sizeof(buf)-1);
									else
										value.append(buf,sizeof(buf)-2);
									r= SQLGetData(stmt_,col,type,buf,sizeof(buf),&len);
									if(len == SQL_NO_TOTAL)
										continue;
									value.append(buf,len);
								}

							}
							else if(len > SQLLEN(sizeof(buf))) {
								if(!wide_)
									value.append(buf,sizeof(buf)-1);
								else
									value.append(buf,sizeof(buf)-2);
								std::vector<char> all(len-sizeof(buf)+2);
								r = SQLGetData(stmt_,col,type,&all.front(),all.size(),&len);
								if(len > 0)
									value.append(&all.front(),len);
							}
							else if(len >= 0)// len <= sizeof(buf)) 
							{
								value.assign(buf,len);
							}
							else {
								throw cppdb_error("cppdb::odbc::iternal error");
							}
						}
						else {
							break;
						}
						row.resize(row.size()+1);
						row.back().first = is_null;
						if(wide_) {
							std::string tmp=narrower(value);
							value.swap(tmp);
						}
						row.back().second.swap(value);
					}
					rows.push_back(result::row_type());
					rows.back().swap(row);
				}
				if(r!=SQL_NO_DATA) {
					check_error(r);
				}
				return new result(rows);
			}
			virtual void exec()
			{
				bind_all();
				int r=SQLExecute(stmt_);
				check_error(r);
			}
			// End of API

			statement(std::string const &q,SQLHDBC dbc,bool wide) :
				wide_(wide),
				query_(q)
			{
				bool stmt_created = false;
				SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,dbc,&stmt_);
				check_odbc_error(r,dbc,SQL_HANDLE_DBC);
				stmt_created = true;
				try {
					if(wide_) 
    						r = SQLPrepareW(
							stmt_,
							(SQLWCHAR*)tosqlwide(query_).c_str(),
							SQL_NTS);
					else
    						r = SQLPrepareA(
							stmt_,
							(SQLCHAR*)query_.c_str(),
							SQL_NTS);
					check_error(r);
					params_.reserve(100);
				}
				catch(...) {
					SQLFreeHandle(SQL_HANDLE_STMT,stmt_);
					throw;
				}
			}
			~statement()
			{
				SQLFreeHandle(SQL_HANDLE_STMT,stmt_);
			}
		private:
			void check_error(int code)
			{
				check_odbc_error(code,stmt_,SQL_HANDLE_STMT);
			}


			SQLHSTMT stmt_;
			bool wide_;
			std::string query_;
			std::vector<parameter> params_;
		};
		
		class connection : public backend::connection {
		public:

			connection(connection_info const &ci) :
				backend::connection(ci)
			{
				std::string utf_mode = ci.get("@utf","narrow");
				
				if(utf_mode == "narrow")
					wide_ = false;
				else if(utf_mode == "wide")
					wide_ = true;
				else
					throw cppdb_error("cppdb::odbc:: @utf property can be either 'narrow' or 'wide'");

				bool env_created = false;
				bool dbc_created = false;
				bool dbc_connected = false;

				try {
					SQLRETURN r = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&env_);
					if(!SQL_SUCCEEDED(r)) {
						throw cppdb_error("cppdb::odbc::Failed to allocate environment handle");
					}
					env_created = true;
					r = SQLSetEnvAttr(env_,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3, 0);
					check_odbc_error(r,env_,SQL_HANDLE_ENV);
					r = SQLAllocHandle(SQL_HANDLE_DBC,env_,&dbc_);
					check_odbc_error(r,env_,SQL_HANDLE_ENV);
					dbc_created = true;
					if(ci.get("@utf","narrow")=="wide") {
						r = SQLDriverConnectW(dbc_,0,
									(SQLWCHAR*)tosqlwide(conn_str(ci)).c_str(),
									SQL_NTS,0,0,0,SQL_DRIVER_COMPLETE);
					}
					else {
						r = SQLDriverConnectA(dbc_,0,
									(SQLCHAR*)conn_str(ci).c_str(),
									SQL_NTS,0,0,0,SQL_DRIVER_COMPLETE);
					}
					check_odbc_error(r,dbc_,SQL_HANDLE_DBC);
					set_autocommit(true);
				}
				catch(...) {
					if(dbc_connected)
						SQLDisconnect(dbc_);
					if(dbc_created)
						SQLFreeHandle(SQL_HANDLE_DBC,dbc_);
					if(env_created)
						SQLFreeHandle(SQL_HANDLE_ENV,env_);
					throw;
				}
			}

			std::string conn_str(connection_info const &ci)
			{
				std::map<std::string,std::string>::const_iterator p;
				std::string str;
				for(p=ci.properties.begin();p!=ci.properties.end();p++) {
					if(p->first.empty() || p->first[0]=='@')
						continue;
					str+=p->first;
					str+="=";
					str+=p->second;
					str+=";";
				}
				return str;
			}
			
			~connection()
			{
				SQLDisconnect(dbc_);
				SQLFreeHandle(SQL_HANDLE_DBC,dbc_);
				SQLFreeHandle(SQL_HANDLE_ENV,env_);
			}

			/// API 
			virtual void begin()
			{
				set_autocommit(false);
			}
			virtual void commit()
			{
				SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,dbc_,SQL_COMMIT);
				check_odbc_error(r,dbc_,SQL_HANDLE_DBC);
				set_autocommit(true);
			}

			virtual void rollback() 
			{
				try {
					SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,dbc_,SQL_ROLLBACK);
					check_odbc_error(r,dbc_,SQL_HANDLE_DBC);
				}
				catch(...) {}
				try {
					set_autocommit(true);
				}
				catch(...) {}
			}
			virtual statement *real_prepare(std::string const &q)
			{
				return new statement(q,dbc_,wide_);
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
				throw not_supported_by_backend("cppcms::odbc:: string escaping is not supported");
			}
			virtual std::string name() 
			{
				return "odbc";
			}

			void set_autocommit(bool on)
			{
				SQLPOINTER mode = (SQLPOINTER)(on ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF);
				SQLRETURN r = SQLSetConnectAttr(
							dbc_, // handler
							SQL_ATTR_AUTOCOMMIT, // option
							mode, //value
							0);
				check_odbc_error(r,dbc_,SQL_HANDLE_DBC);
			}

		private:
			SQLHENV env_;
			SQLHDBC dbc_;
			bool wide_;
		};


	} // odbc_backend
} // cppdb

extern "C" {
	cppdb::backend::connection *cppdb_odbc_get_connection(cppdb::connection_info const &cs)
	{
		return new cppdb::odbc_backend::connection(cs);
	}
}
