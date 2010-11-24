#ifdef _WIN32 
#include <windows.h>
#endif

#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

/*
 * see Retrieving ODBC Diagnostics
 * for a definition of extract_error().
 */
static void extract_error(
    int error,
    int line,
    SQLHANDLE h,
    SQLSMALLINT type)
{
	if(SQL_SUCCEEDED(error))
		return;
/*	if(SQL_SUCCEEDED(error) && error!=SQL_SUCCESS_WITH_INFO )
        return;*/
	SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH + 2] = {0};
	SQLCHAR stat[SQL_SQLSTATE_SIZE + 1] = {0};
	SQLINTEGER err;
	SQLSMALLINT len;
	SQLGetDiagRec(type,h,1,stat,&err,msg,sizeof(msg),&len);
	fprintf(stderr,"Failed %d:msg=%s stat=%s code=%d rval=%d\n",line,msg,stat,err,error);
	if(SQL_SUCCEEDED(error))
		return;
	exit(1);
}


#define STMT_CHECK() extract_error(ret,__LINE__,stmt,SQL_HANDLE_STMT)

void bind_val(int col,char *s,int type,SQLHSTMT stmt)
{
   int col_size = strlen(s);
   int prec = 0;
   int ret = SQLBindParameter(stmt,col,SQL_PARAM_INPUT,SQL_C_CHAR,type,col_size,prec,s,strlen(s),0);
   STMT_CHECK();
}

int main(int argc,char **argv) 
{
  if(argc<1) {
	  printf("%s usage ConnectionString [drop]");
  }
  int drop = 0;
  if(argc==3 && strcmp(argv[2],"drop")==0)
	  drop = 1;
  setlocale(LC_ALL,"");
  SQLHENV env;
  SQLHDBC dbc;
  SQLHSTMT stmt;
  SQLRETURN ret; /* ODBC API return status */
  SQLCHAR outstr[1024];
  SQLSMALLINT outstrlen;

  /* Allocate an environment handle */
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
  /* We want ODBC 3 support */
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
  /* Allocate a connection handle */
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
  /* Connect to the DSN mydsn */
  ret = SQLDriverConnect(dbc, NULL, argv[1], SQL_NTS,
			 0, 0, 0,
			 SQL_DRIVER_COMPLETE);
  if (SQL_SUCCEEDED(ret)) {
    printf("Connected\n");
    if (ret == SQL_SUCCESS_WITH_INFO) {
      printf("Driver reported the following diagnostics\n");
      extract_error(ret,__LINE__, dbc, SQL_HANDLE_DBC);
    }
    SQLAllocHandle(SQL_HANDLE_STMT, dbc ,&stmt);
 
    if(drop) {
	    ret = SQLExecDirect(stmt,"drop table test",SQL_NTS);
	    ret = SQLExecDirect(stmt,"create table test ( id integer, name varchar(1024), tm datetime, fl real)",SQL_NTS);
	    STMT_CHECK();
    }
	
    ret = SQLPrepare(stmt, "INSERT into test(id,name,tm,fl) VALUES(?,?,?,?)", SQL_NTS) ;
    STMT_CHECK();

    bind_val(1,"124",SQL_INTEGER,stmt);
    bind_val(2,"To be or not to be, To be or not to be To be or not to be To be or not to be "
                "To be or not to be, To be or not to be To be or not to be To be or not to be "
                "To be or not to be, To be or not to be To be or not to be To be or not to be "
            ,SQL_LONGVARCHAR,stmt);
    bind_val(3,"2010-01-01 20:32:22",SQL_TYPE_TIMESTAMP,stmt);
    bind_val(4,"13.123456789123456789e-15",SQL_DOUBLE,stmt);
    ret = SQLExecute(stmt);
    STMT_CHECK();
    SQLLEN rows;
    ret = SQLRowCount(stmt,&rows);
    STMT_CHECK();
    printf("Affected rows %d\n",rows);

    ret = SQLPrepare(stmt, "SELECT * FROM test", SQL_NTS) ;
    STMT_CHECK();
    ret = SQLExecute(stmt);
    STMT_CHECK();
    while((ret = SQLFetch(stmt))!=SQL_NO_DATA) {
        int col;
        for(col=1;col<=4;col++){
            SQLLEN len=0;
            char buf[64] = {0};
            ret = SQLGetData(stmt,col,SQL_C_CHAR,buf,sizeof(buf),&len);
            if(ret==SQL_SUCCESS_WITH_INFO) {
		if(len == SQL_NO_TOTAL) {
			do{
                		printf("%s",buf);
				memset(buf,0,sizeof(buf));
			}while(SQLGetData(stmt,col,SQL_C_CHAR,buf,sizeof(buf),&len) == SQL_SUCCESS_WITH_INFO);
			printf("|");
		}
		else {
			char *array = calloc(len+1,1);
			ret = SQLGetData(stmt,col,SQL_C_CHAR,array,len+1,&len);
			STMT_CHECK();
			printf("%s%s|",buf,array);
			free(array);
		}
            }
            else {
                printf("%s|",buf);
            }
        }
        printf("\n");
   }


    SQLDisconnect(dbc);		/* disconnect from driver */
  } else {
    fprintf(stderr, "Failed to connect\n");
    extract_error(ret,__LINE__, dbc, SQL_HANDLE_DBC);
  }
  /* free up allocated handles */
  SQLFreeHandle(SQL_HANDLE_DBC, dbc);
  SQLFreeHandle(SQL_HANDLE_ENV, env);
}
