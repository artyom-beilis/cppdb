#ifndef CPPDB_ERRORS_H
#define CPPDB_ERRORS_H
#include <stdexcept>

namespace cppdb {
	class cppdb_error : public std::runtime_error {
	public:
		cppdb_error(std::string const &v) : std::runtime_error(v) {}
	};

	class bad_value_cast : public cppdb_error {
	public:
		bad_value_cast() : cppdb_error("cppdb::bad_value_cast can't convert data")
		{
		}
	};

	class invalid_column : public cppdb_error {
	public:
		invalid_column() : cppdb_error("cppdb::invalid_colunm attempt access to invalid column")
		{
		}
	};
	class invalid_placeholder : public cppdb_error {
	public:
		invalid_placeholder() : cppdb_error("cppdb::invalid_placeholder attempt bind to invalid placeholder")
		{
		}
	};

}

#endif
