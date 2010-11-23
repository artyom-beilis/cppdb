#include "backend.h"

#if defined(_WIN32) || defined(__WIN32) || defined(WIN32) 
#include <windows.h>
#endif
#include <sqlext.h>

/*

    ret = SQLPrepare(stmt, "INSERT into test VALUES(?,?)", SQL_NTS) ;
    STMT_CHECK();

    ret = SQLBindParameter(stmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_NUMERIC,0,0,"123",10,0);
    STMT_CHECK();
    SQLBindParameter(stmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_LONGVARBINARY,0,0,"שלום",8,0);
    STMT_CHECK();
    ret = SQLExecute(stmt);
    STMT_CHECK();
    ret = SQLPrepare(stmt, "SELECT id,name FROM test", SQL_NTS) ;
    STMT_CHECK();
    ret = SQLExecute(stmt);
    STMT_CHECK();
    while((ret = SQLFetch(stmt))!=SQL_NO_DATA) {
	    SQLLEN len,len2;
	    SQLGetData(stmt,1,SQL_C_CHAR,0,0,&len);
	    char *buf=malloc(len+1);
	    SQLGetData(stmt,1,SQL_C_CHAR,buf,len+1,&len2);
	    buf[len2]=0;
	    printf("x=[%s]\n",buf);
	    free(buf);
    	    SQLGetData(stmt,2,SQL_C_CHAR,0,0,&len);
	    buf=malloc(len+1);
	    SQLGetData(stmt,2,SQL_C_CHAR,buf,len+1,&len2);
	    buf[len2]=0;
	    printf("y=[%s] of size %d\n",buf,len);
	    free(buf);
   }


*/


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
			throw cppdb_error("cppdb::odbc:: Failed with error `" + std::string((char *)msg)+"'");
		}
		typedef std::basic_string<SQLWCHAR> sqlwstring;
		sqlwstring tosqlwide(std::string const &s)
		{
			return tosqlwide(s.c_str(),s.c_str()+s.size());
		}
		sqlwstring tosqlwide(char const *b,char const *e)
		{
			// FIXME!!!
			return sqlwstring(b,e);
		}


		class result : public ref_counted {
		public:
			// Begin of API
			typedef enum {
				last_row_reached,
				next_row_exists,
				next_row_unknown
			} next_row;

			virtual next_row has_next() = 0;
			virtual bool next() = 0;
			virtual bool fetch(int col,short &v) = 0;
			virtual bool fetch(int col,unsigned short &v) = 0;
			virtual bool fetch(int col,int &v) = 0;
			virtual bool fetch(int col,unsigned &v) = 0;
			virtual bool fetch(int col,long &v) = 0;
			virtual bool fetch(int col,unsigned long &v) = 0;
			virtual bool fetch(int col,long long &v) = 0;
			virtual bool fetch(int col,unsigned long long &v) = 0;
			virtual bool fetch(int col,float &v) = 0;
			virtual bool fetch(int col,double &v) = 0;
			virtual bool fetch(int col,long double &v) = 0;
			virtual bool fetch(int col,std::string &v) = 0;
			virtual bool fetch(int col,std::ostream &v) = 0;
			virtual bool fetch(int col,std::tm &v) = 0;
			virtual bool is_null(int col) = 0;
			virtual int cols() = 0;
			virtual int name_to_column(std::string const &) = 0;
			virtual std::string column_to_name(int) = 0;

			// End of API
			
			result();
			virtual ~result();
		private:
			struct data;
			std::auto_ptr<data> d;
		};

		class statements_cache;

		class statement : public backend::statement {
		public:
			// Begin of API
			virtual void reset()
			{
				SQLFreeStmt(stmt_,SQL_UNBIND);
				params_.resize(0);

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
				parameter &p = param_at(col);
				if(wide_) {
					sqlwstring tmp = tosqlwide(b,e);
					p.wvalue.swap(tmp);
					p.null = false;
					p.sqltype = SQL_WLONGVARCHAR;
				}
				else {
					p.value.assign(b,e-b);
					p.null = false;
					p.sqltype = SQL_LONGVARCHAR;
				} 
			}
			void do_bind(int col,std::string &v,SQLSMALLINT type)
			{
				parameter &p = param_at(col);
				p.null = false;
				p.sqltype = type;
				if(wide_) {
					sqlwstring tmp = tosqlwide(b,e);
					p.wvalue.swap(tmp);
				}
				else {
					p.value.swap(v);
				} 
			}
			virtual void bind(int col,std::tm const &s)
			{
				std::string s=format_time(s);
				do_bind(col,s,SQL_TYPE_TIMESTAMP);
			}
			virtual void bind(int col,std::istream const &in) 
			{
				std::ostringstream ss;
				ss << in.rdbuf();
				if(!wide_) {
					std::string tmp=ss.str();
					bind(col,tmp,SQL_LONGVARCHAR);
				}
				else {
					sqlwstring tmp=tosqlwide(ss.str());
					bind(col,tmp,SQL_WLONGVARCHAR);
				}
					
			}
			template<typename T>
			void do_bind_num(int col,T v)
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss << v;
				std::string tmp=ss.str();
				do_bind(col,tmp,SQL_NUMERIC);
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
				parameter &p = param_at(col);
				p.null = true;
				p.value.clear();
				p.wvalue.clear();
				p.sqltype = SQL_NUMERIC;
			}
			virtual long long sequence_last(std::string const &sequence) = 0;
			virtual unsigned long long affected() = 0;
			virtual result *query() = 0;
			virtual void exec() = 0;
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
							tosqlwide(query).c_str(),
							SQL_NTS);
					else
    						r = SQLPrepareA(
							stmt_,
							query.c_str(),
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

			struct parameter {
				parameter() : 
					null(true),
					sqltype(SQL_C_NUMERIC)
				{
				}
				std::string value;
				sqlwstring wvalue;
				bool null;
				SQLSMALLINT sqltype;
			};

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
				else if(utf_mode = "wide");
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
					check_odbc_error(r,SQL_HANDLE_ENV,env_);
					r = SQLAllocHandle(SQL_HANDLE_DBC,env_,&dbc_);
					check_odbc_error(r,SQL_HANDLE_ENV,env_);
					dbc_created = true;
					if(ci.get("@utf","narrow")=="wide") {
						r = SQLDriverConnectW(dbc_,0,tosqlwide(conn_str(ci)).c_str(),
									SQL_NTS,0,0,0,SQL_DRIVER_COMPLETE);
					}
					else {
						r = SQLDriverConnectA(dbc_,0,conn_str(ci).c_str(),
									SQL_NTS,0,0,0,SQL_DRIVER_COMPLETE);
					}
					check_odbc_error(r,SQL_HANDLE_DBC,dbc_);
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
					pq_str+=p->first;
					pq_str+="=";
					pq_str+=p->second;
					pq_str+=";";
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
				check_odbc_error(r,SQL_HANDLE_DBC,dbc_);
				set_autocommit(true);
			}

			virtual void rollback() 
			{
				try {
					SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,dbc_,SQL_ROLLBACK);
					check_odbc_error(r,SQL_HANDLE_DBC,dbc_);
				}
				catch(...) {}
				try {
					set_autocommit(true);
				}
				catch(...) {}
			}
			virtual statement *real_prepare(std::string const &q)
			{
				return new statement(q,dbc_);
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
		};


	} // backend
} // cppdb

#endif
