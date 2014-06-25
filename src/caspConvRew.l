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


    /*string int2string(int number)
    {
        stringstream ss;//create a stringstream
        ss << number << endl;//add number to the stream
        return ss.str();//return a string with the contents of the stream
    }*/

int CnsNum = 0, LineNum = 1, VarNum = 0, quoteflag = 0, iii = 0, len = 0, dcn = 0;
bool flag = 0, headguess = 0, body = 0;
char * tmp, * out;
string info = "", TMPS = "";
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

/***yyout << endl << "CONSTRAINT FOUND: " << YYText() << endl;*/

            dcn = 0; nguess.str(string());
tmp = new char [YYLeng()]; tmp = strdup(YYText()); len = 0; iii = 0;
/**yyout << "TMP = " << tmp << endl;*/
for(int k=0;k<YYLeng();k++){
if(tmp[k] != ' ' && tmp[k] != '\t'){ len++; }
if(tmp[k] == '$'){ dcn++;}
}
/**yyout << "LEN = " << len << endl;*/
/**yyout << "DCN = " << dcn << endl;*/
out = new char [len]; for(int k=0;k<len;k++){out[k]='\0';}
for(int k=0;k<YYLeng();k++){
if(tmp[k] != ' ' && tmp[k] != '\t'){ out[iii] = tmp[k]; /**yyout << "III = " << iii << "\t" << out << "\t" << tmp[k] << endl;*/ iii++; }
}
/**yyout << "OUT = " << out << endl;
*yyout << "OUT[" << len-1 << "] = " << out[len-1] << endl;*/
            if(cnsids.count(string(out))==0){ CnsNum++; cnsids.insert({string(out),CnsNum}); depmap.insert({CnsNum,""});}
*yyout << "expr" << CnsNum << "(";
            nguess << "expr" << CnsNum << "(";
            for(int k=0;k<len;k++){*yyout << out[k]; nguess << out[k];}
delete tmp; delete out;
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
out = new char [iii];
for(int k=0;k<iii;k++){ out[k] = tmp[k]; }
*yyout << ",";


nguess << ",";
for(int k=0;k<iii;k++){*yyout << out[k]; nguess << out[k]; }

TMPS = " "; TMPS.append(out);
depmap.at(CnsNum).append(TMPS);
if(varids.count(string(out)) == 0){ varids.insert({string(out),VarNum}); VarNum++; }
delete tmp; delete out;
yyless(0); dcn--;
BEGIN(OPSCAN);
}

<ENDCA>({COMPARISON}|{BINOPERAT}){ANYSPACE}({BINOPERAT}?{ATOM}{BINOPERAT})*{ATOM}{ANYSPACE}([\.,\|]|{IFSYMBOL}|{DISJ}) {

            /***yyout << endl << "VARIABLE FOUND: " << YYText() << " (end)" << endl;*/

tmp = new char [YYLeng()]; tmp = strdup(YYText()); iii = 0; len = 0; flag = 0;
for(int k=0;k<YYLeng();k++){
if( tmp[k]!=' ' && tmp[k]!='\t' && tmp[k]!='=' && tmp[k]!='<' && tmp[k]!='>' && tmp[k]!='!' && (tmp[k]!='+' || (k>0 && tmp[k-1]!='$')) && (tmp[k]!='-' || (k>0 && tmp[k-1]!='$' && tmp[k-1]!=':')) && (tmp[k]!='*' || (k>0 && tmp[k-1]!='$')) && (tmp[k]!='\\' || (k>0 && tmp[k-1]!='$')) && tmp[k]!='.' && (tmp[k]!=',' || flag) && tmp[k]!=':' && tmp[k]!='|' ){
len++; if(len==1){ iii=k; }
if( tmp[k] == '('){ flag = 1; }
if( tmp[k] == ')' && flag) { flag = 0;}
}
}
out = new char [len];
for(int k=0;k<len;k++){ out[k] = tmp[iii+k]; }
*yyout << ",";
for(int k=0;k<len;k++){ *yyout << out[k]; }
*yyout << ")";
if(tmp[YYLeng()-2]==':' && tmp[YYLeng()-1]=='-'){*yyout << " ";}
if(body){
nguess << ",";
for(int k=0;k<len;k++){ nguess << out[k];}
                nguess << ")";
                guesses << nguess.str() << " | not_" << nguess.str() << "." << endl;
}
TMPS = " "; TMPS.append(out);
depmap.at(CnsNum).append(TMPS);
if(varids.count(string(out)) == 0){ varids.insert({string(out),VarNum}); VarNum++; }
delete tmp; delete out;
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
                guesses << nguess.str() << " | not_" << nguess.str() << "." << endl;
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

void convert(istream& i, ostream& o)
{
    string S = ""; bool B = 0;
    bool erased = 1, copied = 0;
    unordered_map<int,bool*> Map = {};

    CnsNum = 0; LineNum = 1; VarNum = 0; quoteflag = 0; iii = 0; len = 0; dcn = 0;
    flag = 0; body = 0;
    info = ""; TMPS = "";
    guesses.str(string()); nguess.str(string()); domain.str(string());
    depmap = {}; varids = {}; cnsids = {};

    istream * inp = &i; ostream * outp = &o;

    FlexLexer* lexer = new yyFlexLexer;
    while(lexer->yylex(inp, outp) != 0);
          /*   while(lexer->yylex() != 0)
                 ;*/


    o << endl << guesses.str() << endl;
    guesses.str(string());

    Map = {};
    for (int h=1; h<depmap.size()+1; h++){ Map[h] = new bool [VarNum]; for (int j=0;j<VarNum;j++){ Map[h][j] = 0;}}
    S = ""; B = 0;
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
    erased = 1, copied = 0;
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

    /**yyout << "\n# Conversion of file " << argv[L] << " ends.\n" << endl;*/
   return;
}
