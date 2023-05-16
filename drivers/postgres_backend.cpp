#include "../cppdb/postgres_backend.h"
CPPDB_DRIVER_API cppdb::backend::connection *cppdb_postgresql_get_connection(cppdb::connection_info const &cs)
{
	return new cppdb::postgresql::connection(cs);
}
