#include "base.h"

enum error_lexical{ILLEGAL_CHAR,ILLEGAL_COLON,OVER_LEXICAL,ILLEGAL_IDENTIFIER};

char current_char;
int current_char_flag;
char current_lexical[LEXICAL_MAX_SIZE];
int current_lexical_size;

void lexicalPhase();

void initLexical();

void analysis();
void analysisIdentifier();
void analysisConst();

char readChar();
char readCharNotSpace();
void retract();
int storeChar();
void writeLexical(int number);

void doErrorLexical(int number);

void lexicalPhase(){
    pas_fp=fopen(PAS,"r");
    if(!pas_fp) return;
    dyd_fp=fopen(DYD,"w");
    err_fp=fopen(ERR,"w");
    initLexical();
    analysis();
    fclose(pas_fp);
    fclose(dyd_fp);
    fclose(err_fp);
}

void initLexical(){
    current_char_flag=0;
    file_current_line=1;
}

void analysis(){
    while(readCharNotSpace()!=(char)EOF){
        if(isLetter(current_char)){
            analysisIdentifier();
        }else if(isDigit(current_char)){
            analysisConst();
        }else{
            switch(current_char){
                case '=':
                    writeLexical(EQUAL);
                    break;
                case '-':
                    writeLexical(REDUCE);
                    break;
                case '*':
                    writeLexical(MULTIPLY);
                    break;
                case '(':
                    writeLexical(LEFT_BRACKETS);
                    break;
                case ')':
                    writeLexical(RIGHT_BRACKETS);
                    break;
                case ';':
                    writeLexical(SEMICOLON);
                    break;
                case '\n':
                    file_current_line++;
                    writeLexical(END_OF_LINE);
                    break;
                case '<':
                    readChar();
                    if(current_char=='='){
                        writeLexical(LESS_EQUAL);
                    }else if(current_char=='>'){
                        writeLexical(NOT_EQUAL);
                    }else{
                        retract();
                        writeLexical(LESS);
                    }
                    break;
                case '>':
                    readChar();
                    if(current_char=='='){
                        writeLexical(MORE_EQUAL);
                    }else{
                        retract();
                        writeLexical(MORE);
                    }
                    break;
                case ':':
                    readChar();
                    if(current_char=='='){
                        writeLexical(VALUE);
                    }else{
                        doErrorLexical(ILLEGAL_COLON);
                    }
                    break;
                default:
                    doErrorLexical(ILLEGAL_CHAR);
            }
        }
    }
    writeLexical(END_OF_FILE);
}
void analysisIdentifier(){
    int flag=1;
    while(isLetter(current_char) || isDigit(current_char)){
        if(storeChar()){
            flag=0;
            break;
        }else readChar();
    }
    if(flag){
        retract();
        int result=isReserve(current_lexical,current_lexical_size);
        if(result) writeLexical(result);
        else writeLexical(IDENTIFIER);
    }
}
void analysisConst(){
    int flag=1;
    while(isDigit(current_char)){
        if(storeChar()){
            flag=0;
            break;
        }else readChar();
    }
    if(flag){
        if(isLetter(current_char)){
            doErrorLexical(ILLEGAL_IDENTIFIER);
        }else{
            retract();
            writeLexical(CONST);
        }
    }
}

char readChar(){
    if(current_char_flag) current_char_flag=0;
    else current_char=(char)getc(pas_fp);
    return current_char;
}
char readCharNotSpace(){
    current_lexical_size=0;
    while(readChar()==' ');
    return current_char;
}
void retract(){
    current_char_flag=1;
}
int storeChar(){
    if(current_lexical_size<LEXICAL_MAX_SIZE){
        current_lexical[current_lexical_size]=current_char;
        current_lexical_size++;
        return 0;
    }else{
        doErrorLexical(OVER_LEXICAL);
        return 1;
    }
}
void writeLexical(int number){
    char * string;
    int size;
    if(number==IDENTIFIER || number==CONST){
        string=current_lexical;
        size=current_lexical_size;
    }else{
        string=category_string[number];
        size=category_string_size[number];
    }
    for(int i=0;i<LEXICAL_MAX_SIZE-size;i++) putc(' ',dyd_fp);
    for(int i=0;i<size;i++) putc(string[i],dyd_fp);
    putc(' ',dyd_fp);
    int number_10=number/10;
    int number_01=number%10;
    if(number_10) putc('0'+number_10,dyd_fp);
    else putc(' ',dyd_fp);
    putc('0'+number_01,dyd_fp);
    putc('\n',dyd_fp);
}

void doErrorLexical(int number){
    fputs("LINE:",err_fp);
    writeNotZeroNumber(file_current_line,err_fp);
    fputs("  ",err_fp);
    switch(number){
        case ILLEGAL_CHAR:
            fputs("illegal char",err_fp);
            break;
        case ILLEGAL_COLON:
            fputs("illegal colon",err_fp);
            retract();
            break;
        case OVER_LEXICAL:
            fputs("identifier length overflow",err_fp);
            do{
                readChar();
            }while(isLetter(current_char) || isDigit(current_char));
            retract();
            break;
        case ILLEGAL_IDENTIFIER:
            fputs("illegal identifier",err_fp);
            do{
                readChar();
            }while(isLetter(current_char) || isDigit(current_char));
            retract();
            break;
        default:
            fputs("unknown error",err_fp);
    }
    putc('\n',err_fp);
}
