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

			///
			/// Check if the next row in the result exists. If the DB engine can't perform
			/// this check without loosing data for current row, it should return next_row_unknown.
			///
			virtual next_row has_next() = 0;
			///
			/// Move to next row. Should be called before first access to any of members. If no rows remain
			/// return false, otherwise return true
			///
			virtual bool next() = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,short &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,unsigned short &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,int &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,unsigned &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,long &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,unsigned long &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,long long &v) = 0;
			///
			/// Fetch an integer value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to integer or its range is not supported by the integer type.
			///
			virtual bool fetch(int col,unsigned long long &v) = 0;
			///
			/// Fetch a floating point value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to floating point value.
			///
			virtual bool fetch(int col,float &v) = 0;
			///
			/// Fetch a floating point value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to floating point value.
			///
			virtual bool fetch(int col,double &v) = 0;
			///
			/// Fetch a floating point value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, should throw bad_value_cast() if the underlying data
			/// can't be converted to floating point value.
			///
			virtual bool fetch(int col,long double &v) = 0;
			///
			/// Fetch a string value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
			/// text value (as formatting integer, floating point value or date-time as string).
			///
			virtual bool fetch(int col,std::string &v) = 0;
			///
			/// Fetch a BLOB value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid, any data should be convertible to
			/// BLOB value as text (as formatting integer, floating point value or date-time as string).
			///
			virtual bool fetch(int col,std::ostream &v) = 0;
			///
			/// Fetch a date-time value for column \a col starting from 0.
			/// Returns true if ok, returns false if the column value is NULL and the referenced object should remain unchanged
			///
			/// Should throw invalid_column() \a col value is invalid. If the data can't be converted
			/// to date-time it should throw bad_value_cast()
			///
			virtual bool fetch(int col,std::tm &v) = 0;
			///
			/// Check if the column \a col is NULL starting from 0, should throw invalid_column() if the index out of range
			///
			virtual bool is_null(int col) = 0;
			///
			/// Return the number of columns in the result. Should be valid even without calling next() first time.
			///
			virtual int cols() = 0;
			///
			/// Return the number of columns by its name. Return -1 if the name is invalid
			/// Should be able to work even without calling next() first time.
			///
			virtual int name_to_column(std::string const &) = 0;
			///
			/// Return the column name for column index starting from 0.
			/// Should throw invalid_column() if the index out of range
			/// Should be able to work even without calling next() first time.
			///
			virtual std::string column_to_name(int) = 0;

			// End of API
			
			result();
			virtual ~result();
		private:
			struct data;
			std::auto_ptr<data> d;
		};

		class statements_cache;

		class statement : public ref_counted {
		public:
			// Begin of API
			///
			/// Reset the prepared statement to initial state as before the operation. It is
			/// called by front-end each time before new query() or exec() are called.
			///
			virtual void reset() = 0;
			///
			/// Get the query the statement works with. Return it as is, used as key for statement
			/// caching 
			///
			virtual std::string const &sql_query() = 0;

			///
			/// Bind a text value to column \a col (starting from 1). You may assume
			/// that the reference remains valid until real call of query() or exec()
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,std::string const &) = 0;
			///
			/// Bind a text value to column \a col (starting from 1). You may assume
			/// that the reference remains valid until real call of query() or exec()
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,char const *s) = 0;
			///
			/// Bind a text value to column \a col (starting from 1). You may assume
			/// that the reference remains valid until real call of query() or exec()
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,char const *b,char const *e) = 0;
			///
			/// Bind a date-time value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,std::tm const &) = 0;
			///
			/// Bind a BLOB value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,std::istream const &) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,int v) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			/// May throw bad_value_cast() if the value out of supported range by the DB. 
			///
			virtual void bind(int col,unsigned v) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			/// May throw bad_value_cast() if the value out of supported range by the DB. 
			///
			virtual void bind(int col,long v) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			/// May throw bad_value_cast() if the value out of supported range by the DB. 
			///
			virtual void bind(int col,unsigned long v) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			/// May throw bad_value_cast() if the value out of supported range by the DB. 
			///
			virtual void bind(int col,long long v) = 0;
			///
			/// Bind an integer value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			/// May throw bad_value_cast() if the value out of supported range by the DB. 
			///
			virtual void bind(int col,unsigned long long v) = 0;
			///
			/// Bind a floating point value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,double v) = 0;
			///
			/// Bind a floating point value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind(int col,long double v) = 0;
			///
			/// Bind a NULL value to column \a col (starting from 1).
			///
			/// Should throw invalid_placeholder() if the value of col is out of range. May
			/// ignore if it is impossible to know whether the placeholder exists without special
			/// support from back-end.
			///
			virtual void bind_null(int col) = 0;
			///
			/// Fetch the last sequence generated for last inserted row. May use sequence as parameter
			/// if the database uses sequences, should ignore the parameter \a sequence if the last
			/// id is fetched without parameter.
			///
			/// Should be called after exec() for insert statement, otherwise the behavior is undefined.
			///
			/// MUST throw not_supported_by_backend() if such option is not supported by the DB engine.
			///
			virtual long long sequence_last(std::string const &sequence) = 0;
			///
			/// Return the number of affected rows by last statement.
			///
			/// Should be called after exec(), otherwise behavior is undefined.
			///
			virtual unsigned long long affected() = 0;
			///
			/// Return SQL Query result, MAY throw cppdb_error if the statement is not a query
			///
			virtual result *query() = 0;
			///
			/// Execute a statement, MAY throw cppdb_error if the statement returns results.
			///
			virtual void exec() = 0;
			// End of API

			// Caching support
			static void dispose(statement *selfp);
			
			void cache(statements_cache *c);
			statement();
			virtual ~statement() ;
		protected:
			struct data;
			std::auto_ptr<data> d;
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
			virtual ~connection();
			void set_pool(ref_ptr<pool> p);
			void set_driver(ref_ptr<loadable_driver> drv);
			static void dispose(connection *c);
			ref_ptr<statement> prepare(std::string const &q);

			// API 

			///
			/// Start new isolated transaction. Would not be called
			/// withing other transaction on current connection.
			///
			virtual void begin() = 0;
			///
			/// Commit the transaction, you may assume that is called after begin()
			/// was called.
			///
			virtual void commit() = 0;
			///
			/// Rollback the transaction. MUST never throw!!!
			///
			virtual void rollback() = 0;
			///
			/// Create a prepared statement \a q. May throw if preparation had failed.
			/// Should never return null value.
			///
			virtual statement *real_prepare(std::string const &q) = 0;
			///
			/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
			///
			virtual std::string escape(std::string const &) = 0;
			///
			/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
			///
			virtual std::string escape(char const *s) = 0;
			///
			/// Escape a string for inclusion in SQL query. May throw not_supported_by_backend() if not supported by backend.
			///
			virtual std::string escape(char const *b,char const *e) = 0;
			///
			/// Get the name of the driver, for example sqlite3, odbc
			///
			virtual std::string driver() = 0;
			///
			/// Get the name of the SQL Server, for example sqlite3, mssql, oracle, differs from driver() when
			/// the backend supports multiple databases like odbc backend.
			///
			virtual std::string engine() = 0;

			// API
			
		private:
			struct data;
			std::auto_ptr<data> d;
			statements_cache cache_;
			ref_ptr<loadable_driver> driver_;
			ref_ptr<pool> pool_;
		};


	} // backend
} // cppdb

#endif
