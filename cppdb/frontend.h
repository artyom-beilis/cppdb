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
#define CPPDB_FRONTEND_H
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
	/// \brief  Create a pair of value and tag for fetching a value from row.
	///
	/// The fetched
	/// value will be stored in \a value if the column is not null and the flag
	/// if the value is null or not saved in \a tag
	///
	template<typename T>
	tags::into_tag<T> into(T &value,null_tag_type &tag)
	{
		return tags::into_tag<T>(value,tag);
	}

	///
	/// \brief Create a pair of a string value and tag for storing it to DB
	///

	inline tags::use_tag<std::string const &> use(std::string const &v,null_tag_type tag)
	{
		return tags::use_tag<std::string const &>(v,tag);
	}

	///
	/// \brief Create a pair of a string value and tag for storing it to DB
	///

	inline tags::use_tag<char const *> use(char const *v,null_tag_type tag)
	{
		return tags::use_tag<char const *>(v,tag);
	}
	
	///
	/// \brief Create a pair of value and tag for storing it to DB
	///
	template<typename T>
	tags::use_tag<T> use(T value,null_tag_type tag)
	{
		return tags::use_tag<T>(value,tag);
	}


	///
	/// \brief This object represents query result.
	///
	/// This object and it is generally created by statement::query() call, default constructor
	/// is provided for consistency, but access to any member function with exception of empty() would
	/// throw an exception.
	///
	class CPPDB_API result {
	public:
		///
		/// Create an empty result, it is not useful except for having default constructor
		///
		result();
		///
		/// Destroys the result, note, if the result of statement is not destroyed, it would 
		/// not be returned to statements cache.
		///
		~result();
		///
		/// Copy result, note it only keeps the reference to actual object so copy is just 
		/// copy of the reference
		///
		result(result const &);
		///
		/// Assign result, note it only keeps the reference to actual object so assignment is just 
		/// copy of the reference
		///
		result const &operator=(result const &);

		///
		/// Return the number of columns in the result
		///
		int cols();
		///
		/// Move forward to next row, returns false if no more rows available.
		///
		/// Notes:
		///
		/// - You should call next() at least once before you use fetch() functions
		/// - You must not call fetch() functions if next() returned false, it would cause empty_row_access exception.
		///
		bool next();
		
		///
		/// Convert column name \a n to its index, throws invalid_column if the name is not valid.
		///
		int index(std::string const &n);
		///
		/// Convert column name \a n to its index, returns -1 if the name is not valid.
		///
		int find_column(std::string const &name);

		///
		/// Convert column index to column name, throws invalid_column if col is not in range 0<= col < cols()
		///
		std::string name(int col);

		///
		/// Return true if the column number \a col (starting from 0) has NULL value
		///
		bool is_null(int col);
		///
		/// Return true if the column named \a n has NULL value
		///
		bool is_null(std::string const &n);

		///
		/// Clears the result, no further use of the result should be done until it is assigned again with a new statement result.
		///
		/// It is useful when you want to release all data and return the statement to cache
		///
		void clear();
		///
		/// Reset current column index, so fetch without column index can be used once again
		///
		void rewind_column();
		///
		/// Check if the current row is empty, it is in 3 cases:
		///
		/// -# Empty result
		/// -# next() wasn't called first time
		/// -# next() returned false;
		///
		bool empty();

		
		///
		/// Fetch a value from column \a col (starting from 0) into \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same it tries to cast the data, if casting fails or the
		/// data is out of the type range, throws bad_value_cast().
		///
		bool fetch(int col,short &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,unsigned short &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,int &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,unsigned &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,long &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,unsigned long &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,long long &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,unsigned long long &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,float &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,double &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,long double &v);
		///
		/// Fetch a textual value from column \a col (starting from 0) into \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same, if possible it converts it into textual representation.
		///
		bool fetch(int col,std::string &v);
		///
		/// \copydoc fetch(int,short&)
		///
		bool fetch(int col,std::tm &v);
		///
		/// Fetch a binary large object value from column \a col (starting from 0) into a stream \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not blob, it may throw bad_value_cast()
		///
		bool fetch(int col,std::ostream &v);

		///
		/// Fetch a value from column named \a n into \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same it tries to cast the data, if casting fails or the
		/// data is out of the type range, throws bad_value_cast().
		///
		/// If the \a n value is invalid throws invalid_column exception
		///
		bool fetch(std::string const &n,short &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,unsigned short &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,int &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,unsigned &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,long &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,unsigned long &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,long long &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,unsigned long long &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,float &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,double &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,long double &v);
		///
		/// Fetch a textual value from column named \a n into \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same, if possible it converts it into textual representation. If
		/// the \a n value is invalid throws invalid_column exception
		///
		bool fetch(std::string const &n,std::string &v);
		///
		/// \copydoc fetch(std::string const &,short&)
		///
		bool fetch(std::string const &n,std::tm &v);
		///
		/// Fetch a binary large object value from column named \a name into a stream \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not blob, it may throw bad_value_cast(). If
		/// the \a n value is invalid throws invalid_column exception
		///
		bool fetch(std::string const &n,std::ostream &v);


		///
		/// Fetch a value from the next column in the row starting from the first one. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same it tries to cast the data, if casting fails or the
		/// data is out of the type range, throws bad_value_cast().
		///
		/// If fetch was called more times then cols() it throws invalid_column exception, to use
		/// it once again from the beginning on the same row call rewind_column() member function.
		/// It is not required to call rewind_column() after calling next() as column index is reset
		/// automatically.
		///
		bool fetch(short &v);
		/// \copydoc fetch(short&)
		bool fetch(unsigned short &v);
		/// \copydoc fetch(short&)
		bool fetch(int &v);
		/// \copydoc fetch(short&)
		bool fetch(unsigned &v);
		/// \copydoc fetch(short&)
		bool fetch(long &v);
		/// \copydoc fetch(short&)
		bool fetch(unsigned long &v);
		/// \copydoc fetch(short&)
		bool fetch(long long &v);
		/// \copydoc fetch(short&)
		bool fetch(unsigned long long &v);
		/// \copydoc fetch(short&)
		bool fetch(float &v);
		/// \copydoc fetch(short&)
		bool fetch(double &v);
		/// \copydoc fetch(short&)
		bool fetch(long double &v);
		///
		/// Fetch a textual value from the next column in the row starting from the first one. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not same, if possible it converts it into textual representation.
		///
		/// If fetch was called more times then cols() it throws invalid_column exception, to use
		/// it once again from the beginning on the same row call rewind_column() member function.
		/// It is not required to call rewind_column() after calling next() as column index is reset
		/// automatically.
		///
		bool fetch(std::string &v);
		/// \copydoc fetch(short&)
		bool fetch(std::tm &v);
		///
		/// Fetch a blob value from the next column in the row starting from the first one into stream \a v. Returns false
		/// if the value in NULL and \a v is not updated, otherwise returns true.
		///
		/// If the data type is not blob, it may throw bad_value_cast().
		///
		/// If fetch was called more times then cols() it throws invalid_column exception, to use
		/// it once again from the beginning on the same row call rewind_column() member function.
		/// It is not required to call rewind_column() after calling next() as column index is reset
		/// automatically.
		///
		bool fetch(std::ostream &v);

		///
		/// Get a value of type \a T from column named \a name (starting from 0). If the column
		/// is null throws null_value_fetch(), if the column \a name is invalid throws invalid_column,
		/// if the column value cannot be converted to type T (see fetch functions) it throws bad_value_cast.
		///	
		
		template<typename T>
		T get(std::string const &name)
		{
			T v=T();
			if(!fetch(name,v))
				throw null_value_fetch();
			return v;
		}
	
		///
		/// Get a value of type \a T from column \a col (starting from 0). If the column
		/// is null throws null_value_fetch(), if the column index is invalid throws invalid_column,
		/// if the column value cannot be converted to type T (see fetch functions) it throws bad_value_cast.
		///	
		template<typename T>
		T get(int col)
		{
			T v=T();
			if(!fetch(col,v))
				throw null_value_fetch();
			return v;
		}

		///
		/// Syntactic sugar, used together with into() function.
		///
		/// res << into(x,y) is same as 
		///
		/// \code
		/// y = res.fetch(x) ? not_null_value : null_value 
		/// \endcode
		///
		template<typename T>
		result &operator>>(tags::into_tag<T> ref)
		{
			if(fetch(ref.value))
				ref.tag = not_null_value;
			else
				ref.tag = null_value;
			return *this;
		}

		///
		/// Syntactic sugar, same as fetch(\a value)
		///
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

	///
	/// \brief This class represents a prepared (or ordinary) statement that can be executed.
	///
	/// This object is usually created via session::prepare() function.
	///
	class CPPDB_API statement {
	public:
		///
		/// Default constructor, provided for convenience, access to any member function
		/// of empty statement will cause an exception being thrown. 
		///
		statement();
		///
		/// Destructor, it releases prepared statement and if the statements cache is enabled
		/// it returns it into the cache.
		///
		/// Note: if result object created by this statement is alive, the underlying backned::statement would
		/// be on hold until result object is destroyed and only then the statement would be put back
		/// into cache.
		///
		~statement();
		///
		/// Copy statement.
		///
		/// Please note it copies only the reference to underlying statement object, so copies
		/// of same statement represent same object and it is strongly not recommended to access the underlying
		/// backend::statement by two different statement objects.
		///
		statement(statement const &);
		///
		/// Assign a statement.
		///
		/// Please note it copies only the reference to underlying statement object, so copies
		/// of same statement represent same object and it is strongly not recommended to access the underlying
		/// backend::statement by two different statement objects.
		///
		statement const &operator=(statement const &);

		///
		/// Reset the statement - remove all bindings and return it into initial state so query() or exec()
		/// functions can be called once again.
		///
		/// You must use it if you use the same statement multiple times.
		///
		void reset();

		///
		/// Bind a value \a v to the next placeholder (starting from the first) marked with '?' marker in the query.
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind(int v);
		/// \copydoc bind(int)
		statement &bind(unsigned v);
		/// \copydoc bind(int)
		statement &bind(long v);
		/// \copydoc bind(int)
		statement &bind(unsigned long v);
		/// \copydoc bind(int)
		statement &bind(long long v);
		/// \copydoc bind(int)
		statement &bind(unsigned long long v);
		/// \copydoc bind(int)
		statement &bind(double v);
		/// \copydoc bind(int)
		statement &bind(long double v);
		///
		/// Bind a string value \a v to the next placeholder marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind(std::string const &v);
		///
		/// Bind a null terminated string value \a s to the next placeholder marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind(char const *s);
		///
		/// Bind a string value in range [\a b, \a e ) to the next placeholder marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind(char const *b,char const *e);
		/// \copydoc bind(int)
		statement &bind(std::tm const &v);
		///
		/// Bind a BLOB value \a v to the next placeholder marked with '?' marker in the query.
		///
		/// Note: the reference to the stream MUST remain valid until the statement is queried or executed!
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind(std::istream &v);
		///
		/// Bind a NULL value  to the next placeholder marked with '?' marker in the query.
		///
		/// If number of calls is higher then the number placeholders is the statement it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		statement &bind_null();


		///
		/// Bind a value \a v to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind(int col,int v);
		/// \copydoc bind(int,int)
		void bind(int col,unsigned v);
		/// \copydoc bind(int,int)
		void bind(int col,long v);
		/// \copydoc bind(int,int)
		void bind(int col,unsigned long v);
		/// \copydoc bind(int,int)
		void bind(int col,long long v);
		/// \copydoc bind(int,int)
		void bind(int col,unsigned long long v);
		/// \copydoc bind(int,int)
		void bind(int col,double v);
		/// \copydoc bind(int,int)
		void bind(int col,long double v);
		///
		/// Bind a string value \a v to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind(int col,std::string const &v);
		///
		/// Bind a null terminated string value \a s to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind(int col,char const *s);
		///
		/// Bind a string value in range [\a b, \a e ) to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// Note: the reference to the string MUST remain valid until the statement is queried or executed!
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind(int col,char const *b,char const *e);
		/// \copydoc bind(int,int)
		void bind(int col,std::tm const &v);
		///
		/// Bind a BLOB value \a v to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// Note: the reference to the stream MUST remain valid until the statement is queried or executed!
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind(int col,std::istream &v);
		///
		/// Bind a NULL value to the placeholder number \a col (starting from 1) marked with '?' marker in the query.
		///
		/// If \a cols is invalid (less then 1 or higher then the number of the placeholders is the statement) it
		/// may throw invalid_placeholder exception.
		///
		/// If placeholder was not binded the behavior is undefined and may vary between different backends.
		///
		void bind_null(int col);

		///
		/// Get last insert id from the last executed statement, note, it is the same as sequence_last("").
		/// 
		/// Some backends requires explicit sequence name so you should use sequence_last("sequence_name") in such
		/// case.
		///
		/// If the statement is actually query, the behavior is undefined and may vary between backends.
		///
		long long last_insert_id();
		///
		/// Get last created sequence value from the last executed statement.
		///
		/// If the backend does not support named sequences but rather supports "auto increment" columns (like MySQL, Sqlite3),
		/// the \a seq parameter is ignored.
		/// 
		///
		/// If the statement is actually query, the behavior is undefined and may vary between backends.
		///
		long long sequence_last(std::string const &seq);
		///
		/// Get the number of affected rows by the last statement, 
		///
		///
		/// If the statement is actually query, the behavior is undefined and may vary between backends.
		///
		unsigned long long affected();

		///
		/// Fetch a single row from the query. Unlike query(), you should not call result::next()
		/// function as it is already called. You may check if the data was fetched using result::empty()
		/// function.
		///
		/// If the result set consists of more then one row it throws multiple_rows_query exception, however some backends
		/// may ignore this.
		///
		/// If the statement is not query statement (like SELECT) it would likely
		/// throw an exception, however the behavior may vary between backends that may ignore this error.
		///
		result row();
		///
		/// Fetch a result of the query, if the statement is not query statement (like SELECT) it would likely
		/// throw an exception, however the behavior may vary between backends that may ignore this error.
		///
		result query();
		///
		/// Same as query() - syntactic sugar
		///
		operator result();

		///
		/// Execute a statement, of the statement is actually SELECT like operator, it throws cppdb_error exception,
		/// however the behavior may vary between backends that may ignore this error.
		///
		void exec();

		///
		/// Same as bind(v);
		///
		statement &operator<<(std::string const &v);
		///
		/// Same as bind(s);
		///
		statement &operator<<(char const *s);
		///
		/// Same as bind(v);
		///
		statement &operator<<(std::tm const &v);
		///
		/// Same as bind(v);
		///
		statement &operator<<(std::istream &v);
		///
		/// Apply manipulator on the statement, same as manipulator(*this).
		///
		statement &operator<<(void (*manipulator)(statement &st));
		///
		/// Apply manipulator on the statement, same as manipulator(*this).
		///
		result operator<<(result (*manipulator)(statement &st));

		///
		/// Used together with use() function. 
		///
		/// The call st<<use(x,tag) is same as
		///
		/// \code
		///  (tag == null_value) ?  st.bind_null() : st.bind(x)
		/// \endcode
		///
		template<typename T>
		statement &operator<<(tags::use_tag<T> const &val)
		{
			if(val.tag == null_value)
				return bind_null();
			else 
				return bind(val.value);
		}
	
		///
		/// Same as bind(v);
		///	
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

	///
	/// \brief Manipulator that causes statement execution. Used as:
	///
	/// \code
	///  sql << "delete from test" << cppdb::exec;
	/// \endcode
	///
	inline void exec(statement &st)
	{
		st.exec();
	}
	
	///
	/// \brief Manipulator that binds null value. Used as:
	///
	/// \code
	///  sql << "insert into foo values(?,?,?)" << x << cppdb::null << y << cppdb::exec;
	/// \endcode
	///
	inline void null(statement &st)
	{
		st.bind_null();
	}

	///
	/// \brief Manipulator that fetches a single row. Used as:
	///
	/// \code
	///  cppdb::result r = sql << "SELECT name where uid=?" << id << cppdb::row;
	/// if(!r.empty()) {
	///  ...
	/// }
	/// \endcode
	///
	/// Or:
	///
	/// \code
	///  sql << "SELECT name where uid=?" << id << cppdb::row >> name;
	/// \endcode
	///
	/// Which would throw empty_row_access exception on attempt to fetch name if the result is empty.
	///
	inline result row(statement &st)
	{
		return st.row();
	}

	///
	/// \brief SQL session object that represents a single connection and is the gateway to SQL database
	///
	/// It is the main class that is used for access to the DB, it uses various singleton classes to
	/// load drivers open connections and cache them. 
	///
	class CPPDB_API session {
	public:

		///
		/// Create an empty session object, it should not be used until it is opened with calling open() function.
		///
		session();
		///
		/// Copy a session object, note - it copies only the reference to the underlying connection, so you should
		/// be very careful when you do it.
		///
		session(session const &);
		///
		/// Assign a session object, note - it copies only the reference to the underlying connection, so you should
		/// be very careful when you do it.
		///
		session const &operator=(session const &);
		///
		/// Destroys the session object, if connection pool is used it returns the object to connection pool.
		///
		/// Note: the connection would not be returned to the pool until all statement and result objects
		/// created using this session are not destroyed.
		///
		~session();

		///
		/// Create a session using a connection string \a cs.
		///
		/// \copydetails cppdb::parse_connection_string(std::string const&,std::string&,std::map<std::string,std::string>&);
		///
		session(std::string const &cs);

		///
		/// Create a session using a pointer to backend::connection.
		///
		session(ref_ptr<backend::connection> conn);
		
		///
		/// Open a session using a connection string \a cs.
		///
		/// \copydetails cppdb::parse_connection_string(std::string const&,std::string&,std::map<std::string,std::string>&);
		///
		void open(std::string const &cs);
		///
		/// Close current connection, note, if connection pooling is used the connection is not actually become closed but
		/// rather recycled for future use.
		///
		void close();
		///
		/// Check if the session was opened.
		///
		bool is_open();

		///
		/// Create a new statement, by default is creates prepared statement - create_prepared_statement() unless \@use_prepared connection string
		/// property is set to off, then it uses normal statements by calling create_statement()
		///
		/// This is the most convenient function to create statements with.
		///
		statement prepare(std::string const &query);
		///
		/// Syntactic sugar, same as prepare(q)
		///
		statement operator<<(std::string const &q);
		///
		/// Syntactic sugar, same as prepare(s)
		///
		statement operator<<(char const *s);


		///
		/// Create ordinary statement it generally unprepared statement and it is never cached. It should
		/// be used when such statement is executed rarely or very customized.
		///
		statement create_statement(std::string const &q);
		///
		/// Create prepared statement that will be cached for next calls.		
		///
		statement create_prepared_statement(std::string const &q);
		///
		/// Create prepared statement however don't use statements cache and for it. Useful for creation
		/// of custom or rarely executed statements that should be executed several times at this point in program.
		///
		statement create_prepared_uncached_statement(std::string const &q);

		///
		/// Remove all statements from the cache.
		///
		void clear_cache();

		///
		/// Begin a transaction. Don't use it directly for RAII reasons. Use transaction class instead.
		///
		void begin();
		///
		/// Commit a transaction. Don't use it directly for RAII reasons. Use transaction class instead.
		///
		void commit();
		///
		/// Rollback a transaction. Don't use it directly for RAII reasons. Use transaction class instead.
		///
		void rollback();

		///
		/// Escape a string in range [\a b,\a e) for inclusion in SQL statement. It does not add quotation marks at beginning and end.
		/// It is designed to be used with text, don't use it with generic binary data.
		///
		/// Some backends (odbc) may not support this.
		///
		std::string escape(char const *b,char const *e);
		///
		/// Escape a NULL terminated string \a s for inclusion in SQL statement. It does not add quotation marks at beginning and end.
		/// It is designed to be used with text, don't use it with generic binary data.
		///
		/// Some backends (odbc) may not support this.
		///
		std::string escape(char const *s);
		///
		/// Escape a string \a s for inclusion in SQL statement. It does not add quotation marks at beginning and end.
		/// It is designed to be used with text, don't use it with generic binary data.
		///
		/// Some backends (odbc) may not support this.
		///
		std::string escape(std::string const &s);
		///
		/// Get the driver name, as mysql, postgresql, odbc, sqlite3.
		/// 
		std::string driver();
		///
		/// Get an SQL engine name, it may be not the same as driver name for multiple engine drivers like odbc.
		///
		std::string engine();

	private:
		struct data;
		std::auto_ptr<data> d;
		ref_ptr<backend::connection> conn_;
	};

	///
	/// \brief The transaction guard
	///
	/// This class is RAII transaction guard that causes automatic transaction rollback on stack unwind, unless
	/// the transaction is committed
	///
	class CPPDB_API transaction {
		transaction(transaction const &);
		void operator=(transaction const &);
	public:
		///
		/// Begin a transaction on session \a s, calls s.begin()
		///
		transaction(session &s);
		///
		/// If the transaction wasn't committed or rolled back calls session::rollback() for the session it was created with.
		///
		~transaction();
		///
		/// Commit a transaction on the session.  Calls session::commit() for the session it was created with.
		///
		void commit();
		///
		/// Rollback a transaction on the session.  Calls session::rollback() for the session it was created with.
		///
		void rollback();
	private:
		struct data;
		session *s_;
		bool commited_;
		std::auto_ptr<data> d;
	};


} // cppdb

#endif
