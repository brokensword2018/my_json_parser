#include "print.h"
#include <string.h>
#include <cmath>
#include <limits.h>
#include <float.h>


static char* print_value(JSON* item, int depth);


static char* JSON_strdup(const char* str)//字符串的复制,空间的重新申请
{
      size_t len;
      char* copy;

      len = strlen(str) + 1;
      if (!(copy = (char*)malloc(len))) return 0;
      memcpy(copy,str,len);
      return copy;
}


static char* print_number(JSON* item){
    char* str = 0;
    double d = item->getValueDouble();
    if(d == 0){
        str = (char*)malloc(2);
        strcpy(str, "0");
    }else if(fabs((double)item->getValueInt() - d) <= __DBL_EPSILON__ && d <= INT_MAX && d >= INT_MIN){
        str = (char*)malloc(21);
        sprintf(str, "%d", item->getValueInt());
    }else{
        str=(char*)malloc(64);	/* This is a nice tradeoff. */
        if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60)sprintf(str,"%.0f",d);
        else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)			sprintf(str,"%e",d);
        else												sprintf(str,"%f",d);		
    }
    return str;
}

static char* print_string_ptr(const char* str){
    const char* ptr;
    char* ptr2, *out;
    int len = 0, flag = 0; 
    unsigned char token;
    for(ptr = str; *ptr; ++ptr)/* 标记是否有特殊字符 */
        flag|=((*ptr>0 && *ptr<32)||(*ptr=='\"')||(*ptr=='\\'))?1:0;
    
    if(!flag){
        len = ptr - str;
        out = (char*)malloc(len + 3);/* 一个空字符和两个“ 字符 */
        ptr2 = out; *ptr2++ = '\"';
        strcpy(ptr2, str);
        ptr2[len] = '\"';
        ptr2[len + 1] = 0;
        return out;
    }
    /* 加上特殊字符需要的长度 */
    ptr=str;while ((token=*ptr) && ++len) {if (strchr("\"\\\b\f\n\r\t",token)) len++; else if (token<32) len+=5;ptr++;}

    out = (char*)malloc(len + 3);

    ptr2=out;ptr=str;
	*ptr2++='\"';
	while (*ptr)
	{
		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
		else
		{
			*ptr2++='\\';
			switch (token=*ptr++)
			{
				case '\\':	*ptr2++='\\';	break;
				case '\"':	*ptr2++='\"';	break;
				case '\b':	*ptr2++='b';	break;
				case '\f':	*ptr2++='f';	break;
				case '\n':	*ptr2++='n';	break;
				case '\r':	*ptr2++='r';	break;
				case '\t':	*ptr2++='t';	break;
				default: sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
			}
		}
	}
	*ptr2++='\"';*ptr2++=0;
    return out;
}


static char* print_string(JSON* item){
    return print_string_ptr(item->getValueString().c_str());
}

static char* print_array(JSON* item, int depth){
    char** entries;
    char* out = 0, *ptr, *ret;
    int len = 5;
    JSON* child = item->getChild().get();
    int numentries = 0, i = 0, fail = 0;
    size_t tmplen = 0;

    while(child) numentries++, child = child->getNext().get();

    /* empty array */
    if(numentries == 0){
        out = (char*)malloc(3);
        strcpy(out, "[]");
        return out;
    }

    entries=(char**)malloc(numentries*sizeof(char*));
    if (!entries) return 0;
    memset(entries,0,numentries*sizeof(char*));
    /* Retrieve all the results: */
    child=item->getChild().get();
    while (child)
    {
        ret =  print_value(child, depth+1);
        entries[i++] = ret;
        if(ret) len += strlen(ret) + 3;
        child = child->getNext().get();
    }

    out = (char*)malloc(len);

    *out = '[';
    ptr = out + 1; *ptr=0;
    for(int i = 0; i < numentries; ++i){
        tmplen = strlen(entries[i]);
        memcpy(ptr, entries[i], tmplen);
        ptr += tmplen;
        if(i != numentries - 1){ *ptr++ = ','; *ptr++ = ' '; *ptr = 0;}
        free(entries[i]);
    }
    free(entries);
    *ptr++ = ']';
    *ptr = 0;
    return out;
}



/* Render an object to text. */
static char *print_object(JSON *item, int depth)
{
	char **entries=0,**names=0;
	char *out=0,*ptr,*ret,*str;int len=7,i=0,j;
	JSON *child=item->getChild().get();
	int numentries=0,fail=0;
	size_t tmplen=0;
	/* Count the number of entries. */
	while (child) {
        numentries++;
        child=child->getNext().get();
    }
	/* Explicitly handle empty object case */
	if (!numentries)
	{
		out=(char*)malloc(depth+4);
		if (!out)	return 0;
		ptr=out;*ptr++='{';
		*ptr++='\n';for (i=0;i<depth-1;i++) *ptr++='\t';
		*ptr++='}';*ptr++=0;
		return out;
	}



    /* Allocate space for the names and the objects */
    entries=(char**)malloc(numentries*sizeof(char*));
    if (!entries) return 0;
    names=(char**)malloc(numentries*sizeof(char*));
    if (!names) {free(entries);return 0;}
    memset(entries,0,sizeof(char*)*numentries);
    memset(names,0,sizeof(char*)*numentries);

    /* Collect all the results into our arrays: */
    child=item->getChild().get();depth++; len+=depth;
    while (child)
    {   
        names[i]=str=print_string_ptr(child->getName().c_str());
        entries[i++]=ret=print_value(child,depth);
        if (str && ret) len+=strlen(ret)+strlen(str)+2+(2 + depth); 
        child=child->getNext().get();
    }
    
    /* Try to allocate the output string */
    out=(char*)malloc(len);



    
    /* Compose the output: */
    *out='{';ptr=out+1; *ptr++='\n';*ptr=0;
    for (i=0;i<numentries;i++)
    {
        for (j=0;j<depth;j++) *ptr++='\t';
        tmplen=strlen(names[i]);memcpy(ptr,names[i],tmplen);ptr+=tmplen;
        *ptr++=':'; *ptr++='\t';
        strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
        if (i!=numentries-1) *ptr++=',';
        *ptr++='\n';*ptr=0;
        free(names[i]);
        free(entries[i]);
    }
    
    free(names);free(entries);
    for (i=0;i<depth-1;i++) *ptr++='\t';
    *ptr++='}';*ptr++=0;

	return out;	
}




static char* print_value(JSON* item, int depth){
    char* out = 0;
    if(!item) return 0;
    switch(item->getJSONType())
    {
        case JSONFalse: out = JSON_strdup("false");  break;/* 结尾的空字符也会负责 */
        case JSONTrue: out = JSON_strdup("true"); break;
        case JSONNULL: out = JSON_strdup("null"); break;
        case JSONNumber: out = print_number(item); break;
        case JSONString: out = print_string(item); break;
        case JSONArray: out = print_array(item, 0); break;
        case JSONObject: out = print_object(item, 0); break;
    }
    return out;
}


char* JSON_Print(JSON* item){
    return print_value(item, 0);
}

















