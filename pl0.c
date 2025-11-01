/*
 * PL/0 编译程序（在 Win32 平台上用 C 实现）
 *
 * 本程序在 Visual C++ 6.0、Visual C++.NET 以及 Visual C++.NET 2003
 * 在 Win98、WinNT、Win2000、WinXP 和 Win2003 上测试通过。
 *
 * 使用方法：
 * 运行后输入 PL/0 源程序文件名
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp 输出虚拟机代码
 * fa1.tmp 输出源文件及其各行对应的首地址
 * fa2.tmp 输出解释执行的结果
 * fas.tmp 输出名字表
 */

#include <stdio.h>

#include "pl0.h"
#include "string.h"

/* 解释执行时使用的栈 */
#define stacksize 500


int main()
{
	bool nxtlev[symnum];

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* 输入文件名 */

	fin = fopen(fname, "r");

	if (fin)
	{
		printf("List object code?(Y/N)");   /* 是否输出虚拟机代码 */
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");  /* 是否输出名字表 */
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* 初始化 */

		err = 0;
		cc = cx = ll = 0;
		ch = ' ';

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))   /* 调用编译程序 */
			{
				fclose(fa);
				fclose(fa1);
				fclose(fas);
				fclose(fin);
				printf("\n");
				return 0;
			}
			fclose(fa);
			fclose(fa1);
			fclose(fas);

			if (sym != period)
			{
				error(9);
			}

			if (err == 0)
			{
				fa2 = fopen("fa2.tmp", "w");
				interpret();    /* 调用解释执行程序 */
				fclose(fa2);
			}
			else
			{
				printf("Errors in pl/0 program");
			}
		}

		fclose(fin);
	}
	else
	{
		printf("Can't open file!\n");
	}

	printf("\n");
	return 0;
}

/*
* 初始化
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i=0; i<=255; i++)
	{
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['%'] = modsym;
	ssym['!'] = notsym;
	ssym['^'] = powsym;    /* 新增：'^' 符号 */
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['['] = lbrack;
	ssym[']'] = rbrack;

	/* 设置保留字名字,按照字母顺序，便于折半查找 */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "else");
	strcpy(&(word[5][0]), "end");
	strcpy(&(word[6][0]), "if");
	strcpy(&(word[7][0]), "odd");
	strcpy(&(word[8][0]), "procedure");
	strcpy(&(word[9][0]), "read");
	strcpy(&(word[10][0]), "then");
	strcpy(&(word[11][0]), "var");
	strcpy(&(word[12][0]), "while");
	strcpy(&(word[13][0]), "write");

	/* 设置保留字符号 */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
	wsym[4] = elsesym;
	wsym[5] = endsym;
	wsym[6] = ifsym;
	wsym[7] = oddsym;
	wsym[8] = procsym;
	wsym[9] = readsym;
	wsym[10] = thensym;
	wsym[11] = varsym;
	wsym[12] = whilesym;
	wsym[13] = writesym;

	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[lodi][0]), "lodi");
	strcpy(&(mnemonic[sti][0]), "sti");

	/* 设置符号集 */
	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* 设置声明开始符号集 */
	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;

	/* 设置语句开始符号集 */
	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[ident] = true;
	statbegsys[plusplus] = true;
	statbegsys[minusminus] = true;

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
	facbegsys[notsym] = true;
	facbegsys[powsym] = false; // ^ is not a prefix operator
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]||s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&(!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&s2[i];
	}
	return 0;
}

/*
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
	char space[81];
	memset(space,32,81);

	space[cc-1]=0; //出错时当前符号已经读完，所以cc-1

	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);

	err++;
}

/*
* 漏掉空格，读取一个字符。
*/
int getch()
{
	if (cc == ll)
	{
		if (feof(fin))
		{
			printf("program incomplete");
			return -1;
		}
		ll=0;
		cc=0;
		printf("%d ", cx);
		fprintf(fa1,"%d ", cx);
		ch = ' ';
		while (ch != 10)
		{
			if (EOF == fscanf(fin,"%c", &ch))
			{
				line[ll] = 0;
				break;
			}
			printf("%c", ch);
			fprintf(fa1, "%c", ch);
			line[ll] = ch;
			ll++;
		}
		printf("\n");
		fprintf(fa1, "\n");
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* 词法分析，获取一个符号
*/
int getsym()
{
	int i,j,k;

	while (ch==' ' || ch==10 || ch==13 || ch==9)
	{
		getchdo;
	}
	if (ch>='a' && ch<='z')
	{           /* 名字或保留字以a..z开头 */
		k = 0;
		do {
			if(k<al)
			{
				a[k] = ch;
				k++;
			}
			getchdo;
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9' || ch == '_');
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw-1;
		do {
			k = (i+j)/2;
			if (strcmp(id,word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id,word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i-1 > j)
		{
			sym = wsym[k];
		}
		else
		{
			sym = ident;
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9');
			k--;
			if (k > nmax)
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;
				}
			}
			else
			{
				if (ch == '<')
				{
					getchdo;
					if (ch == '=')
					{
						sym = leq;
						getchdo;
					}
					else
					{
						sym = lss;
					}
				}
				else
				{
					if (ch=='>')
					{
						getchdo;
						if (ch == '=')
						{
							sym = geq;
							getchdo;
						}
						else
						{
							sym = gtr;
						}
					}
					else
					{
						if (ch == '+')
						{
							getchdo;
							if (ch == '+')
							{
								sym = plusplus;
								getchdo;
							}
							else if (ch == '=')
							{
								sym = plusequal;
								getchdo;
							}
							else
							{
								sym = plus;
							}
						}
						else if (ch == '-')
						{
							getchdo;
							if (ch == '-')
							{
								sym = minusminus;
								getchdo;
							}
							else if (ch == '=')
							{
								sym = minusequal;
								getchdo;
							}
							else
							{
								sym = minus;
							}
						}
						else if (ch == '*')
						{
							getchdo;
							if (ch == '=')
							{
								sym = timesequal;
								getchdo;
							}
							else
							{
								sym = times;
							}
						}
						else if (ch == '/')
						{
							getchdo;
							if (ch == '=')
							{
								sym = slashequal;
								getchdo;
							}
							else
							{
								sym = slash;
							}
						}
						else
						{
							sym = ssym[ch];
							if (sym != period)
							{
								getchdo;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* 生成虚拟机代码
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long");
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}

/*
* 测试当前符号是否合法
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* 编译程序主函数
*/
int block(int lev, int tx, bool* fsys)
{
	int i;
	int dx;
	int tx0;
	int cx0;
	bool nxtlev[symnum];

	dx = 3;
	tx0 = tx;
	table[tx].adr = cx;
	gendo(jmp, 0, 0);

	if (lev > levmax)
	{
		error(32);
	}

	do {
		if (sym == constsym)
		{
			getsymdo;
			constdeclarationdo(&tx, lev, &dx);
			while (sym == comma)
			{
				getsymdo;
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
		}

		if (sym == varsym)
		{
			getsymdo;
			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma)
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
		}

		while (sym == procsym)
		{
			getsymdo;
			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx);
				getsymdo;
			}
			else
			{
				error(4);
			}

			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))
			{
				return -1;
			}

			if(sym == semicolon)
			{
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;
				nxtlev[procsym] = true;
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));

	code[table[tx0].adr].a = cx;
	table[tx0].adr = cx;
	table[tx0].size = dx;
	cx0 = cx;
	gendo(inte, 0, dx);

	if (tableswitch)
	{
		printf("TABLE:\n");
		if (tx0+1 > tx)
		{
			printf("    NULL\n");
		}
		for (i=tx0+1; i<=tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(fas, "    %d const %s ", i, table[i].name);
				fprintf(fas, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(fas, "    %d var   %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedur:
				printf("    %d proc  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d proc  %s ", i, table[i].name);
				fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			case array:
				printf("    %d array %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d array %s ", i, table[i].name);
				fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
	}

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);
	memset(nxtlev, 0, sizeof(bool)*symnum);
	testdo(fsys, nxtlev, 8);
	listcode(cx0);
	return 0;
}

/*
* 在名字表中加入一项
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id);
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:
		if (num > amax)
		{
			error(31);
			num = 0;
		}
		table[(*ptx)].val = num;
		break;
	case variable:
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:
		table[(*ptx)].level = lev;
		break;
	case array:
		if (num <= 0)
		{
			error(31);
			num = 1;
		}
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		table[(*ptx)].size = num;
		(*pdx) += num;
		break;
	}
}

/*
* 查找名字的位置
*/
int position(char* idt, int tx)
{
	int i;
	strcpy(table[0].name, idt);
	i = tx;
	while (strcmp(table[i].name, idt) != 0)
	{
		i--;
	}
	return i;
}

/*
* 常量声明处理
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		getsymdo;
		if (sym==eql || sym==becomes)
		{
			if (sym == becomes)
			{
				error(1);
			}
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);
			}
		}
		else
		{
			error(3);
		}
	}
	else
	{
		error(4);
	}
	return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)
	{
		char saved_id[al+1];
		strcpy(saved_id, id);
		getsymdo;

		if (sym == lbrack)
		{
			getsymdo;
			if (sym == number)
			{
				strcpy(id, saved_id);
				enter(array, ptx, lev, pdx);
				getsymdo;
				if (sym == rbrack)
				{
					getsymdo;
				}
				else
				{
					error(36);
				}
			}
			else
			{
				error(31);
			}
		}
		else
		{
			strcpy(id, saved_id);
			enter(variable, ptx, lev, pdx);
		}
	}
	else
	{
		error(4);
	}
	return 0;
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		for (i=cx0; i<cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == plusplus || sym == minusminus)
	{
		enum symbol op = sym;
		getsymdo;
		if (sym == ident)
		{
			i = position(id, *ptx);
			if (i == 0)
			{
				error(11);
			}
			else if (table[i].kind != variable)
			{
				error(12);
			}
			else
			{
				gendo(lod, lev - table[i].level, table[i].adr);
				gendo(lit, 0, 1);
				if (op == plusplus)
					gendo(opr, 0, 2);
				else
					gendo(opr, 0, 3);
				gendo(sto, lev - table[i].level, table[i].adr);
			}
			getsymdo;
		}
		else
		{
			error(4);
		}
		return 0;
	}

	if (sym == ident)
	{
		i = position(id, *ptx);
		if (i == 0)
		{
			error(11);
		}
		else
		{
			getsymdo;

			if (sym == plusplus || sym == minusminus)
			{
				if (table[i].kind != variable)
				{
					error(12);
				}
				else
				{
					gendo(lod, lev - table[i].level, table[i].adr);
					gendo(lit, 0, 1);
					if (sym == plusplus)
						gendo(opr, 0, 2);
					else
						gendo(opr, 0, 3);
					gendo(sto, lev - table[i].level, table[i].adr);
				}
				getsymdo;
				return 0;
			}

			if (sym == plusequal || sym == minusequal || sym == timesequal || sym == slashequal)
			{
				if (table[i].kind != variable)
				{
					error(12);
				}
				else
				{
					enum symbol op = sym;
					getsymdo;
					gendo(lod, lev - table[i].level, table[i].adr);
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					expressiondo(nxtlev, ptx, lev);
					
					if (op == plusequal)
						gendo(opr, 0, 2);
					else if (op == minusequal)
						gendo(opr, 0, 3);
					else if (op == timesequal)
						gendo(opr, 0, 4);
					else if (op == slashequal)
						gendo(opr, 0, 5);
					
					gendo(sto, lev - table[i].level, table[i].adr);
				}
				return 0;
			}

			if (table[i].kind == array && sym == lbrack)
			{
				getsymdo;
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				nxtlev[rbrack] = true;
				expressiondo(nxtlev, ptx, lev);

				if (sym == rbrack)
				{
					getsymdo;
				}
				else
				{
					error(22);
				}

				if (sym == becomes)
				{
					getsymdo;
					gendo(lit, 0, table[i].adr);
					gendo(opr, 0, 2);
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					expressiondo(nxtlev, ptx, lev);
					gendo(sti, lev - table[i].level, 0);
				}
				else
				{
					error(13);
				}
				return 0;
			}

			if (table[i].kind == variable)
			{
				if(sym == becomes)
				{
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					expressiondo(nxtlev, ptx, lev);
					gendo(sto, lev-table[i].level, table[i].adr);
				}
				else
				{
					error(13);
				}
				return 0;
			}

			error(12);
			return 0;
		}
	}
	else
	{
		if (sym == readsym)
		{
			getsymdo;
			if (sym != lparen)
			{
				error(34);
			}
			else
			{
				do {
					getsymdo;
					if (sym == ident)
					{
						i = position(id, *ptx);
					}
					else
					{
						i=0;
					}

					if (i == 0)
					{
						error(35);
					}
					else if (table[i].kind != variable)
					{
						error(32);
					}
					else
					{
						gendo(opr, 0, 16);
						gendo(sto, lev-table[i].level, table[i].adr);
					}
					getsymdo;

				} while (sym == comma);
			}
			if(sym != rparen)
			{
				error(33);
				while (!inset(sym, fsys))
				{
					getsymdo;
				}
			}
			else
			{
				getsymdo;
			}
		}
		else
		{
			if (sym == writesym)
			{
				getsymdo;
				if (sym == lparen)
				{
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[rparen] = true;
						nxtlev[comma] = true;
						expressiondo(nxtlev, ptx, lev);
						gendo(opr, 0, 14);
					} while (sym == comma);
					if (sym != rparen)
					{
						error(33);
					}
					else
					{
						getsymdo;
					}
				}
				gendo(opr, 0, 15);
			}
			else
			{
				if (sym == callsym)
				{
					getsymdo;
					if (sym != ident)
					{
						error(14);
					}
					else
					{
						i = position(id, *ptx);
						if (i == 0)
						{
							error(11);
						}
						else
						{
							if (table[i].kind == procedur)
							{
								gendo(cal, lev-table[i].level, table[i].adr);
							}
							else
							{
								error(15);
							}
						}
						getsymdo;
					}
				}
				else
				{
					if (sym == ifsym)
					{
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;
						conditiondo(nxtlev, ptx, lev);
						if (sym == thensym)
						{
							getsymdo;
						}
						else
						{
							error(16);
						}
						cx1 = cx;
						gendo(jpc, 0, 0);
						statementdo(fsys, ptx, lev);
						
						if (sym == elsesym)
						{
							cx2 = cx;
							gendo(jmp, 0, 0);
							code[cx1].a = cx;
							getsymdo;
							statementdo(fsys, ptx, lev);
							code[cx2].a = cx;
						}
						else
						{
							code[cx1].a = cx;
						}
					}
					else
					{
						if (sym == beginsym)
						{
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)*symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;
							statementdo(nxtlev, ptx, lev);

							while (inset(sym, statbegsys) || sym==semicolon)
							{
								if (sym == semicolon)
								{
									getsymdo;
								}
								else
								{
									error(10);
								}
								statementdo(nxtlev, ptx, lev);
							}
							if(sym == endsym)
							{
								getsymdo;
							}
							else
							{
								error(17);
							}
						}
						else
						{
							if (sym == whilesym)
							{
								cx1 = cx;
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								nxtlev[dosym] = true;
								conditiondo(nxtlev, ptx, lev);
								cx2 = cx;
								gendo(jpc, 0, 0);
								if (sym == dosym)
								{
									getsymdo;
								}
								else
								{
									error(18);
								}
								statementdo(fsys, ptx, lev);
								gendo(jmp, 0, cx1);
								code[cx2].a = cx;
							}
							else
							{
								memset(nxtlev, 0, sizeof(bool)*symnum);
								testdo(fsys, nxtlev, 19);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;
	bool nxtlev[symnum];

	if(sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);
		if (addop == minus)
		{
			gendo(opr,0,1);
		}
	}
	else
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);
		if (addop == plus)
		{
			gendo(opr, 0, 2);
		}
		else
		{
			gendo(opr, 0, 3);
		}
	}
	return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	nxtlev[modsym] = true;
	power_expressiondo(nxtlev, ptx, lev);
	while(sym==times || sym==slash || sym==modsym)
	{
		mulop = sym;
		getchdo;
		power_expressiondo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4);
		}
		else if(mulop == slash)
		{
			gendo(opr, 0, 5);
		}
		else if(mulop == modsym)
		{
			gendo(opr, 0, 7);
		}
	}
	return 0;
}

/*
* 幂运算处理 (右结合)
*/
int power_expression(bool* fsys, int* ptx, int lev)
{
	bool nxtlev[symnum];
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[powsym] = true;
	factordo(nxtlev, ptx, lev);
	if (sym == powsym)
	{
		getchdo;
		power_expressiondo(nxtlev, ptx, lev);
		gendo(opr, 0, 18); /* 生成幂运算指令 */
	}
	return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	
	if (sym == notsym)
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		factordo(nxtlev, ptx, lev);
		gendo(opr, 0, 17);
		return 0;
	}
	
	testdo(facbegsys, fsys, 24);
	if(inset(sym,facbegsys))
	{
		if(sym == ident)
		{
			i = position(id, *ptx);
			if (i == 0)
			{
				error(11);
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:
					gendo(lit, 0, table[i].val);
					getsymdo;
					break;
				case variable:
					gendo(lod, lev-table[i].level, table[i].adr);
					getsymdo;
					break;
				case array:
					getsymdo;
					if (sym == lbrack)
					{
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[rbrack] = true;
						expressiondo(nxtlev, ptx, lev);
						if (sym == rbrack)
						{
							getsymdo;
						}
						else
						{
							error(22);
						}
						gendo(lit, 0, table[i].adr);
						gendo(opr, 0, 2);
						gendo(lodi, lev - table[i].level, 0);
					}
					else
					{
						error(21);
					}
					break;
				case procedur:
					error(21);
					getsymdo;
					break;
				}
			}
		}
		else
		{
			if(sym == number)
			{
				if (num > amax)
				{
					error(31);
					num = 0;
				}
				gendo(lit, 0, num);
				getsymdo;
			}
			else
			{
				if (sym == lparen)
				{
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					nxtlev[rparen] = true;
					expressiondo(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsymdo;
					}
					else
					{
						error(22);
					}
				}
				testdo(fsys, facbegsys, 23);
			}
		}
	}
	return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);
	}
	else
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)
			{
			case eql:
				gendo(opr, 0, 8);
				break;
			case neq:
				gendo(opr, 0, 9);
				break;
			case lss:
				gendo(opr, 0, 10);
				break;
			case geq:
				gendo(opr, 0, 11);
				break;
			case gtr:
				gendo(opr, 0, 12);
				break;
			case leq:
				gendo(opr, 0, 13);
				break;
			}
		}
	}
	return 0;
}

void interpret()
{
	int p, b, t;
	struct instruction i;
	int s[stacksize];

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		i = code[p];
		p++;
		switch (i.f)
		{
		case lit:
			s[t] = i.a;
			t++;
			break;
		case opr:
			switch (i.a)
			{
			case 0:
				t = b;
				p = s[t+2];
				b = s[t+1];
				break;
			case 1:
				s[t-1] = -s[t-1];
				break;
			case 2:
				t--;
				s[t-1] = s[t-1]+s[t];
				break;
			case 3:
				t--;
				s[t-1] = s[t-1]-s[t];
				break;
			case 4:
				t--;
				s[t-1] = s[t-1]*s[t];
				break;
			case 5:
				t--;
				s[t-1] = s[t-1]/s[t];
				break;
			case 6:
				s[t-1] = s[t-1]%2;
				break;
			case 7:
				t--;
				s[t-1] = s[t-1] % s[t];
				break;
			case 8:
				t--;
				s[t-1] = (s[t-1] == s[t]);
				break;
			case 9:
				t--;
				s[t-1] = (s[t-1] != s[t]);
				break;
			case 10:
				t--;
				s[t-1] = (s[t-1] < s[t]);
				break;
			case 11:
				t--;
				s[t-1] = (s[t-1] >= s[t]);
				break;
			case 12:
				t--;
				s[t-1] = (s[t-1] > s[t]);
				break;
			case 13:
				t--;
				s[t-1] = (s[t-1] <= s[t]);
				break;
			case 14:
				printf("%d", s[t-1]);
				fprintf(fa2, "%d", s[t-1]);
				t--;
				break;
			case 15:
				printf("\n");
				fprintf(fa2,"\n");
				break;
			case 16:
				printf("?");
				fprintf(fa2, "?");
				scanf("%d", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			case 17:
				s[t-1] = !s[t-1];
				break;
			case 18: /* 幂运算 */
				{
					int exponent = s[t-1];
					int base_val = s[t-2];
					int result = 1;
					if (exponent < 0) { // 不支持负指数
						result = 0;
					} else {
						for (int k = 0; k < exponent; k++) {
							result *= base_val;
						}
					}
					s[t-2] = result;
					t--;
				}
				break;
			}
			break;
		case lod:
			s[t] = s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case lodi:
			{
				int off;
				t--;
				off = s[t];
				s[t] = s[base(i.l, s, b) + off];
				t++;
			}
			break;
		case sti:
			{
				int val, off;
				t--;
				val = s[t];
				t--;
				off = s[t];
				s[base(i.l, s, b) + off] = val;
			}
			break;
		case cal:
			s[t] = base(i.l, s, b);
			s[t+1] = b;
			s[t+2] = p;
			b = t;
			p = i.a;
			break;
		case inte:
			t += i.a;
			break;
		case jmp:
			p = i.a;
			break;
		case jpc:
			t--;
			if (s[t] == 0)
			{
				p = i.a;
			}
			break;
		}
	} while (p != 0);
}

/* 通过过程基址求上 l 层过程的基址 */
int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}

