#include "frontend.h"
#include "driver_manager.h"
#include "pool.h"
#include <iostream>
#include <sstream>

#define TEST(x) do { if(x) break; std::ostringstream ss; ss<<"Failed in " << __LINE__ <<' '<< #x; throw std::runtime_error(ss.str()); } while(0)


int main(int argc,char **argv)
{
	std::string cs = "sqlite3:db=db.db";
	if(argc >= 2) {
		cs = argv[1];
	}
	try {
		cppdb::session sql(cs);
		
		try {
			sql << "DROP TABLE test" << cppdb::exec;
		}
		catch(cppdb::cppdb_error const &e){}
		if(sql.engine() == "sqlite3") {
			sql<<	"create table test ( id integer primary key autoincrement not null, "
				"n integer, f real , t timestamp ,name text )" << cppdb::exec;
		}
		else if(sql.engine() == "mysql") {
			sql<<	"create table test ( id integer primary key auto_increment not null, "
				"n integer, f real , t timestamp ,name text )" << cppdb::exec;
		}
		else if(sql.engine() == "postgresql" )  {
			sql<<	"create table test ( id  serial  primary key not null "
				",n integer, f double precision , t timestamp ,name text )" << cppdb::exec;
		}
		else if(sql.engine() == "mssql" )  {
			sql<<	"create table test ( id integer identity(1,1) primary key not null "
				",n integer, f double precision , t datetime ,name text )" << cppdb::exec;
		}
		else {
			std::cerr << "Unknown engine: " << sql.engine() << std::endl;
			return 1;
		}
		std::tm t;
		time_t tt;
		tt=time(NULL);
		t = *localtime(&tt);
		std::cout<<asctime(&t);
		std::string torig=asctime(&t);
		int n;
		long long rowid;
		{
			cppdb::statement stat = sql<<"insert into test(n,f,t,name) values(?,?,?,?)"
				<<10<<3.1415926565<<t
				<<"Hello \'World\'";
			stat.exec();
			rowid = stat.sequence_last("test_id_seq"); 
			TEST(rowid==1);
			TEST(stat.affected()==1);
			std::cout<<"ID: "<<rowid<<", Affected rows "<<stat.affected()<<std::endl;
		}
		{
			cppdb::statement stat = sql<<"insert into test(n,f,t,name) values(?,?,?,?)"
				<< cppdb::use(10,cppdb::not_null_value)
				<< cppdb::use(3.1415926565,cppdb::null_value)
			<< cppdb::use(t,cppdb::not_null_value)
			<< cppdb::use("Hello \'World\'",cppdb::not_null_value)
			<< cppdb::exec;
			rowid = stat.sequence_last("test_id_seq"); 
			std::cout<<"ID: "<<rowid<<", Affected rows "<<stat.affected()<<std::endl;
			TEST(rowid==2);
			TEST(stat.affected()==1);
		}

		cppdb::result res;
		if(sql.engine()=="mssql")
			res = sql<<"select top 10 id,n,f,t,name from test";
		else
			res = sql<<"select id,n,f,t,name from test limit 10";
		n=0;
		while(res.next()){
			double f=-1;
			int id=-1,k=-1;
			std::tm atime={0};
			std::string name="nonset";
			cppdb::null_tag_type tag;
			res >> id >> k >> cppdb::into(f,tag) >> atime >> name;
			std::cout <<id << ' '<<k <<' '<<f<<' '<<name<<' '<<asctime(&atime)<<std::endl;
			TEST(id==n+1);
			TEST(k==10);
			TEST(n==0 ? f==3.1415926565: f==-1);
			TEST(n==0 ? tag==cppdb::not_null_value : tag==cppdb::null_value);
			TEST(asctime(&atime) == torig);
			TEST(name=="Hello 'World'");
			n++;
		}
		TEST(n==2);

		res = sql << "SELECT n FROM test WHERE id=?" << 1 << cppdb::row;
		TEST(!res.empty());
		int val;
		res >> val;
		TEST(!res.next());
		TEST(val == 10);

		cppdb::statement stat = sql<<"delete from test where 1<>0" << cppdb::exec;
		std::cout<<"Deleted "<<stat.affected()<<" rows\n";
		TEST(stat.affected()==2);
		return 0;
			
	}
	catch(std::exception const &e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
