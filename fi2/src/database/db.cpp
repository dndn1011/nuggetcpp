#include <iostream>
#include "notice.h"
#include "system.h"
#include "notice.h"
#include "../../external/sqlite3.h"
#include "debug.h"
#include "db.h"

namespace nugget::db {
    using namespace identifier;
    namespace N = Notice;

    sqlite3* databaseConnection;
    bool Init() {


        constexpr IDType db_node = ID("properties.database");
        constexpr IDType prop_file = ID(db_node, "file");
        constexpr IDType prop_service = ID(db_node, "service");
        check(N::GetString(prop_service) == "sqlite3", "db not supported");
        std::string path = N::GetString(prop_file);

        int rc = sqlite3_open(path.c_str(), &databaseConnection);  // Open an in-memory database
        if (rc != SQLITE_OK) {
            check(0, "Cannot open database: {}", sqlite3_errmsg(databaseConnection));
            return false;
        }
        sqlite3_db_config(databaseConnection, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, -1, nullptr);
        return true;
    }

    bool AddAsset(std::string path, std::string name, std::string type) {
            // SQL query for insertion
            const char* sqlQuery = "INSERT INTO assets (path, name, type) VALUES (?, ?, ?)";

            sqlite3_stmt* stmt;
            // Prepare the SQL statement
            auto rc = sqlite3_prepare_v2(databaseConnection, sqlQuery, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                check(0, "Error preparing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
                return false;
            }

            // Bind values to the prepared statement
            rc = sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                check(0, "Error binding path parameter: {}\n", sqlite3_errmsg(databaseConnection));
                sqlite3_finalize(stmt);
                return false;
            }

            rc = sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                check(0, "Error binding name parameter: {}\n", sqlite3_errmsg(databaseConnection));
                sqlite3_finalize(stmt);
                return false;
            }

            rc = sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK) {
                check(0, "Error binding type parameter: {}\n", sqlite3_errmsg(databaseConnection));
                sqlite3_finalize(stmt);
                return false;
            }

            // Execute the statement to insert the data
            rc = sqlite3_step(stmt);

            if (rc != SQLITE_DONE) {
                check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
                sqlite3_finalize(stmt);
                return false;
            }

            // Finalize the statement and close the database
            sqlite3_finalize(stmt);

            return true;
        }
#if 0


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
#endif

    size_t init_dummy = nugget::system::RegisterModule([]() {
        Init();
        return 0;
        }, 110);
}
