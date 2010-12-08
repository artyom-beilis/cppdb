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
#ifndef CPPDB_SHARED_OBJECT_H
#define CPPDB_SHARED_OBJECT_H

#include <cppdb/defs.h>
#include <cppdb/ref_ptr.h>


namespace cppdb {
	///
	/// \brief This class allows to load and unload shared objects in simple and exception safe way.
	///
	class CPPDB_API shared_object : public ref_counted {
		shared_object() : handle_(0) {}
		shared_object(std::string name,void *h);
		shared_object(shared_object const &);
		void operator=(shared_object const &);
	public:
		~shared_object();
		///
		/// Load shared object, returns empty pointer if the object does not exits or not loadable
		///
		static ref_ptr<shared_object> open(std::string const &name);
		///
		/// Resolve symbol \a name and return pointer on it, throws cppdb_error if the symbol can't be resolved 
		///
		void *safe_sym(std::string const &name);

		///
		/// Resolve symbol \a name and return pointer on it, returns NULL if the symbol can't be resolved 
		///
		void *sym(std::string const &name);

		///
		/// Resolve symbol \a name and assign it to \a v, returns false if the symbol can't be resolved 
		///
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
		///
		/// Resolve symbol \a name and assign it to v, throws cppdb_error if the symbol can't be resolved 
		///
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
