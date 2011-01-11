#!/bin/bash


# This is not supported by freetds driver
#	'odbc:DSN=MSSQL;UID=root;PWD=rootroot;@engine=mssql' \
#	'odbc:DSN=MSSQL;UID=root;PWD=rootroot;@engine=mssql;@utf=wide' \

rm Fail.txt

for CS in \
	'sqlite3:db=test.db' \
	'postgresql:dbname=test' \
	'postgresql:dbname=test;@blob=bytea' \
	'mysql:database=test;user=root;password=root' \
	'odbc:Driver=MySQL;UID=root;PWD=root;Database=test;@engine=mysql' \
	'odbc:Driver=PostgreSQL ANSI;Database=test;@engine=postgresql' \
	'odbc:Driver=Sqlite3;Database=/tmp/test.db;@engine=sqlite3' \

do
	for OP in '' ';@use_prepared=off' ';@pool_size=8' ';@use_prepared=off;@pool_size=8'
	do
		echo $CS$OP
		if ! ./test_backend "$CS$OP" &> rep.txt
		then
			echo " ------------------ FAIL test_backend ------"
			cat rep.txt >> Fail.txt
			echo " -------------------------------------------"
		else
			echo " test_backend - OK"
		fi
		if ! ./test_basic "$CS$OP" &> rep.txt
		then
			echo " ------------------ FAIL test_basic   ------"
			cat rep.txt >> Fail.txt
			echo " -------------------------------------------"
		else
			echo " test_basic - OK"
		fi
	done
done

if ! ./test_caching &> rep.txt
then
	echo " ------------------ FAIL test_caching   ------"
	cat rep.txt >> Fail.txt
	echo " -------------------------------------------"
else
	echo " test_caching - OK"
fi

