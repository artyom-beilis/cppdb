#include "backend.h"
#include <sstream>
#include <iostream>
#include <stdexcept>

extern "C" { 
	cppdb::backend::driver *cppdb_sqlite3_get_driver(); 
	cppdb::backend::driver *cppdb_postgres_get_driver();
}

using namespace std;

#define TEST(x) do { if(x) break; std::ostringstream ss; ss<<"Failed in " << __LINE__ << #x; throw std::runtime_error(ss.str()); } while(0)

void test(cppdb::backend::driver *drv,char const *conn_str)
{
	auto_ptr<cppdb::backend::connection> sql;
	auto_ptr<cppdb::backend::statement> stmt;
	auto_ptr<cppdb::backend::result> res;
	
	sql.reset(drv->open(conn_str));
	try {
		stmt.reset(sql->prepare("drop table test"));
		stmt->exec(); 
	}catch(...) {}
	stmt.reset(sql->prepare("create table test ( x integer not null, y varchar )"));
	stmt->exec();
	stmt.reset(sql->prepare("select * from test"));
	res.reset(stmt->query());
	TEST(!res->next());
	stmt.reset(sql->prepare("insert into test(x,y) values(10,'foo')"));
	stmt->exec();
	stmt.reset(sql->prepare("select x,y from test"));
	res.reset(stmt->query());
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
	stmt.reset(sql->prepare("insert into test(x,y) values(20,NULL)"));
	stmt->exec();
	stmt.reset(sql->prepare("select y from test where x=?"));
	stmt->bind(1,20);
	res.reset(stmt->query());
	TEST(res->next());
	TEST(res->is_null(0));
	TEST(!res->next());

}



int main(int argc,char **argv)
{
	try {
		std::cout << "Testing sqlite3" << std::endl;
		test(cppdb_sqlite3_get_driver(),"test.db");
		std::cout << "Testing postgresql" << std::endl;
		test(cppdb_postgres_get_driver(),"dbname='test'");
		std::cout << "Ok" << std::endl;
	}
	catch(std::exception const &e) {
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
