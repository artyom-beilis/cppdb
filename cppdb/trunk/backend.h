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
	class connection_info;
	class pool;

	namespace backend {	

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
			
			virtual ~result() {}
		};

		class statements_cache;

		class statement : public ref_counted {
		public:
			// Begin of API
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
			// End of API

			// Caching support
			static void dispose(statement *selfp);

			void cache(statements_cache *c)
			{
				cache_ = c;
			}
			statement() : 
				cache_(0)
			{
			}
			virtual ~statement() 
			{
			}
		protected:
			statements_cache *cache_;
		};
		
		class statements_cache {
			statements_cache(statements_cache const &);
			void operator=(statements_cache const &);
		public:
			statements_cache();
			bool active();
			void set_size(size_t n);
			void put(statement *p_in);
			ref_ptr<statement> fetch(std::string const &q);
			~statements_cache();
		private:
			struct data;
			std::auto_ptr<data> d;
		};


		class connection;

		class driver : public ref_counted {
		public:
			virtual ~driver() {}
			virtual bool in_use() = 0;
			virtual connection *open(connection_info const &cs) = 0;
			virtual connection *connect(connection_info const &cs)
			{
				return open(cs);
			}
		};
		
		class loadable_driver : public driver {
		public:
			virtual ~loadable_driver() {}
			virtual connection *connect(connection_info const &cs);
		};

		class connection : public ref_counted {
		public:
			connection(connection_info const &info);
			void set_driver(ref_ptr<loadable_driver> drv) 
			{
				driver_ = drv;
			}
			static void dispose(connection *c);
			virtual ~connection() {}
			virtual void begin() = 0;
			virtual void commit() = 0;
			virtual void rollback() = 0;
			virtual statement *real_prepare(std::string const &q) = 0;
			virtual std::string escape(std::string const &) = 0;
			virtual std::string escape(char const *s) = 0;
			virtual std::string escape(char const *b,char const *e) = 0;
			virtual std::string name() = 0;
			
			ref_ptr<statement> prepare(std::string const &q);

			void recycle_pool(pool *p)
			{
				pool_ = p;
			}
		private:
			statements_cache cache_;
			ref_ptr<loadable_driver> driver_;
			pool *pool_;
		};

		inline connection *loadable_driver::connect(connection_info const &cs)
		{
			connection *c = open(cs);
			c->set_driver(ref_ptr<loadable_driver>(this));
			return c;
		}

	} // backend
} // cppdb

#endif
