#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string>

namespace dbwrap {

    static int callback(void *data, int argc, char **argv, char **azColName) {
       for(int i = 0; i<argc; i++) {
          printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
       }
       printf("\n");
       return 0;
    }

    int runTest() {
       sqlite3 *db;
       char *zErrMsg = nullptr;
       int rc;
       const char *sql;
       const char* data = "Callback function called";

       /* Open database */
       rc = sqlite3_open("test.db", &db);
       if( rc ) {
           fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
           return 1;
       } else {
           fprintf(stderr, "Opened database successfully\n");
       }

       sql = "CREATE TABLE COMPANY("  \
          "ID INT PRIMARY KEY     NOT NULL," \
          "NAME           TEXT    NOT NULL," \
          "AGE            INT     NOT NULL," \
          "ADDRESS        CHAR(50)," \
          "SALARY         REAL );";
       rc = sqlite3_exec(db, sql, callback, nullptr, &zErrMsg);
       if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       } else {
          fprintf(stdout, "Table created successfully\n");
       }


       sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
             "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
             "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
             "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
             "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
             "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
             "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
             "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
       rc = sqlite3_exec(db, sql, callback, nullptr, &zErrMsg);
       if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       } else {
          fprintf(stdout, "Records created successfully\n");
       }


       sql = "SELECT * from COMPANY";
       rc = sqlite3_exec(db, sql, callback, (void*)(data), &zErrMsg);
       if( rc != SQLITE_OK ) {
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       } else {
          fprintf(stdout, "Operation done successfully\n");
       }


       sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1; " \
             "SELECT * from COMPANY";
       rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
       if( rc != SQLITE_OK ) {
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       } else {
          fprintf(stdout, "Operation done successfully\n");
       }


       sql = "DELETE from COMPANY where ID=2; " \
             "SELECT * from COMPANY";
       rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
       if( rc != SQLITE_OK ) {
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       } else {
          fprintf(stdout, "Operation done successfully\n");
       }

       sqlite3_close(db);
       return 0;
    }
}
