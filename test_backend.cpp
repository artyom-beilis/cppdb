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
	if(!(sql->engine()=="mssql" && (conn_str.find("utf=wide")==std::string::npos || conn_str.find("utf=narrow")!=std::string::npos))) {
		std::cout << "Testing unicode" << std::endl;
		stmt = sql->prepare("insert into test(x,y) values(?,?)");
		stmt->bind(1,15);
		stmt->bind(2,"שלום");
		stmt->exec();
		stmt = sql->prepare("select x,y from test");
		res = stmt->query();
		TEST(res->next());
		sv="";
		res->fetch(1,sv);
		TEST(sv=="שלום");
	}
	stmt = sql->prepare("drop table test");
	stmt->exec();
	if(sql->engine() == "mssql") 
		stmt = sql->prepare("create table test ( i integer, r real, t datetime, s varchar(1000))");
	else if(sql->engine() == "mysql") 
		stmt = sql->prepare("create table test ( i integer, r real, t datetime default null, s varchar(1000)) Engine = innodb");
	else
		stmt = sql->prepare("create table test ( i integer, r real, t timestamp, s varchar(1000))");
	stmt->exec();
	stmt = sql->prepare("insert into test values(?,?,?,?)");
	stmt->bind_null(1);
	stmt->bind_null(2);
	stmt->bind_null(3);
	stmt->bind_null(4);
	stmt->exec();
	TEST(stmt->affected()==1);
	stmt = sql->prepare("select i,r,t,s from test");
	res = stmt->query();
	TEST(res->next());
	{
		int i=-1; double r=-1; std::tm t=std::tm(); std::string s="def";
		TEST(!res->fetch(0,i));
		TEST(!res->fetch(1,r));
		TEST(!res->fetch(2,t));
		TEST(!res->fetch(3,s));
		TEST(i==-1 && r==-1 && t.tm_year == 0 && s=="def");
		TEST(!res->next());
	}
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	TEST(stmt->affected()==1);
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->commit();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	TEST(res->fetch(0,iv) && iv==2);
	iv=-1;
	stmt = sql->prepare("DELETE FROM test where 1<>0");
	stmt->exec();
	sql->begin();
	stmt = sql->prepare("insert into test(i) values(10)");
	stmt->exec();
	stmt = sql->prepare("insert into test(i) values(20)");
	stmt->exec();
	sql->rollback();
	stmt = sql->prepare("select count(*) from test");
	res = stmt->query();
	TEST(res->next());
	iv=-1;
	TEST(res->fetch(0,iv));
	TEST(iv==0);
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
