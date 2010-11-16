#ifndef CPPDB_FRONTEND_H
#define CPPDB_FRONEND_H
#include <iosfwd>
#include <ctime>
#include <string>
#include "backend.h"
#include "errors.h"
#include <assert.h>

namespace cppdb {
	
	typedef enum {
		null_value,
		not_null_value
	} null_tag_type;
	
	namespace tags {
		struct null_tag {};
		struct exec_tag {};
		struct single_row_tag {};
		template<typename T>
		
		struct into_tag {
			T *value;
			null_tag_type *tag;
		};

		template<typename T>
		struct use_tag {
			T value;
			null_tag_type tag;
		};

	} // tags

	tags::null_tag null()
	{
		return tags::null_tag();
	}
	
	template<typename T>
	tags::into_tag<T> into(T &value,null_tag_type &tag)
	{
		tags::into_tag<T> res = { &value, &tag };
		return res;
	}


	tags::use_tag<std::string const &> use(std::string const &v,null_tag_type tag)
	{
		tags::use_tag<std::string const &> res = { v, tag };
		return res;
	}

	tags::use_tag<char const *> use(char const *v,null_tag_type tag)
	{
		tags::use_tag<char const *> res = { v, tag };
		return res;
	}
	
	template<typename T>
	tags::use_tag<T> use(T value,null_tag_type tag)
	{
		tags::use_tag<T> res = { value, tag };
		return res;
	}

	inline tags::exec_tag exec() { return tags::exec_tag(); }
	inline tags::single_row_tag row() { return tags::single_row_tag(); }

	class result {
	public:
		result() : eof_(true),current_col_(0)
		{
		}
		result(	ref_ptr<backend::result> res,
			ref_ptr<backend::statement> stat,
			ref_ptr<backend::connection> conn,
			bool fetched=false)
		: eof_(false),
		  fetched_(fetched),
		  current_col_(0),
		  res_(res),
		  stat_(stat),
		  conn_(conn)
		{
		}

		~result()
		{
			clear();
		}
		
		int cols()
		{
			if(res_)
				return res_->cols();
			throw empty_row_access();
		}
		
		template<typename T>
		T get(std::string const &name)
		{
			check();
			return get<T>(res_->name_to_column(name));
		}
		
		template<typename T>
		T get(int col)
		{
			check();
			T value;
			if(res_->fetch(col,value))
				return value;
			throw null_value_fetch(); 
		}

		template<typename T>
		bool fetch(int col,T &value)
		{
			check();
			return res_->fetch(col,value);
		}

		template<typename T>
		result &operator>>(tags::into_tag<T> value)
		{
			check();
			assert(value.value!=0);
			assert(value.tag!=0);
			*value.tag = res_->fetch(current_col_++,*value.value) ? not_null_value :  null_value;
			return *this;
		}

		bool is_null(int col)
		{
			check();
			return res_->is_null(col);
		}
		bool is_null(std::string const &n)
		{
			check();
			return res_->is_null(res_->name_to_column(n));
		}

		template<typename T>
		result &operator>>(T &value)
		{
			check();
			if(!res_->fetch(current_col_++,value))
				throw null_value_fetch();
			return *this;
		}

		bool next()
		{
			if(eof_ && fetched_)
				return false;
			fetched_=true;
			current_col_ = 0;
			eof_ = res_->next()==false;
			if(eof_) {
				res_.reset();
				stat_.reset();
				conn_.reset();
			}
			return !eof_;
		}

		bool empty()
		{
			return eof_ || !fetched_;
		}
		void clear()
		{
			eof_ = true;
			fetched_ = true;
			res_.reset();
			stat_.reset();
			conn_.reset();
		}
		
	private:
		void check()
		{
			if(empty())
				throw empty_row_access();
		}
		bool eof_;
		bool fetched_;
		int current_col_;
		ref_ptr<backend::result> res_;
		ref_ptr<backend::statement> stat_;
		ref_ptr<backend::connection> conn_;
	};

	class statement {
	public:
		statement() :	placeholder_(1)
		{
		}
		~statement()
		{
			stat_.reset();
			conn_.reset();
		}
		void reset()
		{
			placeholder_ = 1;
			stat_->reset();
		}

		statement &operator<<(std::string const &v)
		{
			return bind(v);
		}
		statement &operator<<(char const *s)
		{
			return bind(s);
		}
		
		statement &operator<<(std::tm const &v)
		{
			return bind(v);
		}
		
		statement &operator<<(std::istream const &v)
		{
			return bind(v);
		}

		result operator<<(tags::single_row_tag const &)
		{
			return row();
		}

		statement &operator<<(tags::exec_tag const &)
		{
			exec();
			return *this;
		}

		statement &operator<<(tags::null_tag const &)
		{
			bind_null();
			return *this;
		}

		template<typename T>
		statement &operator<<(tags::use_tag<T> const &val)
		{
			if(val.tag == null_value) {
				bind_null();
			}
			else {
				bind(val.value);
			}
		}
		
		template<typename T>
		statement &operator<<(T v)
		{
			return bind(v);
		}
		
		statement &bind(std::string const &v)
		{
			stat_->bind(placeholder_++,v);
			return *this;
		}
		statement &bind(char const *s)
		{
			stat_->bind(placeholder_++,s);
			return *this;
		}
		statement &bind(char const *b,char const *e)
		{
			stat_->bind(placeholder_++,b,e);
			return *this;
		}
		statement &bind(std::tm const &v)
		{
			stat_->bind(placeholder_++,v);
			return *this;
		}
		statement &bind(std::istream const &v)
		{
			stat_->bind(placeholder_++,v);
			return *this;
		}
		template<typename T>
		statement &bind(T v)
		{
			stat_->bind(placeholder_++,v);
			return *this;
		}
		statement &bind(int col,std::string const &v)
		{
			stat_->bind(col,v);
			return *this;
		}
		statement &bind(int col,char const *s)
		{
			stat_->bind(col,s);
			return *this;
		}
		statement &bind(int col,char const *b,char const *e)
		{
			stat_->bind(col,b,e);
			return *this;
		}
		statement &bind(int col,std::tm const &v)
		{
			stat_->bind(col,v);
			return *this;
		}
		statement &bind(int col,std::istream const &v)
		{
			stat_->bind(col,v);
			return *this;
		}
		template<typename T>
		statement &bind(int col,T v)
		{
			stat_->bind(col,v);
			return *this;
		}
		statement &bind_null()
		{
			stat_->bind_null(placeholder_++);
			return *this;
		}
		statement &bind_null(int col)
		{
			stat_->bind_null(col);
			return *this;
		}
		long long sequence_last(std::string const &seq = std::string())
		{
			return stat_->sequence_last(seq);
		}
		unsigned long long affected()
		{
			return stat_->affected();
		}
		result row()
		{
			ref_ptr<backend::result> res(stat_->query());
			if(!res->next())
				return result();
			if(res->has_next() == backend::result::next_row_exists) {
				throw multiple_rows_query();
			}
			return result(res,stat_,conn_,true);
		}
		result query()
		{
			ref_ptr<backend::result> res(stat_->query());
			return result(res,stat_,conn_);
		}
		operator result()
		{
			return query();
		}
		void exec() 
		{
			stat_->exec();
		}
		statement(ref_ptr<backend::statement> stat,ref_ptr<backend::connection> conn) :
			placeholder_(1),
			stat_(stat),
			conn_(conn)
		{
		}
	private:
		int placeholder_;
		ref_ptr<backend::statement> stat_;
		ref_ptr<backend::connection> conn_;
	};

	class session {
	public:
		statement prepare(std::string const &query)
		{
			ref_ptr<backend::statement> stat_ptr(conn_->prepare(query));
			statement stat(stat_ptr,conn_);
			return stat;
		}
		statement operator<<(std::string const &q)
		{
			return prepare(q);
		}
		statement operator<<(char const *s)
		{
			return prepare(s);
		}
		void begin()
		{
			conn_->begin();
		}
		void commit()
		{
			conn_->commit();
		}
		void rollback()
		{
			conn_->rollback();
		}
		std::string escape(char const *b,char const *e)
		{
			return conn_->escape(b,e);
		}
		std::string escape(char const *s)
		{
			return conn_->escape(s);
		}
		std::string escape(std::string const &s)
		{
			return conn_->escape(s);
		}
		session(ref_ptr<backend::connection> conn) : conn_(conn)
		{
		}
		session()
		{
		}
	private:
		ref_ptr<backend::connection> conn_;
	};

} // cppdb

#endif
