#include "utils.h"
#include "errors.h"

#include <sstream>
#include <locale>

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

	namespace {
		bool is_blank_char(char c)
		{
			return c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='\f';
		}
		std::string trim(std::string const &s)
		{
			if(s.empty())
				return s;
			size_t start=0,end=s.size()-1;
			while(start < s.size() && is_blank_char(s[start])) {
				start++;
			}
			while(end > start && is_blank_char(s[end])) {
				end--;
			}
			return s.substr(start,end-start+1);
		}
	}

	void parse_connection_string(	std::string const &connection_string,
					std::string &driver,
					std::map<std::string,std::string> &params)
	{
		params.clear();
		size_t p = connection_string.find(':');
		if( p == std::string::npos )
			throw cppdb_error("cppdb::Invalid connection string - no driver given");
		driver = connection_string.substr(0,p);
		p++;
		while(p<connection_string.size()) {
			size_t n=connection_string.find('=',p);
			if(n==std::string::npos)
				throw cppdb_error("Invalid connection string - invalid property");
			std::string key = trim(connection_string.substr(p,n-p));
			p=n+1;
			std::string value;
			while(p<connection_string.size() && is_blank_char(connection_string[p]))
			{
				++p;
			}
			if(p>=connection_string.size()) {
				/// Nothing - empty property
			}
			else if(connection_string[p]=='\'') {
				p++;
				while(true) {
					if(p>=connection_string.size()) {
						throw cppdb_error("Invalid connection string unterminated string");
					}
					if(connection_string[p]=='\'') {
						if(p+1 < connection_string.size() && connection_string[p+1]=='\'') {
							value+='\'';
							p+=2;
						}
						else {
							p++;
							break;
						}
					}
					else {
						value+=connection_string[p];
						p++;
					}
				}
			}
			else {
				size_t n=connection_string.find(';',p);
				if(n==std::string::npos) {
					value=trim(connection_string.substr(p));
					p=connection_string.size();
				}
				else {
					value=trim(connection_string.substr(p,n-p));
					p=n;
				}
			}
			if(params.find(key)!=params.end()) {
				throw cppdb_error("cppdb::invalid connection string duplicate key");
			}
			params[key]=value;
			while(p<connection_string.size()) {
				char c=connection_string[p];
				if(is_blank_char(c))
					++p;
				else if(c==';') {
					++p;
					break;
				}
			}
		}
	} //
	std::string connection_info::get(std::string const &prop,std::string const &default_value) const
	{
		properties_type::const_iterator p=properties.find(prop);
		if(p==properties.end())
			return default_value;
		else
			return p->second;
	}

	int connection_info::get(std::string const &prop,int default_value) const
	{
		properties_type::const_iterator p=properties.find(prop);
		if(p==properties.end())
			return default_value;
		std::istringstream ss;
		ss.imbue(std::locale::classic());
		ss.str(p->second);
		int val;
		ss >> val;
		if(!ss || !ss.eof()) {
			throw cppdb_error("cppdb::connection_info property " + prop + " expected to be integer value");
		}
		return val;
	}

}
