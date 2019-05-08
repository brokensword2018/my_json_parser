#include "JSON.h"

typedef std::shared_ptr<JSON> SPJSON;
typedef std::weak_ptr<JSON> WPJSON;

JSON::JSON()
{
    next_.reset();
    prev_ = nullptr;
    child_.reset();

    type_ = JSONObject;
    valueString_ = "";
    valueint_ = 0;
    valuedouble_ = 0.0;
}

JSON::~JSON(){

}

void JSON::setNext(SPJSON next){
    next_ = next;
}

SPJSON JSON::getNext(){
    return next_;
}

void JSON::setPre(JSON* pre){
    prev_ = pre;
}

JSON* JSON::getPre(){
    return prev_;
}

void JSON::setChild(SPJSON child){
    child_ = child;
}
SPJSON JSON::getChild(){
    return child_;
}
void JSON::setJSONType(JSONType type){
    type_ = type;
}
JSONType JSON::getJSONType(){
    return type_;
}
void JSON::setValueString(std::string valuestring){
    valueString_ = valuestring;
}
std::string JSON::getValueString(){
    return valueString_;
}
void JSON::setValueInt(int valueint){
    valueint_ = valueint;
}
int JSON::getValueInt(){
    return valueint_;
}
void JSON::setValueDouble(double valuedouble){
    valuedouble_ = valuedouble;
}
double JSON::getValueDouble(){
    return valuedouble_;
}

std::string JSON::getName(){
    return name_;    
}

void JSON::setName(std::string name){
    name_ = name;
}

    


