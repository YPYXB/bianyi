/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0,  Visual C++.NET and
 * Visual C++.NET 2003,  on Win98,  WinNT,  Win2000,  WinXP and Win2003
 *
 */

typedef enum {
    false,
    true
} bool;


#define norw 14     /* 保留字个数（增加 else） */
#define txmax 100   /* 符号表最大长度 */
#define nmax 14     /* 数字的最大位数 */
#define al 10       /* 标识符最大长度 */
#define amax 2047   /* 地址上限 */
#define levmax 3    /* 最大过程嵌套层数 [0, levmax] */
#define cxmax 500   /* 代码区的最大长度 */

/* 符号表征 */
enum symbol {
    nul,         ident,     number,     plus,      minus,
    times,       slash,     oddsym,     eql,       neq,
    lss,         leq,       gtr,        geq,       lparen,
    rparen,      comma,     semicolon,  period,    becomes,
    beginsym,    endsym,    ifsym,      thensym,   whilesym,
    writesym,    readsym,   dosym,      callsym,   constsym,
    varsym,      procsym,   elsesym,   /* 新增 else 符号 */
    lbrack,      rbrack,    /* 数组语法 '[' 和 ']' */
    plusplus,    minusminus, /* 新增 ++ 和 -- 符号 */
    plusequal,   minusequal, timesequal, slashequal, /* 新增 +=, -=, *=, /= 符号 */
    modsym,      /* 新增求余符号 % */
    notsym       /* 新增逻辑非符号 ! */
};
#define symnum 43  // 增加以包含逻辑非符号

/* 标识符种类 */
enum object {
    constant,
    variable,
    procedur,
    array       // 新增：一维数组
};

/* 指令种类 */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     inte,
    jmp,     jpc,
    lodi,    sti      // 新增：数组的间接装载/存储指令
};
#define fctnum 10  // 因 lodi 和 sti 而增加

/* 指令结构 */
struct instruction
{
    enum fct f; /* 指令的操作码 */
    int l;      /* 指令的层次差 */
    int a;      /* 指令的参数 */
};

FILE* fas;  /* 名称表输出文件 */
FILE* fa;   /* 代码输出文件 */
FILE* fa1;  /* 源文件及每行首地址输出文件 */
FILE* fa2;  /* 解释执行输出文件 */
bool listswitch;    /* 是否列出目标代码 */
bool tableswitch;   /* 是否列出名字表 */
char ch;            /* 由 getch 读取的当前字符 */
enum symbol sym;    /* 当前符号 */
char id[al+1];      /* 当前标识符，末尾有 '\0' */
int num;            /* 当前数字值 */
int cc, ll;         /* getch 使用的计数：cc 表示当前字符位置 */
int cx;             /* 代码生成位置，取值范围 [0, cxmax-1] */
char line[81];      /* 当前行缓冲区 */
char a[al+1];       /* 暂存标识符字符，末尾有 '\0' */
struct instruction code[cxmax]; /* 代码数组 */
char word[norw][al];        /* 保留字表 */
enum symbol wsym[norw];     /* 保留字对应的符号 */
enum symbol ssym[256];      /* 单字符对应的符号 */
char mnemonic[fctnum][5];   /* 指令助记符 */
bool declbegsys[symnum];    /* 声明开始的符号集合 */
bool statbegsys[symnum];    /* 语句开始的符号集合 */
bool facbegsys[symnum];     /* 因子开始的符号集合 */

/* 符号表结构 */
struct tablestruct
{
    char name[al];      /* 名称 */
    enum object kind;   /* 种类：const, var, array 或 procedure */
    int val;            /* 常量值（const 使用） */
    int level;          /* 所在层次（const 可不使用） */
    int adr;            /* 地址（const 可不使用） */
    int size;           /* 占用单元数量（数组或 procedure 使用） */
};
struct tablestruct table[txmax]; /* 符号表 */

FILE* fin;
FILE* fout;
char fname[al];
int err; /* 错误码 */

/* 错误处理宏定义 */
#define getsymdo                      if(-1 == getsym()) return -1
#define getchdo                       if(-1 == getch()) return -1
#define testdo(a, b, c)               if(-1 == test(a, b, c)) return -1
#define gendo(a, b, c)                if(-1 == gen(a, b, c)) return -1
#define expressiondo(a, b, c)         if(-1 == expression(a, b, c)) return -1
#define factordo(a, b, c)             if(-1 == factor(a, b, c)) return -1
#define termdo(a, b, c)               if(-1 == term(a, b, c)) return -1
#define conditiondo(a, b, c)          if(-1 == condition(a, b, c)) return -1
#define statementdo(a, b, c)          if(-1 == statement(a, b, c)) return -1
#define constdeclarationdo(a, b, c)   if(-1 == constdeclaration(a, b, c)) return -1
#define vardeclarationdo(a, b, c)     if(-1 == vardeclaration(a, b, c)) return -1

void error(int n);
int getsym();
int getch();
void init();
int gen(enum fct x, int y, int z);
int test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
int block(int lev, int tx, bool* fsys);
void interpret();
int factor(bool* fsys, int* ptx, int lev);
int term(bool* fsys, int* ptx, int lev);
int condition(bool* fsys, int* ptx, int lev);
int expression(bool* fsys, int* ptx, int lev);
int statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
int vardeclaration(int* ptx, int lev, int* pdx);
int constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);
