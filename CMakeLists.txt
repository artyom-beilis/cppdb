cmake_minimum_required(VERSION 2.6)
project(cppdb)
include(CheckLibraryExists)
include(CheckCXXSourceCompiles)

find_library(ODBC_LIB odbc NAMES odbc32)
find_path(ODBC_PATH sqlext.h)
if(ODBC_LIB AND ODBC_PATH)
	set(ENABLE_ODBC ON)
endif()

find_library(SQLITE3_LIB sqlite3)
find_path(SQLITE3_PATH sqlite3.h)
if(SQLITE3_LIB AND SQLITE3_PATH)
	set(ENABLE_SQLITE3 ON)
endif()

find_library(PQ_LIB pq)
find_path(PQ_PATH postgresql/libpq-fe.h)
if(PQ_LIB AND PQ_PATH)
	set(ENABLE_PQ ON)
endif()

if(NOT WIN32)
	find_library(DL_LIB dl)
endif()

check_cxx_source_compiles(
	"int main() { volatile int v=0; return __sync_bool_compare_and_swap(&v,0,1); }"
	CPPDB_HAS_GCC_SYNC)
if(CPPDB_HAS_GCC_SYNC)
	add_definitions(-DCPPDB_HAS_GCC_SYNC)
endif()

check_cxx_source_compiles(
	"#include <bits/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	CPPDB_HAVE_GCC_BITS_EXCHANGE_AND_ADD)
if(CPPDB_HAVE_GCC_BITS_EXCHANGE_AND_ADD)
	add_definitions(-DCPPDB_HAVE_GCC_BITS_EXCHANGE_AND_ADD)
endif()

check_cxx_source_compiles(
	"#include <ext/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	CPPDB_HAVE_GCC_EXT_EXCHANGE_AND_ADD)
if(CPPDB_HAVE_GCC_EXT_EXCHANGE_AND_ADD)
	add_definitions(-DCPPDB_HAVE_GCC_EXT_EXCHANGE_AND_ADD)
endif()

check_cxx_source_compiles(
	"#include <sys/types.h>
	#include <machine/atomic.h>
	int main() { volatile unsigned v=0; return atomic_cmpset_int(&v,1,0); }"
	CPPDB_HAVE_FREEBSD_ATOMIC)

if(CPPDB_HAVE_FREEBSD_ATOMIC)
	add_definitions(-DCPPDB_HAVE_FREEBSD_ATOMIC)
endif()

check_cxx_source_compiles(
	"#include <atomic.h>
	int main() { volatile unsigned v=0; return atomic_add_int_nv(&v,1); }"
	CPPDB_HAVE_SOLARIS_ATOMIC)
if(CPPDB_HAVE_SOLARIS_ATOMIC)
	add_definitions(-DCPPDB_HAVE_SOLARIS_ATOMIC)
endif()


check_cxx_source_compiles(
	"#include <libkern/OSAtomic.h>
	int main() { volatile int v=0; return OSAtomicAdd32(1,&v); }"
	CPPDB_HAVE_MAC_OS_X_ATOMIC)

if(CPPDB_HAVE_MAC_OS_X_ATOMIC)
	add_definitions(-DCPPDB_HAVE_MAC_OS_X_ATOMIC)
endif()

set(LIBSRC 
	utils.cpp
	mutex.cpp
	driver_manager.cpp
	conn_manager.cpp
	shared_object.cpp
	pool.cpp
	backend.cpp
	frontend.cpp
	atomic_counter.cpp
	)


if(ENABLE_SQLITE3)
	set(LIBSRC ${LIBSRC} sqlite3_backend.cpp)
	add_definitions(-DCPPDB_WITH_SQLITE3)
	include_directories(${SQLITE3_PATH})
endif()

if(ENABLE_PQ)
	set(LIBSRC ${LIBSRC} postgres_backend.cpp)
	add_definitions(-DCPPDB_WITH_PQ)
	include_directories(${PQ_PATH})
endif()

if(ENABLE_ODBC)
	set(LIBSRC ${LIBSRC} odbc_backend.cpp)
	add_definitions(-DCPPDB_WITH_ODBC)
	include_directories(${ODBC_PATH})
endif()

add_library(cppdb ${LIBSRC})
if(ENABLE_SQLITE3)
	target_link_libraries(cppdb ${SQLITE3_LIB})
endif()
if(ENABLE_PQ)
	target_link_libraries(cppdb ${PQ_LIB})
endif()

if(ENABLE_ODBC)
	target_link_libraries(cppdb ${ODBC_LIB})
endif()

if(NOT WIN32)
	target_link_libraries(cppdb ${DL_LIB})
endif()

add_executable(test_basic test_basic.cpp)
add_executable(test_backend test_backend.cpp)

target_link_libraries(test_basic cppdb)
target_link_libraries(test_backend cppdb)
