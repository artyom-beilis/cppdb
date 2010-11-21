cmake_minimum_required(VERSION 2.6)
project(cppdb)
include(CheckLibraryExists)

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

find_library(DL_LIB dl)

set(LIBSRC utils.cpp mutex.cpp driver_manager.cpp shared_object.cpp pool.cpp)
if(ENABLE_SQLITE3)
	set(LIBSRC ${LIBSRC} sqlite3_backend.cpp)
	add_definitions(-DCPPDB_WITH_SQLITE3)
	include_directories(SQLITE3_PATH)
endif()

if(ENABLE_PQ)
	set(LIBSRC ${LIBSRC} postgres_backend.cpp)
	add_definitions(-DCPPDB_WITH_PQ)
	include_directories(PQ_PATH)
endif()

add_library(cppdb ${LIBSRC})
if(ENABLE_SQLITE3)
	target_link_libraries(cppdb ${SQLITE3_LIB})
endif()
if(ENABLE_PQ)
	target_link_libraries(cppdb ${PQ_LIB})
endif()

target_link_libraries(cppdb ${DL_LIB})

add_executable(test_basic test_basic.cpp)
add_executable(test_backend test_backend.cpp)

target_link_libraries(test_basic cppdb)
target_link_libraries(test_backend cppdb)
