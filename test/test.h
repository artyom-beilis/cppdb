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
#ifndef CPPDB_TEST_H
#define CPPDB_TEST_H
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <stdlib.h>

int last_line = 0;
int passed = 0;
int failed = 0;

#define TEST(x) do { last_line = __LINE__; if(x) { passed ++; break; } failed++; std::cerr<<"Failed in " << __LINE__ <<' '<< #x << std::endl; } while(0)

#define THROWS(x,ex) do { last_line = __LINE__; try { x ; failed++; std::cerr << "Failed in " << __LINE__ <<' '<< #x << std::endl; }catch(ex const &/*un*/) { passed++; } } while(0)
#endif

#define CATCH_BLOCK() catch(std::exception const &e) { std::cerr << "Fail " << e.what() << std::endl; std::cerr << "Last tested line " << last_line  << std::endl; failed++; }
#define SUMMARY() do {\
	std::cout << "Tests: " << passed+failed << " failed: " << failed << std::endl; \
	if(failed > 0) { std::cerr << "Fail!" << std::endl; return 1; } \
	std::cout << "Ok" << std::endl; \
	return 0; \
} while(0)

