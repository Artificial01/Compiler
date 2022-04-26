#ifndef BASE_H
#define BASE_H

#include <stdio.h>

#define DEBUG

#define PAS "sample.pas"
#define DYD "sample.dyd"
#define DYS "sample.dys"
#define ERR "sample.err"
#define VAR "sample.var"
#define PRO "sample.pro"

#define DYD_LINE_SIZE 20
#define LEXICAL_MAX_SIZE 16

#define CATEGORY_MAX_AMOUNT 26

enum category{NOT_CATEGORY,BEGIN,END,INTEGER,IF,THEN,
    ELSE,FUNCTION,READ,WRITE,IDENTIFIER,
    CONST,EQUAL,NOT_EQUAL,LESS_EQUAL,LESS,
    MORE_EQUAL,MORE,REDUCE,MULTIPLY,VALUE,
    LEFT_BRACKETS,RIGHT_BRACKETS,SEMICOLON,END_OF_LINE,END_OF_FILE};
enum type{TYPE_INTEGER};

FILE * pas_fp;
FILE * dyd_fp;
FILE * dys_fp;
FILE * err_fp;
FILE * var_fp;
FILE * pro_fp;
int file_current_line;

char * category_string[CATEGORY_MAX_AMOUNT];
int category_string_size[CATEGORY_MAX_AMOUNT];

int isReserve(char * string,int size);
int isLetter(char character);
int isDigit(char character);
int equalString(const char * string_1,const char * string_2,int size);
void writeNumber(int number,FILE * fp);
void writeNotZeroNumber(int number,FILE * fp);

#endif
