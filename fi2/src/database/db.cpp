#include <iostream>
#include "notice.h"
#include "system.h"
#include "notice.h"
#include "../../external/sqlite3.h"
#include "debug.h"
#include "db.h"

namespace nugget::db {
    using namespace nugget::identifier;
    namespace N = Notice;

    bool DeleteAllFromTable(const std::string &str);

    sqlite3* databaseConnection;
    bool Init() {
        constexpr IDType db_node = ID("properties.database");
        constexpr IDType prop_file = ID(db_node, "file");
        constexpr IDType prop_extension = ID(db_node, "extension");
        constexpr IDType prop_extension_entry = ID(db_node, "extension_entry");
        constexpr IDType prop_service = ID(db_node, "service");
        check(N::GetString(prop_service) == "sqlite3", "db not supported");
        std::string path = N::GetString(prop_file);
        std::string extension = N::GetString(prop_extension);
        std::string extension_entry = N::GetString(prop_extension_entry);

        int rc = sqlite3_open(path.c_str(), &databaseConnection);
        if (rc != SQLITE_OK) {
            check(0, "Cannot open database: {}", sqlite3_errmsg(databaseConnection));
            return false;
        }
        sqlite3_enable_load_extension(databaseConnection, 1);
        char* errMsg;
        rc = sqlite3_load_extension(databaseConnection, extension.c_str(), extension_entry.c_str(), &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "could not load extensions: {}\n",errMsg);
            return false;
        }
        DeleteAllFromTable("assets");
        DeleteAllFromTable("asset_meta");
        return true;
    }

#define SQL(a) if(int rc = (a) != SQLITE_OK) { check(0,"SQL ERROR: {}",sqlite3_errmsg(databaseConnection)); return false; }
#define SQL2(a,b) if(int rc = (a) != b) { check(0,"SQL ERROR: {}",sqlite3_errmsg(databaseConnection)); return false; }
    struct SQLite {
        sqlite3_stmt* stmt = nullptr;
        bool Query(const std::string& str) {
            SQL(sqlite3_prepare_v2(databaseConnection, str.c_str(), -1, &stmt, nullptr));
            return true;
        }
        bool Bind(int num, const std::string& str) {
            SQL(sqlite3_bind_text(stmt, num, str.c_str(), -1, SQLITE_STATIC));
            return true;
        }
        bool Bind(int num, IDType id) {
            SQL(sqlite3_bind_int64(stmt, num, (int64_t)id));
            return true;
        }
        bool Step() {
            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_ROW) {
                return false;
            }
            return true;
        }
        bool Apply() {
            SQL2(sqlite3_step(stmt), SQLITE_DONE);
            return true;
        }

        bool FetchRow(const std::vector<ValueAny::Type>& types, std::vector<ValueAny>& result) {
            int col = 0;
            for (auto&& x : types) {
                switch (x) {
                    case ValueAny::Type::int64_t_: {
                        result.emplace_back(sqlite3_column_int64(stmt, col));
                    }break;
                    case ValueAny::Type::string: {
                        auto text = sqlite3_column_text(stmt, col);
                        std::string textstr(reinterpret_cast<const char *>(text));
                        result.push_back(ValueAny(textstr));
                    }break;
                    case ValueAny::Type::IDType: {
                        result.push_back(ValueAny((IDType)sqlite3_column_int64(stmt, col)));
                    }break;
                    default: {
                        check(0, "unhandled type: {}\n", ValueAny::TypeAsString(x));
                        return false;
                    }
                }
                col++;
            }
            return true;
        }

        bool Exec(const std::string& str) {
            SQL(sqlite3_exec(databaseConnection, str.c_str(), 0, 0, nullptr));
            return true;
        }
        ~SQLite() {
            if (stmt) {
                sqlite3_finalize(stmt);
            }
        }
    };

#define ERR(a) if(!(a)) { return false; }

    bool DeleteAllFromTable(const std::string &table) {
        SQLite sql;
        ERR(sql.Exec("delete from "+table));
        return true;
    }

    bool AddAssetMeta(const std::string& idName, const std::string& path, const std::string& type, const std::string& description) {
        SQLite sql;
        ERR(sql.Query("insert into asset_meta (id,path,type,description) values (?,?,?,?)"));
        ERR(sql.Bind(1, idName));
        ERR(sql.Bind(2, path));
        ERR(sql.Bind(3, type));
        ERR(sql.Bind(4, description));
        ERR(sql.Apply());
        return true;
    }

    bool LookupAsset(IDType id,std::string &result) {
        SQLite sql;
        ERR(sql.Query("select path from asset_meta where ? = nidhash(id)"));
        ERR(sql.Bind(1, id));
        ERR(sql.Step());
        std::vector<ValueAny> results;
        ERR(sql.FetchRow({ ValueAny::Type::string }, results));
        result = results[0].AsString();
        return true;
    }
          
    bool AddAsset(std::string path, std::string name, std::string type) {
            // SQL query for insertion
            const char* sqlQuery = "INSERT INTO assets (path, name, type, hash) VALUES (?, ?, ?, nidhash(?))";

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

            rc = sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_STATIC);
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
