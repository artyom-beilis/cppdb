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

	class null_value_fetch : public cppdb_error {
	public:
		null_value_fetch() : cppdb_error("cppdb::null_value_fetch attempt fetch null column")
		{
		}
	};
	class empty_row_access : public cppdb_error {
	public:
		empty_row_access() : cppdb_error("cppdb::empty_row_access attempt to fetch from empty column")
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
	class multiple_rows_query : public cppdb_error {
	public:
		multiple_rows_query() : cppdb_error(	"cppdb::multiple_rows_query "
							"multiple rows result for a single row request")
		{
		}
	};

	class not_supported_by_backend : public cppdb_error {
	public:
		not_supported_by_backend(std::string const &e) :
			cppdb_error(e)
		{
		}
	};

}

#endif
