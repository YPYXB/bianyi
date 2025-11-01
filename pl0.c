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
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['['] = lbrack;    /* 已添加：'[' 符号 */
	ssym[']'] = rbrack;    /* 已添加：']' 符号 */

	/* 设置保留字名字,按照字母顺序，便于折半查找 */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "else");    /* 新增 else */
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
	wsym[4] = elsesym;    /* 新增 else */
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
	strcpy(&(mnemonic[lodi][0]), "lodi");  /* 已添加：间接装载指令助记符 */
	strcpy(&(mnemonic[sti][0]), "sti");    /* 已添加：间接存储指令助记符 */

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

	/* 设置因子开始符号集 */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
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
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
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
			//fscanf(fin,"%c", &ch)
			//richard
			if (EOF == fscanf(fin,"%c", &ch))
			{
				line[ll] = 0;
				break;
			}
			//end richard
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
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;
		strcpy(id, a);
		i = 0;
		j = norw-1;
		do {    /* 搜索当前符号是否为保留字 */
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
			sym = ident; /* 搜索失败则，是名字或数字 */
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{           /* 检测是否为数字：以0..9开头 */
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9'); /* 获取数字的值 */
			k--;
			if (k > nmax)
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')      /* 检测赋值符号 */
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;  /* 不能识别的符号 */
				}
			}
			else
			{
				if (ch == '<')      /* 检测小于或小于等于符号 */
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
					if (ch=='>')        /* 检测大于或大于等于符号 */
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
						/* 新增：识别 ++ 和 -- */
						if (ch == '+')
						{
							getchdo;
							if (ch == '+')
							{
								sym = plusplus;
								getchdo;
							}
							else
							{
								sym = plus;
								/* ch 已指向下一字符，无需再 getchdo */
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
							else
							{
								sym = minus;
								/* ch 已指向下一字符，无需再 getchdo */
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
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* 程序过长 */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


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
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* 编译程序主函数
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   当前模块的后跟符号集合
*/
int block(int lev, int tx, bool* fsys)
{
	int i;

	int dx;                 /* 名字分配到的相对地址 */
	int tx0;                /* 保留初始tx */
	int cx0;                /* 保留初始cx */
	bool nxtlev[symnum];    /* 由于符号集合使用数组实现，传参时会传指针；为避免下级函数修改上级集合，复制一份传递给下级 */

	dx = 3;
	tx0 = tx;               /* 记录本层名字的初始位置 */
	table[tx].adr = cx;

	gendo(jmp, 0, 0);

	if (lev > levmax)
	{
		error(32);
	}

	do {

		if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
		{
			getsymdo;

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do { */
			constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
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
				error(5);   /*漏掉了逗号或者分号*/
			}
			/* } while (sym == ident); */
		}

		if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
		{
			getsymdo;

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do {  */
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
			/* } while (sym == ident);  */
		}

		while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
				getsymdo;
			}
			else
			{
				error(4);   /* procedure后应为标识符 */
			}

			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))
			{
				return -1;  /* 递归调用 */
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
				error(5);   /* 漏掉了分号 */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   /* 直到没有声明符号 */

	code[table[tx0].adr].a = cx;    /* 开始生成当前过程代码 */
	table[tx0].adr = cx;            /* 当前过程代码地址 */
	table[tx0].size = dx;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gendo(inte, 0, dx);             /* 生成分配内存代码 */

	if (tableswitch)        /* 输出名字表 */
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
			}
		}
		printf("\n");
	}

	/* 语句后跟符号为分号或end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);                       /* 每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
	testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
	listcode(cx0);                          /* 输出代码 */
	return 0;
}

/*
* 在名字表中加入一项
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:  /* 常量名字 */
		if (num > amax)
		{
			error(31);  /* 数越界 */
			num = 0;
		}
		table[(*ptx)].val = num;
		break;
	case variable:  /* 变量名字 */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:  /*　过程名字　*/
		table[(*ptx)].level = lev;
		break;
	case array:     /* 一维数组声明：num 作为数组大小 */
		if (num <= 0)
		{
			error(31); /* 非法数组大小 */
			num = 1;
		}
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx); /* 记录数组基址（相对地址） */
		table[(*ptx)].size = num;   /* 记录数组长度 */
		(*pdx) += num; /* 为数组分配连续的 num 个单元 */
		break;
	}
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
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
				error(1);   /* 把=写成了:= */
			}
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* 常量说明=后应是数字 */
			}
		}
		else
		{
			error(3);   /* 常量说明标识后应是= */
		}
	}
	else
	{
		error(4);   /* const后应是标识 */
	}
	return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	/* 修改：支持形如 ident '[' number ']' 的数组声明 */
	if (sym == ident)
	{
		char saved_id[al+1];
		strcpy(saved_id, id);
		getsymdo; /* 读取标识符后的下一个符号 */

		if (sym == lbrack) /* array declaration: ident '[' number ']' */
		{
			getsymdo;
			if (sym == number)
			{
				/* 把标识名恢复到全局 id，num 保持为数组大小，以供 enter 使用 */
				strcpy(id, saved_id);
				/* 调用 enter(array, ...) 使用当前全局 num 作为数组长度 */
				enter(array, ptx, lev, pdx);
				getsymdo; /* 消耗 number 后的符号（原代码逻辑） */
				if (sym == rbrack)
				{
					getsymdo; /* 消耗 ']' */
				}
				else
				{
					error(36); /* 缺少 ']' */
				}
			}
			else
			{
				error(31); /* 非法数组大小 */
			}
		}
		else
		{
			/* 普通变量声明 */
			strcpy(id, saved_id);
			enter(variable, ptx, lev, pdx);
		}
	}
	else
	{
		error(4);   /* var 后应是标识符 */
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

	/* 前缀 ++/-- 处理 */
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

			/* 后缀 ++/-- */
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

			/* 数组赋值 */
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

			/* 普通变量赋值 */
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
							gendo(jmp, 0, 0);     /* then 分支结束后跳过 else 分支 */
							code[cx1].a = cx;     /* jpc 跳转到 else 分支开始 */
							getsymdo;
							statementdo(fsys, ptx, lev);
							code[cx2].a = cx;     /* jmp 跳转到 else 分支结束后 */
						}
						else
						{
							code[cx1].a = cx;     /* 无 else，jpc 直接跳到 then 后 */
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
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* 程序过长 */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


void interpret()
{
	int p, b, t;    /* 指令指针，指令基址，栈顶指针 */
	struct instruction i;   /* 存放当前指令 */
	int s[stacksize];   /* 栈 */

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		i = code[p];    /* 读当前指令 */
		p++;
		switch (i.f)
		{
		case lit:   /* 将a的值取到栈顶 */
			s[t] = i.a;
			t++;
			break;
		case opr:   /* 数学、逻辑运算 */
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
			}
			break;
		case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			s[t] = s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case lodi: /* 间接装载: 弹出 offset，并把 s[base(l)+offset] 压入栈 */
			{
				int off;
				t--;
				off = s[t];
				s[t] = s[base(i.l, s, b) + off];
				t++;
			}
			break;
		case sti: /* 间接存储: 弹出 value 和 offset, 存入 s[base(l)+offset] = value */
			{
				int val, off;
				t--;
				val = s[t];   /* pop value */
				t--;
				off = s[t];   /* pop offset */
				s[base(i.l, s, b) + off] = val;
			}
			break;
		case cal:   /* 调用子过程 */
			s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
			s[t+1] = b; /* 将本过程基地址入栈，此两项用于base函数 */
			s[t+2] = p; /* 将当前指令指针入栈 */
			b = t;  /* 改变基地址指针值为新过程的基地址 */
			p = i.a;    /* 跳转 */
			break;
		case inte:  /* 分配内存 */
			t += i.a;
			break;
		case jmp:   /* 直接跳转 */
			p = i.a;
			break;
		case jpc:   /* 条件跳转 */
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

