/*
 *  Project   : tin - a Usenet reader
 *  Module    : keymap.h
 *  Author    : J. Faultless, D. Nimmich
 *  Created   : 1999
 *  Updated   : 2000-06-10
 *  Notes     :
 *
 * Copyright (c) 1999-2000 Jason Faultless <jason@radar.tele2.co.uk>
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

#	ifndef MENUKEYS_H
#		include "menukeys.h"
#  endif /* !MENUKEYS_H */

/* TODO permanently move here from tin.h */
#define ctrl(c)	((c) & 0x1F)
#define ESC		27

/*
 * Global keys
 */
struct k_global {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode PageUp;						/* ctrl('B') */
	t_keynode PageDown;					/* ctrl('D') */
	t_keynode PageDown2;					/* ctrl('F') */
	t_keynode RedrawScr;					/* ctrl('L') */
	t_keynode Down;						/* ctrl('N') */
	t_keynode Postponed;					/* ctrl('O') */
	t_keynode Up;							/* ctrl('P') */
	t_keynode PageUp2;					/* ctrl('U') */
	t_keynode Abort;						/* ESCAPE */
	t_keynode PageDown3;					/* ' ' */
#ifndef NO_SHELL_ESCAPE
	t_keynode ShellEscape;				/* '!' */
#endif /* !NO_SHELL_ESCAPE */
	t_keynode SetRange;					/* '#' */
	t_keynode LastPage;					/* '$' */
#ifdef HAVE_COLOR
	t_keynode ToggleColor;				/* '&' */
#endif /* HAVE_COLOR */
	t_keynode LastViewed;				/* '-' */
	t_keynode SearchSubjF;				/* '/' */
	t_keynode Zero;
	t_keynode One;
	t_keynode Two;
	t_keynode Three;
	t_keynode Four;
	t_keynode Five;
	t_keynode Six;
	t_keynode Seven;
	t_keynode Eight;
	t_keynode Nine;
	t_keynode SearchSubjB;				/* '?' */
	t_keynode SearchAuthB;				/* 'A' */
	t_keynode SearchBody;				/* 'B' */
	t_keynode ToggleHelpDisplay;		/* 'H' */
	t_keynode ToggleInverseVideo;		/* 'I' */
	t_keynode LookupMessage;			/* 'L' */
	t_keynode OptionMenu;				/* 'M' */
	t_keynode Postponed2;				/* 'O' */
	t_keynode QuitTin;					/* 'Q' */
	t_keynode DisplayPostHist;			/* 'W' */
	t_keynode FirstPage;					/* '^' */
	t_keynode SearchAuthF;				/* 'a' */
	t_keynode PageUp3;					/* 'b' */
	t_keynode Help;						/* 'h' */
	t_keynode ToggleInfoLastLine;		/* 'i' */
	t_keynode Down2;						/* 'j' */
	t_keynode Up2;							/* 'k' */
	t_keynode Quit;						/* 'q' */
	t_keynode Version;					/* 'v' */
	t_keynode Post;						/* 'w' */
	t_keynode Pipe;						/* '|' */
	t_keynode CatchupLeft;				/* special, for internal use only */
	t_keynode MouseToggle;				/* special, for internal use only */
	t_keynode null;						/* End of group */
};

struct k_config {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode FirstPage2;
	t_keynode LastPage2;
	t_keynode NoSave;
	t_keynode Select;
	t_keynode Select2;
	t_keynode null;
};

struct k_feed {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode Art;
	t_keynode Hot;
	t_keynode Pat;
	t_keynode Repost;
	t_keynode Supersede;
	t_keynode Tag;
	t_keynode Thd;
	t_keynode null;
};

struct k_filter {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode Edit;
	t_keynode Save;
	t_keynode null;
};

struct k_group {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode AutoSel;					/* ctrl('A') */
	t_keynode NextUnreadArtOrGrp;		/* '\t' */
	t_keynode ReadBasenote;				/* '\n' */
	t_keynode Kill;						/* ctrl('K') */
	t_keynode ReadBasenote2;			/* '\r' */
	t_keynode SelThd;						/* '*' */
	t_keynode DoAutoSel;					/* '+' */
	t_keynode ToggleThdSel;				/* '.' */
	t_keynode SelThdIfUnreadSelected;	/* ';' */
	t_keynode SelPattern;				/* '=' */
	t_keynode ReverseSel;				/* '@' */
	t_keynode CatchupNextUnread;		/* 'C' */
	t_keynode EditFilter;				/* 'E' */
	t_keynode ToggleGetartLimit;		/* 'G' */
	t_keynode MarkThdRead;				/* 'K' */
	t_keynode NextUnreadArt;			/* 'N' */
	t_keynode PrevUnreadArt;			/* 'P' */
	t_keynode BugReport;					/* 'R' */
	t_keynode AutoSaveTagged;			/* 'S' */
	t_keynode TagParts;					/* 'T' */
	t_keynode Untag;						/* 'U' */
	t_keynode MarkUnselArtRead;		/* 'X' */
	t_keynode MarkThdUnread;			/* 'Z' */
	t_keynode QuickAutoSel;				/* '[' */
	t_keynode QuickKill;					/* ']' */
	t_keynode Catchup;					/* 'c' */
	t_keynode ToggleSubjDisplay;		/* 'd' */
	t_keynode Goto;						/* 'g' */
	t_keynode ListThd;					/* 'l' */
	t_keynode Mail;						/* 'm' */
	t_keynode NextGroup;					/* 'n' */
#ifndef DISABLE_PRINTING
	t_keynode Print;						/* 'o' */
#endif /* !DISABLE_PRINTING */
	t_keynode PrevGroup;					/* 'p' */
	t_keynode ToggleReadUnread;		/* 'r' */
	t_keynode Save;						/* 's' */
	t_keynode Tag;							/* 't' */
	t_keynode ToggleThreading;			/* 'u' */
	t_keynode Repost;						/* 'x' */
	t_keynode MarkArtUnread;			/* 'z' */
	t_keynode UndoSel;					/* '~' */
	t_keynode null;						/* End of group */
};

struct k_help {
	t_keynode tag;
	t_keynode FirstPage2;
	t_keynode LastPage2;
	t_keynode null;
};

struct k_nrctbl {
	t_keynode tag;
	t_keynode Alternative;
	t_keynode Create;
	t_keynode Default;
	t_keynode Quit;
	t_keynode null;
};

struct k_page {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode AutoSel;					/* ctrl('A') */
	t_keynode ReplyQuoteHeaders;		/* ctrl('E') */
#ifdef HAVE_PGP_GPG
	t_keynode PGPCheckArticle;			/* ctrl('G') */
#endif /* HAVE_PGP_GPG */
	t_keynode ToggleHeaders;			/* ctrl('H') */
	t_keynode NextUnread;				/* '\t' */
	t_keynode NextThd;					/* '\n' */
	t_keynode AutoKill;					/* ctrl('K') */
	t_keynode NextThd2;					/* '\r' */
	t_keynode ToggleTabs;				/* ctrl('T') */
	t_keynode FollowupQuoteHeaders;	/* ctrl('W') */
	t_keynode ToggleTex2iso;			/* '\"' */
	t_keynode ToggleRot;					/* '%' */
	t_keynode ToggleUue;					/* '(' */
	t_keynode Reveal;						/* ')' */
	t_keynode SkipIncludedText;		/* ':' */
	t_keynode TopThd;						/* '<' */
	t_keynode BotThd;						/* '>' */
	t_keynode CatchupNextUnread;		/* 'C' */
	t_keynode Cancel;						/* 'D' */
	t_keynode EditFilter;				/* 'E' */
	t_keynode Followup;					/* 'F' */
	t_keynode LastPage2;					/* 'G' */
	t_keynode KillThd;					/* 'K' */
	t_keynode NextUnreadArt;			/* 'N' */
	t_keynode PrevUnreadArt;			/* 'P' */
	t_keynode Reply;						/* 'R' */
	t_keynode AutoSaveTagged;			/* 'S' */
	t_keynode GroupSel;					/* 'T' */
	t_keynode ViewUrl;					/* 'U' */
	t_keynode ViewAttach;				/* 'V' */
	t_keynode MarkThdUnread;			/* 'Z' */
	t_keynode QuickAutoSel;				/* '[' */
	t_keynode QuickKill;					/* ']' */
#ifdef HAVE_COLOR
	t_keynode ToggleHighlight;			/* '_' */
#endif /* HAVE_COLOR */
	t_keynode Catchup;					/* 'c' */
	t_keynode EditArticle;				/* 'e' */
	t_keynode FollowupQuote;			/* 'f' */
	t_keynode FirstPage2;				/* 'g' */
	t_keynode ListThd;					/* 'l' */
	t_keynode Mail;						/* 'm' */
	t_keynode NextArt;					/* 'n' */
#ifndef DISABLE_PRINTING
	t_keynode Print;						/* 'o' */
#endif /* !DISABLE_PRINTING */
	t_keynode PrevArt;					/* 'p' */
	t_keynode ReplyQuote;				/* 'r' */
	t_keynode Save;						/* 's' */
	t_keynode Tag;							/* 't' */
	t_keynode GotoParent;				/* 'u' */
	t_keynode Repost;						/* 'x' */
	t_keynode MarkArtUnread;			/* 'z' */
	t_keynode null;						/* End of group */
};

struct k_pgp {
	t_keynode tag;
	t_keynode EncSign;
	t_keynode Encrypt;
	t_keynode Includekey;
	t_keynode Sign;
	t_keynode null;
};

struct k_post {
	t_keynode tag;
	t_keynode Cancel;
	t_keynode Edit;
#ifdef HAVE_PGP_GPG
	t_keynode PGP;
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
	t_keynode Ispell;
#endif /* HAVE_ISPELL */
	t_keynode Abort;
	t_keynode Continue;
	t_keynode Ignore;
	t_keynode Mail;
	t_keynode Post2;
	t_keynode Post3;
	t_keynode Postpone;
	t_keynode Send;
	t_keynode Send2;
	t_keynode Supersede;
	t_keynode null;
};

struct k_postpone {
	t_keynode tag;
	t_keynode All;
	t_keynode Override;
	t_keynode null;
};

struct k_pproc {
	t_keynode tag;
	t_keynode None;
	t_keynode Shar;
	t_keynode UUDecode;
	t_keynode null;
};

struct k_prompt {
	t_keynode tag;
	t_keynode No;
	t_keynode Yes;
	t_keynode null;
};

struct k_save {
	t_keynode tag;
	t_keynode AppendFile;
	t_keynode OverwriteFile;
	t_keynode null;
};

struct k_select {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode EnterNextUnreadGrp;		/* '\t' */
	t_keynode ReadGrp;					/* '\n' */
	t_keynode ReadGrp2;					/* '\r' */
	t_keynode ResetNewsrc;				/* ctrl('R') */
	t_keynode CatchupNextUnread;		/* 'C' */
	t_keynode NextUnreadGrp;			/* 'N' */
	t_keynode BugReport;					/* 'R' */
	t_keynode SubscribePat;				/* 'S' */
	t_keynode UnsubscribePat;			/* 'U' */
	t_keynode QuitNoWrite;				/* 'X' */
	t_keynode SyncWithActive;			/* 'Y' */
	t_keynode MarkGrpUnread2;			/* 'Z' */
	t_keynode Catchup;					/* 'c' */
	t_keynode ToggleDescriptions;		/* 'd' */
	t_keynode Goto;						/* 'g' */
	t_keynode MoveGrp;					/* 'm' */
	t_keynode EnterNextUnreadGrp2;	/* 'n' */
	t_keynode ToggleReadDisplay;		/* 'r' */
	t_keynode Subscribe;					/* 's' */
	t_keynode Unsubscribe;				/* 'u' */
	t_keynode YankActive;				/* 'y' */
	t_keynode MarkGrpUnread;			/* 'z' */
	t_keynode null;						/* End of group */
};

struct k_thread {
	t_keynode tag;							/* Stores name of this keygroup */
	t_keynode ReadNextArtOrThread;	/* '\t' */
	t_keynode ReadArt;					/* '\n' */
	t_keynode ReadArt2;					/* '\r' */
	t_keynode SelArt;						/* '*' */
	t_keynode ToggleArtSel;				/* '.' */
	t_keynode ReverseSel;				/* '@' */
	t_keynode CatchupNextUnread;		/* 'C' */
	t_keynode MarkArtRead;				/* 'K' */
	t_keynode BugReport;					/* 'R' */
	t_keynode AutoSaveTagged;			/* 'S' */
	t_keynode Untag;						/* 'U' */
	t_keynode MarkThdUnread;			/* 'Z' */
	t_keynode Catchup;					/* 'c' */
	t_keynode ToggleSubjDisplay;		/* 'd' */
	t_keynode Mail;						/* 'm' */
	t_keynode Save;						/* 's' */
	t_keynode Tag;							/* 't' */
	t_keynode MarkArtUnread;			/* 'z' */
	t_keynode UndoSel;					/* '~' */
	t_keynode null;						/* End of group */
};

struct keymap {
	struct k_global Global;
	struct k_config Config;
	struct k_feed Feed;
	struct k_filter Filter;
	struct k_group Group;
	struct k_help Help;
	struct k_nrctbl Nrctbl;
	struct k_page Page;
	struct k_pgp Pgp;
	struct k_post Post;
	struct k_postpone Postpone;
	struct k_pproc PProc;
	struct k_prompt Prompt;
	struct k_save Save;
	struct k_select Select;
	struct k_thread Thread;
};

#endif /* !KEYMAP_H */
