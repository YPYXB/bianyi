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

#include "pl0.h"
#include "string.h"

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
		printf("List object code?(Y/N)");   /* �Ƿ������������� */
		scanf("%s", fname);
		listswitch = (fname[0]=='y' || fname[0]=='Y');

		printf("List symbol table?(Y/N)");  /* �Ƿ�������ֱ� */
		scanf("%s", fname);
		tableswitch = (fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* ��ʼ�� */

		err = 0;
		cc = cx = ll = 0;
		ch = ' ';

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if(-1 == block(0, 0, nxtlev))   /* ���ñ������ */
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
				interpret();    /* ���ý���ִ�г��� */
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

	/* ���õ��ַ����� */
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
	ssym['['] = lbrack;    /* ����ӣ�'[' ���� */
	ssym[']'] = rbrack;    /* ����ӣ�']' ���� */

	/* ���ñ���������,������ĸ˳�򣬱����۰���� */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "else");    /* ���� else */
	strcpy(&(word[5][0]), "end");
	strcpy(&(word[6][0]), "if");
	strcpy(&(word[7][0]), "odd");
	strcpy(&(word[8][0]), "procedure");
	strcpy(&(word[9][0]), "read");
	strcpy(&(word[10][0]), "then");
	strcpy(&(word[11][0]), "var");
	strcpy(&(word[12][0]), "while");
	strcpy(&(word[13][0]), "write");

	/* ���ñ����ַ��� */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
	wsym[4] = elsesym;    /* ���� else */
	wsym[5] = endsym;
	wsym[6] = ifsym;
	wsym[7] = oddsym;
	wsym[8] = procsym;
	wsym[9] = readsym;
	wsym[10] = thensym;
	wsym[11] = varsym;
	wsym[12] = whilesym;
	wsym[13] = writesym;

	/* ����ָ������ */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[lodi][0]), "lodi");  /* ����ӣ����װ��ָ�����Ƿ� */
	strcpy(&(mnemonic[sti][0]), "sti");    /* ����ӣ���Ӵ洢ָ�����Ƿ� */

	/* ���÷��ż� */
	for (i=0; i<symnum; i++)
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* ����������ʼ���ż� */
	declbegsys[constsym] = true;
	declbegsys[varsym] = true;
	declbegsys[procsym] = true;

	/* ������俪ʼ���ż� */
	statbegsys[beginsym] = true;
	statbegsys[callsym] = true;
	statbegsys[ifsym] = true;
	statbegsys[whilesym] = true;
	statbegsys[readsym] = true;
	statbegsys[writesym] = true;

	/* �������ӿ�ʼ���ż� */
	facbegsys[ident] = true;
	facbegsys[number] = true;
	facbegsys[lparen] = true;
}

/*
* ������ʵ�ּ��ϵļ�������
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
*   ��������ӡ����λ�úʹ������
*/
void error(int n)
{
	char space[81];
	memset(space,32,81);

	space[cc-1]=0; //����ʱ��ǰ�����Ѿ����꣬����cc-1

	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);

	err++;
}

/*
* ©���ո񣬶�ȡһ���ַ���
*
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
*
* ������getsym���á�
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
* �ʷ���������ȡһ������
*/
int getsym()
{
	int i,j,k;

	while (ch==' ' || ch==10 || ch==13 || ch==9)
	{
		getchdo;
	}
	if (ch>='a' && ch<='z')
	{           /* ���ֻ�������a..z��ͷ */
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
		do {    /* ������ǰ�����Ƿ�Ϊ������ */
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
			sym = ident; /* ����ʧ���������ֻ����� */
		}
	}
	else
	{
		if (ch>='0' && ch<='9')
		{           /* ����Ƿ�Ϊ���֣���0..9��ͷ */
			k = 0;
			num = 0;
			sym = number;
			do {
				num = 10*num + ch - '0';
				k++;
				getchdo;
			} while (ch>='0' && ch<='9'); /* ��ȡ���ֵ�ֵ */
			k--;
			if (k > nmax)
			{
				error(30);
			}
		}
		else
		{
			if (ch == ':')      /* ��⸳ֵ���� */
			{
				getchdo;
				if (ch == '=')
				{
					sym = becomes;
					getchdo;
				}
				else
				{
					sym = nul;  /* ����ʶ��ķ��� */
				}
			}
			else
			{
				if (ch == '<')      /* ���С�ڻ�С�ڵ��ڷ��� */
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
					if (ch=='>')        /* �����ڻ���ڵ��ڷ��� */
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
						/* ������ʶ�� ++ �� -- */
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
								/* ch ��ָ����һ�ַ��������� getchdo */
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
								/* ch ��ָ����һ�ַ��������� getchdo */
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
* �������������
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* ������� */
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
* ���Ե�ǰ�����Ƿ�Ϸ�
*
* ��ĳһ���֣���һ����䣬һ�����ʽ����Ҫ����ʱʱ����ϣ����һ����������ĳ��?
* ���ò��ֵĺ�����ţ���test���������⣬���Ҹ��𵱼�ⲻͨ��ʱ�Ĳ��ȴ�ʩ��
* ��������Ҫ���ʱָ����ǰ��Ҫ�ķ��ż��ϺͲ����õļ��ϣ���֮ǰδ��ɲ��ֵĺ��
* ���ţ����Լ���ⲻͨ��ʱ�Ĵ���š�
*
* s1:   ������Ҫ�ķ���
* s2:   �������������Ҫ�ģ�����Ҫһ�������õļ�?
* n:    �����
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ��� */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{
			getsymdo;
		}
	}
	return 0;
}

/*
* �������������
*
* lev:    ��ǰ�ֳ������ڲ�
* tx:     ���ֱ�ǰβָ��
* fsys:   ��ǰģ��ĺ�����ż���
*/
int block(int lev, int tx, bool* fsys)
{
	int i;

	int dx;                 /* ���ַ��䵽����Ե�ַ */
	int tx0;                /* ������ʼtx */
	int cx0;                /* ������ʼcx */
	bool nxtlev[symnum];    /* ���ڷ��ż���ʹ������ʵ�֣�����ʱ�ᴫָ�룻Ϊ�����¼������޸��ϼ����ϣ�����һ�ݴ��ݸ��¼� */

	dx = 3;
	tx0 = tx;               /* ��¼�������ֵĳ�ʼλ�� */
	table[tx].adr = cx;

	gendo(jmp, 0, 0);

	if (lev > levmax)
	{
		error(32);
	}

	do {

		if (sym == constsym)    /* �յ������������ţ���ʼ���������� */
		{
			getsymdo;

			/* the original do...while(sym == ident) is problematic, thanks to calculous */
			/* do { */
			constdeclarationdo(&tx, lev, &dx);  /* dx��ֵ�ᱻconstdeclaration�ı䣬ʹ��ָ�� */
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
				error(5);   /*©���˶��Ż��߷ֺ�*/
			}
			/* } while (sym == ident); */
		}

		if (sym == varsym)      /* �յ������������ţ���ʼ����������� */
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

		while (sym == procsym) /* �յ������������ţ���ʼ����������� */
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* ��¼�������� */
				getsymdo;
			}
			else
			{
				error(4);   /* procedure��ӦΪ��ʶ�� */
			}

			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);   /* ©���˷ֺ� */
			}

			memcpy(nxtlev, fsys, sizeof(bool)*symnum);
			nxtlev[semicolon] = true;
			if (-1 == block(lev+1, tx, nxtlev))
			{
				return -1;  /* �ݹ���� */
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
				error(5);   /* ©���˷ֺ� */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   /* ֱ��û���������� */

	code[table[tx0].adr].a = cx;    /* ��ʼ���ɵ�ǰ���̴��� */
	table[tx0].adr = cx;            /* ��ǰ���̴����ַ */
	table[tx0].size = dx;           /* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gendo(inte, 0, dx);             /* ���ɷ����ڴ���� */

	if (tableswitch)        /* ������ֱ� */
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

	/* ���������Ϊ�ֺŻ�end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* ÿ��������ż��Ͷ������ϲ������ż��ͣ��Ա㲹�� */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);                       /* ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
	memset(nxtlev, 0, sizeof(bool)*symnum); /*�ֳ���û�в��ȼ��� */
	testdo(fsys, nxtlev, 8);                /* �����������ȷ�� */
	listcode(cx0);                          /* ������� */
	return 0;
}

/*
* �����ֱ��м���һ��
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ȫ�ֱ���id���Ѵ��е�ǰ���ֵ����� */
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:  /* �������� */
		if (num > amax)
		{
			error(31);  /* ��Խ�� */
			num = 0;
		}
		table[(*ptx)].val = num;
		break;
	case variable:  /* �������� */
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:  /*���������֡�*/
		table[(*ptx)].level = lev;
		break;
	case array:     /* һά����������num ��Ϊ�����С */
		if (num <= 0)
		{
			error(31); /* �Ƿ������С */
			num = 1;
		}
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx); /* ��¼�����ַ����Ե�ַ�� */
		table[(*ptx)].size = num;   /* ��¼���鳤�� */
		(*pdx) += num; /* Ϊ������������� num ����Ԫ */
		break;
	}
}

/*
* �������ֵ�λ��.
* �ҵ��򷵻������ֱ��е�λ��,���򷵻�0.
*
* idt:    Ҫ���ҵ�����
* tx:     ��ǰ���ֱ�βָ��
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
* ������������
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
				error(1);   /* ��=д����:= */
			}
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* ����˵��=��Ӧ������ */
			}
		}
		else
		{
			error(3);   /* ����˵����ʶ��Ӧ��= */
		}
	}
	else
	{
		error(4);   /* const��Ӧ�Ǳ�ʶ */
	}
	return 0;
}

/*
* ������������
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	/* �޸ģ�֧������ ident '[' number ']' ���������� */
	if (sym == ident)
	{
		char saved_id[al+1];
		strcpy(saved_id, id);
		getsymdo; /* ��ȡ��ʶ�������һ������ */

		if (sym == lbrack) /* array declaration: ident '[' number ']' */
		{
			getsymdo;
			if (sym == number)
			{
				/* �ѱ�ʶ���ָ���ȫ�� id��num ����Ϊ�����С���Թ� enter ʹ�� */
				strcpy(id, saved_id);
				/* ���� enter(array, ...) ʹ�õ�ǰȫ�� num ��Ϊ���鳤�� */
				enter(array, ptx, lev, pdx);
				getsymdo; /* ���� number ��ķ��ţ�ԭ�����߼��� */
				if (sym == rbrack)
				{
					getsymdo; /* ���� ']' */
				}
				else
				{
					error(36); /* ȱ�� ']' */
				}
			}
			else
			{
				error(31); /* �Ƿ������С */
			}
		}
		else
		{
			/* ��ͨ�������� */
			strcpy(id, saved_id);
			enter(variable, ptx, lev, pdx);
		}
	}
	else
	{
		error(4);   /* var ��Ӧ�Ǳ�ʶ�� */
	}
	return 0;
}

/*
* ���Ŀ������嵥
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

	/* ǰ׺ ++/-- ���� */
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

			/* ��׺ ++/-- */
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

			/* ���鸳ֵ */
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

			/* ��ͨ������ֵ */
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
							gendo(jmp, 0, 0);     /* then ��֧���������� else ��֧ */
							code[cx1].a = cx;     /* jpc ��ת�� else ��֧��ʼ */
							getsymdo;
							statementdo(fsys, ptx, lev);
							code[cx2].a = cx;     /* jmp ��ת�� else ��֧������ */
						}
						else
						{
							code[cx1].a = cx;     /* �� else��jpc ֱ������ then �� */
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
* �������������
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* ������� */
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
	int p, b, t;    /* ָ��ָ�룬ָ���ַ��ջ��ָ�� */
	struct instruction i;   /* ��ŵ�ǰָ�� */
	int s[stacksize];   /* ջ */

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		i = code[p];    /* ����ǰָ�� */
		p++;
		switch (i.f)
		{
		case lit:   /* ��a��ֵȡ��ջ�� */
			s[t] = i.a;
			t++;
			break;
		case opr:   /* ��ѧ���߼����� */
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
		case lod:   /* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			s[t] = s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:   /* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case lodi: /* ���װ��: ���� offset������ s[base(l)+offset] ѹ��ջ */
			{
				int off;
				t--;
				off = s[t];
				s[t] = s[base(i.l, s, b) + off];
				t++;
			}
			break;
		case sti: /* ��Ӵ洢: ���� value �� offset, ���� s[base(l)+offset] = value */
			{
				int val, off;
				t--;
				val = s[t];   /* pop value */
				t--;
				off = s[t];   /* pop offset */
				s[base(i.l, s, b) + off] = val;
			}
			break;
		case cal:   /* �����ӹ��� */
			s[t] = base(i.l, s, b); /* �������̻���ַ��ջ */
			s[t+1] = b; /* �������̻���ַ��ջ������������base���� */
			s[t+2] = p; /* ����ǰָ��ָ����ջ */
			b = t;  /* �ı����ַָ��ֵΪ�¹��̵Ļ���ַ */
			p = i.a;    /* ��ת */
			break;
		case inte:  /* �����ڴ� */
			t += i.a;
			break;
		case jmp:   /* ֱ����ת */
			p = i.a;
			break;
		case jpc:   /* ������ת */
			t--;
			if (s[t] == 0)
			{
				p = i.a;
			}
			break;
		}
	} while (p != 0);
}

/* ͨ�����̻�ַ���� l ����̵Ļ�ַ */
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

