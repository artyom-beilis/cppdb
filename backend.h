#ifndef CPPDB_BACKEND_H
#define CPPDB_BACKEND_H
#include <iosfwd>
#include <ctime>
#include <string>

namespace cppdb {
	namespace backend {

		class result {
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

		class statement {
		public:
			virtual ~statement() {}
			virtual void reset() = 0;
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
			virtual void bind_sequence(long long &value,std::string const &sequence) = 0;
			virtual unsigned long long affected() = 0;
			virtual result *query() = 0;
			virtual void exec() = 0;
		};

		class connection {
		public:
			virtual ~connection() {}
			virtual void begin() = 0;
			virtual void commit() = 0;
			virtual void rollback() = 0;
			virtual statement *prepare(std::string const &) = 0;
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
