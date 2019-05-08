#ifndef DBWRAP_H
#define DBWRAP_H

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

class SqliteStatement {

public:
    sqlite3* db;
    sqlite3_stmt *ppStmt;
    int lastReturnCode = SQLITE_MISUSE;

    SqliteStatement (sqlite3_stmt *newppStmt, sqlite3* newdb) {
        this->ppStmt = newppStmt;
        this->db = newdb;
    }

    ~SqliteStatement () {
        if (this->ppStmt) sqlite3_finalize(this->ppStmt);
    }

    bool isValid () {
        return this->ppStmt == nullptr;
    }

    bool setParameter(std::string paramName, const std::string *paramValue) {
        return setParameter(paramName, paramValue->c_str(), paramValue->length());
    }

    bool setParameter(std::string paramName, const char *paramValue, int strLength = -1) {
        fprintf(stderr, "setParameter %s = %s\n", paramName.c_str(), paramValue ? paramValue : "NULL");
        if (paramValue == nullptr) return this->setParameterNULL(paramName);
        int parIndex = sqlite3_bind_parameter_index(this->ppStmt, paramName.c_str());
        if (!parIndex) return  false;
        fprintf(stderr, "setParameter string %s (%i) = %s (%i)\n", paramName.c_str(), parIndex, paramValue, strLength);
        int rc = sqlite3_bind_text(this->ppStmt, parIndex, paramValue, strLength, nullptr);
        if (rc) fprintf(stderr, "%s\n", sqlite3_errmsg(this->db));
        return rc == 0;
    }

    bool setParameter(std::string paramName, int paramValue) {
        fprintf(stderr, "setParameter %s = %i\n", paramName.c_str(), paramValue);
        int parIndex = sqlite3_bind_parameter_index(this->ppStmt, paramName.c_str());
        if (!parIndex) return  false;
        int rc = sqlite3_bind_int(this->ppStmt, parIndex, paramValue);
        if (rc) fprintf(stderr, "%s\n", sqlite3_errmsg(this->db));
        return rc == 0;
    }

    bool setParameter(std::string paramName, double paramValue) {
        fprintf(stderr, "setParameter %s = %f\n", paramName.c_str(), paramValue);
        int parIndex = sqlite3_bind_parameter_index(this->ppStmt, paramName.c_str());
        if (!parIndex) return  false;
        int rc = sqlite3_bind_double(this->ppStmt, parIndex, paramValue);
        if (rc) fprintf(stderr, "%s\n", sqlite3_errmsg(this->db));
        return rc == 0;
    }

    bool setParameterInt64(std::string paramName, sqlite3_int64 paramValue) {
        fprintf(stderr, "setParameterInt64 %s\n", paramName.c_str());
        int parIndex = sqlite3_bind_parameter_index(this->ppStmt, paramName.c_str());
        if (!parIndex) return  false;
        int rc = sqlite3_bind_int64(this->ppStmt, parIndex, paramValue);
        if (rc) fprintf(stderr, "%s\n", sqlite3_errmsg(this->db));
        return rc == 0;
    }

    bool setParameterNULL(std::string paramName) {
        fprintf(stderr, "setParameterNULL %s\n", paramName.c_str());
        int parIndex = sqlite3_bind_parameter_index(this->ppStmt, paramName.c_str());
        if (!parIndex) return  false;
        int rc = sqlite3_bind_null(this->ppStmt, parIndex);
        if (rc) fprintf(stderr, "%s\n", sqlite3_errmsg(this->db));
        return rc == 0;
    }

    int step () {
        if (this->ppStmt == nullptr) {
            this->lastReturnCode = SQLITE_MISUSE;
        }
        this->lastReturnCode = sqlite3_step(this->ppStmt);
        fprintf(stderr, "step => %i\n", this->lastReturnCode);
        return this->lastReturnCode;
    }

    bool execute () {
        return (this->step() == SQLITE_DONE);
    }

    int col(std::string columnName) {
        int ncols = sqlite3_column_count(this->ppStmt);
        for (int i = 0; i < ncols; i++) {
            if (columnName.compare(sqlite3_column_name(this->ppStmt, i)) == 0) {
                return i;
            }
        }
        return -1;
    }

    bool valueIsNull(int columnIndex) {
        return (sqlite3_column_type(this->ppStmt, columnIndex) == SQLITE_NULL);
    }

    int getIntValue(int columnIndex) {
        return sqlite3_column_int(this->ppStmt, columnIndex);
    }

    double getDoubleValue(int columnIndex) {
        return sqlite3_column_double(this->ppStmt, columnIndex);
    }

    sqlite3_int64 getInt64Value(int columnIndex) {
        return sqlite3_column_int64(this->ppStmt, columnIndex);
    }

    const unsigned char* getTextValue(int columnIndex) {
        return sqlite3_column_text(this->ppStmt, columnIndex);
    }
};

class SqliteConnection {
private:
    sqlite3* db;

    SqliteConnection (sqlite3* conndb) {
        this->db = conndb;
    }

public:
    ~SqliteConnection () {
        sqlite3_close(this->db);
    }

    static SqliteConnection* create (std::string dbPath, std::string* retMessage = nullptr) {
        sqlite3* db = SqliteConnection::open(dbPath, retMessage);
        if (db == nullptr) return nullptr;
        if (retMessage) *retMessage = nullptr;
        return new SqliteConnection(db);
    }

    static sqlite3* open (std::string dbPath, std::string* retMessage = nullptr) {
        if (retMessage) *retMessage = nullptr;
        sqlite3 *db;
        int rc;

        /* Open database */
        rc = sqlite3_open(dbPath.c_str(), &db);
        if( rc ) {
            if (retMessage) *retMessage = std::string("Can't open database: ") + sqlite3_errmsg(db);
            return nullptr;
        } else {
            if (retMessage) *retMessage = std::string("Opened database successfully");
            return db;
        }
    }

    SqliteStatement* prepare (std::string sentence, std::string* retMessage = nullptr) {
        sqlite3_stmt *ppStmt = nullptr;

        int rc = sqlite3_prepare_v2(
          this->db,            /* Database handle */
          sentence.c_str(),       /* SQL statement, UTF-8 encoded */
          sentence.length(),              /* Maximum length of zSql in bytes. */
          &ppStmt,  /* OUT: Statement handle */
          nullptr     /* OUT: Pointer to unused portion of zSql */
        );
        if( rc ) {
            if (retMessage) *retMessage = std::string("Can't prepare statement: ") + sqlite3_errmsg(this->db);
            return new SqliteStatement(nullptr, this->db);
        } else {
            if (retMessage) *retMessage = std::string("Opened database successfully");
            return new SqliteStatement(ppStmt, this->db);
        }
    }
};

#endif // DBWRAP_H
