#ifndef CPPDB_BACKEND_H
#define CPPDB_BACKEND_H
#include <iosfwd>
#include <ctime>
#include <string>

namespace cppdb {
	using boost::shared_ptr;
	
	class result {
	public:
		result(statement &st)
		{
			res_.reset(st.stat_->query());
		}
		int cols()
		{
			return res_->cols();
		}
		template<typename T>
		T get(std::string const &name)
		{
			return get<T>(res_->name_to_column(name));
		}
		template<typename T>
		T get(int col)
		{
			T value;
			if(res_->fetch(col,value))
				return value;
			throw null_value(); 
		}

		template<typename T>
		bool fetch(int col,T &value)
		{
			return res_->fetch(col,value);
		}

		template<typename T>
		result &operator>>(std::pair<T*,bool *> value)
		{
			assert(value.first!=0);
			assert(value.second!=0);
			*value.second = res_->fetch(current_col_++,*value.first);
			return *this;
		}

		bool is_null(int col)
		{
			return res_->is_null(col);
		}
		bool is_null(std::string const &n)
		{
			return res_->is_null(res_->name_to_column(n));
		}

		template<typename T>
		result &operator>>(T &value)
		{
			if(!res_->fetch(current_col_++,value))
				throw null_value();
			return *this;
		}

		bool next()
		{
			current_col_ = 0;
			return res_->next();
		}
		
	private:
		shared_ptr<backend::result> res_;
	};

	class row : public result {
	};


	class statement {
	public:
		virtual ~statement() {}
		virtual void reset() = 0;
		virtual void bind(int col,std::string const &) = 0;
		virtual void bind(int col,char const *s) = 0;
		virtual void bind(int col,char const *b,char const *e) = 0;
		virtual void bind(int col,std::tm const &) = 0;
		virtual void bind(int col,std::istream const &) = 0;
		virtual void bind(int col,int v) = 0;
		virtual void bind(int col,unsigned v) = 0;
		virtual void bind(int col,long v) = 0;
		virtual void bind(int col,unsigned long v) = 0;
		virtual void bind(int col,long long v) = 0;
		virtual void bind(int col,unsigned long long v) = 0;
		virtual void bind(int col,double v) = 0;
		virtual void bind(int col,long double v) = 0;
		virtual void bind_null(int col) = 0;
		virtual void bind_sequence(long long &value,std::string const &sequence) = 0;
		virtual unsigned long long affected() = 0;
		virtual result *query() = 0;
		virtual void exec() = 0;
	};

	class connection {
	public:
		virtual ~connection() {}
		virtual void begin() = 0;
		virtual void commit() = 0;
		virtual void rollback() = 0;
		virtual statement *prepare(std::string const &) = 0;
		virtual std::string escape(std::string const &) = 0;
		virtual std::string escape(char const *s) = 0;
		virtual std::string escape(char const *b,char const *e) = 0;
	};

} // cppdb

#endif
