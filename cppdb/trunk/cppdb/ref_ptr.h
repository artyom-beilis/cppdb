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
#ifndef CPPDB_REF_PTR_H
#define CPPDB_REF_PTR_H
#include <cppdb/errors.h>
#include <cppdb/atomic_counter.h>

namespace cppdb {
	///
	/// \brief This is a smart intrusive reference counting pointer that throws a error on empty
	/// access.
	///
	template<typename T>
	class ref_ptr {
	public:
		ref_ptr(T *v=0) : p(0)
		{
			reset(v);
		}
		~ref_ptr()
		{
			reset();
		}
		ref_ptr(ref_ptr const &other) : p(0)
		{
			reset(other.p);
		}
		ref_ptr const &operator=(ref_ptr const &other)
		{
			reset(other.p);
			return *this;
		}
		T *get() const
		{
			return p;
		}
		operator bool() const
		{
			return p!=0;
		}
		T *operator->() const
		{
			if(!p)
				throw cppdb_error("Attempt to access an empty object");
			return p;
		}
		T &operator*() const
		{
			if(!p)
				throw cppdb_error("Attempt to access an empty object");
			return *p;
		}
		void reset(T *v=0)
		{
			if(v==p)
				return;
			if(p) {
				if(p->del_ref() == 0) {
					T::dispose(p);
				}
				p=0;
			}
			if(v) {
				v->add_ref();
			}
			p=v;
		}
	private:
		T *p;
	};

	///
	/// \brief This is a class that implements reference counting and designed to be used with ref_ptr
	///
	class ref_counted {
	public:
		ref_counted() : count_(0) 
		{
		}
		virtual ~ref_counted()
		{
		}
		long add_ref()
		{
			return ++count_;
		}
		long use_count() const
		{
			long val = count_;
			return val;
		}
		long del_ref()
		{
			return --count_;
		}
		static void dispose(ref_counted *p)
		{
			delete p;
		}
	private:
		atomic_counter count_;
	};
} // cppdb
#endif
