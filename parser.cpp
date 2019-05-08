#include "parser.h"
#include <string.h>
#include <cmath>

static const char* ep;

static const char* parse_value(JSON* item, const char* value);



static const char* skip(const char* in){
    while(in && *in && (unsigned char)*in <= 32) in++;
    return in;
}

extern JSON* JSON_Parse(const char* value){
    JSON* item = new JSON();

    parse_value(item, value);

    return item;
}

static unsigned parse_hex4(const char *str)//解析4位16进制的数， 一位16进制站4个字节
{
	unsigned h=0;  //unsigned等价于unsigned int
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	return h;
}

static const char *parse_number(JSON *item,const char *num)
{
	//n：整数部分  sign:底的符号  scale:底的缩小倍数    subscale:指数部分的值
	double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;

	if (*num=='-') sign=-1,num++;	/* Has sign? */

	if (*num=='0') num++;			/* is zero */

	if (*num>='1' && *num<='9')	do	n=(n*10.0)+(*num++ -'0');	while (*num>='0' && *num<='9');	/* Number? */
	if (*num=='.' && num[1]>='0' && num[1]<='9') {num++;		do	n=(n*10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');}	/* Fractional part? */
	if (*num=='e' || *num=='E')		/* Exponent? */
	{	num++;if (*num=='+') num++;	else if (*num=='-') signsubscale=-1,num++;		/* With sign? */
		while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');	/* Number? */
	}

	n=sign*n*pow(10.0,(scale+subscale*signsubscale));	/* number = +/- number.fraction * 10^+/- exponent */
	
	item->setValueDouble(n);
    item->setValueInt((int)n);
    item->setJSONType(JSONNumber);
	return num;
}




static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(JSON *item,const char *str)
{
	const char *ptr=str+1;
	char *ptr2;
	char *out;//解析出的字符串
	int len=0;
	unsigned uc,uc2;

	if (*str!='\"') {ep=str;return 0;}	/* not a string! 现在str指向的应该是冒号*/
	
	while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. ptr一直增加到下一个冒号*/
	
	out=(char*)malloc(len+1);	/* This is how long we need for the string, roughly. */
	if (!out) return 0;
	
	ptr=str+1; //ptr指向”的下一个字符
	ptr2=out;
	while (*ptr!='\"' && *ptr)
	{
		if (*ptr!='\\') 
			*ptr2++=*ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
				case 'b': *ptr2++='\b';	break; //退格
				case 'f': *ptr2++='\f';	break; //换页
				case 'n': *ptr2++='\n';	break; //换行
				case 'r': *ptr2++='\r';	break; //回车
				case 't': *ptr2++='\t';	break; //水平制表
				case 'u':	 /* transcode utf16 to utf8. */
					uc=parse_hex4(ptr+1);//前导代理，前导代理和后尾代理前6位都是定值
					ptr+=4;	/* get the unicode char. */

					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/

					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
					{
						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
						uc2=parse_hex4(ptr+3);ptr+=6;  //后尾代理
						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
					}

					len=4;
					if (uc<0x80) len=1;            //len = 1, 2,3 是utf8, 可变长编码， 表示编码的有多少位
					else if (uc<0x800) len=2;
					else if (uc<0x10000) len=3; 
					ptr2+=len;
					
					//就是这样的转换规则
					switch (len) {	
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
					}
					ptr2+=len;
					break;
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	item->setValueString(out);
    free(out);

	item->setJSONType(JSONString);
	return ptr;
}

static const char* parse_array(JSON* item, const char* value){
    JSON* child;
    if(*value != '['){ ep = value; return nullptr;}

    item->setJSONType(JSONArray);
    value=skip(value+1); /* skip space */
    if(*value == ']') return value + 1; /* empty array */

    item->setChild(std::make_shared<JSON>());/* child is for array */
    child = item->getChild().get(); 

    value = skip(parse_value(child, skip(value)));
    if(!value) return nullptr;

    while(*value == ','){/* parse the whole array */
        JSON* new_item;
        new_item = new JSON();
        child->setNext(std::shared_ptr<JSON>(new_item));

        new_item->setPre(child);/* 普通指针不可以和智能指针混合使用 */

        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if(!value) return 0;
    }
    
    if(*value == ']') return value + 1;

    /* no ']' to match '[' */
    ep = value;
    return 0;
}

static const char* parse_object(JSON* item, const char* value){
    JSON* child;
    
    item->setJSONType(JSONObject);
    value = skip(value + 1);
    if(*value == '}') return value + 1; /* empty object */

    item->setChild(std::make_shared<JSON>());
    child = item->getChild().get();

    /* 对象里面一定是key/value 所以显示解析字符串， 再parse_value */

    value = skip(parse_string(child, skip(value)));
    child->setName(child->getValueString());
    child->setValueString("");

    if(*value != ':'){ep = value; return 0;}
    value = skip(parse_value(child, skip(value + 1)));

    while(*value == ','){
        JSON* new_item = new JSON();

        /* 造成多次释放 */
        child->setNext(std::shared_ptr<JSON>(new_item));/* 为了提供给外部的接口是普通指针，将普通指针和智能指针进行了混用危险 */
        new_item->setPre(child);
        child = new_item;

        value = skip(parse_string(child, skip(value + 1))); 
        child->setName(child->getValueString());
        child->setValueString("");
        if(*value != ':'){ ep = value; return 0;}

        value = skip(parse_value(child, skip(value + 1)));
        if(!value) return 0;
    }

    if(*value == '}') return value + 1;
    ep = value; return 0;
}


static const char* parse_value(JSON* item, const char* value){
    if(!value) return nullptr;
    if(!strncmp(value, "null", 4)){
        item->setJSONType(JSONNULL);
        return value + 4;
    }
    if(!strncmp(value, "false", 5)){
        item->setJSONType(JSONFalse);
        return value + 5;
    }
    if(!strncmp(value, "true", 4)){
        item->setJSONType(JSONTrue);
        return value + 4;
    }
    if(*value == '\"'){
        return parse_string(item, value);
    }
    if(*value == '-' || (*value >= '0' && *value <= '9')){
        return parse_number(item, value);
    }
    if(*value == '['){
        return parse_array(item, value);
    }
    if(*value == '{'){
        return parse_object(item, value);
    }
    ep = value; return 0;
}