%option noyywrap
%option c++

%x ENDCA
%x OPSCAN
%x QUOTEDSTRING
%x VARSCAN
%x COMMENT

IFSYMBOL :\-
DISJ " v "
COMPARISON (!=|<>|<=|>=|<|>|=)
BINOPERAT [\+\-\*\/]
DCOMPARISON \$(!=|<>|<=|>=|<|>|=)
DBINOPERAT \$[\+\-\*\/]
TERM [A-Za-z][0-9a-zA-Z\-\_]*
ARGUMENT \([^\.\:\|\)]+[^\.\:\|\(]*\)
ATMARG {TERM}{ARGUMENT}?
ATOM {ATMARG}(\(({ATMARG}\,)*{ATMARG}\))?
DHCOMMENT \%[.\n]*\n
ANYSPACE [ \t]*
AOD {ATOM}|[0-9]+
NODOLLAR (({BINOPERAT}{ANYSPACE})?{AOD}{ANYSPACE}{BINOPERAT}{ANYSPACE})*

%{
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;

int CnsNum = 0, LineNum = 1, VarNum = 0, quoteflag = 0, iii = 0, len = 0, dcn = 0, Ref = 0;
bool flag = 0, headguess = 0, body = 0;
char * tmp;
string info = "", TMPS = "";
stringstream out;
stringstream guesses;
stringstream nguess;
stringstream domain;

unordered_map<int,string> depmap = {};
unordered_map<string,int> varids = {};
unordered_map<string,int> cnsids = {};

%}

%%

\% { BEGIN(COMMENT); quoteflag = 0; ECHO; }
<ENDCA>\% { BEGIN(COMMENT); quoteflag = 1; ECHO; }
<OPSCAN>\% { BEGIN(COMMENT); quoteflag = 2; ECHO; }
<COMMENT>\n { ++LineNum; if(quoteflag==0){BEGIN(INITIAL);}
else if(quoteflag==1){BEGIN(ENDCA);}
else if(quoteflag==2){BEGIN(OPSCAN);}
else if(quoteflag==3){BEGIN(ENDCA);}
ECHO; }

\" { BEGIN(QUOTEDSTRING); quoteflag = 0; ECHO; }
<ENDCA>\" { BEGIN(QUOTEDSTRING); quoteflag = 1; ECHO; }
<OPSCAN>\" { BEGIN(QUOTEDSTRING); quoteflag = 2; ECHO; }
<QUOTEDSTRING>\" { if(quoteflag==0){BEGIN(INITIAL);}
else if(quoteflag==1){BEGIN(ENDCA);}
else if(quoteflag==2){BEGIN(ENDCA);}
else if(quoteflag==3){BEGIN(ENDCA);} ECHO; }


<INITIAL,ENDCA,OPSCAN,VARSCAN,QUOTEDSTRING>\n { ++LineNum; ECHO; }

\$"domain"\(.+\) { ECHO; /***yyout << "DOM FOUND: " << YYText() << endl;*/ for(int k=1;k<YYLeng();k++){domain << YYText()[k];} /**yyout << "DOMAIN = " << domain.str() << endl;*/}

\$("maximize"|"minimize")\(.+\) { ECHO; /***yyout << "MINMAX FOUND: " << YYText() << endl;*/ domain << ","; for(int k=1;k<YYLeng();k++){ domain << YYText()[k];} /**yyout << "DOMAIN = " << domain.str() << endl;*/ }

({NODOLLAR}{AOD}{ANYSPACE}{DBINOPERAT}{ANYSPACE})*{NODOLLAR}{AOD}{ANYSPACE}{DCOMPARISON}{ANYSPACE}({NODOLLAR}{AOD}{ANYSPACE}{DBINOPERAT}{ANYSPACE})*{NODOLLAR}{AOD} {

        /** *yyout << endl << "CONSTRAINT FOUND: |" << YYText() << "|" << endl; */

        dcn = 0; nguess.str(string()); out.str(string());
        tmp = new char [YYLeng()]; tmp = strdup(YYText()); len = 0;
        /** *yyout << "TMP = " << tmp << endl;*/
        for(int k=0;k<YYLeng();k++){
            if(tmp[k] != ' ' && tmp[k] != '\t' && tmp[k] != '\0'){ len++; }
            if(tmp[k] == '$'){ dcn++;}
        }
        /** *yyout << "LEN = " << len << endl;
        *yyout << "DCN = " << dcn << endl; */

        for(int k=0;k<YYLeng();k++){
            if(tmp[k] != ' ' && tmp[k] != '\t'){ out << tmp[k]; len--; if(len==0){break;} }
        }
        /** *yyout << "OUT = " << out << endl;
        *yyout << "OUT[" << len-1 << "] = " << out[len-1] << endl;*/
        if(cnsids.count(out.str())==0){ CnsNum++; cnsids.insert({out.str(),CnsNum}); depmap.insert({CnsNum,""});}
        Ref = cnsids[out.str()];
        *yyout << "expr" << Ref << "(\"" << out.str();
        nguess << "expr" << Ref << "(\"" << out.str();
        *yyout << "\""; nguess << "\"";
        delete tmp; out.str(string());
        yyless(0);
        BEGIN(VARSCAN);
    }

<VARSCAN>[0-9]+{ANYSPACE}\$ {

        /***yyout << endl << "NUMBER FOUND: " << YYText() << " (middle)" << endl;*/

        yyless(0); dcn--;
        BEGIN(OPSCAN);
    }

<VARSCAN>({BINOPERAT}?{ATOM}{BINOPERAT})*{ATOM}{ANYSPACE}\$ {

        /***yyout << endl << "VARIABLE FOUND: " << YYText() << " (middle)" << endl;*/

        tmp = new char [YYLeng()]; tmp = strdup(YYText());
        iii = 0;
        for(int k=0;k<YYLeng();k++){
        if(tmp[k] == ' '||tmp[k] == '$'||tmp[k] == '\t'){ iii = k; break; }
        }
        out.str(string());
        for(int k=0;k<iii;k++){ out << tmp[k]; }
        *yyout << ",";
        nguess << ",";
        *yyout << out.str(); nguess << out.str();
        TMPS = " "; TMPS.append(out.str());
        depmap.at(Ref).append(TMPS);
        if(varids.count(out.str()) == 0){ varids.insert({out.str(),VarNum}); VarNum++; }
        delete tmp; out.str(string());
        yyless(0); dcn--;
        BEGIN(OPSCAN);
    }

<ENDCA>({COMPARISON}|{BINOPERAT}){ANYSPACE}({BINOPERAT}?{ATOM}{BINOPERAT})*{ATOM}{ANYSPACE}([\.,\|]|{IFSYMBOL}|{DISJ}) {

        /**  *yyout << endl << "VARIABLE FOUND: " << YYText() << " (end)" << endl; */

        tmp = new char [YYLeng()]; tmp = strdup(YYText()); iii = 0; len = 0; flag = 0;
        for(int k=0;k<YYLeng();k++){
            if( tmp[k]!=' ' && tmp[k]!='\t' && tmp[k]!='=' && tmp[k]!='<' && tmp[k]!='>' && tmp[k]!='!'
                    && (tmp[k]!='+' || (k>0 && tmp[k-1]!='$')) && (tmp[k]!='-' || (k>0 && tmp[k-1]!='$' && tmp[k-1]!=':'))
                    && (tmp[k]!='*' || (k>0 && tmp[k-1]!='$')) && (tmp[k]!='\\' || (k>0 && tmp[k-1]!='$')) && tmp[k]!='.'
                    && (tmp[k]!=',' || flag) && tmp[k]!=':' && tmp[k]!='|' && (tmp[k] != 'v' ||(k>0 && tmp[k-1]!=' ')) ){
                len++; if(len==1){ iii=k; }
                if( tmp[k] == '('){ flag = 1; }
                if( tmp[k] == ')' && flag) { flag = 0;}
            }
        }
        out.str(string());
        for(int k=0;k<len;k++){ out << tmp[iii+k]; }
        *yyout << ",";
        *yyout << out.str() << ")";
        if(tmp[YYLeng()-2]==':' && tmp[YYLeng()-1]=='-'){*yyout << " ";}
        if(body){
            nguess << ",";
            nguess << out.str();
            nguess << ")";
            guesses << nguess.str() << " v not_" << nguess.str() << "." << endl;
        }
        TMPS = " "; TMPS.append(out.str());
        depmap.at(Ref).append(TMPS);
        if(varids.count(out.str()) == 0){ varids.insert({out.str(),VarNum}); VarNum++; }
        delete tmp; out.str(string());
        if((YYText()[YYLeng()-3]!=' ' && YYText()[YYLeng()-2]==':' && YYText()[YYLeng()-1]=='-')||(YYText()[YYLeng()-2]==' ' && YYText()[YYLeng()-1]=='|')){yyless(YYLeng()-2);}
        else if((YYText()[YYLeng()-3]==' ' && YYText()[YYLeng()-2]=='v' && YYText()[YYLeng()-1]==' ')||(YYText()[YYLeng()-3]==' ' && YYText()[YYLeng()-2]==':' && YYText()[YYLeng()-1]=='-')){yyless(YYLeng()-3);}
        else{yyless(YYLeng()-1);}
        BEGIN(INITIAL);
    }

<ENDCA>({COMPARISON}|{BINOPERAT}){ANYSPACE}[0-9]+{ANYSPACE}([\.,\|]|{IFSYMBOL}|{DISJ}) {

        /***yyout << endl << "NUMBER FOUND: " << YYText() << " (end)" << endl;*/

        *yyout << ")";
        if(tmp[YYLeng()-2]==':' && tmp[YYLeng()-1]=='-'){*yyout << " ";}
        if(body){
            nguess << ")";
            guesses << nguess.str() << " v not_" << nguess.str() << "." << endl;
        }
        if((YYText()[YYLeng()-3]!=' ' && YYText()[YYLeng()-2]==':' && YYText()[YYLeng()-1]=='-')||(YYText()[YYLeng()-2]==' ' && YYText()[YYLeng()-1]=='|')){yyless(YYLeng()-2);}
        else if((YYText()[YYLeng()-3]==' ' && YYText()[YYLeng()-2]=='v' && YYText()[YYLeng()-1]==' ')||(YYText()[YYLeng()-3]==' ' && YYText()[YYLeng()-2]==':' && YYText()[YYLeng()-1]=='-')){yyless(YYLeng()-3);}
        else{yyless(YYLeng()-1);}

        BEGIN(INITIAL);
}

<OPSCAN>{DCOMPARISON} { yyless(1); if( dcn==0 ){ BEGIN(ENDCA); }else{ BEGIN(VARSCAN); } }

<OPSCAN>{DBINOPERAT} { yyless(1); if( dcn==0 ){ BEGIN(ENDCA); }else{ BEGIN(VARSCAN); } }


<VARSCAN,OPSCAN,ENDCA>. ;

{IFSYMBOL} {ECHO; body = 1;}

\. {ECHO; body = 0;}

%%

char* AppendDotConv(char* array)
{
    size_t len = strlen(array);

    char* ret = new char[len+6];

    strcpy(ret, array);
    ret[len] = '.'; ret[len+1] = 'c'; ret[len+2] = 'o'; ret[len+3] = 'n'; ret[len+4] = 'v';
    ret[len+5] = '\0';

    return ret;
}

void CaspConverter::convert(istream& i, ostream& o)
{

    istream * inp = &i; ostream * outp = &o;

    FlexLexer* lexer = new yyFlexLexer;
    while(lexer->yylex(inp, outp) != 0);

    o << endl << guesses.str() << endl;
    guesses.str(string());

    unordered_map<int,bool*> Map = {};
    for (int h=1; h<depmap.size()+1; h++){ Map[h] = new bool [VarNum]; for (int j=0;j<VarNum;j++){ Map[h][j] = 0;}}
    string S = "";
    bool B = 0;
    for (int h=1; h<depmap.size()+1; h++){
        S = "";
        for (int j=0; j<depmap.at(h).size(); j++){
            if (depmap.at(h)[j] != ' '){ S.append(1u,depmap.at(h)[j]); B = 1; }
            if ((B && depmap.at(h)[j]==' ') || j == depmap.at(h).size()-1){
                B = 0;
                for( auto it = varids.begin(); it != varids.end(); ++it ){
                    if (S == it->first){ Map[h][it->second] = 1; S = ""; break; }
                }
            }
        }
    }
    /** // Printing Map:
    * *yyout << "\n\nMatrice delle adiacenze:" << endl;
    * for ( auto it = Map.begin(); it != Map.end(); ++it ){
    * *yyout << "{" << it->first << ": ";
    * for (int j=0;j<VarNum;j++){
    * *yyout << it->second[j] << " ";
    * }
    * *yyout << "}" << endl;
    * }
    */
    bool erased = 1, copied = 0;
    int * list; list = new int [2*CnsNum]; for(int i=0;i<2*CnsNum;i++){list[i] = 0;} iii = 0; len = 1;
    while(true){
        while(Map.count(len)==0){ len++; }
        erased = 0;
        if(Map.size()==1){ list[iii] = len; /**yyout << "LIST = ";for(int h=0;h<=iii;h++){*yyout << list[h] << " ";} *yyout << endl;*/ iii++; list[iii] = 0; Map.erase(len); break;}
        for(int i=len+1;i<CnsNum+1;i++){
            copied = 0;
        if(Map.count(i)!=0){
                for(int j=0;j<VarNum;j++){
                    if(Map[len][j] && Map[i][j]){
                for(int k=j+1;k<VarNum;k++){
                    if(Map[i][k]){Map[len][k] = 1; Map[i][k] = 0; copied = 1; }
                }
                }
            }
            if(copied){list[iii] = i; /**yyout << "LIST = ";for(int h=0;h<=iii;h++){*yyout << list[h] << " ";} *yyout << endl;*/ iii++; Map.erase(i); erased = 1;
            }}
        }
        if(!erased){ list[iii] = len; /**yyout << "LIST = ";for(int h=0;h<=iii;h++){*yyout << list[h] << " ";} *yyout << endl;*/ iii++; list[iii] = 0; iii++; Map.erase(len); len++; /**yyout << "LEN = " << len << endl;*/ }
    }
    /** // Printing Map:
    * *yyout << "\n\nMatrice delle adiacenze:" << endl;
    * for ( auto it = Map.begin(); it != Map.end(); ++it ){
    * *yyout << "{" << it->first << ": ";
    * for (int j=0;j<VarNum;j++){
    * *yyout << it->second[j] << " ";
    * }
    * *yyout << "}" << endl;
    * }
    */
    iii=0;
    while(iii<2*CnsNum){
        o << ":- not &casp[" << domain.str();
        while(list[iii]!=0){
        *outp << ",expr" << list[iii] << ",not_expr" << list[iii];
            iii++;
        }
        o << "]()." << endl;
        iii++;
        if(list[iii]==0){ break;}
    }
    delete list;
   return;
}
