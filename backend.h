#ifndef CPPDB_BACKEND_H
#define CPPDB_BACKEND_H
#include <iosfwd>
#include <ctime>
#include <string>
#include <memory>
#include <map>
#include "errors.h"
#include "ref_ptr.h"

namespace cppdb {
	namespace backend {	

		class result : public ref_counted {
		public:
			typedef enum {
				last_row_reached,
				next_row_exists,
				next_row_unknown
			} next_row;

			virtual ~result() {}
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
		};

		class statements_cache;

		class statement : public ref_counted {
		public:
			virtual ~statement() 
			{
			}
			static void dispose(statement *selfp);
			virtual void reset() = 0;
			virtual std::string const &sql_query() = 0;
			virtual void bind(int col,std::string const &) = 0;
			virtual void bind(int col,char const *s) = 0;
			virtual void bind(int col,char const *b,char const *e) = 0;
			virtual void bind(int col,std::tm const &) = 0;
			virtual void bind(int col,std::istream const &) = 0;
			virtual void bind(int col,int v) = 0;
			virtual void bind(int col,unsigned v) = 0;
			virtual void bind(int col,long v) = 0;
			virtual void bind(int col,unsigned long v) = 0;
			virtual void bind(int col,long long v) = 0;
			virtual void bind(int col,unsigned long long v) = 0;
			virtual void bind(int col,double v) = 0;
			virtual void bind(int col,long double v) = 0;
			virtual void bind_null(int col) = 0;
			virtual long long sequence_last(std::string const &sequence) = 0;
			virtual unsigned long long affected() = 0;
			virtual result *query() = 0;
			virtual void exec() = 0;
		protected:
			statement(statements_cache *sc = 0) : cache_(sc)
			{
			}
			statements_cache *cache_;
		};
		
		class statements_cache {
			statements_cache(statements_cache const &);
			void operator=(statements_cache const &);
		public:
			statements_cache()
			{
			}

			void put(statement *p_in)
			{
				std::auto_ptr<statement> ptr(p_in);
				std::map<std::string,statement *>::iterator p;
				if((p=cache_.find(ptr->sql_query()))==cache_.end()) {
					statement *&ptr_ref = cache_[ptr->sql_query()];
					ptr_ref = ptr.release();
				}
				else {
					delete p->second;
					p->second = ptr.release();
				}
			}
			statement *fetch(std::string const &q)
			{
				std::map<std::string,statement *>::iterator p;
				p=cache_.find(q);
				if(p==cache_.end())
					return 0;
				std::auto_ptr<statement> st(p->second);
				cache_.erase(p);
				return st.release();
			}
			~statements_cache()
			{
				std::map<std::string,statement *>::iterator p;
				for(p=cache_.begin();p!=cache_.end();++p) {
					delete p->second;
				}
			}
		private:
			std::map<std::string,statement *> cache_;
		};

		inline void statement::dispose(statement *ptr)
		{
			std::auto_ptr<statement> p(ptr);
			if(!p.get())
				return;
			statements_cache *cache = p->cache_;
			if(cache) {
				p->reset();
				cache->put(p.release());
			}
		}

		class connection;

		class driver : public mt_ref_counted {
		public:
			virtual ~driver() {}
			virtual bool in_use() = 0;
			virtual connection *open(std::string const &connection_string) = 0;
			virtual connection *connect(std::string const &connection_string)
			{
				return open(connection_string);
			}
		};
		
		class loadable_driver : public driver {
		public:
			virtual ~loadable_driver() {}
			virtual connection *connect(std::string const &cs);
		};

		class connection : public ref_counted {
		public:
			void set_driver(ref_ptr<loadable_driver> drv) 
			{
				driver_ = drv;
			}
			virtual ~connection() {}
			virtual void begin() = 0;
			virtual void commit() = 0;
			virtual void rollback() = 0;
			virtual statement *prepare(std::string const &q) = 0;
			virtual std::string escape(std::string const &) = 0;
			virtual std::string escape(char const *s) = 0;
			virtual std::string escape(char const *b,char const *e) = 0;
			virtual std::string name() = 0;
		private:
			ref_ptr<loadable_driver> driver_;
		};

		inline connection *loadable_driver::connect(std::string const &cs)
		{
			connection *c = open(cs);
			c->set_driver(ref_ptr<loadable_driver>(this));
			return c;
		}


	} // backend
} // cppdb

#endif
