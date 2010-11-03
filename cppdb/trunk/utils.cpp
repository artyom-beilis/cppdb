#include "utils.h"
#include "errors.h"

namespace cppdb {
	std::string format_time(std::tm const &v)
	{
		char buf[64]= {0};
		strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&v);
		return buf;
	}

	std::tm parse_time(char const *v)
	{
		std::tm tmp = std::tm();
		tmp.tm_isdst = -1;
		char const *ptr = strptime(v,"%Y-%m-%d",&tmp);
		if(!ptr)
			throw bad_value_cast();
		if(!*ptr) {
			mktime(&tmp);
			return tmp;
		}
		ptr = strptime(ptr," %H:%M",&tmp);
		if(!ptr)
			throw bad_value_cast();
		if(!*ptr) {
			mktime(&tmp);
			return tmp;
		}
		ptr = strptime(ptr,":%S",&tmp);
		if(!ptr || *ptr)
			throw bad_value_cast();
		mktime(&tmp);
		return tmp;
	}
}
