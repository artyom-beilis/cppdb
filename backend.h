#ifndef CPPDB_BACKEND_H
#define CPPDB_BACKEND_H
#include <iosfwd>
#include <ctime>
#include <string>
#include <memory>
#include <map>
#include "errors.h"

namespace cppdb {
	namespace backend {

		template<typename T>
		class ref_ptr {
		public:
			ref_ptr(T *v=0) : p(0)
			{
				reset(v);
			}
			~ref_ptr()
			{
				reset();
			}
			ref_ptr(ref_ptr const &other) : p(0)
			{
				reset(other.p);
			}
			ref_ptr const &operator=(ref_ptr const &other)
			{
				reset(other.p);
				return *this;
			}
			T *get() const
			{
				return p;
			}
			operator bool() const
			{
				return p!=0;
			}
			T *operator->() const
			{
				if(!p)
					throw cppdb_error("Attempt to access an empty object");
				return p;
			}
			T &operator*() const
			{
				if(!p)
					throw std::runtime_error("Dereferencing empty pointer");
				return *p;
			}
			void reset(T *v=0)
			{
				if(v==p)
					return;
				if(p) {
					if(p->del_ref() == 0) {
						T::dispose(p);
					}
					p=0;
				}
				if(v) {
					v->add_ref();
				}
				p=v;
			}
		private:
			T *p;
		};

		//
		// We can use not atomic counters as we expect that users do not share
		// DB connections between different threads as it commont requirement
		// for all SQL client libraries
		//
		class ref_counted {
		public:
			ref_counted() : count_(0)
			{
			}
			void add_ref()
			{
				++count_;
			}
			int del_ref()
			{
				return --count_;
			}
		private:
			int count_;
		};
	
		class result : public ref_counted {
		public:
			static void dispose(result *p)
			{
				delete p;
			}
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


		class connection : public ref_counted {
		public:
			static void dispose(connection *conn)
			{
				delete conn;
			}
			virtual ~connection() {}
			virtual void begin() = 0;
			virtual void commit() = 0;
			virtual void rollback() = 0;
			virtual statement *prepare(std::string const &q) = 0;
			virtual std::string escape(std::string const &) = 0;
			virtual std::string escape(char const *s) = 0;
			virtual std::string escape(char const *b,char const *e) = 0;
		};

		class driver {
		public:
			virtual ~driver() {}
			virtual std::string name() = 0;
			virtual connection *open(std::string const &connection_string) = 0;
		};

	} // backend
} // cppdb

#endif
