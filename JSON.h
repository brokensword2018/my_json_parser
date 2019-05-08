#pragma once
#include <string>
#include <memory>

//一个7中json类型， false, true, null, number, string, array, object;
//类JSON是这7种类的基类。

enum JSONType{
    JSONFalse = 0,
    JSONTrue,
    JSONNULL,
    JSONNumber,
    JSONString,
    JSONArray,
    JSONObject
};

class JSON{
public:
    typedef std::shared_ptr<JSON> SPJSON;


    JSON();
    ~JSON();

    void setNext(SPJSON next);
    SPJSON getNext();
    void setPre(JSON*  pre);
    JSON* getPre();
    void setChild(SPJSON child);
    SPJSON getChild();
    void setJSONType(JSONType type);
    JSONType getJSONType();
    void setValueString(std::string valuestring);
    std::string getValueString();
    void setValueInt(int valueint);
    int getValueInt();
    void setValueDouble(double valuedouble);
    double getValueDouble();

    std::string getName();
    void setName(std::string name);

private:
    SPJSON next_;//使用智能指针进行内存的管理
    JSON* prev_;
    SPJSON child_;

    JSONType type_;

    std::string valueString_;
    int valueint_;
    double valuedouble_;

    std::string name_;
};