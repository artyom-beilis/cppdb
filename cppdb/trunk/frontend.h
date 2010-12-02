#ifndef CPPDB_FRONTEND_H
#define CPPDB_FRONEND_H
#include <iosfwd>
#include <ctime>
#include <string>
#include "backend.h"
#include "errors.h"
#include <assert.h>

///
/// The namespace of all data related to the cppdb api
///

namespace cppdb {
	
	///
	/// Null value marker
	///
	typedef enum {
		null_value,  	///< The value is null value
		not_null_value	///< The valus is not a null value
	} null_tag_type;

	/// \cond INTERNAL
	namespace tags {
		template<typename T>
		struct into_tag {
			T &value;
			null_tag_type &tag;
			into_tag(T &v, null_tag_type &t) : value(v),tag(t) {}
		};

		template<typename T>
		struct use_tag {
			T value;
			null_tag_type tag;
			use_tag(T v,null_tag_type t) : value(v),tag(t) {}
		};

	} // tags
	/// \endcond

	template<typename T>
	tags::into_tag<T> into(T &value,null_tag_type &tag)
	{
		return tags::into_tag<T>(value,tag);
	}


	inline tags::use_tag<std::string const &> use(std::string const &v,null_tag_type tag)
	{
		return tags::use_tag<std::string const &>(v,tag);
	}

	inline tags::use_tag<char const *> use(char const *v,null_tag_type tag)
	{
		return tags::use_tag<char const *>(v,tag);
	}
	
	template<typename T>
	tags::use_tag<T> use(T value,null_tag_type tag)
	{
		return tags::use_tag<T>(value,tag);
	}

	class statement;

	class result {
	public:
		result();
		~result();
		result(result const &);
		result const &operator=(result const &);

		int cols();
		bool next();
		
		int index(std::string const &n);
		std::string name(int col);
		int find_column(std::string const &name);

		bool is_null(int col);
		bool is_null(std::string const &n);

		void clear();
		void rewind_column();
		bool empty();

		bool fetch(int col,short &v);
		bool fetch(int col,unsigned short &v);
		bool fetch(int col,int &v);
		bool fetch(int col,unsigned &v);
		bool fetch(int col,long &v);
		bool fetch(int col,unsigned long &v);
		bool fetch(int col,long long &v);
		bool fetch(int col,unsigned long long &v);
		bool fetch(int col,float &v);
		bool fetch(int col,double &v);
		bool fetch(int col,long double &v);
		bool fetch(int col,std::string &v);
		bool fetch(int col,std::tm &v);
		bool fetch(int col,std::ostream &v);

		bool fetch(std::string const &n,short &v);
		bool fetch(std::string const &n,unsigned short &v);
		bool fetch(std::string const &n,int &v);
		bool fetch(std::string const &n,unsigned &v);
		bool fetch(std::string const &n,long &v);
		bool fetch(std::string const &n,unsigned long &v);
		bool fetch(std::string const &n,long long &v);
		bool fetch(std::string const &n,unsigned long long &v);
		bool fetch(std::string const &n,float &v);
		bool fetch(std::string const &n,double &v);
		bool fetch(std::string const &n,long double &v);
		bool fetch(std::string const &n,std::string &v);
		bool fetch(std::string const &n,std::tm &v);
		bool fetch(std::string const &n,std::ostream &v);


		bool fetch(short &v);
		bool fetch(unsigned short &v);
		bool fetch(int &v);
		bool fetch(unsigned &v);
		bool fetch(long &v);
		bool fetch(unsigned long &v);
		bool fetch(long long &v);
		bool fetch(unsigned long long &v);
		bool fetch(float &v);
		bool fetch(double &v);
		bool fetch(long double &v);
		bool fetch(std::string &v);
		bool fetch(std::tm &v);
		bool fetch(std::ostream &v);

		
		template<typename T>
		T get(std::string const &name)
		{
			T v;
			if(!fetch(name,v))
				throw null_value_fetch();
			return v;
		}
		
		template<typename T>
		T get(int col)
		{
			T v;
			if(!fetch(col,v))
				throw null_value_fetch();
			return v;
		}

		template<typename T>
		result &operator>>(tags::into_tag<T> ref)
		{
			if(fetch(ref.value))
				ref.tag = not_null_value;
			else
				ref.tag = null_value;
			return *this;
		}

		template<typename T>
		result &operator>>(T &value)
		{
			fetch(value);
			return *this;
		}

		
	private:
		result(	ref_ptr<backend::result> res,
			ref_ptr<backend::statement> stat,
			ref_ptr<backend::connection> conn);

		void check();
		
		friend class statement;

		struct data;

		std::auto_ptr<data> d;

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
		
		statement &operator<<(std::istream &v)
		{
			return bind(v);
		}

		statement &operator<<(void (*manipulator)(statement &st))
		{
			manipulator(*this);
			return *this;
		}
		result operator<<(result (*manipulator)(statement &st))
		{
			return manipulator(*this);
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
			return *this;
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
		statement &bind(std::istream &v)
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
		statement &bind(int col,std::istream &v)
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
			ref_ptr<backend::result> backend_res = stat_->query();
			result res(backend_res,stat_,conn_);
			if(res.next()) {
				if(res.res_->has_next() == backend::result::next_row_exists) {
					throw multiple_rows_query();
				}
			}
			return res;
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


	inline void exec(statement &st)
	{
		st.exec();
	}
	
	inline void null(statement &st)
	{
		st.bind_null();
	}

	inline result row(statement &st)
	{
		return st.row();
	}

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
		std::string driver()
		{
			return conn_->driver();
		}
		std::string engine()
		{
			return conn_->engine();
		}
		session(ref_ptr<backend::connection> conn);
		session();
		~session();
		session(std::string const &cs);
		void open(std::string const &cs);
		void close();
	private:
		ref_ptr<backend::connection> conn_;
	};

	class transaction {
		transaction(transaction const &);
		void operator=(transaction const &);
	public:
		transaction(session &s) :
			s_(s),
			commited_(false)
		{
			s_.begin();
		}
		
		void commit()
		{
			s_.commit();
			commited_ =true;
		}
		void rollback()
		{
			if(!commited_)
				s_.rollback();
			commited_=true;
		}
		~transaction()
		{
			rollback();
		}
	private:
		session &s_;
		bool commited_;
	};


} // cppdb

#endif
