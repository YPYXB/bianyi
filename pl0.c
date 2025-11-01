/*
 * PL/0 ��������� Win32 ƽ̨���� C ʵ�֣�
 *
 * �������� Visual C++ 6.0��Visual C++.NET �Լ� Visual C++.NET 2003
 * �� Win98��WinNT��Win2000��WinXP �� Win2003 �ϲ���ͨ����
 *
 * ʹ�÷�����
 * ���к����� PL/0 Դ�����ļ���
 * �ش��Ƿ�������������
 * �ش��Ƿ�������ֱ�
 * fa.tmp ������������
 * fa1.tmp ���Դ�ļ�������ж�Ӧ���׵�ַ
 * fa2.tmp �������ִ�еĽ��
 * fas.tmp ������ֱ�
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pl0.h"

/* ����ִ��ʱʹ�õ�ջ */
#define stacksize 500

int main()
{
	bool nxtlev[symnum];

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* �����ļ��� */

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
* ��ʼ��
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
	strcpy(&(mnemonic[dup][0]), "dup");  // ���dupָ�����Ƿ�

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
* ������ʵ�ּ��ϵļ�������
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
* ������
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
* ��ȡ�ַ������У���֧������ '\' ע�ͽض�
* ������ȥ�� UTF-8 BOM�������˷� ASCII �ɴ�ӡ�ַ�����������ע�͵ȸ��Ŵʷ�����
* �������Ƴ�������ע�� { ... }��֧��ע�Ϳ��в�����ע�ͺ����Ĵ���
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

			/* ������� UTF-8 BOM ��ͷ������ BOM */
			if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
			{
				size_t l = strlen(line+3);
				memmove(line, line+3, l+1);
			}

			/* ���ַ�ɨ�裬�Ƴ�������ע�� { ... }��֧�ֿ��У�������ע�ͺ������� */
			{
				char buf[sizeof(line)];
				int src = 0, dst = 0;
				bool in_comment = false;
				while (1)
				{
					/* �����ǰ�н�����������ע���У����Զ�ȡ��һ��ֱ��ע�ͽ����� EOF */
					if (line[src] == '\0')
					{
						if (in_comment)
						{
							/* ��ȡ�������м�ע���У���������ǣ���������Ҫ��¼�к� */
							if (fgets(line, sizeof(line), fin) == NULL)
							{
								/* EOF reached while in comment -> treat as end */
								break;
							}
							linenum++;
							/* ȥ�����ܵ� BOM��ֻ�����п��ܳ��֣� */
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
							/* ���д������ */
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
						/* ��������ע���У������ַ� */
					}
				}
				/* �����������ݿ��� line ��������������� */
				strncpy(line, buf, sizeof(line)-1);
				line[sizeof(line)-1] = '\0';
			}

			/* �������е�һ�� '\'��ע����ʼ�������������ضϸ�λ�� */
			{
				char* p = strchr(line, '\\');
				if (p) *p = '\0';
			}

			/* ���˵��� ASCII �ɴ�ӡ�ַ������� 0x20-0x7E�����кͻس����Ʊ� */
			{
				int src = 0, dst = 0;
				while (line[src] != '\0')
				{
					unsigned char c = (unsigned char)line[src];
					if (c == '\n' || c == '\r' || c == '\t' || (c >= 0x20 && c <= 0x7E))
					{
						line[dst++] = line[src];
					}
					/* �����������ַ���ȥ�����ĵȷ� ASCII �ַ��� */
					src++;
				}
				line[dst] = '\0';
			}

			/* ���ض�/���˺��Ƿ�����Ϊ�հ� */
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

			/* ��������û��壨��������ֻ�� ASCII �ɴ�ӡ�ַ��뻻�У� */
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
* �ʷ�����
* �Ľ�����ʶ���ַ�ͳһת��ΪСд�洢�������ݴ���
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
			/* ͳһСд�� */
			if (cc >= 'A' && cc <= 'Z') cc = (char)(cc - 'A' + 'a');
			if (k < al) a[k++] = cc;
			getchdo;
		} while ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9') || ch=='_');
		a[k] = 0;
		strcpy(id, a);

		/* ʹ�ñ�׼���ֲ��Ҿ�׼ƥ�䱣���֣������ֱ��Ѱ��ֵ������У� */
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
		/* δƥ�䵽�����֣���Ϊ��ʶ�� */
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
		/* k ������λ����ֱ�ӱȽϣ������������ */
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
* ���ɴ���
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
* ���Լ���
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
			/* ԭ��if (sym == semicolon) getsymdo; else error(5); */
			if (sym == semicolon) {
				getsymdo;
			} else {
				/* �������ѽ������/����ʼ�����/����/�����β�������ȱʧ�ķֺ� */
				if (!inset(sym, statbegsys) && sym != procsym && sym != endsym && sym != period) {
					error(5);
				}
				/* �����ѷ��ţ����ɺ������� */
			}
		}
		if (sym == varsym)
		{
			getsymdo;
			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma) { getsymdo; vardeclarationdo(&tx, lev, &dx); }
			/* ԭ��if (sym == semicolon) getsymdo; else error(5); */
			if (sym == semicolon) {
				getsymdo;
			} else {
				/* �������ѽ������/����ʼ�����/����/�����β�������ȱʧ�ķֺ� */
				if (!inset(sym, statbegsys) && sym != procsym && sym != endsym && sym != period) {
					error(5);
				}
				/* �����ѷ��ţ����ɺ������� */
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
* �����ֱ��м���һ��
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
* �������ֵ�λ��
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
* ������������
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
* ��������������ǿ����ָ���
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
			/* �������� */
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
					/* ����ָ������� ']', ',', �� ';' */
					while (sym != rbrack && sym != semicolon && sym != comma && sym != period) getsymdo;
					if (sym == rbrack) getsymdo;  /* ����ҵ� ']'���ƽ� */
				}
				else
				{
					/* �ɹ����ƽ������Ż�ֺ� */
					getsymdo;
				}
			}
			else
			{
				error(31);
				/* ����ָ� */
				while (sym != rbrack && sym != semicolon && sym != comma && sym != period) getsymdo;
				if (sym == rbrack) getsymdo;
			}
		}
		else
		{
			/* ��ͨ������sym ���Ƕ��Ż�ֺ� */
			strcpy(id, saved_id);
			enter(variable, ptx, lev, pdx);
		}
	}
	else 
	{
		error(4);
		/* ����ָ���������Ǳ�ʶ�����������Ż�ֺ� */
		while (sym != semicolon && sym != comma && sym != period) getsymdo;
	}
	return 0;
}

/*
* �г�����
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
* ��䴦��
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == plusplus || sym == minusminus)
	{
		// ǰ׺����/�Լ�
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
				
				gendo(lod, levdiff, addr);          // ����ԭֵ
				gendo(lit, 0, 1);                   // ѹ��1
				if (op == plusplus)
					gendo(opr, 0, 2);               // �ӷ�
				else
					gendo(opr, 0, 3);               // ����
				gendo(sto, levdiff, addr);          // �洢��ֵ������
				gendo(lod, levdiff, addr);          // ������ֵ��Ϊ���ʽֵ
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
			// ��׺����/�Լ�
			if (table[i].kind != variable) error(12);
			else
			{
				gendo(lod, levdiff, addr);          // ����ԭֵ���ڷ���
				gendo(lod, levdiff, addr);          // �ٴμ���ԭֵ��������
				gendo(lit, 0, 1);                   // ѹ��1
				if (sym == plusplus)
					gendo(opr, 0, 2);               // �ӷ�
				else
					gendo(opr, 0, 3);               // ����
				gendo(sto, levdiff, addr);          // �洢��ֵ
				gendo(lod, levdiff, addr);          // ����ԭֵ��Ϊ���ʽֵ(ע����ԭֵ)
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
				// �ȼ��ر���ֵ
				gendo(lod, levdiff, addr);
				// ������ֵ���ʽ
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev);
				
				// ִ����Ӧ����
				switch(op) {
                    case plusequal:  gendo(opr, 0, 2); break;
                    case minusequal: gendo(opr, 0, 3); break;
                    case timesequal: gendo(opr, 0, 4); break;
                    case slashequal: gendo(opr, 0, 5); break;
                }
                
                // ������
                gendo(sto, levdiff, addr);
                
                // �ٴμ�������ֵ
                gendo(lod, levdiff, addr);
			}
			return 0;
		}

		if (table[i].kind == array && sym == lbrack)
		{
			getsymdo;
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[rbrack] = true;
			/* Ҳ���� := */
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
		/* write �������������У������������ţ� */
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
		/* �Ƴ� dosym������ while ר�õ� */
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
		/* then �����䣺���� elsesym ��ͬ������ */
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
		
		/* �����һ����� */
		statementdo(nxtlev, ptx, lev);
		
		/* ��������� ; statement ���� */
		while (sym == semicolon)
		{
			getsymdo;
			/* ����Ƿ��ѵ��� end */
			if (sym == endsym)
			{
				break;
			}
			statementdo(nxtlev, ptx, lev);
		}
		
		/* ����Ƿ���ȷ���� */
		if (sym == endsym)
		{
			getsymdo;
		}
		else
		{
			error(17);
			/* ���Իָ������� end �������ս�� */
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
			/* ����ָ����������� begin �����������ʼ�� */
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
* ���ʽ����������
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
* ����˳���ȡ�ࣩ��ʹ�� power_expression �Ա�֤ ^ ���ȼ����
* ���������滻֮ǰ�� term_fixed��ʹ֮��Ϊ����Ψһ�� term ʵ��
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	nxtlev[modsym] = true;

	/* �ȴ����������Σ����� -> �ݣ� */
	power_expressiondo(nxtlev, ptx, lev);

	while (sym==times || sym==slash || sym==modsym)
	{
		mulop = sym;
		getsymdo;
		/* �Ҳ���Ϊ power_expression���Ա�֤ ^ �����ȼ����� * / % */
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
* �����㴦�� (�ҽ��)
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
* ���Ӵ���
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
						/* ȷ�� expression ������ ] ʱֹͣ */
						expressiondo(nxtlev, ptx, lev);
						if (sym == rbrack)
						{
							getsymdo;
						}
						else
						{
							error(22);
							/* ����ָ� */
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
* ��������
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
* ����ִ�У���������ѭ���������������
*/
void interpret()
{
	int p, b, t;
	struct instruction i;
	int s[stacksize];
	const int MAX_STEPS = 1000000; /* �������ޣ�������Ϊ���ܵ�����ѭ�� */
	int step_count = 0;

	printf("start pl0\n");
	t = 0; b = 0; p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		/* ��ⲽ�����ޣ���������ѭ�� */
		if (++step_count > MAX_STEPS)
		{
			fprintf(stderr, "Runtime abort: exceeded max steps (%d). Possible infinite loop.\n", MAX_STEPS);
			fprintf(fa2, "Runtime abort: exceeded max steps (%d). Possible infinite loop.\n", MAX_STEPS);

			/* �����ǰ״̬��������� */
			fprintf(stderr, "State dump: p=%d b=%d t=%d\n", p, b, t);
			fprintf(fa2, "State dump: p=%d b=%d t=%d\n", p, b, t);

			/* �����ǰ������������ָ�� */
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

			/* ��ֹ����ִ�� */
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
			case 2: t--; s[t-1] = s[t-1] + s[t]; break;   // �ӷ�
			case 3: t--; s[t-1] = s[t-1] - s[t]; break;   // ����
			case 4: t--; s[t-1] = s[t-1] * s[t]; break;   // �˷�
			case 5:                                // ����
				t--;
				if (s[t] == 0) {
					printf("Division by zero\n");
					fprintf(fa2, "Division by zero\n");
					p = 0;
					break;
				}
				s[t-1] /= s[t];
				break;
			case 6: s[t-1] = s[t-1] % 2; break;   // odd ����
			case 7:                                // ȡģ
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
			case 17: s[t-1] = (s[t-1] == 0 ? 1 : 0); break;  // �����߼�������
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
		case dup: s[t] = s[t-1]; t++; break;              // ����������ջ��ֵ
		case cal: s[t] = base(i.l,s,b); s[t+1] = b; s[t+2] = p; b = t; p = i.a; break;
		case inte: t += i.a; break;
		case jmp: p = i.a; break;
		case jpc: t--; if (s[t] == 0) p = i.a; break;
		}
	} while (p != 0);
}

/* ͨ�����̻�ַ���� l ����̵Ļ�ַ */
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