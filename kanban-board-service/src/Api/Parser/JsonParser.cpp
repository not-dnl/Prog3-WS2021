#define RAPIDJSON_ASSERT(x)

#include "JsonParser.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include "crow/logging.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace Prog3::Api::Parser;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace rapidjson;
using namespace std;

bool JsonParser::isValidColumn(rapidjson::Document const &document) {
    if (document.HasParseError() || !document["name"].IsString() || !document["position"].IsInt()) {
        return false;
    }

    return true;
}

bool JsonParser::isValidItem(rapidjson::Document const &document) {
    if (document.HasParseError() || !document["title"].IsString() || !document["position"].IsInt()) {
        return false;
    }

    return true;
}

rapidjson::Value JsonParser::getJsonValueFromModel(Column const &column, rapidjson::Document::AllocatorType &allocator) {
    Value jsonColumn(kObjectType);

    jsonColumn.AddMember("id", column.getId(), allocator);
    jsonColumn.AddMember("name", Value(column.getName().c_str(), allocator), allocator);
    jsonColumn.AddMember("position", column.getPos(), allocator);

    Value jsonItems(kArrayType);

    for (auto &item : column.getItems()) {
        Value jsonItem = getJsonValueFromModel(item, allocator);
        jsonItems.PushBack(jsonItem, allocator);
    }

    jsonColumn.AddMember("items", jsonItems, allocator);

    return jsonColumn;
}

rapidjson::Value JsonParser::getJsonValueFromModel(Item const &item, rapidjson::Document::AllocatorType &allocator) {
    Value jsonItem(kObjectType);

    jsonItem.AddMember("id", item.getId(), allocator);
    jsonItem.AddMember("title", Value(item.getTitle().c_str(), allocator), allocator);
    jsonItem.AddMember("position", item.getPos(), allocator);
    jsonItem.AddMember("timestamp", Value(item.getTimestamp().c_str(), allocator), allocator);

    return jsonItem;
}

string JsonParser::jsonValueToString(rapidjson::Value const &json) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    json.Accept(writer);

    return buffer.GetString();
}

string JsonParser::convertToApiString(Board &board) {
    Document document(kObjectType);

    document.AddMember("title", Value(board.getTitle().c_str(), document.GetAllocator()), document.GetAllocator());

    Value columnArray(kArrayType);

    for (auto &column : board.getColumns())
        columnArray.PushBack(getJsonValueFromModel(column, document.GetAllocator()), document.GetAllocator());

    document.AddMember("columns", columnArray, document.GetAllocator());

    return jsonValueToString(document);
}

string JsonParser::convertToApiString(Column &column) {
    Document document(kObjectType);

    Value jsonColumn = getJsonValueFromModel(column, document.GetAllocator());

    return jsonValueToString(jsonColumn);
}

string JsonParser::convertToApiString(std::vector<Column> &columns) {
    Document columnArray(kArrayType);

    for (auto &column : columns)
        columnArray.PushBack(getJsonValueFromModel(column, columnArray.GetAllocator()), columnArray.GetAllocator());

    return jsonValueToString(columnArray);
}

string JsonParser::convertToApiString(Item &item) {
    Document document(kObjectType);

    Value jsonItem = getJsonValueFromModel(item, document.GetAllocator());

    return jsonValueToString(jsonItem);
}

string JsonParser::convertToApiString(std::vector<Item> &items) {
    Document itemArray(kArrayType);

    for (auto &item : items)
        itemArray.PushBack(getJsonValueFromModel(item, itemArray.GetAllocator()), itemArray.GetAllocator());

    return jsonValueToString(itemArray);
}

std::optional<Column> JsonParser::convertColumnToModel(int columnId, std::string &request) {
    Document document;
    document.Parse(request.c_str());

    if (isValidColumn(document)) {
        return Column(columnId, document["name"].GetString(), document["position"].GetInt());
    }

    return {};
}

std::optional<Item> JsonParser::convertItemToModel(int itemId, std::string &request) {
    Document document;
    document.Parse(request.c_str());

    if (isValidItem(document)) {
        // we don't care about the timestamp here
        return Item(itemId, document["title"].GetString(), document["position"].GetInt(), "");
    }

    return {};
}
