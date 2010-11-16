#ifndef CPPDB_DEFS_H
#define CPPDB_DEFS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN__)
#  ifdef DLL_EXPORT
#    ifdef CPPDB_SOURCE
#      define CPPDB_API __declspec(dllexport)
#    else
#      define CPPDB_API __declspec(dllimport)
#    endif
#  else
#    define CPPDB_API
#  endif
#else
#  define CPPDB_API
#endif

#endif
