#include "shared_object.h"

#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#	include <windows.h>
#	define RTLD_LAZY 0

	namespace cppdb {
		namespace {
			void *dlopen(char const *name,int /*unused*/)
			{
				return LoadLibrary(name);
			}
			void dlclose(void *h)
			{
				HMODULE m=(HMODULE)(h);
				FreeLibrary(m);
			}
			void *dlsym(void *h,char const *sym)
			{
				HMODULE m=(HMODULE)(h);
				return (void *)GetProcAddress(m,sym);
			}
		}
	}

#else
#	include <dlfcn.h>
#endif
namespace cppdb {
	shared_object::shared_object(std::string name,void *h) :
		dlname_(name),
		handle_(h)
	{
	}
	shared_object::~shared_object()
	{
		dlclose(handle_);
	}
	ref_ptr<shared_object> shared_object::open(std::string const &name)
	{
		ref_ptr<shared_object> dl;
		void *h=dlopen(name.c_str(),RTLD_LAZY);
		if(!h) {
			return dl;
		}
		try {
			dl.reset(new shared_object(name,h));
			h=0;
			return dl;
		}
		catch(...) {
			if(h) {
				dlclose(h);
			}
			throw;
		}
	}
	void *shared_object::sym(std::string const &s)
	{
		return dlsym(handle_,s.c_str());
	}
	void *shared_object::safe_sym(std::string const &s)
	{
		void *p=sym(s);
		if(!p) {
			throw cppdb_error("cppdb::shared_object::failed to resolve symbol [" + s +"] in " + dlname_);
		}
		return p;
	}
} // cppdb
