/*
 *  Project   : tin - a Usenet reader
 *  Module    : keymap.h
 *  Author    : J. Faultless, D. Nimmich
 *  Created   : 1999
 *  Updated   : 2005-10-19
 *  Notes     :
 *
 * Copyright (c) 1999-2006 Jason Faultless <jason@altarstone.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef KEYMAP_H
#	define KEYMAP_H 1

/* Revised 9 October 1996 by Branden Robinson in ASCII order
 *
 *        Oct   Dec   Hex   Char           Oct   Dec   Hex   Char
 *        ------------------------------------------------------------
 *        000   0     00    NUL '\0'       100   64    40    @
 *        001   1     01    SOH      ^A    101   65    41    A
 *        002   2     02    STX      ^B    102   66    42    B
 *        003   3     03    ETX      ^C    103   67    43    C
 *        004   4     04    EOT      ^D    104   68    44    D
 *        005   5     05    ENQ      ^E    105   69    45    E
 *        006   6     06    ACK      ^F    106   70    46    F
 *        007   7     07    BEL '\a' ^G    107   71    47    G
 *        010   8     08    BS  '\b' ^H    110   72    48    H
 *        011   9     09    HT  '\t' ^I    111   73    49    I
 *        012   10    0A    LF  '\n' ^J    112   74    4A    J
 *        013   11    0B    VT  '\v' ^K    113   75    4B    K
 *        014   12    0C    FF  '\f' ^L    114   76    4C    L
 *        015   13    0D    CR  '\r' ^M    115   77    4D    M
 *        016   14    0E    SO       ^N    116   78    4E    N
 *        017   15    0F    SI       ^O    117   79    4F    O
 *        020   16    10    DLE      ^P    120   80    50    P
 *        021   17    11    DC1      ^Q    121   81    51    Q
 *        022   18    12    DC2      ^R    122   82    52    R
 *        023   19    13    DC3      ^S    123   83    53    S
 *        024   20    14    DC4      ^T    124   84    54    T
 *        025   21    15    NAK      ^U    125   85    55    U
 *        026   22    16    SYN      ^V    126   86    56    V
 *        027   23    17    ETB      ^W    127   87    57    W
 *        030   24    18    CAN      ^X    130   88    58    X
 *        031   25    19    EM       ^Y    131   89    59    Y
 *        032   26    1A    SUB      ^Z    132   90    5A    Z
 *        033   27    1B    ESC            133   91    5B    [
 *        034   28    1C    FS             134   92    5C    \   '\\'
 *        035   29    1D    GS             135   93    5D    ]
 *        036   30    1E    RS             136   94    5E    ^
 *        037   31    1F    US             137   95    5F    _
 *        040   32    20    SPACE          140   96    60    `
 *        041   33    21    !              141   97    61    a
 *        042   34    22    "              142   98    62    b
 *        043   35    23    #              143   99    63    c
 *        044   36    24    $              144   100   64    d
 *        045   37    25    %              145   101   65    e
 *        046   38    26    &              146   102   66    f
 *        047   39    27    '              147   103   67    g
 *        050   40    28    (              150   104   68    h
 *        051   41    29    )              151   105   69    i
 *        052   42    2A    *              152   106   6A    j
 *        053   43    2B    +              153   107   6B    k
 *        054   44    2C    ,              154   108   6C    l
 *        055   45    2D    -              155   109   6D    m
 *        056   46    2E    .              156   110   6E    n
 *        057   47    2F    /              157   111   6F    o
 *        060   48    30    0              160   112   70    p
 *        061   49    31    1              161   113   71    q
 *        062   50    32    2              162   114   72    r
 *        063   51    33    3              163   115   73    s
 *        064   52    34    4              164   116   74    t
 *        065   53    35    5              165   117   75    u
 *        066   54    36    6              166   118   76    v
 *        067   55    37    7              167   119   77    w
 *        070   56    38    8              170   120   78    x
 *        071   57    39    9              171   121   79    y
 *        072   58    3A    :              172   122   7A    z
 *        073   59    3B    ;              173   123   7B    {
 *        074   60    3C    <              174   124   7C    |
 *        075   61    3D    =              175   125   7D    }
 *        076   62    3E    >              176   126   7E    ~
 *        077   63    3F    ?              177   127   7F    DEL
 *
 * Above chart reprinted from Linux manual page.
 *
 * When adding key functionality, be aware of key functions in the "big five"
 * levels of tin operation: top (group selection), group, thread, article
 * (pager), and help.  If possible, when adding a key to any of these levels,
 * check the others to make sure that the key doesn't do something
 * non-analogous elsewhere.  For instance, having "^R" map to "redraw screen"
 * at article level and "reset .newsrc" (a drastic and unreversible action)
 * at top level is a bad idea.
 *
 * [make emacs happy: "]
 */

/*
 * Maximum chars (including null byte) needed to print a key name
 * A multibyte character can use up to MB_CUR_MAX chars. But as MB_CUR_MAX
 * can't be used here, use MB_LEN_MAX instead.
 * Some values for MB_LEN_MAX:
 * - glibc 2.3.5: 16
 * - gcc 4.0: 1
 * - icc 8.0/9.0: 8
 * Use the largest + 1 to be on the safe side.
 */
#define MAXKEYLEN 17

/* TODO: permanently move here from tin.h */
#define ctrl(c)	((c) & 0x1F)
#define ESC		27

/* TODO: get rid of these remaining #define */
#define iKeyAbort ESC
#define iKeyQuitTin 'Q'
#define iKeyQuit 'q'

/* complete list of functions which can be associated with keys */
enum defined_functions {
	NOT_ASSIGNED = 0,
	DIGIT_0,
	DIGIT_1,
	DIGIT_2,
	DIGIT_3,
	DIGIT_4,
	DIGIT_5,
	DIGIT_6,
	DIGIT_7,
	DIGIT_8,
	DIGIT_9,
	SPECIAL_CATCHUP_LEFT,
	SPECIAL_MOUSE_TOGGLE,
	CATCHUP,
	CATCHUP_NEXT_UNREAD,
	CONFIG_SELECT,
	CONFIG_NO_SAVE,
	FEED_ARTICLE,
	FEED_THREAD,
	FEED_HOT,
	FEED_PATTERN,
	FEED_TAGGED,
	FEED_KEY_REPOST,
	FEED_SUPERSEDE,
	FILTER_EDIT,
	FILTER_SAVE,
	GLOBAL_ABORT,
	GLOBAL_BUGREPORT,
	GLOBAL_DISPLAY_POST_HISTORY,
	GLOBAL_EDIT_FILTER,
	GLOBAL_FIRST_PAGE,
	GLOBAL_HELP,
	GLOBAL_LAST_PAGE,
	GLOBAL_LAST_VIEWED,
	GLOBAL_LINE_DOWN,
	GLOBAL_LINE_UP,
	GLOBAL_LOOKUP_MESSAGEID,
	GLOBAL_MENU_FILTER_KILL,
	GLOBAL_MENU_FILTER_SELECT,
	GLOBAL_OPTION_MENU,
	GLOBAL_PAGE_DOWN,
	GLOBAL_PAGE_UP,
	GLOBAL_PIPE,
	GLOBAL_POST,
	GLOBAL_POSTPONED,
#ifndef DISABLE_PRINTING
	GLOBAL_PRINT,
#endif /* !DISABLE_PRINTING */
	GLOBAL_QUICK_FILTER_KILL,
	GLOBAL_QUICK_FILTER_SELECT,
	GLOBAL_QUIT,
	GLOBAL_QUIT_TIN,
	GLOBAL_REDRAW_SCREEN,
	GLOBAL_SCROLL_DOWN,
	GLOBAL_SCROLL_UP,
	GLOBAL_SEARCH_BODY,
	GLOBAL_SEARCH_REPEAT,
	GLOBAL_SEARCH_AUTHOR_BACKWARD,
	GLOBAL_SEARCH_AUTHOR_FORWARD,
	GLOBAL_SEARCH_SUBJECT_BACKWARD,
	GLOBAL_SEARCH_SUBJECT_FORWARD,
	GLOBAL_SET_RANGE,
#ifndef NO_SHELL_ESCAPE
	GLOBAL_SHELL_ESCAPE,
#endif /* NO_SHELL_ESCAPE */
#ifdef HAVE_COLOR
	GLOBAL_TOGGLE_COLOR,
#endif /* HAVE_COLOR */
	GLOBAL_TOGGLE_HELP_DISPLAY,
	GLOBAL_TOGGLE_INFO_LAST_LINE,
	GLOBAL_TOGGLE_INVERSE_VIDEO,
	GLOBAL_VERSION,
	GROUP_AUTOSAVE,
	GROUP_DO_AUTOSELECT,
	GROUP_GOTO,
	GROUP_LIST_THREAD,
	GROUP_MAIL,
	GROUP_MARK_THREAD_READ,
	GROUP_MARK_UNSELECTED_ARTICLES_READ,
	GROUP_NEXT_GROUP,
	GROUP_NEXT_UNREAD_ARTICLE,
	GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP,
	GROUP_PREVIOUS_GROUP,
	GROUP_PREVIOUS_UNREAD_ARTICLE,
	GROUP_READ_BASENOTE,
	GROUP_REPOST,
	GROUP_REVERSE_SELECTIONS,
	GROUP_SAVE,
	GROUP_SELECT_PATTERN,
	GROUP_SELECT_THREAD,
	GROUP_SELECT_THREAD_IF_UNREAD_SELECTED,
	GROUP_TAG,
	GROUP_TAG_PARTS,
	GROUP_TOGGLE_GET_ARTICLES_LIMIT,
	GROUP_TOGGLE_READ_UNREAD,
	GROUP_TOGGLE_SUBJECT_DISPLAY,
	GROUP_TOGGLE_SELECT_THREAD,
	GROUP_TOGGLE_THREADING,
	GROUP_UNDO_SELECTIONS,
	GROUP_UNTAG,
	MARK_ARTICLE_UNREAD,
	MARK_READ_CURRENT,
	MARK_READ_TAGGED,
	MARK_THREAD_UNREAD,
	PAGE_AUTOSAVE,
	PAGE_BOTTOM_THREAD,
	PAGE_CANCEL,
	PAGE_EDIT_ARTICLE,
	PAGE_FOLLOWUP,
	PAGE_FOLLOWUP_QUOTE,
	PAGE_FOLLOWUP_QUOTE_HEADERS,
	PAGE_GOTO_PARENT,
	PAGE_GROUP_SELECT,
	PAGE_LIST_THREAD,
	PAGE_MAIL,
	PAGE_MARK_THREAD_READ,
	PAGE_NEXT_ARTICLE,
	PAGE_NEXT_THREAD,
	PAGE_NEXT_UNREAD,
	PAGE_NEXT_UNREAD_ARTICLE,
	PAGE_PAGE_DOWN3,	/* special handling at page.c */
#ifdef HAVE_PGP_GPG
	PAGE_PGP_CHECK_ARTICLE,
#endif /* HAVE_PGP_GPG */
	PAGE_PREVIOUS_ARTICLE,
	PAGE_PREVIOUS_UNREAD_ARTICLE,
	PAGE_REVEAL,
	PAGE_REPLY,
	PAGE_REPLY_QUOTE,
	PAGE_REPLY_QUOTE_HEADERS,
	PAGE_REPOST,
	PAGE_SAVE,
	PAGE_SKIP_INCLUDED_TEXT,
	PAGE_TAG,
	PAGE_TOGGLE_HEADERS,
	PAGE_TOGGLE_HIGHLIGHTING,
	PAGE_TOGGLE_ROT13,
	PAGE_TOGGLE_TABS,
	PAGE_TOGGLE_TEX2ISO,
	PAGE_TOGGLE_UUE,
	PAGE_TOP_THREAD,
	PAGE_VIEW_ATTACHMENTS,
	PAGE_VIEW_URL,
	PGP_KEY_ENCRYPT,
	PGP_KEY_ENCRYPT_SIGN,
	PGP_INCLUDE_KEY,
	PGP_KEY_SIGN,
	POST_ABORT,
	POST_CANCEL,
	POST_CONTINUE,
	POST_EDIT,
	POST_IGNORE_FUPTO,
#ifdef HAVE_ISPELL
	POST_ISPELL,
#endif /* HAVE_ISPELL */
	POST_MAIL,
#ifdef HAVE_PGP_GPG
	POST_PGP,
#endif /* HAVE_PGP_GPG */
	POST_POSTPONE,
	POST_SEND,
	POST_SUPERSEDE,
	POSTPONE_ALL,
	POSTPONE_OVERRIDE,
	POSTPROCESS_NO,
	POSTPROCESS_SHAR,
	POSTPROCESS_YES,
	PROMPT_NO,
	PROMPT_YES,
	SAVE_APPEND_FILE,
	SAVE_OVERWRITE_FILE,
	SELECT_ENTER_GROUP,
	SELECT_ENTER_NEXT_UNREAD_GROUP,
	SELECT_GOTO,
	SELECT_MARK_GROUP_UNREAD,
	SELECT_MOVE_GROUP,
	SELECT_NEXT_UNREAD_GROUP,
	SELECT_RESET_NEWSRC,
	SELECT_SORT_ACTIVE,
	SELECT_SUBSCRIBE,
	SELECT_SUBSCRIBE_PATTERN,
	SELECT_SYNC_WITH_ACTIVE,
	SELECT_TOGGLE_DESCRIPTIONS,
	SELECT_TOGGLE_READ_DISPLAY,
	SELECT_UNSUBSCRIBE,
	SELECT_UNSUBSCRIBE_PATTERN,
	SELECT_QUIT_NO_WRITE,
	SELECT_YANK_ACTIVE,
	THREAD_AUTOSAVE,
	THREAD_MAIL,
	THREAD_MARK_ARTICLE_READ,
	THREAD_READ_NEXT_ARTICLE_OR_THREAD,
	THREAD_READ_ARTICLE,
	THREAD_REVERSE_SELECTIONS,
	THREAD_SAVE,
	THREAD_SELECT_ARTICLE,
	THREAD_TAG,
	THREAD_TOGGLE_ARTICLE_SELECTION,
	THREAD_TOGGLE_SUBJECT_DISPLAY,
	THREAD_UNDO_SELECTIONS,
	THREAD_UNTAG
};
typedef enum defined_functions t_function;


struct keynode {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t key;
#else
	char key;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_function function;
};


struct keylist {
	struct keynode *list;
	size_t used;
	size_t max;
};


extern struct keylist feed_post_process_keys;
extern struct keylist feed_supersede_article_keys;
extern struct keylist feed_type_keys;
extern struct keylist filter_keys;
extern struct keylist group_keys;
extern struct keylist info_keys;
extern struct keylist mark_read_keys;
extern struct keylist option_menu_keys;
extern struct keylist page_keys;
#ifdef HAVE_PGP_GPG
	extern struct keylist pgp_mail_keys;
	extern struct keylist pgp_news_keys;
#endif /* HAVE_PGP_GPG */
extern struct keylist post_cancel_keys;
extern struct keylist post_continue_keys;
extern struct keylist post_delete_keys;
extern struct keylist post_edit_keys;
extern struct keylist post_edit_ext_keys;
extern struct keylist post_ignore_fupto_keys;
extern struct keylist post_mail_fup_keys;
extern struct keylist post_post_keys;
extern struct keylist post_postpone_keys;
extern struct keylist post_send_keys;
extern struct keylist prompt_keys;
extern struct keylist save_append_overwrite_keys;
extern struct keylist select_keys;
extern struct keylist thread_keys;


extern t_function global_mouse_action(t_function (*left_action) (void), t_function (*right_action) (void));
extern t_function handle_keypad(
	t_function (*left_action) (void),
	t_function (*right_action) (void),
	t_function (*mouse_action) (
		t_function (*left_action) (void),
		t_function (*right_action) (void)),
	const struct keylist keys);
extern t_function prompt_slk_response(t_function default_func, const struct keylist keys, const char *fmt, ...);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern char *printascii(char *buf, wint_t ch);
	extern wchar_t func_to_key(t_function func, const struct keylist keys);
	extern t_function key_to_func(const wchar_t key, const struct keylist keys);
#else
	extern char *printascii(char *buf, int ch);
	extern char func_to_key (t_function func, const struct keylist keys);
	extern t_function key_to_func (const char key, const struct keylist keys);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#endif /* !KEYMAP_H */
