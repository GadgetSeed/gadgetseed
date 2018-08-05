/** @file
    @brief	コマンドシェル

    @date	2007.03.16
    @data	2002.03.24
    @author	Takashi SHUDO
*/

#include "shell.h"
#include "lineedit.h"
#include "str.h"
#include "tprintf.h"
#include "history.h"

static void init_commands(struct st_shell_command * const * coms)
{
	struct st_shell_command * const *cp = coms;

	while(*cp != 0) {
		if((*cp)->init != 0) {
			(*cp)->init();
		}

		if((*cp)->sublist != 0) {
			init_commands((struct st_shell_command * const *)(*cp)->sublist);
		}

		cp ++;
	}
}

/**
   @brief	シェルを初期化する

   @param[in]	shell	shellデータ構造体ポインタ
   @param[in]	coms	初期化するshell_commandのポインタ
   @param[in]	prompt	プロンプト文字列ポインタ
*/
void init_shell(struct st_shell *shell, struct st_shell_command * const *coms,
		const uchar *prompt)
{
	shell->shell_coms = coms;
	shell->lastCom[0] = 0;
	shell->prompt = prompt;

	init_commands(coms);

	init_history(&shell->his);
	init_lineedit(&shell->comLine);
}

/**
   @brief	プロンプトを表示する

   @param[in]	shell	shellデータ構造体ポインタ
*/
void print_prompt(struct st_shell *shell)
{
	tprintf("%s", shell->prompt);
}

/**
   @brief	編集中のコマンドラインを破棄する

   @param[in]	shell	shellデータ構造体ポインタ
*/
void dispose_shell_line(struct st_shell *shell)
{
	new_lineedit(&shell->comLine);
}


/*
  トークンを区切る
*/
static int token_str(struct st_shell *shell, uchar *str, int nullterm)
{
	uchar *tk = str;
	int i;
	static const char nstr[] = "";
	int flg_esc = 0;

	shell->argc = 0;

	//xdump(str, SHELL_MAX_LINE_COLUMS);

	for(i=0; i<GSC_SHELL_MAX_COM_ARGV; i++) {
		shell->argv[i] = (uchar *)nstr;
	}

	for(i=0; i<GSC_SHELL_MAX_COM_ARGV; i++) {
		/*
		  ' '以外の文字が先頭
		*/
		while(tk < &str[GSC_SHELL_MAX_LINE_COLUMS]) {
			if((*tk == 0) || (*tk == ASCII_CR)) {
				if(nullterm) {
					*tk = 0;
				}
				goto next;
			} else if(*tk == '\\') { // "\"バックスラッシュはエスケープシーケンス
				// '/'を削除して詰める
				uchar *p = tk;
				while((*(p+1)) != 0) {
					*p =*(p+1);
					p++;
				}
				*p = 0;
				flg_esc = 1;
				shell->argv[i] = tk;
				shell->argc ++;
				break;
			} else if(*tk != ' ') {
				shell->argv[i] = tk;
				shell->argc ++;
				tk ++;
				break;
			}

			tk ++;
		}

		/*
		  ' 'まで読み捨て
		*/
		while(tk < &str[GSC_SHELL_MAX_LINE_COLUMS]) {
			if((*tk == 0) || (*tk == ASCII_CR) || (*tk == ASCII_LF)) {
				if(nullterm) {
					*tk = 0;
				}
				goto next;
			} else if(*tk == '\\') { // "\"バックスラッシュはエスケープシーケンス
				if(flg_esc == 0) {
					// '/'を削除して詰める
					uchar *p = tk;
					while((*(p+1)) != 0) {
						*p =*(p+1);
						p++;
					}
					*p = 0;
					flg_esc = 1;
					continue;
				}
			} else if(*tk == ' ') {
				if(flg_esc != 0) {
					flg_esc = 0;
				} else {
					flg_esc = 0;
					if(nullterm) {
						*tk = 0;
					}
					tk ++;
					break;
				}
			} else {
				if(flg_esc != 0) {
					flg_esc = 0;
				}
			}

			tk ++;
		}
	}
next:
#if 0 // DEBUG
	eprintf("argc = %d\n", shell->argc);
	for(i=0; i<shell->argc; i++) {
		eprintf("[%d] %s\n", i, shell->argv[i]);
	}
#endif
	return shell->argc;
}

/*
  一致するコマンドを探す
*/
struct st_shell_command * search_command(
		struct st_shell_command * const * coms,
		uchar *argv[], int *arg_top)
{
	struct st_shell_command * const *cp = coms;
	struct st_shell_command *rt = 0;

	while(*cp != 0) {
		if(strcomp((*cp)->name, argv[*arg_top]) == 0) {
			if((*cp)->sublist == 0) {
				if((*cp)->command != 0) {
					rt = (*cp);
					break;
				}
			} else {
				(*arg_top) ++;
				rt = search_command((struct st_shell_command * const *)(*cp)->sublist,
						    argv, arg_top);
				if(rt == 0) {
					if((*cp)->command != 0) {
						rt = (*cp);
					}
				}
				break;
			}
		}
		cp ++;
	}

	return rt;
}


/**
   @brief	str文字列のコマンドを実行する

   @param[in]	shell	shellデータ構造体ポインタ
   @param[in]	str	コマンド文字列ポインタ

   @return	コマンド実行結果
*/
int exec_shell_command(struct st_shell *shell, uchar *str)
{
	struct st_shell_command * const *cp = shell->shell_coms;
	int rt = 0;
	int flgc = 0;
	int arg_top = 0;
	int argc;
	struct st_shell_command * com;

	if(cp == 0) {
		return 0;
	}

	if(*str == 0) {
		flgc = 1;
		if(shell->lastCom[0] != 0) {
			(void)strncopy(str, shell->lastCom, GSC_SHELL_MAX_LINE_COLUMS);
		} else {
			return 0;
		}
	} else {
		(void)strncopy(shell->lastCom, str, GSC_SHELL_MAX_LINE_COLUMS);
	}

	if(token_str(shell, str, 1) == 0) {
		return 0;
	}

	com = search_command(shell->shell_coms, shell->argv, &arg_top);
	if(com != 0) {
		argc = shell->argc - arg_top;
		if(flgc) {
			if(com->attr & CMDATTR_CONT) {
				if(com->attr & CMDATTR_ARGLESS) {
					argc = 1;
				}
			} else {
				goto end;
			}
		}
		rt = com->command(argc, &shell->argv[arg_top]);
	} else {
		int i;
		shell->lastCom[0] = 0;
		tprintf("\"");
		for(i=0; i<arg_top; i++) {
			tprintf("%s ", shell->argv[i]);
		}
		tprintf("%s\" command not found.\n", shell->argv[i]);
	}

end:
	return rt;
}

void print_command_usage(const struct st_shell_command *command)
{
	if(command->usage_str == 0) {
		tprintf("Usage: %s\n", command->name);
	} else {
		tprintf("Usage: %s %s\n", command->name, command->usage_str);
	}
}

static int strcomps(const uchar *s1, const uchar *s2)
{
	uchar s1t, s2t;

	do {
		s1t = *s1;
		if(s1t == ' ') {
			s1t = 0;
		}
		s2t = *s2;
		if(s2t == ' ') {
			s2t = 0;
		}

		if(s1t > s2t) {
			return 1;
		} else if(s1t < s2t) {
			return -1;
		}
		s1 ++;
		s2 ++;
	} while((s1t != 0) || (s2t != 0));

	return 0;
}

#if 0
static long strlengs(const char *str)
{
	long len = 0;

	while((*str != 0) && (*str != ' ')) {
		len ++;
		str ++;
	}

	return len;
}
#endif

static unsigned int str_match_len(uchar *s1, uchar *s2)
{
	unsigned int len = 0;

	do {
		if(*s1 == *s2) {
			len ++;
		} else {
			return len;
		}
		s1 ++;
		s2 ++;
	} while((*s1 != 0) || (*s2 != 0));

	return len;
}

/*
  補完候補コマンドを探す
*/
#define MATCH_NO	0	// 補完不要、不可
#define MATCH_PART	1	// 部分補完
#define MATCH_COMP	2	// 完全一致

static int match_command(struct st_shell_command * const * coms,
			 struct st_shell *shell, int arg,
			 uchar **comstr,// 補完で挿入出来る文字列
			 unsigned int *len,	// 補完文字数
			 int disp)
{
	struct st_shell_command * const *cp = coms;
	int match = MATCH_NO;
	static const uchar *null = (const uchar *)"";

	while(*cp != 0) {
		if(strcomps(shell->argv[arg], (*cp)->name) == 0) {
			// 完全にマッチした場合はサブコマンドを探す
			if(strcomp(shell->argv[arg], (*cp)->name) == 0) {
				*len = 0;
				*comstr = (uchar *)null;
				return MATCH_COMP;
			} else if((*cp)->sublist) {
				arg ++;
				if(disp) {
					tprintf("\n %s ->", (*cp)->name);
				}
				return match_command((struct st_shell_command * const *)(*cp)->sublist, shell, arg,
						     comstr, len, disp);
			} else {
				*len = 0;
				*comstr = (uchar *)null;
				return MATCH_NO;
			}
		} else {
			if(strncomp(shell->argv[arg], (*cp)->name,
				    strleng(shell->argv[arg])) == 0) {
				// 現在のカーソル入力までマッチしてい
				// るコマンドを探す
				uchar* strp = (uchar *)(*cp)->name + strleng(shell->argv[arg]);
				if(match == MATCH_NO) {
					unsigned int comlen, arglen;
					match = MATCH_COMP;
					*comstr = strp;
					comlen = strleng((*cp)->name);
					arglen = strleng(shell->argv[arg]);
					if(comlen > arglen) {
						*len = comlen - arglen;
					} else {
						*len = 0;
					}
				} else {
					unsigned int tmp = str_match_len((uchar *)(*comstr), strp);
					if(*len > tmp) {
						*len = tmp;
						match = MATCH_PART;
					}
				}
//				tprintf("\n### %s:%s", (*comstr), strp);
//				tprintf("\n### len = %d", *len);
				if(disp) {
					tprintf("\n  %s", (*cp)->name);
				}
			}
		}

		cp ++;
	}

//	tprintf("\n### arg = %d, \"%s\"\n", arg, shell->argv[arg]);

	return match;
}

static void complement_command(struct st_shell *shell)
{
	uchar str[GSC_SHELL_MAX_LINE_COLUMS+1];
	static const uchar *space = (const uchar *)" ";
	int match = 0;
	uchar *comstr;
	unsigned int len = 0;

	(void)strncopy(str, shell->comLine.buf, GSC_SHELL_MAX_LINE_COLUMS);
	str[shell->comLine.cur_pos] = 0;

	token_str(shell, str, 0);

	match = match_command(shell->shell_coms, shell, 0,
			      &comstr, &len, 0);

	switch(match) {
	case MATCH_PART:
		match_command(shell->shell_coms, shell, 0,
			      &comstr, &len, 1);
		tprintf("\n");
		print_prompt(shell);
		draw_lineedit(&shell->comLine);
		insert_str_lineedit(&shell->comLine, comstr, len);
		break;

	case MATCH_COMP:
		insert_str_lineedit(&shell->comLine, comstr, len);
		insert_str_lineedit(&shell->comLine, (uchar *)space, 1);
		break;
	}
}


/**
   @brief	文字列編集タスク

   @param[in]	shell	shellデータ構造体ポインタ
   @param	ch	編集文字データ

   @return	コマンド実行結果
*/
int task_shell(struct st_shell *shell, uchar ch)
{
	int rt;

	if(ch == ASCII_CTRL_C) {	// CTRL-C
		init_lineedit(&shell->comLine);
		tprintf("\n");
		print_prompt(shell);
	} else if(ch == ASCII_HT) {	// TAB
		// コマンド補完
		complement_command(shell);
	} else {
		switch(do_lineedit(&shell->comLine, ch)) {
		case LER_NOP:
			break;

		case LER_RETURN: // '\r'受信でコマンド実行
			save_history(&shell->his, shell->comLine.buf);
//			tprintf("\023");	// X Start
			rt = exec_shell_command(shell, shell->comLine.buf);
//			tprintf("\021");	// X Stop
			new_lineedit(&shell->comLine);
//			if(rt) {
//				return rt;
//			}
			print_prompt(shell);
			return rt;

		case LER_BACKLINE:
			back_history(&shell->his, &shell->comLine);
			break;

		case LER_NEXTLINE:
			foward_history(&shell->his, &shell->comLine);
			break;
		}
	}

	return 0;
}

/**
   @brief	文字列をshellで処理可能なようにエスケープシーケンスを追加する

   @param	dsrt	変換後文字列
   @param	ssrt	変換前文字列

   @return	変換後文字数
*/
int escaped_str(uchar *dstr, uchar *sstr)
{
	int len = 0;
	//char *dp = dstr;
	//char *sp = sstr;

	while((*sstr) != 0) {
		if((*sstr == ' ') || (*sstr == '\\')) {
			*dstr = (uchar)'\\';
			dstr ++;
			len ++;
		}
		*dstr = *sstr;
		sstr ++;
		dstr ++;
		len ++;
	}
	*dstr = 0;

	//xdump(sp, len);
	//xdump(dp, len);

	return len;
}
