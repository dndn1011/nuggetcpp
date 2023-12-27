#include <iostream>
#include "sqlite3.h"

int main() {
    // SQLite database connection
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);  // Open an in-memory database

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // Execute SQL commands
    const char* createTableSQL = "CREATE TABLE Test (ID INT, Name TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot create table: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return rc;
    }

    const char* insertDataSQL = "INSERT INTO Test (ID, Name) VALUES (1, 'John Doe');";
    rc = sqlite3_exec(db, insertDataSQL, 0, 0, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot insert data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return rc;
    }

    // Query and print data
    const char* selectDataSQL = "SELECT * FROM Test;";
    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, selectDataSQL, -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

            std::cout << "ID: " << id << ", Name: " << name << std::endl;
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to execute SELECT query: " << sqlite3_errmsg(db) << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);

    return 0;
}
