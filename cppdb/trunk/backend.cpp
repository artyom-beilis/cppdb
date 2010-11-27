#include "backend.h"
#include "utils.h"
#include "pool.h"
#include <map>
#include <list>

namespace cppdb {
	namespace backend {
		//result
		struct result::data {};
		result::result() {}
		result::~result() {}
		
		//statement
		struct statement::data {};

		statement::statement() : cache_(0) 
		{
		}
		statement::~statement()
		{
		}
		void statement::cache(statements_cache *c)
		{
			cache_ = c;
		}

		void statement::dispose(statement *p)
		{
			if(!p)
				return;
			statements_cache *cache = p->cache_;
			p->cache_ = 0;
			if(cache) 
				cache->put(p);
			else
				delete p;
		}
		

		//statements cache//////////////

		struct statements_cache::data {

			data() : 
				size(0),
				max_size(0) 
			{
			}

			struct entry;
			typedef std::map<std::string,entry> statements_type;
			typedef std::list<statements_type::iterator> lru_type;
			struct entry {
				ref_ptr<statement> stat;
				lru_type::iterator lru_ptr;
			};
			
			statements_type statements;

			lru_type lru;
			size_t size;
			size_t max_size;

			void insert(ref_ptr<statement> st)
			{
				statements_type::iterator p;
				if((p=statements.find(st->sql_query()))!=statements.end()) {
					p->second.stat = st;
					lru.erase(p->second.lru_ptr);
					lru.push_front(p);
					p->second.lru_ptr = lru.begin();
				}
				else {
					if(size > 0 && size >= max_size) {
						statements.erase(lru.back());
						lru.pop_back();
						size--;
					}
					std::pair<statements_type::iterator,bool> ins = 
						statements.insert(std::make_pair(st->sql_query(),entry()));
					p = ins.first;
					p->second.stat = st;
					lru.push_front(p);
					p->second.lru_ptr = lru.begin();
					size ++;
				}
			}

			ref_ptr<statement> fetch(std::string const &query)
			{
				ref_ptr<statement> st;
				statements_type::iterator p = statements.find(query);
				if(p==statements.end())
					return st;
				st=p->second.stat;
				lru.erase(p->second.lru_ptr);
				statements.erase(p);
				return st;
			}
		}; // data

		statements_cache::statements_cache() 
		{
		}
		void statements_cache::set_size(size_t n)
		{
			if(n!=0 && !active()) {
				d.reset(new data());
				d->max_size = n;
			}
		}
		void statements_cache::put(statement *p_in)
		{
			if(!active()) {
				delete p_in;
			}
			ref_ptr<statement> p(p_in);
			p->reset();
			d->insert(p);
		}
		ref_ptr<statement> statements_cache::fetch(std::string const &q)
		{
			if(!active())
				return 0;
			return d->fetch(q);
		}
		statements_cache::~statements_cache()
		{
		}

		bool statements_cache::active()
		{
			return d.get()!=0;
		}

		//////////////
		//connection
		//////////////

		struct connection::data {};

		ref_ptr<statement> connection::prepare(std::string const &q)
		{
			ref_ptr<statement> st;
			if(!cache_.active()) {
				st = real_prepare(q);
				return st;
			}
			st = cache_.fetch(q);
			if(!st)
				st = real_prepare(q);
			st->cache(&cache_);
			return st;
		}

		connection::connection(connection_info const &info) :
			pool_(0)
		{
			int cache_size = info.get("@stmt_cache_size",64);
			if(cache_size > 0) {
				cache_.set_size(cache_size);
			}
		}
		connection::~connection()
		{
		}

		void connection::set_pool(ref_ptr<pool> p)
		{
			pool_ = p;
		}
		void connection::set_driver(ref_ptr<loadable_driver> p)
		{
			driver_ = p;
		}
		
		void connection::dispose(connection *c)
		{
			if(!c)
				return;
			ref_ptr<pool> p = c->pool_;
			c->pool_ = 0;
			if(p) {
				p->put(c);
			}
			else
				delete c;
		}
		
		connection *loadable_driver::connect(connection_info const &cs)
		{
			connection *c = open(cs);
			c->set_driver(ref_ptr<loadable_driver>(this));
			return c;
		}

	} // backend
} // cppdb


