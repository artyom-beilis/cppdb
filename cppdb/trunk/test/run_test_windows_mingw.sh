for CS in \
    'sqlite3:db=test.db' \
    'odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql' \
    'odbc:Driver=SQL Server Native Client 10.0;Databse=test;Server=localhost;UID=root;PWD=rootroot;@engine=mssql;@utf=wide' \
    
do
    echo $CS
    echo "----------BASIC-----------"
    ./test_basic "$CS"
    echo "----------BACKEND-----------"
    ./test_backend "$CS"
    echo "$CS;@pool_size=5"
    echo "----------BASIC-----------"
    ./test_basic "$CS;@pool_size=5"
    echo "----------BACKEND-----------"
    ./test_backend "$CS;@pool_size=5"
    echo "$CS;@use_prepared=off"
    echo "----------BASIC-----------"
    ./test_basic "$CS;@use_prepared=off"
    echo "----------BACKEND-----------"
    ./test_backend "$CS;@use_prepared=off"
done
