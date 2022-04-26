#include "base.h"

enum error_syntax{LACK_LEXICAL,MATCH_LEXICAL,NO_DEFINE_VARIABLE,REPEAT_DEFINE_VARIABLE,NO_DEFINE_FUNCTION,REPEAT_DEFINE_FUNCTION,FORBID_DEFINE,OTHER_ERROR};

struct history{
    char pro_name[LEXICAL_MAX_SIZE];
    int pro_type;
    int pro_first_var_address;
    char pro_formal_var_name[LEXICAL_MAX_SIZE];
} history;
char var_name[LEXICAL_MAX_SIZE];
int var_kind;
int var_type;
int var_level;
int var_address;
int pro_level;
int pro_last_var_address;

int buffer_lexical[2];
int buffer_lexical_flag;
int buffer_lexical_interval_line;
char buffer_name[LEXICAL_MAX_SIZE];
int buffer_type;

char * message;
int file_last_error_line;

void syntaxPhase();

void initSyntax();

void programFormat();
void childProgramFormat();
void descriptionTableFormat();
void executeTableFormat();
void descriptionFormat();
void descriptionVariableFormat();
void descriptionFunctionFormat();
void executeFormat();
void executeReadFormat();
void executeWriteFormat();
void executeValueFormat();
void executeConditionFormat();
void conditionFormat();
void calculateFormat();
void reduceFactorFormat();
void multiplyFactorFormat();
void symbolFormat(int number);

int presentLexical();
void nextLexical();
void nextLexicalValidVariable();
void nextSemicolon();
void lastLexical();
void storeBufferName(int destination);
void storeBufferType(int destination);
int isDefinedVariable();
int isDefinedFunction();
void writeVariable();
void writeProcess();

void doErrorSyntax(int number);

void syntaxPhase(){
    err_fp=fopen(ERR,"r");
    if(err_fp){
        if(getc(err_fp)!=EOF){
            fclose(err_fp);
            return;
        }else fclose(err_fp);
    }
    dyd_fp=fopen(DYD,"r");
    if(!dyd_fp) return;
    dys_fp=fopen(DYS,"w");
    err_fp=fopen(ERR,"w");
    var_fp=fopen(VAR,"w");
    pro_fp=fopen(PRO,"w");
    initSyntax();
    programFormat();
    fclose(dyd_fp);
    fclose(dys_fp);
    fclose(err_fp);
    fclose(var_fp);
    fclose(pro_fp);
}

void initSyntax(){
    var_level=0;
    var_address=0;
    for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_name[i]=' ';
    pro_level=0;
    history.pro_first_var_address=0;
    for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_formal_var_name[i]=' ';
    buffer_lexical[0]=NOT_CATEGORY;
    buffer_lexical[1]=NOT_CATEGORY;
    buffer_lexical_flag=0;
    file_current_line=1;
    file_last_error_line=0;
}

void programFormat(){
    nextLexical();
    childProgramFormat();

#ifdef DEBUG
    puts("DONE_PROGRAM");
#endif

    symbolFormat(END_OF_FILE);

#ifdef DEBUG
    puts("DONE_FILE");
#endif

}
void childProgramFormat(){
    symbolFormat(BEGIN);
    var_level++;
    descriptionTableFormat();
    symbolFormat(SEMICOLON);
    executeTableFormat();
    if(presentLexical()==END){
        nextLexical();
    }else if(presentLexical()==END_OF_FILE){
        message=category_string[END];
        doErrorSyntax(MATCH_LEXICAL);
    }
    var_level--;
}
void descriptionTableFormat(){
    if(presentLexical()!=INTEGER){
        message=category_string[INTEGER];
        doErrorSyntax(LACK_LEXICAL);
        nextSemicolon();
    }else{
        nextLexical();
        descriptionFormat();
    }
    while(1){
        if(presentLexical()==SEMICOLON){
            nextLexical();
        }else{
            lastLexical();
            message=category_string[SEMICOLON];
            doErrorSyntax(LACK_LEXICAL);
            nextLexical();
            /* virtual semicolon */
            buffer_lexical[1]=SEMICOLON;
        }
        if(presentLexical()==INTEGER){
            nextLexical();
            descriptionFormat();
        }else{
            lastLexical();
            break;
        }
    }
}
void executeTableFormat(){
    executeFormat();
    while(1){
        if(presentLexical()==SEMICOLON) nextLexical();
        else if(presentLexical()==END || presentLexical()==END_OF_FILE) break;
        else{
            lastLexical();
            message=category_string[SEMICOLON];
            doErrorSyntax(LACK_LEXICAL);
            nextLexical();
        }
        executeFormat();
    }
}
void descriptionFormat(){ // start in the second word
    switch(presentLexical()){
        case IDENTIFIER:
            storeBufferName(0);
            storeBufferType(0);
            nextLexical();
            descriptionVariableFormat();
            break;
        case FUNCTION:
            nextLexical();
            descriptionFunctionFormat();
            break;
        default:
            lastLexical();
            message="variable name or function";
            doErrorSyntax(LACK_LEXICAL);
            nextSemicolon();
    }
}
void descriptionVariableFormat(){ // start in the third word
    var_kind=equalString(history.pro_formal_var_name,var_name,LEXICAL_MAX_SIZE);
    writeVariable();
}
void descriptionFunctionFormat(){ // start in the third word
    struct history temp=history;
    pro_level++;
    if(presentLexical()!=IDENTIFIER){
        message="function name";
        doErrorSyntax(LACK_LEXICAL);
        for(int i=0;i<LEXICAL_MAX_SIZE;i++) buffer_name[i]=' ';
        storeBufferName(1);
        storeBufferType(1);
    }else{
        storeBufferName(1);
        storeBufferType(1);
        nextLexical();
        if(isDefinedFunction()) doErrorSyntax(REPEAT_DEFINE_FUNCTION);
    }
    symbolFormat(LEFT_BRACKETS);
    if(presentLexical()!=IDENTIFIER){
        message="argument";
        doErrorSyntax(LACK_LEXICAL);
    }else{
        storeBufferName(2);
        nextLexical();
    }
    symbolFormat(RIGHT_BRACKETS);
    symbolFormat(SEMICOLON);
    history.pro_first_var_address=var_address;
    childProgramFormat();
    pro_last_var_address=var_address-1;
    writeProcess();
    history=temp;
    pro_level--;
}
void executeFormat(){
    switch(presentLexical()){
        case READ:
            nextLexical();
            executeReadFormat();
            break;
        case WRITE:
            nextLexical();
            executeWriteFormat();
            break;
        case IDENTIFIER:
            nextLexicalValidVariable();
            executeValueFormat();
            break;
        case IF:
            nextLexical();
            executeConditionFormat();
            break;
        case INTEGER:
            doErrorSyntax(FORBID_DEFINE);
            nextSemicolon();
            break;
        case END:
        case END_OF_FILE:
            lastLexical();
            message="unexpected ;";
            doErrorSyntax(OTHER_ERROR);
            nextLexical();
            break;
        default:
            message="read or write or variable or if";
            doErrorSyntax(LACK_LEXICAL);
            nextSemicolon();
    }
}
void executeReadFormat(){ // start in the second word
    symbolFormat(LEFT_BRACKETS);
    if(presentLexical()!=IDENTIFIER){
        message="argument";
        doErrorSyntax(LACK_LEXICAL);
    }else{
        nextLexicalValidVariable();
    }
    symbolFormat(RIGHT_BRACKETS);
}
void executeWriteFormat(){ // start in the second word
    symbolFormat(LEFT_BRACKETS);
    if(presentLexical()!=IDENTIFIER){
        message="argument";
        doErrorSyntax(LACK_LEXICAL);
    }else{
        nextLexicalValidVariable();
    }
    symbolFormat(RIGHT_BRACKETS);
}
void executeValueFormat(){ // start in the second word
    symbolFormat(VALUE);
    calculateFormat();
}
void executeConditionFormat(){ // start in the second word
    conditionFormat();
    symbolFormat(THEN);
    executeFormat();
    symbolFormat(ELSE);
    executeFormat();
}
void conditionFormat(){
    calculateFormat();
    switch(presentLexical()){
        case LESS:
        case LESS_EQUAL:
        case MORE:
        case MORE_EQUAL:
        case EQUAL:
        case NOT_EQUAL:
            nextLexical();
            break;
        default:
            message="relational operator";
            doErrorSyntax(LACK_LEXICAL);
    }
    calculateFormat();
}
void calculateFormat(){
    reduceFactorFormat();
    while(1){
        if(presentLexical()==REDUCE) nextLexical();
        else break;
        reduceFactorFormat();
    }
}
void reduceFactorFormat(){
    multiplyFactorFormat();
    while(1){
        if(presentLexical()==MULTIPLY) nextLexical();
        else break;
        multiplyFactorFormat();
    }
}
void multiplyFactorFormat(){
    switch(presentLexical()){
        case CONST:
            nextLexical();
            break;
        case IDENTIFIER:
        {
            char temp_pro_name[LEXICAL_MAX_SIZE];
            for(int i=0;i<LEXICAL_MAX_SIZE;i++) temp_pro_name[i]=history.pro_name[i];
            storeBufferName(1);
            storeBufferName(0);
            nextLexical();
            if(presentLexical()==LEFT_BRACKETS){
                pro_level++;
                if(!(equalString(temp_pro_name,history.pro_name,LEXICAL_MAX_SIZE) || isDefinedFunction())) doErrorSyntax(NO_DEFINE_FUNCTION);
                for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_name[i]=temp_pro_name[i];
                pro_level--;
                nextLexical();
                calculateFormat();
                symbolFormat(RIGHT_BRACKETS);
            }else{
                for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_name[i]=temp_pro_name[i];
                if(!(equalString(var_name,history.pro_name,LEXICAL_MAX_SIZE) || isDefinedVariable())){
                    lastLexical();
                    doErrorSyntax(NO_DEFINE_VARIABLE);
                    nextLexical();
                }
            }
        }
            break;
        default:
            message="unsigned number or variable or function";
            doErrorSyntax(LACK_LEXICAL);
    }
}
void symbolFormat(int number){
    if(presentLexical()!=number){
        message=category_string[number];
        switch(number){
            case SEMICOLON:
            case BEGIN:
            case LEFT_BRACKETS:
            case VALUE:
            case THEN:
            case ELSE:
                lastLexical();
                doErrorSyntax(LACK_LEXICAL);
                nextLexical();
                break;
            case RIGHT_BRACKETS:
                lastLexical();
                doErrorSyntax(MATCH_LEXICAL);
                nextLexical();
                break;
            case END_OF_FILE:
                message="unexpected code outside the program";
                doErrorSyntax(OTHER_ERROR);
                break;
            default:
                break;
        }
    }else nextLexical();
}

int presentLexical(){

#ifdef DEBUG
    printf("%d\n",buffer_lexical[buffer_lexical_flag]);
#endif

    return buffer_lexical[buffer_lexical_flag];
}
void nextLexical(){
    if(buffer_lexical_flag){
        buffer_lexical_flag=0;
        file_current_line+=buffer_lexical_interval_line;
    }else{
        int result;
        char dyd_line[DYD_LINE_SIZE];
        buffer_lexical_interval_line=0;
        while(1){
            for(int i=0;i<DYD_LINE_SIZE;i++) dyd_line[i]=(char)getc(dyd_fp);
            if(dyd_line[0]==(char)EOF) result=NOT_CATEGORY;
            else if(dyd_line[DYD_LINE_SIZE-3]==' ') result=dyd_line[DYD_LINE_SIZE-2]-'0';
            else result=(dyd_line[DYD_LINE_SIZE-3]-'0')*10+dyd_line[DYD_LINE_SIZE-2]-'0';
            if(result==END_OF_LINE){
                buffer_lexical_interval_line++;
                file_current_line++;
            }else break;
        }
        switch(result){
            case IDENTIFIER:
                for(int i=0;i<LEXICAL_MAX_SIZE;i++) buffer_name[i]=dyd_line[i];
                break;
            case INTEGER:
                buffer_type=TYPE_INTEGER;
                break;
            default:
                break;
        }
        buffer_lexical[1]=buffer_lexical[0];
        buffer_lexical[0]=result;
    }
}
void nextLexicalValidVariable(){
    storeBufferName(0);
    nextLexical();
    if(!(equalString(var_name,history.pro_name,LEXICAL_MAX_SIZE) || isDefinedVariable())) doErrorSyntax(NO_DEFINE_VARIABLE);
}
void nextSemicolon(){
    while(1){
        nextLexical();
        if(buffer_lexical[buffer_lexical_flag]==NOT_CATEGORY) break;
        if(buffer_lexical[buffer_lexical_flag]==END_OF_FILE) break;
        if(buffer_lexical[buffer_lexical_flag]==END) break;
        if(buffer_lexical[buffer_lexical_flag]==SEMICOLON) break;
        /* virtual semicolon */
        if(buffer_lexical_interval_line){
            buffer_lexical[1]=SEMICOLON;
            lastLexical();
            break;
        }
    }
}
void lastLexical(){
    buffer_lexical_flag=1;
    file_current_line-=buffer_lexical_interval_line;
}
void storeBufferName(int destination){
    switch(destination){
        case 0:
            for(int i=0;i<LEXICAL_MAX_SIZE;i++) var_name[i]=buffer_name[i];
            break;
        case 1:
            for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_name[i]=buffer_name[i];
            break;
        case 2:
            for(int i=0;i<LEXICAL_MAX_SIZE;i++) history.pro_formal_var_name[i]=buffer_name[i];
            break;
        default:
            break;
    }
}
void storeBufferType(int destination){
    switch(destination){
        case 0:
            var_type=buffer_type;
            break;
        case 1:
            history.pro_type=buffer_type;
            break;
        default:
            break;
    }
}
int isDefinedVariable(){ // prepare global variable : var_name
    fclose(var_fp);
    var_fp=fopen(VAR,"r");
    int result=0;
    for(int i=0;i<history.pro_first_var_address;){
        if((char)getc(var_fp)=='\n') i++;
    }
    while(1){
        char temp_var_name[LEXICAL_MAX_SIZE];
        for(int i=0;i<LEXICAL_MAX_SIZE;i++) temp_var_name[i]=(char)getc(var_fp);
        if(temp_var_name[0]==(char)EOF) break;
        if(equalString(temp_var_name,var_name,LEXICAL_MAX_SIZE)){
            for(int i=0;i<LEXICAL_MAX_SIZE+1;i++) getc(var_fp);
            for(int i=0;i<3;){
                if((char)getc(var_fp)==' ') i++;
            }
            int number=0;
            while(1){
                char temp_char=(char)getc(var_fp);
                if(temp_char==' ') break;
                number=number*10+temp_char-'0';
            }
            if(number==var_level){
                result=1;
                break;
            }
        }
        while((char)getc(var_fp)!='\n');
    }
    fclose(var_fp);
    var_fp=fopen(VAR,"a");
    return result;
}
int isDefinedFunction(){ // prepare global variable : history.pro_name pro_level
    fclose(pro_fp);
    pro_fp=fopen(PRO,"r");
    int result=0;
    while(1){
        char temp_pro_name[LEXICAL_MAX_SIZE];
        for(int i=0;i<LEXICAL_MAX_SIZE;i++) temp_pro_name[i]=(char)getc(pro_fp);
        if(temp_pro_name[0]==(char)EOF) break;
        for(int i=0;i<2;){
            if((char)getc(pro_fp)==' ') i++;
        }
        int number=0;
        while(1){
            char temp_char=(char)getc(pro_fp);
            if(temp_char==' ') break;
            number=number*10+temp_char-'0';
        }
        /* it may be a bit difficult to understand the following two lines of code */
        /* however they do the right thing */
        if(number<pro_level) result=0;
        else if(number==pro_level && equalString(temp_pro_name,history.pro_name,LEXICAL_MAX_SIZE)) result=1;
        while((char)getc(pro_fp)!='\n');
    }
    fclose(pro_fp);
    pro_fp=fopen(PRO,"a");
    return result;
}
void writeVariable(){
    if(isDefinedVariable()) doErrorSyntax(REPEAT_DEFINE_VARIABLE);
    else{
        for(int i=0;i<LEXICAL_MAX_SIZE;i++) putc(var_name[i],var_fp);
        putc(' ',var_fp);
        for(int i=0;i<LEXICAL_MAX_SIZE;i++) putc(history.pro_name[i],var_fp);
        putc(' ',var_fp);
        writeNumber(var_kind,var_fp);
        putc(' ',var_fp);
        writeNumber(var_type,var_fp);
        putc(' ',var_fp);
        writeNumber(var_level,var_fp);
        putc(' ',var_fp);
        writeNumber(var_address,var_fp);
        var_address++;
        putc('\n',var_fp);
    }
}
void writeProcess(){
    for(int i=0;i<LEXICAL_MAX_SIZE;i++) putc(history.pro_name[i],pro_fp);
    putc(' ',pro_fp);
    writeNumber(history.pro_type,pro_fp);
    putc(' ',pro_fp);
    writeNumber(pro_level,pro_fp);
    putc(' ',pro_fp);
    writeNumber(history.pro_first_var_address,pro_fp);
    putc(' ',pro_fp);
    writeNumber(pro_last_var_address,pro_fp);
    putc('\n',pro_fp);
}

void doErrorSyntax(int number){
    if(file_last_error_line==file_current_line) return;
    file_last_error_line=file_current_line;
    fputs("LINE:",err_fp);
    writeNotZeroNumber(file_current_line,err_fp);
    fputs("  ",err_fp);
    switch(number){
        case LACK_LEXICAL:
            fputs("lack of ",err_fp);
            fputs(message,err_fp);
            break;
        case MATCH_LEXICAL:
            fputs("missing ",err_fp);
            fputs(message,err_fp);
            break;
        case NO_DEFINE_VARIABLE:
            fputs("undefined variable ",err_fp);
            for(int i=0;i<LEXICAL_MAX_SIZE;i++){
                if(var_name[i]!=' ') fputc(var_name[i],err_fp);
            }
            break;
        case REPEAT_DEFINE_VARIABLE:
            fputs("repeatedly defined variable ",err_fp);
            for(int i=0;i<LEXICAL_MAX_SIZE;i++){
                if(var_name[i]!=' ') fputc(var_name[i],err_fp);
            }
            break;
        case NO_DEFINE_FUNCTION:
            fputs("undefined function ",err_fp);
            for(int i=0;i<LEXICAL_MAX_SIZE;i++){
                if(history.pro_name[i]!=' ') fputc(history.pro_name[i],err_fp);
            }
            break;
        case REPEAT_DEFINE_FUNCTION:
            fputs("repeatedly defined function ",err_fp);
            for(int i=0;i<LEXICAL_MAX_SIZE;i++){
                if(history.pro_name[i]!=' ') fputc(history.pro_name[i],err_fp);
            }
            break;
        case FORBID_DEFINE:
            fputs("wrong place to define variable or function",err_fp);
            break;
        case OTHER_ERROR:
            fputs(message,err_fp);
            break;
        default:
            fputs("unknown error",err_fp);
    }
    putc('\n',err_fp);

#ifdef DEBUG
    puts("ERROR");
#endif

}
