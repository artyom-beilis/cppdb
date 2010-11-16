cmake_minimum_required(VERSION 2.6)
project(cppdb)
include(CheckLibraryExists)

find_library(SQLITE3_LIB sqlite3)
find_library(PQ_LIB pq)
find_library(DL_LIB dl)

set(LIBSRC utils.cpp mutex.cpp driver_manager.cpp shared_object.cpp)
if(SQLITE3_LIB)
	set(LIBSRC ${LIBSRC} sqlite3_backend.cpp)
	add_definitions(-DCPPDB_WITH_SQLITE3)
endif()

if(PQ_LIB)
	set(LIBSRC ${LIBSRC} postgres_backend.cpp)
	add_definitions(-DCPPDB_WITH_PQ)
endif()

add_library(cppdb ${LIBSRC})
if(SQLITE3_LIB)
	target_link_libraries(cppdb ${SQLITE3_LIB})
endif()
if(PQ_LIB)
	target_link_libraries(cppdb ${PQ_LIB})
endif()

target_link_libraries(cppdb ${DL_LIB})

add_executable(test_basic test_basic.cpp)
add_executable(test_backend test_backend.cpp)
target_link_libraries(test_basic cppdb)
target_link_libraries(test_backend cppdb)
