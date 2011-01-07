///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2010-2011  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  Distributed under:
//
//                   the Boost Software License, Version 1.0.
//              (See accompanying file LICENSE_1_0.txt or copy at 
//                     http://www.boost.org/LICENSE_1_0.txt)
//
//  or (at your opinion) under:
//
//                               The MIT License
//                 (See accompanying file MIT.txt or a copy at
//              http://www.opensource.org/licenses/mit-license.php)
//
///////////////////////////////////////////////////////////////////////////////
#include <cppdb/frontend.h>
#include <iostream>
#include <stdlib.h>

int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "conn string required" << std::endl;
		return 1;
	}
	try {
		static const int max_val = 10000;
		cppdb::session sql(argv[1]);

		try { sql << "DROP TABLE test" << cppdb::exec; } catch(...) {}

		if(sql.engine() == "mysql")
			sql << "create table test ( id integer primary key, val varchar(100)) Engine=innodb" << cppdb::exec;
		else
			sql << "create table test ( id integer primary key, val varchar(100))" << cppdb::exec;
		
		{
			cppdb::transaction tr(sql);
			for(int i=0;i<max_val;i++) {
				sql << "insert into test values(?,?)" << i << "Hello World" << cppdb::exec;
			}
			tr.commit();
		}

		time_t start = time(0);
		
		for(int j=0;j<max_val * 10;j++) {
			std::string v;
			sql << "select val from test where id = ?" << (rand() % max_val)<< cppdb::row >> v;
			if(v!="Hello World")
				throw std::runtime_error("Wrong");
		}
		time_t stop = time(0);
		std::cout << "Passed " << stop - start << " seconds" << std::endl;
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
