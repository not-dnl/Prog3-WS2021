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

string JsonParser::convertToApiString(Board &board) {
    throw NotImplementedException();
}

string JsonParser::convertToApiString(Column &column) {
    Document doc;
    doc.SetObject();
    doc.AddMember("id", column.getId(), doc.GetAllocator());

    Value nameValue(kStringType);
    nameValue.SetString(column.getName().c_str(), doc.GetAllocator());
    doc.AddMember("name", nameValue, doc.GetAllocator());

    doc.AddMember("position", column.getPos(), doc.GetAllocator());

    Value itemArray(kArrayType);
    doc.AddMember("items", itemArray, doc.GetAllocator());

    StringBuffer sb;
    sb.Clear();

    PrettyWriter<StringBuffer> writer(sb);
    doc.Accept(writer);

    return string(sb.GetString());
}

string JsonParser::convertToApiString(std::vector<Column> &columns) {
    throw NotImplementedException();
}

string JsonParser::convertToApiString(Item &item) {
    Document doc;
    doc.SetObject();
    doc.AddMember("id", item.getId(), doc.GetAllocator());

    Value titleValue(kStringType);
    titleValue.SetString(item.getTitle().c_str(), doc.GetAllocator());
    doc.AddMember("title", titleValue, doc.GetAllocator());

    doc.AddMember("position", item.getPos(), doc.GetAllocator());

    Value timestampValue(kStringType);
    timestampValue.SetString(item.getTimestamp().c_str(), doc.GetAllocator());
    doc.AddMember("timestamp", timestampValue, doc.GetAllocator());

    StringBuffer sb;
    sb.Clear();

    PrettyWriter<StringBuffer> writer(sb);
    doc.Accept(writer);

    return string(sb.GetString());
}

string JsonParser::convertToApiString(std::vector<Item> &items) {
    throw NotImplementedException();
}

std::optional<Column> JsonParser::convertColumnToModel(int columnId, std::string &request) {
    if (request.empty())
        return {};

    Document doc;
    doc.Parse(request.c_str());

    if (doc.HasMember("name") && doc.HasMember("position")) {
        auto name = doc["name"].GetString();
        auto pos = doc["position"].GetInt();

        return Column{columnId, name, pos};
    }

    return {};
}

std::optional<Item> JsonParser::convertItemToModel(int itemId, std::string &request) {
    if (request.empty())
        return {};

    Document doc;
    doc.Parse(request.c_str());

    if (doc.HasMember("title") && doc.HasMember("position")) {
        auto title = doc["title"].GetString();
        auto pos = doc["position"].GetInt();

        time_t ttime = time(0);
        char *timestamp = ctime(&ttime);
        timestamp[strlen(timestamp) - 1] = '\0'; // "remove" newline char

        return Item{itemId, title, pos, timestamp};
    }

    return {};
}
