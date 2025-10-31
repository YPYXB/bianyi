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


#define norw 13     /* ï¿½Ø¼ï¿½ï¿½Ö¸ï¿½ï¿½ï¿½ */
#define txmax 100   /* ï¿½ï¿½ï¿½Ö±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
#define nmax 14     /* numberï¿½ï¿½ï¿½ï¿½ï¿½Î»ï¿½ï¿? */
#define al 10       /* ï¿½ï¿½ï¿½Åµï¿½ï¿½ï¿½ó³¤¶ï¿? */
#define amax 2047   /* ï¿½ï¿½Ö·ï¿½Ï½ï¿½*/
#define levmax 3    /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? [0,  levmax]*/
#define cxmax 500   /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */

/* ï¿½ï¿½ï¿½ï¿½ */
enum symbol {
    nul,         ident,     number,     plus,      minus,
    times,       slash,     oddsym,     eql,       neq,
    lss,         leq,       gtr,        geq,       lparen,
    rparen,      comma,     semicolon,  period,    becomes,
    beginsym,    endsym,    ifsym,      thensym,   whilesym,
    writesym,    readsym,   dosym,      callsym,   constsym,
    varsym,      procsym,
    lbrack,      rbrack    // ÎªÊý×éÓï·¨Ìí¼Ó '[' ºÍ ']'
};
#define symnum 34  // Ôö¼ÓÒÔ°üº¬ lbrack ºÍ rbrack

/* ï¿½ï¿½ï¿½Ö±ï¿½ï¿½Ðµï¿½ï¿½ï¿½ï¿½ï¿½ */
enum object {
    constant,
    variable,
    procedur,
    array       // ÐÂÔö£ºÒ»Î¬Êý×é
};

/* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     inte,
    jmp,     jpc,
    lodi,    sti      // ÐÂÔö£ºÊý×éµÄ¼ä½Ó×°ÔØ/´æ´¢Ö¸Áî
};
#define fctnum 10  // Òò lodi ºÍ sti ¶øÔö¼Ó

/* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½á¹¹ */
struct instruction
{
    enum fct f; /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¸ï¿½ï¿? */
    int l;      /* ï¿½ï¿½ï¿½Ã²ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä²ï¿½Î²ï¿½ */
    int a;      /* ï¿½ï¿½ï¿½ï¿½fï¿½Ä²ï¿½Í¬ï¿½ï¿½ï¿½ï¿½Í¬ */
};

FILE* fas;  /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö±ï¿? */
FILE* fa;   /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
FILE* fa1;  /* ï¿½ï¿½ï¿½Ô´ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½Ó¦ï¿½ï¿½ï¿½×µï¿½Ö· */
FILE* fa2;  /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
bool listswitch;    /* ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
bool tableswitch;   /* ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½Ö±ï¿½ï¿½ï¿½ï¿? */
char ch;            /* ï¿½ï¿½È¡ï¿½Ö·ï¿½ï¿½Ä»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½getch Ê¹ï¿½ï¿½ */
enum symbol sym;    /* ï¿½ï¿½Ç°ï¿½Ä·ï¿½ï¿½ï¿½ */
char id[al+1];      /* ï¿½ï¿½Ç°ident, ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½Ö½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½ï¿½0 */
int num;            /* ï¿½ï¿½Ç°number */
int cc, ll;          /* getchÊ¹ï¿½ÃµÄ¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ccï¿½ï¿½Ê¾ï¿½ï¿½Ç°ï¿½Ö·ï¿½(ch)ï¿½ï¿½Î»ï¿½ï¿½ */
int cx;             /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¸ï¿½ï¿?, È¡Öµï¿½ï¿½Î§[0, cxmax-1]*/
char line[81];      /* ï¿½ï¿½È¡ï¿½Ð»ï¿½ï¿½ï¿½ï¿½ï¿½ */
char a[al+1];       /* ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½, ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½Ö½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½ï¿½0 */
struct instruction code[cxmax]; /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */
char word[norw][al];        /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
enum symbol wsym[norw];     /* ï¿½ï¿½ï¿½ï¿½ï¿½Ö¶ï¿½Ó¦ï¿½Ä·ï¿½ï¿½ï¿½Öµ */
enum symbol ssym[256];      /* ï¿½ï¿½ï¿½Ö·ï¿½ï¿½Ä·ï¿½ï¿½ï¿½Öµ */
char mnemonic[fctnum][5];   /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */
bool declbegsys[symnum];    /* ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê¼ï¿½Ä·ï¿½ï¿½Å¼ï¿½ï¿½ï¿½ */
bool statbegsys[symnum];    /* ï¿½ï¿½Ê¾ï¿½ï¿½ä¿ªÊ¼ï¿½Ä·ï¿½ï¿½Å¼ï¿½ï¿½ï¿? */
bool facbegsys[symnum];     /* ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½Ó¿ï¿½Ê¼ï¿½Ä·ï¿½ï¿½Å¼ï¿½ï¿½ï¿½ */

/* ï¿½ï¿½ï¿½Ö±ï¿½ï¿½á¹¹ */
struct tablestruct
{
    char name[al];      /* ï¿½ï¿½ï¿½ï¿½ */
    enum object kind;   /* ï¿½ï¿½ï¿½Í£ï¿½const, var, array or procedure */
    int val;            /* ï¿½ï¿½Öµï¿½ï¿½ï¿½ï¿½constÊ¹ï¿½ï¿½ */
    int level;          /* ï¿½ï¿½ï¿½ï¿½ï¿½ã£¬ï¿½ï¿½constï¿½ï¿½Ê¹ï¿½ï¿½ */
    int adr;            /* ï¿½ï¿½Ö·ï¿½ï¿½ï¿½ï¿½constï¿½ï¿½Ê¹ï¿½ï¿½ */
    int size;           /* ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Õ¼ï¿?, ï¿½ï¿½procedureÊ¹ï¿½ï¿½ */
};

struct tablestruct table[txmax]; /* ï¿½ï¿½ï¿½Ö±ï¿½ */

FILE* fin;
FILE* fout;
char fname[al];
int err; /* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */

/* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð»á·¢ï¿½ï¿½fatal errorÊ±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½-1ï¿½ï¿½Öªï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Äºï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë³ï¿½ï¿½ï¿½ï¿½ï¿½ */
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
