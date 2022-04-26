#include "base.h"

char * category_string[CATEGORY_MAX_AMOUNT]={"","begin","end","integer","if","then",
                                             "else","function","read","write","identifier",
                                             "const","=","<>","<=","<",
                                             ">=",">","-","*",":=",
                                             "(",")",";","EOLN","EOF"};
int category_string_size[CATEGORY_MAX_AMOUNT]={0,5,3,7,2,4,
                                               4,8,4,5,10,
                                               5,1,2,2,1,
                                               2,1,1,1,2,
                                               1,1,1,4,3};

int isReserve(char * string,int size){
    for(int i=BEGIN;i<IDENTIFIER;i++){
        if(category_string_size[i]==size && equalString(category_string[i],string,size)) return i;
    }
    return 0;
}
int isLetter(char character){
    return ('a'<=character && character<='z') || ('A'<=character && character<='Z');
}
int isDigit(char character){
    return '0'<=character && character<='9';
}
int equalString(const char * string_1,const char * string_2,int size){
    for(int i=0;i<size;i++){
        if(string_1[i]!=string_2[i]) return 0;
    }
    return 1;
}
void writeNumber(int number,FILE * fp){
    if(number) writeNotZeroNumber(number,fp);
    else putc('0',fp);
}
void writeNotZeroNumber(int number,FILE * fp){
    if(number){
        writeNotZeroNumber(number/10,fp);
        putc('0'+number%10,fp);
    }
}
