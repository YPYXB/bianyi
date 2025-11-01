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
#include <string.h>
#include <stdlib.h>

#include "pl0.h"

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
		printf("List object code?(Y/N)");
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();

		err = 0;
		cc = cx = ll = 0;
		linenum = 0;
		ch = ' ';

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))
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
				interpret();
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

	for (i=0; i<=255; i++) ssym[i] = nul;
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['%'] = modsym;
	ssym['!'] = notsym;
	ssym['^'] = powsym;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['['] = lbrack;
	ssym[']'] = rbrack;

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
	strcpy(&(mnemonic[dup][0]), "dup");  // 添加dup指令助记符

	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;

	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;
	statbegsys[ident] = true;
	statbegsys[plusplus] = true;
	statbegsys[minusminus] = true;

	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
	facbegsys[notsym] = true;
	facbegsys[powsym] = false; // ^ is not a prefix operator
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s) { return s[e]; }

int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++) sr[i] = s1[i]||s2[i];
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++) sr[i] = s1[i]&&(!s2[i]);
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++) sr[i] = s1[i]&&s2[i];
	return 0;
}

/*
* 出错处理
*/
void error(int n)
{
	char space[81];
	memset(space,32,81);

	if (cc>0) space[cc-1]=0;
	else space[0]=0;

	printf("Line %d ****%s!%d\n", linenum, space, n);
	fprintf(fa1,"Line %d ****%s!%d\n", linenum, space, n);

	err++;
}

/*
* 读取字符（按行），支持行内 '\' 注释截断
* 新增：去除 UTF-8 BOM，并过滤非 ASCII 可打印字符，避免中文注释等干扰词法分析
* 新增：移除大括号注释 { ... }，支持注释跨行并保留注释后续的代码
*/
int getch()
{
	if (cc == ll)
	{
		while (1)
		{
			if (fgets(line, sizeof(line), fin) == NULL) return -1;

			linenum++;
			printf("%d ", linenum);
			fprintf(fa1, "%d ", linenum);

			/* 如果行以 UTF-8 BOM 开头，跳过 BOM */
			if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
			{
				size_t l = strlen(line+3);
				memmove(line, line+3, l+1);
			}

			/* 逐字符扫描，移除大括号注释 { ... }（支持跨行）并保留注释后续内容 */
			{
				char buf[sizeof(line)];
				int src = 0, dst = 0;
				bool in_comment = false;
				while (1)
				{
					/* 如果当前行结束且我们在注释中，尝试读取下一行直到注释结束或 EOF */
					if (line[src] == '\0')
					{
						if (in_comment)
						{
							/* 读取并丢弃中间注释行（不输出它们），但还需要记录行号 */
							if (fgets(line, sizeof(line), fin) == NULL)
							{
								/* EOF reached while in comment -> treat as end */
								break;
							}
							linenum++;
							/* 去掉可能的 BOM（只在首行可能出现） */
							if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
							{
								size_t l = strlen(line+3);
								memmove(line, line+3, l+1);
							}
							src = 0;
							continue;
						}
						else
						{
							/* 本行处理完毕 */
							buf[dst] = '\0';
							break;
						}
					}

					char c = line[src++];
					if (!in_comment)
					{
						if (c == '{')
						{
							in_comment = true;
						}
						else
						{
							buf[dst++] = c;
						}
					}
					else
					{
						if (c == '}')
						{
							in_comment = false;
						}
						/* 否则仍在注释中，跳过字符 */
					}
				}
				/* 将处理后的内容拷回 line 供后续过滤与输出 */
				strncpy(line, buf, sizeof(line)-1);
				line[sizeof(line)-1] = '\0';
			}

			/* 查找行中第一个 '\'（注释起始），如果存在则截断该位置 */
			{
				char* p = strchr(line, '\\');
				if (p) *p = '\0';
			}

			/* 过滤掉非 ASCII 可打印字符（保留 0x20-0x7E、换行和回车与制表） */
			{
				int src = 0, dst = 0;
				while (line[src] != '\0')
				{
					unsigned char c = (unsigned char)line[src];
					if (c == '\n' || c == '\r' || c == '\t' || (c >= 0x20 && c <= 0x7E))
					{
						line[dst++] = line[src];
					}
					/* 否则跳过该字符（去除中文等非 ASCII 字符） */
					src++;
				}
				line[dst] = '\0';
			}

			/* 检查截断/过滤后是否整行为空白 */
			{
				int only_space = 1;
				for (int i=0; line[i] != '\0'; i++)
				{
					if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n')
					{
						only_space = 0;
						break;
					}
				}
				if (only_space)
				{
					printf("\n");
					fprintf(fa1, "\n");
					continue;
				}
			}

			/* 输出并设置缓冲（现在行中只含 ASCII 可打印字符与换行） */
			fputs(line, stdout);
			fputs(line, fa1);
			ll = (int)strlen(line);
			cc = 0;
			break;
		}
	}

	ch = line[cc];
	cc++;
	return 0;
}

/*
* 词法分析
* 改进：标识符字符统一转换为小写存储，增加容错性
*/
int getsym()
{
	int i,j,k;

	while (ch==' ' || ch==10 || ch==13 || ch==9)
	{
		getchdo;
	}
	if ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'))
	{
		k = 0;
		do {
			char cc = ch;
			/* 统一小写化 */
			if (cc >= 'A' && cc <= 'Z') cc = (char)(cc - 'A' + 'a');
			if (k < al) a[k++] = cc;
			getchdo;
		} while ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9') || ch=='_');
		a[k] = 0;
		strcpy(id, a);

		/* 使用标准二分查找精准匹配保留字（保留字表已按字典序排列） */
		i = 0; j = norw-1;
		while (i <= j)
		{
			int mid = (i + j) / 2;
			int cmp = strcmp(id, word[mid]);
			if (cmp == 0)
			{
				sym = wsym[mid];
				return 0;
			}
			else if (cmp < 0)
			{
				j = mid - 1;
			}
			else
			{
				i = mid + 1;
			}
		}
		/* 未匹配到保留字，即为标识符 */
		sym = ident;
	}
	else if (ch>='0' && ch<='9')
	{
		k = 0;
		num = 0;
		sym = number;
		do {
			num = 10*num + ch - '0';
			k++;
			getchdo;
		} while (ch>='0' && ch<='9');
		/* k 是数字位数，直接比较，不做额外减量 */
		if (k > nmax) error(30);
	}
	else
	{
		if (ch == ':')
		{
			getchdo;
			if (ch == '=') { sym = becomes; getchdo; }
			else sym = nul;
		}
		else if (ch == '<')
		{
			getchdo;
			if (ch == '=') { sym = leq; getchdo; }
			else sym = lss;
		}
		else if (ch == '>')
		{
			getchdo;
			if (ch == '=') { sym = geq; getchdo; }
			else sym = gtr;
		}
		else if (ch == '+')
		{
			getchdo;
			if (ch == '+') { sym = plusplus; getchdo; }
			else if (ch == '=') { sym = plusequal; getchdo; }
			else sym = plus;
		}
		else if (ch == '-')
		{
			getchdo;
			if (ch == '-') { sym = minusminus; getchdo; }
			else if (ch == '=') { sym = minusequal; getchdo; }
			else sym = minus;
		}
		else if (ch == '*')
		{
			getchdo;
			if (ch == '=') { sym = timesequal; getchdo; }
			else sym = times;
		}
		else if (ch == '/')
		{
			getchdo;
			if (ch == '=') { sym = slashequal; getchdo; }
			else sym = slash;
		}
		else
		{
			sym = ssym[(unsigned char)ch];
			if (sym != period) getchdo;
		}
	}
	return 0;
}

/*
* 生成代码
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax) { printf("Program too long"); return -1; }
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}

/*
* 测试集合
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		while ((!inset(sym,s1)) && (!inset(sym,s2))) getsymdo;
	}
	return 0;
}

/*
* block
*/
int block(int lev, int tx, bool* fsys)
{
	int i, dx, tx0, cx0;
	bool nxtlev[symnum];

	dx = 3;
	tx0 = tx;
	table[tx].adr = cx;
	gendo(jmp, 0, 0);

	if (lev > levmax) error(32);

	do {
		if (sym == constsym)
		{
			getsymdo;
			constdeclarationdo(&tx, lev, &dx);
			while (sym == comma) { getsymdo; constdeclarationdo(&tx, lev, &dx); }
			/* 原：if (sym == semicolon) getsymdo; else error(5); */
			if (sym == semicolon) {
				getsymdo;
			} else {
				/* 若后续已进入语句/块起始或过程/结束/程序结尾，则宽容缺失的分号 */
				if (!inset(sym, statbegsys) && sym != procsym && sym != endsym && sym != period) {
					error(5);
				}
				/* 不消费符号，交由后续解析 */
			}
		}
		if (sym == varsym)
		{
			getsymdo;
			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma) { getsymdo; vardeclarationdo(&tx, lev, &dx); }
			/* 原：if (sym == semicolon) getsymdo; else error(5); */
			if (sym == semicolon) {
				getsymdo;
			} else {
				/* 若后续已进入语句/块起始或过程/结束/程序结尾，则宽容缺失的分号 */
				if (!inset(sym, statbegsys) && sym != procsym && sym != endsym && sym != period) {
					error(5);
				}
				/* 不消费符号，交由后续解析 */
			}
		}
		while (sym == procsym)
		{
			getsymdo;
			if (sym == ident) { enter(procedur, &tx, lev, &dx); getsymdo; }
			else error(4);
			if (sym == semicolon) getsymdo; else error(5);
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev)) return -1;
			if (sym == semicolon) { getsymdo; memcpy(nxtlev, statbegsys, sizeof(bool)*symnum); nxtlev[ident] = true; nxtlev[procsym] = true; testdo(nxtlev, fsys, 6); }
			else error(5);
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
		if (tx0+1 > tx) printf("    NULL\n");
		for (i=tx0+1; i<=tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s val=%d\n", i, table[i].name, table[i].val);
				fprintf(fas, "    %d const %s val=%d\n", i, table[i].name, table[i].val);
				break;
			case variable:
				printf("    %d var   %s lev=%d addr=%d\n", i, table[i].name, table[i].level, table[i].adr);
				fprintf(fas, "    %d var   %s lev=%d addr=%d\n", i, table[i].name, table[i].level, table[i].adr);
				break;
			case procedur:
				printf("    %d proc  %s lev=%d addr=%d size=%d\n", i, table[i].name, table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d proc  %s lev=%d addr=%d size=%d\n", i, table[i].name, table[i].level, table[i].adr, table[i].size);
				break;
			case array:
				printf("    %d array %s lev=%d addr=%d size=%d\n", i, table[i].name, table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d array %s lev=%d addr=%d size=%d\n", i, table[i].name, table[i].level, table[i].adr, table[i].size);
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
		if (num > amax) { error(31); num = 0; }
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
		if (num <= 0) { error(31); num = 1; }
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
	while (strcmp(table[i].name, idt) != 0) i--;
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
			if (sym == becomes) error(1);
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else error(2);
		}
		else error(3);
	}
	else error(4);
	return 0;
}

/*
* 变量声明处理（增强错误恢复）
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
			/* 数组声明 */
			getsymdo;

			if (sym == number)
			{
				int size = num;
				strcpy(id, saved_id);
				num = size;
				enter(array, ptx, lev, pdx);

				getsymdo;
				if (sym != rbrack)
				{
					error(36);
					/* 错误恢复：跳到 ']', ',', 或 ';' */
					while (sym != rbrack && sym != semicolon && sym != comma && sym != period) getsymdo;
					if (sym == rbrack) getsymdo;  /* 如果找到 ']'，推进 */
				}
				else
				{
					/* 成功：推进到逗号或分号 */
					getsymdo;
				}
			}
			else
			{
				error(31);
				/* 错误恢复 */
				while (sym != rbrack && sym != semicolon && sym != comma && sym != period) getsymdo;
				if (sym == rbrack) getsymdo;
			}
		}
		else
		{
			/* 普通变量：sym 已是逗号或分号 */
			strcpy(id, saved_id);
			enter(variable, ptx, lev, pdx);
		}
	}
	else 
	{
		error(4);
		/* 错误恢复：如果不是标识符，跳到逗号或分号 */
		while (sym != semicolon && sym != comma && sym != period) getsymdo;
	}
	return 0;
}

/*
* 列出代码
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
		// 前缀自增/自减
		enum symbol op = sym;
		getsymdo;
		if (sym == ident)
		{
			i = position(id, *ptx);
			if (i == 0) error(11);
			else if (table[i].kind != variable) error(12);
			else
			{
				int levdiff = lev - table[i].level;
				int addr = table[i].adr;
				
				gendo(lod, levdiff, addr);          // 加载原值
				gendo(lit, 0, 1);                   // 压入1
				if (op == plusplus)
					gendo(opr, 0, 2);               // 加法
				else
					gendo(opr, 0, 3);               // 减法
				gendo(sto, levdiff, addr);          // 存储新值到变量
				gendo(lod, levdiff, addr);          // 加载新值作为表达式值
			}
			getsymdo;
		}
		else error(4);
		return 0;
	}

	if (sym == ident)
	{
		i = position(id, *ptx);
		if (i == 0) { error(11); return 0; }
		
		int levdiff = lev - table[i].level;
		int addr = table[i].adr;
		
		getsymdo;

		if (sym == plusplus || sym == minusminus)
		{
			// 后缀自增/自减
			if (table[i].kind != variable) error(12);
			else
			{
				gendo(lod, levdiff, addr);          // 加载原值用于返回
				gendo(lod, levdiff, addr);          // 再次加载原值用于运算
				gendo(lit, 0, 1);                   // 压入1
				if (sym == plusplus)
					gendo(opr, 0, 2);               // 加法
				else
					gendo(opr, 0, 3);               // 减法
				gendo(sto, levdiff, addr);          // 存储新值
				gendo(lod, levdiff, addr);          // 加载原值作为表达式值(注意是原值)
			}
			getsymdo;
			return 0;
		}

		if (sym == plusequal || sym == minusequal || sym == timesequal || sym == slashequal)
		{
			if (table[i].kind != variable) error(12);
			else
			{
				enum symbol op = sym;
				int levdiff = lev - table[i].level;
				int addr = table[i].adr;
				
				getsymdo;
				// 先加载变量值
				gendo(lod, levdiff, addr);
				// 计算右值表达式
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev);
				
				// 执行相应运算
				switch(op) {
                    case plusequal:  gendo(opr, 0, 2); break;
                    case minusequal: gendo(opr, 0, 3); break;
                    case timesequal: gendo(opr, 0, 4); break;
                    case slashequal: gendo(opr, 0, 5); break;
                }
                
                // 保存结果
                gendo(sto, levdiff, addr);
                
                // 再次加载最终值
                gendo(lod, levdiff, addr);
			}
			return 0;
		}

		if (table[i].kind == array && sym == lbrack)
		{
			getsymdo;
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[rbrack] = true;
			/* 也允许 := */
			nxtlev[becomes] = true;
			expressiondo(nxtlev, ptx, lev);
			if (sym == rbrack)
			{
				getsymdo;
			}
			else
			{
				error(22);
				while (sym != rbrack && sym != becomes && !inset(sym, fsys))
				{
					getsymdo;
				}
				if (sym == rbrack)
				{
					getsymdo;
				}
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
			else error(13);
			return 0;
		}

		if (table[i].kind == variable)
		{
			if (sym == becomes)
			{
				getsymdo;
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev);
				gendo(sto, lev-table[i].level, table[i].adr);
			}
			else error(13);
			return 0;
		}
		error(12);
		return 0;
	}

	if (sym == readsym)
	{
		getsymdo;
		if (sym != lparen) error(34);
		else
		{
			do {
				getsymdo;
				if (sym == ident) i = position(id, *ptx); else i = 0;
				if (i == 0) error(35);
				else if (table[i].kind != variable) error(32);
				else { gendo(opr, 0, 16); gendo(sto, lev-table[i].level, table[i].adr); }
				getsymdo;
			} while (sym == comma);
		}
		if (sym != rparen) { error(33); while (!inset(sym, fsys)) getsymdo; }
		else getsymdo;
		return 0;
	}

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
			if (sym != rparen) error(33); else getsymdo;
		}
		/* write 语句总是输出换行（不管有无括号） */
		gendo(opr, 0, 15);
		return 0;
	}

	if (sym == callsym)
	{
		getsymdo;
		if (sym != ident) error(14);
		else
		{
			i = position(id, *ptx);
			if (i == 0) error(11);
			else if (table[i].kind == procedur) gendo(cal, lev-table[i].level, table[i].adr);
			else error(15);
			getsymdo;
		}
		return 0;
	}

	if (sym == ifsym)
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[thensym] = true;
		/* 移除 dosym，它是 while 专用的 */
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
		/* then 后的语句：增加 elsesym 到同步集合 */
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[elsesym] = true;
		statementdo(nxtlev, ptx, lev);
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
		return 0;
	}

	if (sym == beginsym)
	{
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[semicolon] = true;
		nxtlev[endsym] = true;
		
		/* 处理第一个语句 */
		statementdo(nxtlev, ptx, lev);
		
		/* 处理后续的 ; statement 序列 */
		while (sym == semicolon)
		{
			getsymdo;
			/* 检查是否已到达 end */
			if (sym == endsym)
			{
				break;
			}
			statementdo(nxtlev, ptx, lev);
		}
		
		/* 检查是否正确结束 */
		if (sym == endsym)
		{
			getsymdo;
		}
		else
		{
			error(17);
			/* 尝试恢复：跳到 end 或其他终结符 */
			while (sym != endsym && !inset(sym, fsys))
			{
				getsymdo;
			}
			if (sym == endsym)
			{
				getsymdo;
			}
		}
		return 0;
	}

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
			/* 错误恢复：尝试跳到 begin 或其他语句起始符 */
			while (sym != dosym && !inset(sym, statbegsys) && !inset(sym, fsys))
			{
				getsymdo;
			}
			if (sym == dosym)
			{
				getsymdo;
			}
		}
		statementdo(fsys, ptx, lev);
		gendo(jmp, 0, cx1);
		code[cx2].a = cx;
		return 0;
	}

	memset(nxtlev, 0, sizeof(bool)*symnum);
	testdo(fsys, nxtlev, 19);
	return 0;
}

/*
* 表达式处理（修正）
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;
	bool nxtlev[symnum];

	if (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);
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
		term(nxtlev, ptx, lev);
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		term(nxtlev, ptx, lev);
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
* 项处理（乘除、取余），使用 power_expression 以保证 ^ 优先级最高
* 重命名并替换之前的 term_fixed，使之成为程序唯一的 term 实现
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	nxtlev[modsym] = true;

	/* 先处理幂运算层次（因子 -> 幂） */
	power_expressiondo(nxtlev, ptx, lev);

	while (sym==times || sym==slash || sym==modsym)
	{
		mulop = sym;
		getsymdo;
		/* 右侧仍为 power_expression，以保证 ^ 的优先级高于 * / % */
		power_expressiondo(nxtlev, ptx, lev);
		if (mulop == times)
		{
			gendo(opr, 0, 4);
		}
		else if (mulop == slash)
		{
			gendo(opr, 0, 5);
		}
		else if (mulop == modsym)
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
		getsymdo;
		power_expressiondo(nxtlev, ptx, lev);
		gendo(opr, 0, 18);
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
	if (inset(sym, facbegsys))
	{
		if (sym == ident)
		{
			i = position(id, *ptx);
			if (i == 0) error(11);
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
						/* 确保 expression 在遇到 ] 时停止 */
						expressiondo(nxtlev, ptx, lev);
						if (sym == rbrack)
						{
							getsymdo;
						}
						else
						{
							error(22);
							/* 错误恢复 */
							while (sym != rbrack && !inset(sym, fsys))
							{
								getsymdo;
							}
							if (sym == rbrack)
							{
								getsymdo;
							}
						}
						gendo(lit, 0, table[i].adr);
						gendo(opr, 0, 2);
						gendo(lodi, lev - table[i].level, 0);
					}
					else error(21);
					break;
				case procedur:
					error(21);
					getsymdo;
					break;
				}
			}
		}
		else if (sym == number)
		{
			if (num > amax) { error(31); num = 0; }
			gendo(lit, 0, num);
			getsymdo;
		}
		else if (sym == lparen)
		{
			getsymdo;
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[rparen] = true;
			expressiondo(nxtlev, ptx, lev);
			if (sym == rparen) getsymdo; else error(22);
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

	if (sym == oddsym)
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);
	}
	else
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[eql] = true; nxtlev[neq] = true; nxtlev[lss] = true;
		nxtlev[leq] = true; nxtlev[gtr] = true; nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq) error(20);
		else
		{
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)
			{
			case eql: gendo(opr, 0, 8); break;
			case neq: gendo(opr, 0, 9); break;
			case lss: gendo(opr, 0, 10); break;
			case geq: gendo(opr, 0, 11); break;
			case gtr: gendo(opr, 0, 12); break;
			case leq: gendo(opr, 0, 13); break;
			}
		}
	}
	return 0;
}

/*
* 解释执行（增加无限循环检测与诊断输出）
*/
void interpret()
{
	int p, b, t;
	struct instruction i;
	int s[stacksize];
	const int MAX_STEPS = 1000000; /* 步数上限，超过视为可能的无限循环 */
	int step_count = 0;

	printf("start pl0\n");
	t = 0; b = 0; p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		/* 检测步数上限，避免无限循环 */
		if (++step_count > MAX_STEPS)
		{
			fprintf(stderr, "Runtime abort: exceeded max steps (%d). Possible infinite loop.\n", MAX_STEPS);
			fprintf(fa2, "Runtime abort: exceeded max steps (%d). Possible infinite loop.\n", MAX_STEPS);

			/* 输出当前状态，便于诊断 */
			fprintf(stderr, "State dump: p=%d b=%d t=%d\n", p, b, t);
			fprintf(fa2, "State dump: p=%d b=%d t=%d\n", p, b, t);

			/* 输出当前及附近若干条指令 */
			int start = p - 5;
			if (start < 0) start = 0;
			int end = p + 5;
			if (end >= cx) end = cx-1;
			fprintf(stderr, "Code around p:\n");
			fprintf(fa2, "Code around p:\n");
			for (int idx = start; idx <= end; idx++)
			{
				fprintf(stderr, "%3d: %s %d %d\n", idx, mnemonic[code[idx].f], code[idx].l, code[idx].a);
				fprintf(fa2, "%3d: %s %d %d\n", idx, mnemonic[code[idx].f], code[idx].l, code[idx].a);
			}

			/* 终止解释执行 */
			break;
		}

		i = code[p++];
		switch (i.f)
		{
		case lit: s[t++] = i.a; break;
		case opr:
			switch (i.a)
			{
			case 0: t = b; p = s[t+2]; b = s[t+1]; break;
			case 1: s[t-1] = -s[t-1]; break;
			case 2: t--; s[t-1] = s[t-1] + s[t]; break;   // 加法
			case 3: t--; s[t-1] = s[t-1] - s[t]; break;   // 减法
			case 4: t--; s[t-1] = s[t-1] * s[t]; break;   // 乘法
			case 5:                                // 除法
				t--;
				if (s[t] == 0) {
					printf("Division by zero\n");
					fprintf(fa2, "Division by zero\n");
					p = 0;
					break;
				}
				s[t-1] /= s[t];
				break;
			case 6: s[t-1] = s[t-1] % 2; break;   // odd 运算
			case 7:                                // 取模
				t--;
				if (s[t] == 0) {
					printf("Modulo by zero\n");
					fprintf(fa2, "Modulo by zero\n");
					p = 0;
					break;
				}
				s[t-1] %= s[t];
				break;
			case 8: t--; s[t-1] = (s[t-1] == s[t]); break;
			case 9: t--; s[t-1] = (s[t-1] != s[t]); break;
			case 10: t--; s[t-1] = (s[t-1] < s[t]); break;
			case 11: t--; s[t-1] = (s[t-1] >= s[t]); break;
			case 12: t--; s[t-1] = (s[t-1] > s[t]); break;
			case 13: t--; s[t-1] = (s[t-1] <= s[t]); break;
			case 14: printf("%d", s[t-1]); fprintf(fa2, "%d", s[t-1]); t--; break;
			case 15: printf("\n"); fprintf(fa2,"\n"); break;
			case 16: printf("?"); fprintf(fa2, "?"); scanf("%d", &(s[t])); fprintf(fa2, "%d\n", s[t]); t++; break;
			case 17: s[t-1] = (s[t-1] == 0 ? 1 : 0); break;  // 修正逻辑非运算
			case 18: {
                t--;
                int exp = s[t];
                int base = s[t-1];
                long long result = 1;
                if (exp < 0) {
                    s[t-1] = 0;
                } else {
                    for (int k = 0; k < exp && result <= INT_MAX; k++) {
                        result *= base;
                    }
                    if (result > INT_MAX) result = INT_MAX;
                    s[t-1] = (int)result;
                }
            } break;
			}
			break;
		case lod: s[t++] = s[base(i.l,s,b)+i.a]; break;
		case sto: t--; s[base(i.l,s,b)+i.a] = s[t]; break;
		case dup: s[t] = s[t-1]; t++; break;              // 新增：复制栈顶值
		case cal: s[t] = base(i.l,s,b); s[t+1] = b; s[t+2] = p; b = t; p = i.a; break;
		case inte: t += i.a; break;
		case jmp: p = i.a; break;
		case jpc: t--; if (s[t] == 0) p = i.a; break;
		}
	} while (p != 0);
}

/* 通过过程基址求上 l 层过程的基址 */
int base(int l, int* s, int b)
{
	int b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}