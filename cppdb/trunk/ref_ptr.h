#ifndef CPPDB_REF_PTR_H
#define CPPDB_REF_PTR_H
#include "errors.h"
#include "mutex.h"

namespace cppdb {
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

	class ref_counted {
	public:
		ref_counted() : count_(0)
		{
		}
		virtual ~ref_counted()
		{
		}
		int add_ref()
		{
			return ++count_;
		}
		int use_count() const
		{
			int val = count_;
			return val;
		}
		int del_ref()
		{
			return --count_;
		}
		static void dispose(ref_counted *p)
		{
			delete p;
		}
	private:
		int count_;
	};

	class mt_ref_counted {
	public:
		mt_ref_counted() : count_(0)
		{
		}
		virtual ~mt_ref_counted()
		{
		}
		static void dispose(mt_ref_counted *p)
		{
			delete p;
		}
		int add_ref()
		{
			mutex::guard g(lock_);
			int new_val = ++count_;
			return new_val;

		}
		int use_count() const
		{
			mutex::guard g(lock_);
			int val = count_;
			return val;
		}
		int del_ref()
		{
			mutex::guard g(lock_);
			int new_val =  --count_;
			return new_val;
		}
	private:
		mutable mutex lock_;
		int count_;
	};
} // cppdb
#endif
