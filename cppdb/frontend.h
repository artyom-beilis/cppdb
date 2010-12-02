///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPDB_FRONTEND_H
#define CPPDB_FRONEND_H
#include <cppdb/defs.h>
#include <cppdb/errors.h>
#include <cppdb/ref_ptr.h>

#include <iosfwd>
#include <ctime>
#include <string>
#include <memory>

///
/// The namespace of all data related to the cppdb api
///

namespace cppdb {

	class result;
	class statement;
	class session;

	
	namespace backend {
		class result;
		class statement;
		class connection;
	}
	
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

	///
	/// Create a pair of value and tag for fetching a value from row, the fetched
	/// value will be stored in \a value if the column is not null and the flag
	/// if the value is null or not saved in \a tag
	///
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


	class CPPDB_API result {
	public:
		result();
		~result();
		result(result const &);
		result const &operator=(result const &);

		int cols();
		bool next();
		
		int index(std::string const &n);
		int find_column(std::string const &name);
		std::string name(int col);

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
			T v=T();
			if(!fetch(name,v))
				throw null_value_fetch();
			return v;
		}
		
		template<typename T>
		T get(int col)
		{
			T v=T();
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

	class CPPDB_API statement {
	public:
		statement();
		~statement();
		statement(statement const &);
		statement const &operator=(statement const &);

		void reset();

		statement &bind(int v);
		statement &bind(unsigned v);
		statement &bind(long v);
		statement &bind(unsigned long v);
		statement &bind(long long v);
		statement &bind(unsigned long long v);
		statement &bind(double v);
		statement &bind(long double v);
		statement &bind(std::string const &v);
		statement &bind(char const *s);
		statement &bind(char const *b,char const *e);
		statement &bind(std::tm const &v);
		statement &bind(std::istream &v);
		statement &bind_null();


		void bind(int col,int v);
		void bind(int col,unsigned v);
		void bind(int col,long v);
		void bind(int col,unsigned long v);
		void bind(int col,long long v);
		void bind(int col,unsigned long long v);
		void bind(int col,double v);
		void bind(int col,long double v);
		void bind(int col,std::string const &v);
		void bind(int col,char const *s);
		void bind(int col,char const *b,char const *e);
		void bind(int col,std::tm const &v);
		void bind(int col,std::istream &v);
		void bind_null(int col);

		long long last_insert_id();
		long long sequence_last(std::string const &seq);
		unsigned long long affected();

		result row();
		result query();
		operator result();
		void exec();

		statement &operator<<(std::string const &v);
		statement &operator<<(char const *s);
		statement &operator<<(std::tm const &v);
		statement &operator<<(std::istream &v);
		statement &operator<<(void (*manipulator)(statement &st));
		result operator<<(result (*manipulator)(statement &st));

		template<typename T>
		statement &operator<<(tags::use_tag<T> const &val)
		{
			if(val.tag == null_value)
				return bind_null();
			else 
				return bind(val.value);
		}
		
		template<typename T>
		statement &operator<<(T v)
		{
			return bind(v);
		}
		
	private:
		statement(ref_ptr<backend::statement> stat,ref_ptr<backend::connection> conn);

		friend class session;

		int placeholder_;
		ref_ptr<backend::statement> stat_;
		ref_ptr<backend::connection> conn_;
		struct data;
		std::auto_ptr<data> d;
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

	class CPPDB_API session {
	public:

		session();
		session(session const &);
		session const &operator=(session const &);
		~session();

		session(std::string const &cs);
		session(ref_ptr<backend::connection> conn);
		
		void open(std::string const &cs);
		void close();
		bool is_open();

		statement prepare(std::string const &query);
		statement operator<<(std::string const &q);
		statement operator<<(char const *s);


		statement create_statement(std::string const &q);
		statement create_prepared_statement(std::string const &q);
		statement create_prepared_uncached_statement(std::string const &q);

		void clear_cache();

		void begin();
		void commit();
		void rollback();

		std::string escape(char const *b,char const *e);
		std::string escape(char const *s);
		std::string escape(std::string const &s);
		std::string driver();
		std::string engine();

	private:
		struct data;
		std::auto_ptr<data> d;
		ref_ptr<backend::connection> conn_;
	};

	class CPPDB_API transaction {
		transaction(transaction const &);
		void operator=(transaction const &);
	public:
		transaction(session &s);
		~transaction();
		void commit();
		void rollback();
	private:
		struct data;
		session *s_;
		bool commited_;
		std::auto_ptr<data> d;
	};


} // cppdb

#endif
