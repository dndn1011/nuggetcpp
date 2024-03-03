#include <iostream>
#include "notice.h"
#include "system.h"
#include "notice.h"
#define SQLITE_THREADSAFE 2
#include "../../external/sqlite3.h"
#include "debug.h"
#include "db.h"
#include "propertytree.h"

namespace nugget::db {
    using namespace nugget::identifier;
    using namespace nugget::properties;

    bool DeleteAllFromTable(const std::string &str);

    sqlite3* databaseConnection;
    
    void ClearAssetMetaData() {
        DeleteAllFromTable("asset_meta");
    }
    
    bool Init() {
        constexpr IDType db_node = ID("database");
        constexpr IDType prop_file = ID(db_node, "file");
        constexpr IDType prop_extension = ID(db_node, "extension");
        constexpr IDType prop_extension_entry = ID(db_node, "extension_entry");
        constexpr IDType prop_service = ID(db_node, "service");
        check(gProps.GetString(prop_service) == "sqlite3", "db not supported");
        std::string path = gProps.GetString(prop_file);
        std::string extension = gProps.GetString(prop_extension);
        std::string extension_entry = gProps.GetString(prop_extension_entry);

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
        ClearAssetMetaData();

        check(sqlite3_threadsafe(), "sqlite not threadsafe");

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

#define ERR(a) if(!(a)) { output("SQL{}\n", sqlite3_errmsg(databaseConnection)); check(0,"db error"); return false; }

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

    bool AddAssetMeta(const std::string& table,const std::string& idName, const std::string& path, const std::string& type, const std::string& description) {
        std::string qstr = std::format("insert into {} (id,path,type,description) values (?,?,?,?)", table);
        const char* sqlQuery = qstr.c_str();

        SQLite sql;
        ERR(sql.Query(sqlQuery));
        ERR(sql.Bind(1, idName));
        ERR(sql.Bind(2, path));
        ERR(sql.Bind(3, type));
        ERR(sql.Bind(4, description));
        ERR(sql.Apply());
        return true;
    }


    bool CopyTable(
        const std::string& table1,
        const std::string& table2) {

        DeleteAllFromTable(table2);
        std::string qstr = std::format("insert into {} select * from {}", table2,table1).c_str();

        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, qstr.c_str(), NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
        return true;
    }

    bool CompareTables(
        const std::string& table1,
        const std::string& table2,
        const std::string& valueField,
        std::vector<IDType>& results
    ) {
        std::string qstr = std::format("select nidhash(b.id) from {} a,{} b where a.id=b.id and a.{}<>b.{}",table1,table2,valueField,valueField);
        const char* sqlQuery = qstr.c_str();
        SQLite sql;
        ERR(sql.Query(sqlQuery));

        std::vector<ValueAny> fields;
        while (sql.Step()) {
            ERR(sql.FetchRow({ ValueAny::Type::IDType }, fields));
            results.push_back(fields[0].AsIDType());
        }

        return true;
    }

    // INSERT OR REPLACE INTO table_name(column1, column2, ... columnN)

    bool db::AddAssetCache(const std::string& path, const std::string &name, const std::string &type,IDType id) {
        SQLite sql;
        ERR(sql.Query("insert or replace into cache_info (path,name,type,hash,epoch) values (?,?,?,?,strftime('%s', 'now'))"));
        ERR(sql.Bind(1, path));
        ERR(sql.Bind(2, name));
        ERR(sql.Bind(3, type));
        ERR(sql.Bind(4, id));
        ERR(sql.Apply());
        return true;
    }

    bool LookupAsset(IDType id, std::string& result) {
        SQLite sql;
        ERR(sql.Query("select path from asset_meta where ? = nidhash(id)"));
        ERR(sql.Bind(1, id));
        ERR(sql.Step());
        std::vector<ValueAny> results;
        ERR(sql.FetchRow({ ValueAny::Type::string }, results));
        result = results[0].AsString();
        return true;
    }

    bool ReverseLookupAsset(const std::string& path,std::vector<IDType>& results) {
        SQLite sql;
        ERR(sql.Query("select nidhash(id) from asset_meta where ? = path"));
        ERR(sql.Bind(1, path));

        std::vector<ValueAny> fields;
        while (sql.Step()) {
            ERR(sql.FetchRow({ ValueAny::Type::IDType }, fields));
            results.push_back(fields[0].AsIDType());
        }

        return true;
    }

    bool IsAssetCacheDirty(IDType id) {
        SQLite sql;

        ERR(sql.Query(
            "select asset_meta.path<>cache_info.path or cache_info.epoch<assets.epoch from cache_info,asset_meta,assets where assets.path=asset_meta.path and asset_meta.id=cache_info.name and nidhash(cache_info.name) = ?;"
        ));
        ERR(sql.Bind(1, id));
        if (sql.Step()) {
            std::vector<ValueAny> results;
            ERR(sql.FetchRow({ ValueAny::Type::int64_t_ }, results));
            int64_t result = results[0].AsInt64();
            return result != 0;
        } else {
            return true;
        }
    }

    bool UpdateAssetEpoch(std::string table, std::string path, int64_t epoch) {
        std::string qstr = std::format("UPDATE {} set epoch=? where path=?", table);
        const char* sqlQuery = qstr.c_str();

        sqlite3_stmt* stmt;
        // Prepare the SQL statement
        auto rc = sqlite3_prepare_v2(databaseConnection, sqlQuery, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            check(0, "Error preparing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
            return false;
        }

        // Bind values to the prepared statement
        rc = sqlite3_bind_int64(stmt, 1, epoch);
        if (rc != SQLITE_OK) {
            check(0, "Error binding path parameter: {}\n", sqlite3_errmsg(databaseConnection));
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

    bool AddAsset(std::string table, std::string path, std::string name, std::string type,int64_t epoch) {
        std::string qstr = std::format("INSERT INTO {} (path, name, type, hash, epoch) VALUES (?, ?, ?, nidhash(?), ?)", table).c_str();

        // todo : what is this back and forth with c_str?
        // SQL query for insertion
        const char* sqlQuery = qstr.c_str();

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

            rc = sqlite3_bind_int64(stmt, 5, epoch);
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

    void StartTransaction() {
        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, "BEGIN TRANSACTION;", NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
    }

    void CommitTransaction() {
        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, "COMMIT;", NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
    }

    void ClearTable(const std::string &table) {
        std::string qstr = std::format("delete from {}", table).c_str();

        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, qstr.c_str(), NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
    }

    void InsertChanges() {
        std::string qstr = std::format("insert into need_to_reconcile (path,type) select * from find_all_changes").c_str();

        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, qstr.c_str(), NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
    }
    void ApplyChanges() {
        ClearTable("assets");

        std::string qstr = std::format("insert into assets select * from asset_changes").c_str();

        char* errMsg = 0;
        int rc;
        rc = sqlite3_exec(databaseConnection, qstr.c_str(), NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            check(0, "Error executing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
        }
    }
    void ReconcileAssetChanges() {
        InsertChanges();
        ApplyChanges();
    }

    bool GetNextToReconcile(ReconcileInfo& result) {
        SQLite sql;
        ERR(sql.Query("select id, path,type,coalesce(state,'') from need_to_reconcile where coalesce(state, '') <> 'done' order by id limit 1"));
        ERR(sql.Step());
        std::vector<ValueAny> results;
        ERR(sql.FetchRow({ ValueAny::Type::int64_t_ ,ValueAny::Type::string, ValueAny::Type::string, ValueAny::Type::string, ValueAny::Type::int64_t_ }, results));
        result.id = results[0].AsInt64();
        result.path = results[1].AsString();
        result.type = results[2].AsString();
        result.state = results[3].AsString();
        return true;
    }

    bool MarkReconciled(int64_t id) {
        std::string sqlQuery = std::format("update need_to_reconcile set state='done' where id=?");

        sqlite3_stmt* stmt;
        // Prepare the SQL statement
        auto rc = sqlite3_prepare_v2(databaseConnection, sqlQuery.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            check(0, "Error preparing SQL statement: {}\n", sqlite3_errmsg(databaseConnection));
            return false;
        }

        // Bind values to the prepared statement
        rc = sqlite3_bind_int64(stmt, 1, (int64_t)id);
        if (rc != SQLITE_OK) {
            check(0, "Error binding path parameter: {}\n", sqlite3_errmsg(databaseConnection));
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
        }, 110, identifier::ID("init"), __FILE__);
}
