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
#ifndef CPPDB_DEFS_H
#define CPPDB_DEFS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN__)
#  if defined(DLL_EXPORT) || defined(CPPDB_EXPORTS)
#    ifdef CPPDB_SOURCE
#      define CPPDB_API __declspec(dllexport)
#    else
#      define CPPDB_API __declspec(dllimport)
#    endif
#  endif
#  if defined(DLL_EXPORT) || defined(CPPDB_DRIVER_EXPORTS)
#    ifdef CPPDB_DRIVER_SOURCE
#      define CPPDB_DRIVER_API __declspec(dllexport)
#    else
#      define CPPDB_DRIVER_API __declspec(dllimport)
#    endif
#  endif
#endif


#ifndef CPPDB_API
#  define CPPDB_API
#endif

#ifndef CPPDB_DRIVER_API
#  define CPPDB_DRIVER_API
#endif

#endif
