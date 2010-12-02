#ifndef CPPDB_SHARED_OBJECT_H
#define CPPDB_SHARED_OBJECT_H

#include "defs.h"
#include "ref_ptr.h"


namespace cppdb {
	class CPPDB_API shared_object : public ref_counted {
		shared_object() : handle_(0) {}
		shared_object(std::string name,void *h);
		shared_object(shared_object const &);
		void operator=(shared_object const &);
	public:
		~shared_object();
		static ref_ptr<shared_object> open(std::string const &name);
		void *safe_sym(std::string const &name);
		void *sym(std::string const &name);

		template<typename T>
		bool resolve(std::string const &s,T *&v)
		{
			void *p=sym(s);
			if(!p) {
				return false;
			}
			v=(T*)(p);
			return true;
		}
		template<typename T>
		void safe_resolve(std::string const &s,T *&v)
		{
			v=(T*)(sym(s));
		}

	private:
		std::string dlname_;
		void *handle_;
	};
}


#endif
