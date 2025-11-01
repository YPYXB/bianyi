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


#define norw 14     /* �����ָ��������� else�� */
#define txmax 100   /* ���ű���󳤶� */
#define nmax 14     /* ���ֵ����λ�� */
#define al 10       /* ��ʶ����󳤶� */
#define amax 2047   /* ��ַ���� */
#define levmax 3    /* ������Ƕ�ײ��� [0, levmax] */
#define cxmax 500   /* ����������󳤶� */

/* ���ű��� */
enum symbol {
    nul,         ident,     number,     plus,      minus,
    times,       slash,     oddsym,     eql,       neq,
    lss,         leq,       gtr,        geq,       lparen,
    rparen,      comma,     semicolon,  period,    becomes,
    beginsym,    endsym,    ifsym,      thensym,   whilesym,
    writesym,    readsym,   dosym,      callsym,   constsym,
    varsym,      procsym,   elsesym,   /* ���� else ���� */
    lbrack,      rbrack,    /* �����﷨ '[' �� ']' */
    plusplus,    minusminus, /* ���� ++ �� -- ���� */
    plusequal,   minusequal, timesequal, slashequal, /* ���� +=, -=, *=, /= ���� */
    modsym,      /* ����������� % */
    notsym       /* �����߼��Ƿ��� ! */
};
#define symnum 43  // �����԰����߼��Ƿ���

/* ��ʶ������ */
enum object {
    constant,
    variable,
    procedur,
    array       // ������һά����
};

/* ָ������ */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     inte,
    jmp,     jpc,
    lodi,    sti      // ����������ļ��װ��/�洢ָ��
};
#define fctnum 10  // �� lodi �� sti ������

/* ָ��ṹ */
struct instruction
{
    enum fct f; /* ָ��Ĳ����� */
    int l;      /* ָ��Ĳ�β� */
    int a;      /* ָ��Ĳ��� */
};

FILE* fas;  /* ���Ʊ�����ļ� */
FILE* fa;   /* ��������ļ� */
FILE* fa1;  /* Դ�ļ���ÿ���׵�ַ����ļ� */
FILE* fa2;  /* ����ִ������ļ� */
bool listswitch;    /* �Ƿ��г�Ŀ����� */
bool tableswitch;   /* �Ƿ��г����ֱ� */
char ch;            /* �� getch ��ȡ�ĵ�ǰ�ַ� */
enum symbol sym;    /* ��ǰ���� */
char id[al+1];      /* ��ǰ��ʶ����ĩβ�� '\0' */
int num;            /* ��ǰ����ֵ */
int cc, ll;         /* getch ʹ�õļ�����cc ��ʾ��ǰ�ַ�λ�� */
int cx;             /* ��������λ�ã�ȡֵ��Χ [0, cxmax-1] */
char line[81];      /* ��ǰ�л����� */
char a[al+1];       /* �ݴ��ʶ���ַ���ĩβ�� '\0' */
struct instruction code[cxmax]; /* �������� */
char word[norw][al];        /* �����ֱ� */
enum symbol wsym[norw];     /* �����ֶ�Ӧ�ķ��� */
enum symbol ssym[256];      /* ���ַ���Ӧ�ķ��� */
char mnemonic[fctnum][5];   /* ָ�����Ƿ� */
bool declbegsys[symnum];    /* ������ʼ�ķ��ż��� */
bool statbegsys[symnum];    /* ��俪ʼ�ķ��ż��� */
bool facbegsys[symnum];     /* ���ӿ�ʼ�ķ��ż��� */

/* ���ű�ṹ */
struct tablestruct
{
    char name[al];      /* ���� */
    enum object kind;   /* ���ࣺconst, var, array �� procedure */
    int val;            /* ����ֵ��const ʹ�ã� */
    int level;          /* ���ڲ�Σ�const �ɲ�ʹ�ã� */
    int adr;            /* ��ַ��const �ɲ�ʹ�ã� */
    int size;           /* ռ�õ�Ԫ����������� procedure ʹ�ã� */
};
struct tablestruct table[txmax]; /* ���ű� */

FILE* fin;
FILE* fout;
char fname[al];
int err; /* ������ */

/* ������궨�� */
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
