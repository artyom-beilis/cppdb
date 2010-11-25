#include "backend.h"
#include "driver_manager.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>

extern "C" { 
	cppdb::backend::connection *cppdb_sqlite3_get_connection(); 
	cppdb::backend::connection *cppdb_postgres_get_connection();
}

using namespace std;

#define TEST(x) do { if(x) break; std::ostringstream ss; ss<<"Failed in " << __LINE__ <<' '<< #x; throw std::runtime_error(ss.str()); } while(0)


void test(std::string conn_str)
{
	cppdb::ref_ptr<cppdb::backend::connection> sql(cppdb::driver_manager::instance().connect(conn_str));
	cppdb::ref_ptr<cppdb::backend::statement> stmt;
	cppdb::ref_ptr<cppdb::backend::result> res;
	
	try {
		stmt = sql->prepare("drop table test");
		stmt->exec(); 
	}catch(...) {}
	stmt = sql->prepare("create table test ( x integer not null, y varchar(1000) )");
	stmt->exec();
	stmt = sql->prepare("select * from test");
	res = stmt->query();
	TEST(!res->next());
	stmt  = sql->prepare("insert into test(x,y) values(10,'foo')");
	stmt->exec();
	stmt = sql->prepare("select x,y from test");

	res = stmt->query();
	TEST(res->next());
	TEST(res->cols()==2);
	int iv;
	std::string sv;
	TEST(res->fetch(0,iv));
	TEST(iv==10);
	TEST(res->fetch(1,sv));
	TEST(sv=="foo");
	TEST(!res->next());
	res.reset();
	stmt = sql->prepare("insert into test(x,y) values(20,NULL)");
	stmt->exec();
	stmt = sql->prepare("select y from test where x=?");
	stmt->bind(1,20);
	res = stmt->query();
	TEST(res->next());
	TEST(res->is_null(0));
	sv="xxx";
	TEST(!res->fetch(0,sv));
	TEST(sv=="xxx");
	TEST(!res->next());
	res.reset();
	stmt->reset();
	stmt->bind(1,10);
	res = stmt->query();
	TEST(res->next());
	sv="";
	TEST(!res->is_null(0));
	TEST(res->fetch(0,sv));
	TEST(sv=="foo");
	TEST(!res->next());
	res.reset();
	stmt = sql->prepare("DELETE FROM test");
	stmt->exec();
}



int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "Usage: test_backend connection_string" << std::endl;
		return 1;
	}
	std::string cs = argv[1];
	try {
		test(cs);
		std::cout << "Ok" << std::endl;
	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
