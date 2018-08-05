/** @file
    @brief	コマンドシェル

    @date	2007.03.16
    @date	2002.03.24
    @author	Takashi SHUDO
*/

#ifndef	SHELL_H
#define	SHELL_H

#include "sysconfig.h"
#include "str.h"
#include "lineedit.h"
#include "history.h"
#include "console.h"

#ifndef GSC_SHELL_MAX_COM_ARGV
#define GSC_SHELL_MAX_COM_ARGV	10	// $gsc shellコマンド最大引数の数
#endif

struct st_file_operation {
	uchar ext[4];	///< 拡張子文字列
	int (* operation)(uchar *fname, uchar *arg);	/// ファイル実行関数
}; ///< ファイル種別に対するコマンド実行定義

// comList.attr 実行後のリターンのみ入力時の動作
#define CMDATTR_CONT	0x0001	// 同じコマンドを実行
#define CMDATTR_ARGLESS	0x0002	// 引数は省略

#define CMDATTR_CTAL	(CMDATTR_CONT|CMDATTR_ARGLESS)

struct st_shell_command {
	uchar name[12];		///< コマンド名文字列
	void (*init)(void);	///< コマンド初期化関数
	int (*command)(int argc, uchar *argv[]);	///< コマンド実行関数
	unsigned short attr;	///< コマンド属性
	char *usage_str;	///< 使用法文字列
	char *manual_str;	///< マニュアル文字列
	const struct st_shell_command * const * sublist;	///< サブコマンド配列
}; ///< シェルコマンド構造体

struct st_shell {
	struct st_lineedit	comLine;	///< コマンドライン
	uchar lastCom[GSC_SHELL_MAX_LINE_COLUMS+1];	///< 最後に実行したコマンド
	struct st_history his;	///< コマンドヒストリ
	const uchar *prompt;	///< コマンドプロンプト文字列
	int argc;		///< コマンド引数数
	uchar *argv[GSC_SHELL_MAX_COM_ARGV];	///< コマンド引数文字列
	struct st_shell_command * const * shell_coms; ///< 各シェルコマンド配列
}; ///< シェルデータ構造体

extern void init_shell(struct st_shell *shell, struct st_shell_command * const *coms, const uchar *prompt);
extern void print_prompt(struct st_shell *shell);
extern void dispose_shell_line(struct st_shell *shell);
extern int exec_shell_command(struct st_shell *shell, uchar *str);
extern void print_command_usage(const struct st_shell_command *command);
extern int task_shell(struct st_shell *shell, uchar ch);
extern void startup_shell(void);
extern int add_shell_command(struct st_shell_command *command);

#ifdef GSC_COMP_ENABLE_SHELL
extern int exec_command(uchar *str);
extern int escaped_str(uchar *dstr, uchar *sstr);
extern void set_file_operation(const struct st_file_operation * const fileop[]);
extern int do_file_operation(uchar *str, uchar *arg);
#endif

#endif	// SHELL_H
