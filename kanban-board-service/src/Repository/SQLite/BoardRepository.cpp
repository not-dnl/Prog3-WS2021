#include "BoardRepository.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include "crow/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <filesystem>
#include <string.h>

using namespace Prog3::Repository::SQLite;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace rapidjson;
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

    string sqlSelectColumn = "select * from column where id = " + to_string(id);
    int amount = 0;

    result = sqlite3_exec(database, sqlSelectColumn.c_str(), selectAmountCallback, &amount, &errorMessage);
    handleSQLError(result, errorMessage);

    // we didn't get any rows back so the column doesn't exist.
    if (amount == 0) {
        return {};
    }

    string sqlSelectItems = "select * from item where column_id = " + to_string(id); // get all items of the column

    Column column;
    result = sqlite3_exec(database, sqlSelectItems.c_str(), columnCallback, &column, &errorMessage); // items get added in the callback.
    handleSQLError(result, errorMessage);

    string sqlUpdateColumn = "update column "
                             "set name = '" +
                             name + "', position = " + std::to_string(position) +
                             " where id = " + std::to_string(id);

    // time to update the column
    result = sqlite3_exec(database, sqlUpdateColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // if everything went well we set the new id etc. and return that.
    if (result == SQLITE_OK) {
        column.setID(id);
        column.setName(name);
        column.setPos(position);

        return column;
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

    string sqlSelectItem = "select * from item "
                           "where id = " +
                           std::to_string(itemId) +
                           " and column_id = " + std::to_string(columnId);

    Item item; // default constructor sets id to -1
    result = sqlite3_exec(database, sqlSelectItem.c_str(), itemCallback, &item, &errorMessage);
    handleSQLError(result, errorMessage);

    // the item doesn't exist
    if (item.getId() == -1) {
        return {};
    }

    string sqlPutItem = "update item "
                        "set title = '" +
                        title + "', position = " + std::to_string(position) +
                        "' where id = " + std::to_string(itemId) + " and column_id = " + std::to_string(columnId);

    // if we got here, the item exists and it's time to update it.
    result = sqlite3_exec(database, sqlPutItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // everything went alright.
    if (result == SQLITE_OK) {
        return Item(itemId, title, position, item.getTimestamp()); // use the old timestamp.
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

int BoardRepository::itemCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    if (data) {
        auto item = static_cast<Item *>(data);

        item->setID(stoi(fieldValues[0]));
        item->setTitle(fieldValues[1]);
        item->setTimestamp(fieldValues[2]);
        item->setPos(stoi(fieldValues[3]));
    }

    return 0;
}

int BoardRepository::columnCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    if (data) {
        auto column = static_cast<Column *>(data);

        Item item = {
            stoi(fieldValues[0]),
            fieldValues[1],
            stoi(fieldValues[3]),
            fieldValues[2]};

        column->addItem(item);
    }

    return 0;
}

int BoardRepository::selectAmountCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    int *c = static_cast<int *>(data);
    *c = atoi(fieldValues[0]);

    return 0;
}
