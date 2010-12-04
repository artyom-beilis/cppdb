#include <cppdb/driver_manager.h>
#include <cppdb/conn_manager.h>
#include "test.h"
#include "dummy_driver.h" 

#if ( defined(WIN32) || defined(_WIN32) || defined(__WIN32) ) && !defined(__CYGWIN__)
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <windows.h>
void sleep(int x)
{
	Sleep(1000*x);
}
#else
# include <stdlib.h>
#endif

void test_driver_manager()
{
	cppdb::ref_ptr<cppdb::backend::connection> c1,c2,c3,c4;
	cppdb::driver_manager &dm = cppdb::driver_manager::instance();
	cppdb::connections_manager &cm = cppdb::connections_manager::instance();
	dm.install_driver("dummy",new dummy::loadable_driver());
	TEST(dummy::drivers==1);
	dm.collect_unused();
	TEST(dummy::drivers==0);
	dm.install_driver("dummy",new dummy::loadable_driver());
	c1=dm.connect("dummy:");
	TEST(dummy::connections==1);
	dm.collect_unused();
	TEST(dummy::drivers==1);
	TEST(dummy::connections==1);
	c2=dm.connect("dummy:");
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	c1=0;
	TEST(dummy::connections==1);
	c2=0;
	TEST(dummy::connections==0);
	TEST(dummy::drivers==1);
	dm.collect_unused();
	TEST(dummy::drivers==0);
	THROWS(c1=dm.connect("dummy:"),cppdb::cppdb_error);
	dm.install_driver("dummy",new dummy::loadable_driver());
	c1=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==1);
	c2=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==2);
	c3=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==3);
	c1.reset();
	TEST(dummy::connections==3);
	c2.reset();
	TEST(dummy::connections==3);
	c3.reset();
	TEST(dummy::connections==2);
	c3=cm.open("dummy:@pool_size=2;@pool_max_idle=2");
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	sleep(3);
	TEST(dummy::connections==2);
	TEST(dummy::drivers==1);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==1);
	TEST(dummy::drivers==1);
	c3.reset();
	TEST(dummy::connections==1);
	TEST(dummy::drivers==1);
	sleep(3);
	cm.gc();
	dm.collect_unused();
	TEST(dummy::connections==0);
	TEST(dummy::drivers==0);
}

int main()
{
	try {
		test_driver_manager();
	}
	CATCH_BLOCK()
	try {
		//test_pool();
	}
	CATCH_BLOCK()
	SUMMARY();

}
