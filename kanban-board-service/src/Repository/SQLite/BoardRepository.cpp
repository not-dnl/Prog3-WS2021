#include "BoardRepository.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include "crow/logging.h"
#include <filesystem>
#include <string.h>

using namespace Prog3::Repository::SQLite;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace std;

#ifdef RELEASE_SERVICE
string const BoardRepository::databaseFile = "./data/kanban-board.db";
#else
string const BoardRepository::databaseFile = "../data/kanban-board.db";
#endif

BoardRepository::BoardRepository() : database(nullptr) {

    string databaseDirectory = filesystem::path(databaseFile).parent_path().string();

    if (filesystem::is_directory(databaseDirectory) == false) {
        filesystem::create_directory(databaseDirectory);
    }

    int result = sqlite3_open(databaseFile.c_str(), &database);

    if (SQLITE_OK != result) {
        cout << "Cannot open database: " << sqlite3_errmsg(database) << endl;
    }

    initialize();
}

BoardRepository::~BoardRepository() {
    sqlite3_close(database);
}

void BoardRepository::initialize() {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlCreateTableColumn =
        "create table if not exists column("
        "id integer not null primary key autoincrement,"
        "name text not null,"
        "position integer not null UNIQUE);";

    string sqlCreateTableItem =
        "create table if not exists item("
        "id integer not null primary key autoincrement,"
        "title text not null,"
        "date text not null,"
        "position integer not null,"
        "column_id integer not null,"
        "unique (position, column_id),"
        "foreign key (column_id) references column (id));";

    result = sqlite3_exec(database, sqlCreateTableColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    result = sqlite3_exec(database, sqlCreateTableItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // only if dummy data is needed ;)
    // createDummyData();
}

Board BoardRepository::getBoard() {
    throw NotImplementedException();
}

std::vector<Column> BoardRepository::getColumns() {
    throw NotImplementedException();
}

std::optional<Column> BoardRepository::getColumn(int id) {
    throw NotImplementedException();
}

std::optional<Column> BoardRepository::postColumn(std::string name, int position) {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlInsertColumn =
        "insert into column (name, position)"
        "values ('" +
        name + "', " + std::to_string(position) + ");";

    result = sqlite3_exec(database, sqlInsertColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    if (result == SQLITE_OK) {
        auto const columnId = sqlite3_last_insert_rowid(database);
        return Column(columnId, name, position);
    }

    return {};
}

std::optional<Prog3::Core::Model::Column> BoardRepository::putColumn(int id, std::string name, int position) {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlPutColumn = "update column "
                          "set name = '" +
                          name + "', position = " + std::to_string(position) +
                          " where id = " + std::to_string(id);

    // nothing bad happens when the id doesn't exist, might be bad for the front-end
    // but the doc doesn't say anything about error handling
    result = sqlite3_exec(database, sqlPutColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // constraints etc. are handled by SQL errors tho
    if (result == SQLITE_OK) {
        return Column(id, name, position);
    }

    return {};
}

void BoardRepository::deleteColumn(int id) {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlDeleteColumn =
        "delete from column "
        "where id = " +
        std::to_string(id);

    // error handling? does the item exist? prolly irrelevant
    result = sqlite3_exec(database, sqlDeleteColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

std::vector<Item> BoardRepository::getItems(int columnId) {
    throw NotImplementedException();
}

std::optional<Item> BoardRepository::getItem(int columnId, int itemId) {
    throw NotImplementedException();
}

std::optional<Item> BoardRepository::postItem(int columnId, std::string title, int position) {
    int result = 0;
    char *errorMessage = nullptr;

    time_t ttime = time(0);
    char *timestamp = ctime(&ttime);
    timestamp[strlen(timestamp) - 1] = '\0'; // "remove" newline char

    string sqlInsertItem =
        "insert into item (title, date, position, column_id)"
        "values ('" +
        title + "', '" + std::string(timestamp) +
        "', " + std::to_string(position) + ", " + std::to_string(columnId) + ");";

    result = sqlite3_exec(database, sqlInsertItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    if (result == SQLITE_OK) {
        auto const columnId = sqlite3_last_insert_rowid(database);
        return Item(columnId, title, position, timestamp);
    }

    return {};
}

std::optional<Prog3::Core::Model::Item> BoardRepository::putItem(int columnId, int itemId, std::string title, int position) {
    int result = 0;
    char *errorMessage = nullptr;

    time_t ttime = time(0);
    char *timestamp = ctime(&ttime);
    timestamp[strlen(timestamp) - 1] = '\0'; // "remove" newline char

    string sqlPutItem = "update item "
                        "set title = '" +
                        title + "', position = " + std::to_string(position) +
                        ", date = '" + std::string(timestamp) +
                        "' where id = " + std::to_string(itemId) + " and column_id = " + std::to_string(columnId);

    // nothing bad happens when the ids don't exist, might be bad for the front-end
    // but the doc doesn't say anything about error handling
    result = sqlite3_exec(database, sqlPutItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // constraints etc. are handled by SQL errors tho
    if (result == SQLITE_OK) {
        return Item(itemId, title, position, timestamp);
    }

    return {};
}

void BoardRepository::deleteItem(int columnId, int itemId) {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlDeleteItem =
        "delete from item "
        "where id = " +
        std::to_string(itemId) +
        " and column_id = " + std::to_string(columnId);

    // error handling? does the item exist? prolly irrelevant
    result = sqlite3_exec(database, sqlDeleteItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

void BoardRepository::handleSQLError(int statementResult, char *errorMessage) {

    if (statementResult != SQLITE_OK) {
        cout << "SQL error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

void BoardRepository::createDummyData() {

    cout << "creatingDummyData ..." << endl;

    int result = 0;
    char *errorMessage;
    string sqlInsertDummyColumns =
        "insert into column (name, position)"
        "VALUES"
        "(\"prepare\", 1),"
        "(\"running\", 2),"
        "(\"finished\", 3);";

    result = sqlite3_exec(database, sqlInsertDummyColumns.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    string sqlInserDummyItems =
        "insert into item (title, date, position, column_id)"
        "VALUES"
        "(\"in plan\", date('now'), 1, 1),"
        "(\"some running task\", date('now'), 1, 2),"
        "(\"finished task 1\", date('now'), 1, 3),"
        "(\"finished task 2\", date('now'), 2, 3);";

    result = sqlite3_exec(database, sqlInserDummyItems.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

int BoardRepository::callback0(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    // don't need this yet.
    int &lastRowId = *static_cast<int *>(data);

    lastRowId = stoi(*fieldValues);

    return 0;
}
