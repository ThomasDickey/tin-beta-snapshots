/*
 *  Project   : tin - a Usenet reader
 *  Module    : keymap.c
 *  Author    : D. Nimmich, J. Faultless
 *  Created   : 2000-05-25
 *  Updated   : 2000-06-10
 *  Notes     : This file contains key mapping routines and variables.
 *
 * Copyright (c) 2000 Dirk Nimmich <nimmich@uni-muenster.de>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
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

#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef KEYMAP_H
#	include "keymap.h"
#endif /* !KEYMAP_H */
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */

static int keymapsize (t_keynode *ptr[]);
static t_bool check_duplicates(t_keynode *keyptr1, t_keynode *keyptr2);
static t_bool processkey(t_keynode *keyptr, char *kname, char key);

char *ch_post_process;

struct keymap Key = {
	{	/* Global keys */
		{ 0, 0, "" },
		{ iKeyPageUp, iKeyPageUp, "PageUp" },
		{ iKeyPageDown, iKeyPageDown, "PageDown" },
		{ iKeyPageDown2, iKeyPageDown2, "PageDown2" },
		{ iKeyRedrawScr, iKeyRedrawScr, "RedrawScr" },
		{ iKeyDown, iKeyDown, "Down" },
		{ iKeyPostponed, iKeyPostponed, "Postponed" },
		{ iKeyUp, iKeyUp, "Up" },
		{ iKeyPageUp2, iKeyPageUp2, "PageUp2" },
		{ iKeyAbort, iKeyAbort, "" },			/* Was "Abort", but we don't allow rebinding the <ESC> key */
		{ iKeyPageDown3, iKeyPageDown3, "PageDown3" },
#ifndef NO_SHELL_ESCAPE
		{ iKeyShellEscape, iKeyShellEscape, "ShellEscape" },
#endif /* !NO_SHELL_ESCAPE */
		{ iKeySetRange, iKeySetRange, "SetRange" },
		{ iKeyLastPage, iKeyLastPage, "LastPage" },
#ifdef HAVE_COLOR
		{ iKeyToggleColor, iKeyToggleColor, "ToggleColor" },
#endif /* HAVE_COLOR */
		{ iKeyLastViewed, iKeyLastViewed, "LastViewed" },
		{ iKeySearchSubjF, iKeySearchSubjF, "SearchSubjF" },
		{ '0', '0', "" },	/* We don't allow redefinition of digits */
		{ '1', '1', "" },
		{ '2', '2', "" },
		{ '3', '3', "" },
		{ '4', '4', "" },
		{ '5', '5', "" },
		{ '6', '6', "" },
		{ '7', '7', "" },
		{ '8', '8', "" },
		{ '9', '9', "" },
		{ iKeySearchSubjB, iKeySearchSubjB, "SearchSubjB" },
		{ iKeySearchAuthB, iKeySearchAuthB, "SearchAuthB" },
		{ iKeySearchBody, iKeySearchBody, "SearchBody" },
		{ iKeyToggleHelpDisplay, iKeyToggleHelpDisplay, "ToggleHelpDisplay" },
		{ iKeyToggleInverseVideo, iKeyToggleInverseVideo, "ToggleInverseVideo" },
		{ iKeyLookupMessage, iKeyLookupMessage, "LookupMessage" },
		{ iKeyOptionMenu, iKeyOptionMenu, "OptionMenu" },
		{ iKeyPostponed2, iKeyPostponed2, "Postponed2" },
		{ iKeyQuitTin, iKeyQuitTin, "QuitTin" },
		{ iKeyDisplayPostHist, iKeyDisplayPostHist, "DisplayPostHist" },
		{ iKeyFirstPage, iKeyFirstPage, "FirstPage" },
		{ iKeySearchAuthF, iKeySearchAuthF, "SearchAuthF" },
		{ iKeyPageUp3, iKeyPageUp3, "PageUp3" },
		{ iKeyHelp, iKeyHelp, "Help" },
		{ iKeyToggleInfoLastLine, iKeyToggleInfoLastLine, "ToggleInfoLastLine" },
		{ iKeyDown2, iKeyDown2, "Down2" },
		{ iKeyUp2, iKeyUp2, "Up2" },
		{ iKeyQuit, iKeyQuit, "Quit" },
		{ iKeyVersion, iKeyVersion, "Version" },
		{ iKeyPost, iKeyPost, "Post" },
		{ iKeyPipe, iKeyPipe, "Pipe" },
		/*
		 * The following two are "internal" keys that don't have a real
		 * mapping.
		 */
		{ iKeyCatchupLeft, iKeyCatchupLeft, "" },
		{ iKeyMouseToggle, iKeyMouseToggle, "" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Config" },
		{ iKeyConfigFirstPage2, iKeyConfigFirstPage2, "FirstPage2" },
		{ iKeyConfigLastPage2, iKeyConfigLastPage2, "LastPage2" },
		{ iKeyConfigNoSave, iKeyConfigNoSave, "NoSave" },
		{ iKeyConfigSelect, iKeyConfigSelect, "Select" },
		{ iKeyConfigSelect2, iKeyConfigSelect2, "Select2" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Feed" },
		{ iKeyFeedArt, iKeyFeedArt, "Art" },
		{ iKeyFeedHot, iKeyFeedHot, "Hot" },
		{ iKeyFeedPat, iKeyFeedPat, "Pat" },
		{ iKeyFeedRepost, iKeyFeedRepost, "Repost" },
		{ iKeyFeedSupersede, iKeyFeedSupersede, "Supersede" },
		{ iKeyFeedTag, iKeyFeedTag, "Tag" },
		{ iKeyFeedThd, iKeyFeedThd, "Thd" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Filter" },
		{ iKeyFilterEdit, iKeyFilterEdit, "Edit" },
		{ iKeyFilterSave, iKeyFilterSave, "Save" },
		{ 0, 0, NULL }
	},
	{	/* Group keys */
		{ 0, 0, "Group" },
		{ iKeyGroupAutoSel, iKeyGroupAutoSel, "AutoSel" },
		{ iKeyGroupNextUnreadArtOrGrp, iKeyGroupNextUnreadArtOrGrp, "NextUnreadArtOrGrp" },
		{ iKeyGroupReadBasenote, iKeyGroupReadBasenote, "ReadBasenote" },
		{ iKeyGroupKill, iKeyGroupKill, "Kill" },
		{ iKeyGroupReadBasenote2, iKeyGroupReadBasenote2, "ReadBasenote2" },
		{ iKeyGroupSelThd, iKeyGroupSelThd, "SelThd" },
		{ iKeyGroupDoAutoSel, iKeyGroupDoAutoSel, "DoAutoSel" },
		{ iKeyGroupToggleThdSel, iKeyGroupToggleThdSel, "ToggleThdSel" },
		{ iKeyGroupSelThdIfUnreadSelected, iKeyGroupSelThdIfUnreadSelected, "SelThdIfUnreadSelected" },
		{ iKeyGroupSelPattern, iKeyGroupSelPattern, "SelPattern" },
		{ iKeyGroupReverseSel, iKeyGroupReverseSel, "ReverseSel" },
		{ iKeyGroupCatchupNextUnread, iKeyGroupCatchupNextUnread, "CatchupNextUnread" },
		{ iKeyGroupEditFilter, iKeyGroupEditFilter, "EditFilter" },
		{ iKeyGroupToggleGetartLimit, iKeyGroupToggleGetartLimit, "ToggleGetartLimit" },
		{ iKeyGroupMarkThdRead, iKeyGroupMarkThdRead, "MarkThdRead" },
		{ iKeyGroupNextUnreadArt, iKeyGroupNextUnreadArt, "NextUnreadArt" },
		{ iKeyGroupPrevUnreadArt, iKeyGroupPrevUnreadArt, "PrevUnreadArt" },
		{ iKeyGroupBugReport, iKeyGroupBugReport, "BugReport" },
		{ iKeyGroupAutoSaveTagged, iKeyGroupAutoSaveTagged, "AutoSaveTagged" },
		{ iKeyGroupTagParts, iKeyGroupTagParts, "TagParts" },
		{ iKeyGroupUntag, iKeyGroupUntag, "Untag" },
		{ iKeyGroupMarkUnselArtRead, iKeyGroupMarkUnselArtRead, "MarkUnselArtRead" },
		{ iKeyGroupMarkThdUnread, iKeyGroupMarkThdUnread, "MarkThdUnread" },
		{ iKeyGroupQuickAutoSel, iKeyGroupQuickAutoSel, "QuickAutoSel" },
		{ iKeyGroupQuickKill, iKeyGroupQuickKill, "QuickKill" },
		{ iKeyGroupCatchup, iKeyGroupCatchup, "Catchup" },
		{ iKeyGroupToggleSubjDisplay, iKeyGroupToggleSubjDisplay, "ToggleSubjDisplay" },
		{ iKeyGroupGoto, iKeyGroupGoto, "Goto" },
		{ iKeyGroupListThd, iKeyGroupListThd, "ListThd" },
		{ iKeyGroupMail, iKeyGroupMail, "Mail" },
		{ iKeyGroupNextGroup, iKeyGroupNextGroup, "NextGroup" },
#ifndef DISABLE_PRINTING
		{ iKeyGroupPrint, iKeyGroupPrint, "Print" },
#endif /* !DISABLE_PRINTING */
		{ iKeyGroupPrevGroup, iKeyGroupPrevGroup, "PrevGroup" },
		{ iKeyGroupToggleReadUnread, iKeyGroupToggleReadUnread, "ToggleReadUnread" },
		{ iKeyGroupSave, iKeyGroupSave, "Save" },
		{ iKeyGroupTag, iKeyGroupTag, "Tag" },
		{ iKeyGroupToggleThreading, iKeyGroupToggleThreading, "ToggleThreading" },
		{ iKeyGroupRepost, iKeyGroupRepost, "Repost" },
		{ iKeyGroupMarkArtUnread, iKeyGroupMarkArtUnread, "MarkArtUnread" },
		{ iKeyGroupUndoSel, iKeyGroupUndoSel, "UndoSel" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Help" },
		{ iKeyHelpFirstPage2, iKeyHelpFirstPage2, "FirstPage2" },
		{ iKeyHelpLastPage2, iKeyHelpLastPage2, "LastPage2" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Nrctbl" },
		{ iKeyNrctblAlternative, iKeyNrctblAlternative, "Alternative" },
		{ iKeyNrctblCreate, iKeyNrctblCreate, "Create" },
		{ iKeyNrctblDefault, iKeyNrctblDefault, "Default" },
		{ iKeyNrctblQuit, iKeyNrctblQuit, "Quit" },
		{ 0, 0, NULL }
	},
	{	/* Page keys */
		{ 0, 0, "Page" },
		{ iKeyPageAutoSel, iKeyPageAutoSel, "AutoSel" },
		{ iKeyPageReplyQuoteHeaders, iKeyPageReplyQuoteHeaders, "ReplyQuoteHeaders" },
#ifdef HAVE_PGP_GPG
		{ iKeyPagePGPCheckArticle, iKeyPagePGPCheckArticle, "PGPCheckArticle" },
#endif /* HAVE_PGP_GPG */
		{ iKeyPageToggleHeaders, iKeyPageToggleHeaders, "ToggleHeaders" },
		{ iKeyPageNextUnread, iKeyPageNextUnread, "NextUnread" },
		{ iKeyPageNextThd, iKeyPageNextThd, "NextThd" },
		{ iKeyPageAutoKill, iKeyPageAutoKill, "AutoKill" },
		{ iKeyPageNextThd2, iKeyPageNextThd2, "NextThd2" },
		{ iKeyPageToggleTabs, iKeyPageToggleTabs, "ToggleTabs" },
		{ iKeyPageFollowupQuoteHeaders, iKeyPageFollowupQuoteHeaders, "FollowupQuoteHeaders" },
		{ iKeyPageToggleTex2iso, iKeyPageToggleTex2iso, "ToggleTex2iso" },
		{ iKeyPageToggleRot, iKeyPageToggleRot, "ToggleRot" },
		{ iKeyPageToggleUue, iKeyPageToggleUue, "ToggleUue" },
		{ iKeyPageReveal, iKeyPageReveal, "Reveal" },
		{ iKeyPageSkipIncludedText, iKeyPageSkipIncludedText, "SkipIncludedText" },
		{ iKeyPageTopThd, iKeyPageTopThd, "TopThd" },
		{ iKeyPageBotThd, iKeyPageBotThd, "BotThd" },
		{ iKeyPageCatchupNextUnread, iKeyPageCatchupNextUnread, "CatchupNextUnread" },
		{ iKeyPageCancel, iKeyPageCancel, "Cancel" },
		{ iKeyPageEditFilter, iKeyPageEditFilter, "EditFilter" },
		{ iKeyPageFollowup, iKeyPageFollowup, "Followup" },
		{ iKeyPageLastPage2, iKeyPageLastPage2, "LastPage2" },
		{ iKeyPageKillThd, iKeyPageKillThd, "KillThd" },
		{ iKeyPageNextUnreadArt, iKeyPageNextUnreadArt, "NextUnreadArt" },
		{ iKeyPagePrevUnreadArt, iKeyPagePrevUnreadArt, "PrevUnreadArt" },
		{ iKeyPageReply, iKeyPageReply, "Reply" },
		{ iKeyPageAutoSaveTagged, iKeyPageAutoSaveTagged, "AutoSaveTagged" },
		{ iKeyPageGroupSel, iKeyPageGroupSel, "GroupSel" },
		{ iKeyPageViewUrl, iKeyPageViewUrl, "ViewUrl" },
		{ iKeyPageViewAttach, iKeyPageViewAttach, "ViewAttach" },
		{ iKeyPageMarkThdUnread, iKeyPageMarkThdUnread, "MarkThdUnread" },
		{ iKeyPageQuickAutoSel, iKeyPageQuickAutoSel, "QuickAutoSel" },
		{ iKeyPageQuickKill, iKeyPageQuickKill, "QuickKill" },
#ifdef HAVE_COLOR
		{ iKeyPageToggleHighlight, iKeyPageToggleHighlight, "ToggleHighlight" },
#endif /* HAVE_COLOR */
		{ iKeyPageCatchup, iKeyPageCatchup, "Catchup" },
		{ iKeyPageEditArticle, iKeyPageEditArticle, "EditArticle" },
		{ iKeyPageFollowupQuote, iKeyPageFollowupQuote, "FollowupQuote" },
		{ iKeyPageFirstPage2, iKeyPageFirstPage2, "FirstPage2" },
		{ iKeyPageListThd, iKeyPageListThd, "ListThd" },
		{ iKeyPageMail, iKeyPageMail, "Mail" },
		{ iKeyPageNextArt, iKeyPageNextArt, "NextArt" },
#ifndef DISABLE_PRINTING
		{ iKeyPagePrint, iKeyPagePrint, "Print" },
#endif /* !DISABLE_PRINTING */
		{ iKeyPagePrevArt, iKeyPagePrevArt, "PrevArt" },
		{ iKeyPageReplyQuote, iKeyPageReplyQuote, "ReplyQuote" },
		{ iKeyPageSave, iKeyPageSave, "Save" },
		{ iKeyPageTag, iKeyPageTag, "Tag" },
		{ iKeyPageGotoParent, iKeyPageGotoParent, "GotoParent" },
		{ iKeyPageRepost, iKeyPageRepost, "Repost" },
		{ iKeyPageMarkArtUnread, iKeyPageMarkArtUnread, "MarkArtUnread" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Pgp" },
		{ iKeyPgpEncSign, iKeyPgpEncSign, "EncSign" },
		{ iKeyPgpEncrypt, iKeyPgpEncrypt, "Encrypt" },
		{ iKeyPgpIncludekey, iKeyPgpIncludekey, "Includekey" },
		{ iKeyPgpSign, iKeyPgpSign, "Sign" },
		{ 0, 0, NULL }
	},
	{ /* Post keys */
		{ 0, 0, "Post" },
		{ iKeyPostCancel, iKeyPostCancel, "Cancel" },
		{ iKeyPostEdit, iKeyPostEdit, "Edit" },
#ifdef HAVE_PGP_GPG
		{ iKeyPostPGP, iKeyPostPGP, "PGP" },
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
		{ iKeyPostIspell, iKeyPostIspell, "Ispell" },
#endif /* HAVE_ISPELL */
		{ iKeyPostAbort, iKeyPostAbort, "Abort" },
		{ iKeyPostContinue, iKeyPostContinue, "Continue" },
		{ iKeyPostIgnore, iKeyPostIgnore, "Ignore" },
		{ iKeyPostMail, iKeyPostMail, "Mail" },
		{ iKeyPostPost2, iKeyPostPost2, "Post2" },
		{ iKeyPostPost3, iKeyPostPost3, "Post3" },
		{ iKeyPostPostpone, iKeyPostPostpone, "Postpone" },
		{ iKeyPostSend, iKeyPostSend, "Send" },
		{ iKeyPostSend2, iKeyPostSend2, "Send2" },
		{ iKeyPostSupersede, iKeyPostSupersede, "Supersede" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Postpone" },
		{ iKeyPostponeAll, iKeyPostponeAll, "All" },
		{ iKeyPostponeOverride, iKeyPostponeOverride, "Override" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "PProc" },
		{ iKeyPProcNone, iKeyPProcNone, "None" },
		{ iKeyPProcShar, iKeyPProcShar, "Shar" },
		{ iKeyPProcUUDecode, iKeyPProcUUDecode, "UUDecode" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Prompt" },
		{ iKeyPromptNo, iKeyPromptNo, "No" },
		{ iKeyPromptYes, iKeyPromptYes, "Yes" },
		{ 0, 0, NULL }
	},
	{
		{ 0, 0, "Save" },
		{ iKeySaveAppendFile, iKeySaveAppendFile, "AppendFile" },
		{ iKeySaveOverwriteFile, iKeySaveOverwriteFile, "OverwriteFile" },
		{ 0, 0, NULL }
	},
	{	/* Select keys */
		{ 0, 0, "Select" },
		{ iKeySelectEnterNextUnreadGrp, iKeySelectEnterNextUnreadGrp, "EnterNextUnreadGrp" },
		{ iKeySelectReadGrp, iKeySelectReadGrp, "ReadGrp" },
		{ iKeySelectReadGrp2, iKeySelectReadGrp2, "ReadGrp2" },
		{ iKeySelectResetNewsrc, iKeySelectResetNewsrc, "ResetNewsrc" },
		{ iKeySelectCatchupNextUnread, iKeySelectCatchupNextUnread, "CatchupNextUnread" },
		{ iKeySelectNextUnreadGrp, iKeySelectNextUnreadGrp, "NextUnreadGrp" },
		{ iKeySelectBugReport, iKeySelectBugReport, "BugReport" },
		{ iKeySelectSubscribePat, iKeySelectSubscribePat, "SubscribePat" },
		{ iKeySelectUnsubscribePat, iKeySelectUnsubscribePat, "UnsubscribePat" },
		{ iKeySelectQuitNoWrite, iKeySelectQuitNoWrite, "QuitNoWrite" },
		{ iKeySelectSyncWithActive, iKeySelectSyncWithActive, "SyncWithActive" },
		{ iKeySelectMarkGrpUnread2, iKeySelectMarkGrpUnread2, "MarkGrpUnread2" },
		{ iKeySelectCatchup, iKeySelectCatchup, "Catchup" },
		{ iKeySelectToggleDescriptions, iKeySelectToggleDescriptions, "ToggleDescriptions" },
		{ iKeySelectGoto, iKeySelectGoto, "Goto" },
		{ iKeySelectMoveGrp, iKeySelectMoveGrp, "MoveGrp" },
		{ iKeySelectEnterNextUnreadGrp2, iKeySelectEnterNextUnreadGrp2, "EnterNextUnreadGrp2" },
		{ iKeySelectToggleReadDisplay, iKeySelectToggleReadDisplay, "ToggleReadDisplay" },
		{ iKeySelectSubscribe, iKeySelectSubscribe, "Subscribe" },
		{ iKeySelectUnsubscribe, iKeySelectUnsubscribe, "Unsubscribe" },
		{ iKeySelectYankActive, iKeySelectYankActive, "YankActive" },
		{ iKeySelectMarkGrpUnread, iKeySelectMarkGrpUnread, "MarkGrpUnread" },
		{ 0, 0, NULL }
	},
	{	/* Thread keys */
		{ 0, 0, "Thread" },
		{ iKeyThreadReadNextArtOrThread, iKeyThreadReadNextArtOrThread, "ReadNextArtOrThread" },
		{ iKeyThreadReadArt, iKeyThreadReadArt, "ReadArt" },
		{ iKeyThreadReadArt2, iKeyThreadReadArt2, "ReadArt2" },
		{ iKeyThreadSelArt, iKeyThreadSelArt, "SelArt" },
		{ iKeyThreadToggleArtSel, iKeyThreadToggleArtSel, "ToggleArtSel" },
		{ iKeyThreadReverseSel, iKeyThreadReverseSel, "ReverseSel" },
		{ iKeyThreadCatchupNextUnread, iKeyThreadCatchupNextUnread, "CatchupNextUnread" },
		{ iKeyThreadMarkArtRead, iKeyThreadMarkArtRead, "MarkArtRead" },
		{ iKeyThreadBugReport, iKeyThreadBugReport, "BugReport" },
		{ iKeyThreadAutoSaveTagged, iKeyThreadAutoSaveTagged, "AutoSaveTagged" },
		{ iKeyThreadUntag, iKeyThreadUntag, "Untag" },
		{ iKeyThreadMarkThdUnread, iKeyThreadMarkThdUnread, "MarkThdUnread" },
		{ iKeyThreadCatchup, iKeyThreadCatchup, "Catchup" },
		{ iKeyThreadToggleSubjDisplay, iKeyThreadToggleSubjDisplay, "ToggleSubjDisplay" },
		{ iKeyThreadMail, iKeyThreadMail, "Mail" },
		{ iKeyThreadSave, iKeyThreadSave, "Save" },
		{ iKeyThreadTag, iKeyThreadTag, "Tag" },
		{ iKeyThreadMarkArtUnread, iKeyThreadMarkArtUnread, "MarkArtUnread" },
		{ iKeyThreadUndoSel, iKeyThreadUndoSel, "UndoSel" },
		{ 0, 0, NULL }
	}
};

/* NULL terminated list of pointers to the start of all the keygroups */
t_keynode *keygroups[] = {
	&Key.Global.tag,		/* It is important that global be 1st for duplicate checking */
	&Key.Config.tag,
	&Key.Feed.tag,
	&Key.Filter.tag,
	&Key.Group.tag,
	&Key.Help.tag,
	&Key.Nrctbl.tag,
	&Key.Page.tag,
	&Key.Pgp.tag,
	&Key.Post.tag,
	&Key.Postpone.tag,
	&Key.PProc.tag,
	&Key.Prompt.tag,
	&Key.Save.tag,
	&Key.Select.tag,
	&Key.Thread.tag,
	NULL
};

/* Keymaps for various menus and screens */

t_keynode *keys_config_change[] = {
	&Key.Global.Quit, &Key.Config.NoSave, &Key.Global.Up, &Key.Global.Up2,
	&Key.Global.Down, &Key.Global.Down2, &Key.Global.FirstPage,
	&Key.Config.FirstPage2, &Key.Global.LastPage, &Key.Config.LastPage2,
	&Key.Global.PageUp, &Key.Global.PageUp2, &Key.Global.PageUp3,
	&Key.Global.PageDown, &Key.Global.PageDown2, &Key.Global.PageDown3,
	&Key.Global.SearchSubjF, &Key.Global.SearchSubjB, &Key.Config.Select,
	&Key.Config.Select2, &Key.Global.RedrawScr, &Key.Global.One,
	&Key.Global.Two, &Key.Global.Three, &Key.Global.Four, &Key.Global.Five,
	&Key.Global.Six, &Key.Global.Seven, &Key.Global.Eight, &Key.Global.Nine,
	NULL };

t_keynode *keys_feed_art_thread_regex_tag[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Feed.Art, &Key.Feed.Hot,
	&Key.Feed.Thd, &Key.Feed.Tag, &Key.Feed.Pat, NULL };

/*
 * WARNING!
 *
 * Don't change the order of these items if you are not fully aware of the
 * side effects! The order from PProc.None onwards corresponds to the
 * enumeration of the post processing types defined in tin.h (POST_PROC_*
 * #defines) which is also used as the index stored in the post_process_type
 * tinrc variable.
 *
 * If you want to add keys that are not used to start any post processing
 * action, insert it before &Key.PProc.None. Change the index of
 * &menukeymap.feed_post_process_type.defaultkeys[] at the end of
 * build_keymaps() so that it always points to the key for no post
 * processing.
 *
 * If you want to add keys that are used for post processing, put them at
 * the end (before the NULL entry).
 *
 * If you want to delete a post processing key and add a new one, have in
 * mind that this will probably confuse users who are upgrading from an
 * older tin because the new action replaces the old one.
 */
t_keynode *keys_feed_post_process_type[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.PProc.None, &Key.PProc.Shar,
	&Key.PProc.UUDecode,
	NULL };

t_keynode *keys_feed_supersede_article[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Feed.Repost, &Key.Feed.Supersede,
	NULL };

t_keynode *keys_filter_quit_edit_save[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Filter.Edit, &Key.Filter.Save,
	NULL };

t_keynode *keys_group_nav[] = {
	&Key.Global.Abort, &Key.Global.One, &Key.Global.Two, &Key.Global.Three,
	&Key.Global.Four, &Key.Global.Five, &Key.Global.Six, &Key.Global.Seven,
	&Key.Global.Eight, &Key.Global.Nine,
#ifndef NO_SHELL_ESCAPE
	&Key.Global.ShellEscape,
#endif /* !NO_SHELL_ESCAPE */
	&Key.Global.FirstPage, &Key.Global.LastPage, &Key.Global.LastViewed,
	&Key.Global.Pipe, &Key.Group.Mail,
#ifndef DISABLE_PRINTING
	&Key.Group.Print,
#endif /* !DISABLE_PRINTING */
	&Key.Group.Repost, &Key.Group.Save, &Key.Group.AutoSaveTagged,
	&Key.Global.SetRange, &Key.Global.SearchAuthF, &Key.Global.SearchAuthB,
	&Key.Global.SearchSubjF, &Key.Global.SearchSubjB, &Key.Global.SearchBody,
	&Key.Group.ReadBasenote, &Key.Group.ReadBasenote2,
	&Key.Group.NextUnreadArtOrGrp, &Key.Global.PageDown, &Key.Global.PageDown2,
	&Key.Global.PageDown3, &Key.Group.AutoSel, &Key.Group.Kill,
	&Key.Group.EditFilter, &Key.Group.QuickAutoSel, &Key.Group.QuickKill,
	&Key.Global.RedrawScr, &Key.Global.Down, &Key.Global.Down2,
	&Key.Global.Up, &Key.Global.Up2, &Key.Global.PageUp, &Key.Global.PageUp2,
	&Key.Global.PageUp3, &Key.Global.CatchupLeft, &Key.Group.Catchup,
	&Key.Group.CatchupNextUnread, &Key.Group.ToggleSubjDisplay,
	&Key.Group.Goto, &Key.Global.Help, &Key.Global.ToggleHelpDisplay,
	&Key.Global.ToggleInverseVideo,
#ifdef HAVE_COLOR
	&Key.Global.ToggleColor,
#endif /* HAVE_COLOR */
	&Key.Group.MarkThdRead, &Key.Group.ListThd, &Key.Global.LookupMessage,
	&Key.Global.OptionMenu, &Key.Group.NextGroup, &Key.Group.NextUnreadArt,
	&Key.Group.PrevUnreadArt, &Key.Group.PrevGroup, &Key.Global.Quit,
	&Key.Global.QuitTin, &Key.Group.ToggleReadUnread,
	&Key.Group.ToggleGetartLimit, &Key.Group.BugReport, &Key.Group.TagParts,
	&Key.Group.Tag, &Key.Group.ToggleThreading, &Key.Group.Untag,
	&Key.Global.Version, &Key.Global.Post, &Key.Global.Postponed,
	&Key.Global.Postponed2, &Key.Global.DisplayPostHist,
	&Key.Group.MarkArtUnread, &Key.Group.MarkThdUnread, &Key.Group.SelThd,
	&Key.Group.ToggleThdSel, &Key.Group.ReverseSel, &Key.Group.UndoSel,
	&Key.Group.SelPattern, &Key.Group.SelThdIfUnreadSelected,
	&Key.Group.MarkUnselArtRead, &Key.Group.DoAutoSel,
	&Key.Global.ToggleInfoLastLine, NULL };

t_keynode *keys_nrctbl_create[] = {
   &Key.Global.Abort, &Key.Nrctbl.Quit, &Key.Nrctbl.Alternative,
   &Key.Nrctbl.Create, &Key.Nrctbl.Default, NULL };

t_keynode *keys_page_nav[] = {
	&Key.Global.Abort, &Key.Global.Zero, &Key.Global.One, &Key.Global.Two,
	&Key.Global.Three, &Key.Global.Four, &Key.Global.Five, &Key.Global.Six,
	&Key.Global.Seven, &Key.Global.Eight, &Key.Global.Nine,
#ifndef NO_SHELL_ESCAPE
	&Key.Global.ShellEscape,
#endif /* !NO_SHELL_ESCAPE */
	&Key.Global.MouseToggle, &Key.Global.PageUp, &Key.Global.PageUp2,
	&Key.Global.PageUp3, &Key.Global.PageDown, &Key.Global.PageDown2,
	&Key.Global.PageDown3, &Key.Page.NextUnread, &Key.Global.FirstPage,
	&Key.Page.FirstPage2, &Key.Global.LastPage, &Key.Page.LastPage2,
	&Key.Global.Up, &Key.Global.Up2, &Key.Global.Down, &Key.Global.Down2,
	&Key.Global.LastViewed, &Key.Global.LookupMessage, &Key.Page.GotoParent,
	&Key.Global.Pipe, &Key.Page.Mail,
#ifndef DISABLE_PRINTING
	&Key.Page.Print,
#endif /* !DISABLE_PRINTING */
	&Key.Page.Repost, &Key.Page.Save, &Key.Page.AutoSaveTagged,
	&Key.Global.SearchSubjF, &Key.Global.SearchSubjB, &Key.Global.SearchBody,
	&Key.Page.TopThd, &Key.Page.BotThd, &Key.Page.NextThd, &Key.Page.NextThd2,
#ifdef HAVE_PGP_GPG
	&Key.Page.PGPCheckArticle,
#endif /* HAVE_PGP_GPG */
	&Key.Page.ToggleHeaders, &Key.Page.ToggleTex2iso, &Key.Page.ToggleTabs,
	&Key.Page.ToggleUue, &Key.Page.Reveal, &Key.Page.QuickAutoSel,
	&Key.Page.QuickKill, &Key.Page.AutoSel, &Key.Page.AutoKill,
	&Key.Page.EditFilter, &Key.Global.RedrawScr, &Key.Page.ToggleRot,
	&Key.Page.Catchup, &Key.Page.CatchupNextUnread, &Key.Page.MarkThdUnread,
	&Key.Page.Cancel, &Key.Page.EditArticle, &Key.Page.FollowupQuote,
	&Key.Page.FollowupQuoteHeaders, &Key.Page.Followup, &Key.Global.Help,
	&Key.Global.ToggleHelpDisplay, &Key.Global.Quit,
	&Key.Global.ToggleInverseVideo,
#ifdef HAVE_COLOR
	&Key.Global.ToggleColor,
#endif /* HAVE_COLOR */
	&Key.Page.ListThd, &Key.Global.OptionMenu, &Key.Page.NextArt,
	&Key.Page.KillThd, &Key.Page.NextUnreadArt, &Key.Page.PrevArt,
	&Key.Page.PrevUnreadArt, &Key.Global.QuitTin, &Key.Page.ReplyQuote,
	&Key.Page.ReplyQuoteHeaders, &Key.Page.Reply, &Key.Page.Tag,
	&Key.Page.GroupSel, &Key.Global.Version, &Key.Global.Post,
	&Key.Global.Postponed, &Key.Global.Postponed2, &Key.Global.DisplayPostHist,
	&Key.Page.MarkArtUnread, &Key.Page.SkipIncludedText,
	&Key.Global.ToggleInfoLastLine,
#ifdef HAVE_COLOR
	&Key.Page.ToggleHighlight,
#endif /* HAVE_COLOR */
	&Key.Page.ViewAttach, &Key.Page.ViewUrl, NULL };

t_keynode *keys_pgp_mail[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Pgp.EncSign, &Key.Pgp.Encrypt,
	&Key.Pgp.Sign, NULL };

t_keynode *keys_pgp_news[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Pgp.Includekey, &Key.Pgp.Sign,
	NULL };

t_keynode *keys_post_cancel[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Post.Cancel, &Key.Post.Edit,
	NULL };

t_keynode *keys_post_cont[] = {
	&Key.Global.Abort, &Key.Post.Abort, &Key.Post.Continue, NULL };

t_keynode *keys_post_delete[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Post.Cancel, &Key.Post.Supersede,
	NULL };

t_keynode *keys_post_edit[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Post.Edit, &Key.Post.Postpone,
	NULL };

t_keynode *keys_post_edit_ext[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Post.Edit,
	&Key.Global.OptionMenu, NULL };

t_keynode *keys_post_ignore_fupto[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Post.Ignore, &Key.Global.Post,
	&Key.Post.Post2, &Key.Post.Post3, NULL };

t_keynode *keys_post_mail_fup[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Global.Post, &Key.Post.Post2,
	&Key.Post.Post3, &Key.Post.Mail, NULL };

t_keynode *keys_post_post[] = {
	&Key.Global.Abort, &Key.Global.Quit,
#ifdef HAVE_PGP_GPG
	&Key.Post.PGP,
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
	&Key.Post.Ispell,
#endif /* HAVE_ISPELL */
	&Key.Post.Edit, &Key.Global.Post, &Key.Post.Post2, &Key.Post.Post3,
	&Key.Post.Postpone, NULL };

t_keynode *keys_post_postpone[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Prompt.Yes,
	&Key.Postpone.All, &Key.Postpone.Override, &Key.Prompt.No, NULL };

t_keynode *keys_post_send[] = {
	&Key.Global.Abort, &Key.Global.Quit,
#ifdef HAVE_PGP_GPG
	&Key.Post.PGP,
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
	&Key.Post.Ispell,
#endif /* HAVE_ISPELL */
	&Key.Post.Edit, &Key.Post.Send, &Key.Post.Send2, NULL };

t_keynode *keys_prompt_yn[] = {
	&Key.Global.Abort, &Key.Prompt.Yes, &Key.Prompt.No, NULL };

t_keynode *keys_save_append_overwrite_quit[] = {
	&Key.Global.Abort, &Key.Global.Quit, &Key.Save.AppendFile,
	&Key.Save.OverwriteFile, NULL };

t_keynode *keys_select_nav[] = {
	&Key.Global.Abort, &Key.Global.One, &Key.Global.Two, &Key.Global.Three,
	&Key.Global.Four, &Key.Global.Five, &Key.Global.Six, &Key.Global.Seven,
	&Key.Global.Eight, &Key.Global.Nine,
#ifndef NO_SHELL_ESCAPE
	&Key.Global.ShellEscape,
#endif /* NO_SHELL_ESCAPE */
	&Key.Global.FirstPage, &Key.Global.LastPage, &Key.Global.PageUp,
	&Key.Global.PageUp2, &Key.Global.PageUp3, &Key.Global.PageDown,
	&Key.Global.PageDown2, &Key.Global.PageDown3, &Key.Global.Up,
	&Key.Global.Up2, &Key.Global.Down, &Key.Global.Down2, &Key.Global.SetRange,
	&Key.Global.SearchSubjF, &Key.Global.SearchSubjB,
	&Key.Select.ReadGrp, &Key.Select.ReadGrp2, &Key.Select.EnterNextUnreadGrp,
	&Key.Select.EnterNextUnreadGrp2, &Key.Global.RedrawScr,
	&Key.Select.ResetNewsrc, &Key.Select.Catchup,
	&Key.Select.CatchupNextUnread, &Key.Select.ToggleDescriptions,
	&Key.Select.Goto, &Key.Global.Help, &Key.Global.ToggleHelpDisplay,
	&Key.Global.ToggleInverseVideo,
#ifdef HAVE_COLOR
	&Key.Global.ToggleColor,
#endif /* HAVE_COLOR */
	&Key.Global.ToggleInfoLastLine, &Key.Select.MoveGrp,
	&Key.Global.OptionMenu, &Key.Select.NextUnreadGrp, &Key.Global.Quit,
	&Key.Global.QuitTin, &Key.Select.QuitNoWrite,
	&Key.Select.ToggleReadDisplay, &Key.Select.BugReport,
	&Key.Select.Subscribe, &Key.Select.SubscribePat, &Key.Select.Unsubscribe,
	&Key.Select.UnsubscribePat, &Key.Global.Version, &Key.Global.Post,
	&Key.Global.Postponed, &Key.Global.Postponed2, &Key.Global.DisplayPostHist,
	&Key.Select.YankActive, &Key.Select.SyncWithActive,
	&Key.Select.MarkGrpUnread, &Key.Select.MarkGrpUnread2, NULL };

t_keynode *keys_thread_nav[] = {
	&Key.Global.Abort, &Key.Global.One, &Key.Global.Two, &Key.Global.Three,
	&Key.Global.Four, &Key.Global.Five, &Key.Global.Six, &Key.Global.Seven,
	&Key.Global.Eight, &Key.Global.Nine,
#ifndef NO_SHELL_ESCAPE
	&Key.Global.ShellEscape,
#endif /* !NO_SHELL_ESCAPE */
	&Key.Global.FirstPage, &Key.Global.LastPage, &Key.Global.LastViewed,
	&Key.Global.SetRange, &Key.Global.Pipe, &Key.Thread.Mail,
	&Key.Thread.Save, &Key.Thread.AutoSaveTagged, &Key.Thread.ReadArt,
	&Key.Thread.ReadArt2, &Key.Thread.ReadNextArtOrThread, &Key.Global.Post,
	&Key.Global.RedrawScr, &Key.Global.Down, &Key.Global.Down2,
	&Key.Global.Up, &Key.Global.Up2, &Key.Global.PageUp, &Key.Global.PageUp2,
	&Key.Global.PageUp3, &Key.Global.PageDown, &Key.Global.PageDown2,
	&Key.Global.PageDown3, &Key.Global.CatchupLeft, &Key.Thread.Catchup,
	&Key.Thread.CatchupNextUnread, &Key.Thread.MarkArtRead,
	&Key.Thread.ToggleSubjDisplay, &Key.Global.Help,
	&Key.Global.LookupMessage, &Key.Global.SearchBody,
	&Key.Global.SearchSubjF, &Key.Global.SearchSubjB,
	&Key.Global.SearchAuthF, &Key.Global.SearchAuthB,
	&Key.Global.ToggleHelpDisplay, &Key.Global.ToggleInverseVideo,
#ifdef HAVE_COLOR
	&Key.Global.ToggleColor,
#endif /* HAVE_COLOR */
	&Key.Global.Quit, &Key.Global.QuitTin, &Key.Thread.Tag,
	&Key.Thread.BugReport, &Key.Thread.Untag, &Key.Global.Version,
	&Key.Thread.MarkArtUnread, &Key.Thread.MarkThdUnread, &Key.Thread.SelArt,
	&Key.Thread.ToggleArtSel, &Key.Thread.ReverseSel, &Key.Thread.UndoSel,
	&Key.Global.Postponed, &Key.Global.Postponed2, &Key.Global.DisplayPostHist,
	&Key.Global.ToggleInfoLastLine, NULL };

t_menukeymap menukeymap = {
	{ keys_config_change, NULL, NULL },
	{ keys_feed_art_thread_regex_tag, NULL, NULL },
	{ keys_feed_post_process_type, NULL, NULL },
	{ keys_feed_supersede_article, NULL, NULL },
	{ keys_filter_quit_edit_save, NULL, NULL },
	{ keys_group_nav, NULL, NULL },
	{ keys_nrctbl_create, NULL, NULL },
	{ keys_page_nav, NULL, NULL },
	{ keys_pgp_mail, NULL, NULL },
	{ keys_pgp_news, NULL, NULL },
	{ keys_post_cancel, NULL, NULL },
	{ keys_post_cont, NULL, NULL },
	{ keys_post_delete, NULL, NULL },
	{ keys_post_edit, NULL, NULL },
	{ keys_post_edit_ext, NULL, NULL },
	{ keys_post_ignore_fupto, NULL, NULL },
	{ keys_post_mail_fup, NULL, NULL },
	{ keys_post_post, NULL, NULL },
	{ keys_post_postpone, NULL, NULL },
	{ keys_post_send, NULL, NULL },
	{ keys_prompt_yn, NULL, NULL },
	{ keys_save_append_overwrite_quit, NULL, NULL },
	{ keys_select_nav, NULL, NULL },
	{ keys_thread_nav, NULL, NULL }
};

/*
 * Return the number of keys in a menu
 */
static int
keymapsize (
	t_keynode *ptr[])
{
	int i = 0;

	if (!ptr) return 0;

	while (*ptr) {
		i++;
		ptr++;
	}
	return i;
}

/*
 * Compile keymaps for faster access and conversion
 */
void
build_keymaps (
	void)
{
	char *dkey, *lkey;
	int cnt = sizeof (menukeymap) / sizeof (t_menukeys);
	int size;
	t_keynode *keyptr;
	t_menukeys *menuptr = &menukeymap.config_change;

	while (cnt--) {
		size = keymapsize (menuptr->keys);
		if (menuptr->defaultkeys)
			menuptr->defaultkeys = my_realloc (menuptr->defaultkeys, size + 1);
		else
			menuptr->defaultkeys = my_malloc (size + 1);
		if (menuptr->localkeys)
			menuptr->localkeys = my_realloc (menuptr->localkeys, size + 1);
		else
			menuptr->localkeys = my_malloc (size + 1);
		dkey = menuptr->defaultkeys;
		lkey = menuptr->localkeys;
		dkey[size] = '\0';
		lkey[size] = '\0';
		while (size--) {
			keyptr = menuptr->keys[size];
			dkey[size] = keyptr->defaultkey;
			lkey[size] = keyptr->localkey;
		}
		menuptr++;
	}
	ch_post_process = &menukeymap.feed_post_process_type.defaultkeys[2];
}

/*
 * convert a local key to the internal (default) mapping
 */
int
map_to_default (
	const char key,
	const t_menukeys *menukeys)
{
	char *ptr = strchr (menukeys->localkeys, key);

	if (ptr)
		return menukeys->defaultkeys[ptr - menukeys->localkeys];

	return 0;
}

/*
 * convert an internal (default) key to a local one
 */
int
map_to_local (
	const char key,
	const t_menukeys *menukeys)
{
	char *ptr = strchr (menukeys->defaultkeys, key);

	if (ptr)
		return menukeys->localkeys[ptr - menukeys->defaultkeys];

	return 0;
}

/*
 * Free all memory for keymaps.
 */
void
free_keymaps (
	void)
{
	int cnt = sizeof (menukeymap) / sizeof (t_menukeys);
	t_menukeys *menuptr = &menukeymap.config_change;

	while (cnt--) {
		FreeIfNeeded (menuptr->localkeys);
		FreeIfNeeded (menuptr->defaultkeys);
		menuptr++;
	}
}

/* TODO -> tin.h */
#define KEYMAP_FILE	"keymap"

/*
 * Render ch in human readable ASCII
 * Is there no lib function to do this ?
 */
char *
printascii (
	char *buf,
	int ch)
{
	if (isgraph(ch)) {			/* Regular printables */
		buf[0] = ch;
		buf[1] = '\0';
	} else if (ch == '\t') {	/* TAB */
		strcpy (buf, _(txt_tab));
	} else if (iscntrl(ch)) {	/* Control keys */
		buf[0] = '^';
		buf[1] = (ch & 0xFF) + '@';
		buf[2] = '\0';
	} else if (ch == ' ')		/* SPACE */
		strcpy (buf, _(txt_space));
	else
		strcpy (buf, "???");	/* Never happen ? */

	return buf;
}

/*
 * Find any key clashes between groups keyptr1 and keyptr2.  This is just an
 * ascending brute force search.  We need a pointer to the tag node in order
 * to report errors correctly, the pointers start 1 above this on the first
 * key definition.  If we are checking against the same keygroup, then ptr2
 * starts at (ptr1+1) else the algorithm doesn't work!
 */
static t_bool
check_duplicates(
	t_keynode *keyptr1,
	t_keynode *keyptr2)
{
	char buf[10];
	t_keynode *ptr1, *ptr2;

	for (ptr1 = keyptr1 + 1; ptr1->localkey != '\0'; ++ptr1) {
		for (ptr2 = (keyptr1 == keyptr2) ? ptr1 + 1 : keyptr2 + 1; ptr2->localkey != '\0'; ++ptr2) {
			if (ptr1->localkey == ptr2->localkey) {
				fprintf (stderr, _(txt_keymap_conflict), printascii (buf, ptr1->localkey),
								keyptr1->t, ptr1->t, keyptr2->t, ptr2->t);
				return FALSE;
			}
		}
	}

	return TRUE;
}


/*
 * Try and lookup a key description. If found, make the assignment
 */
static t_bool
processkey(
	t_keynode *keyptr,		/* Where in map to start search */
	char *kname,				/* Keyname we're searching for */
	char key)					/* Key to assign to keyname if found */
{
	char buf[LEN];
	int i;
#ifdef DEBUG
	char was[10], is[10];
#endif /* DEBUG */

	strcpy (buf, keyptr->t);	/* Preload the groupname for speed */
	keyptr++;						/* Advance to 1st key */
	i = strlen (buf);

	for (; keyptr->t != NULL; ++keyptr) {
		strcpy (buf+i, keyptr->t);

		if (strcasecmp (kname, buf) == 0) {
#ifdef DEBUG
			fprintf (stderr, _(txt_keymap_redef), buf, printascii (was, keyptr->localkey), printascii (is, key));
#endif /* DEBUG */
			keyptr->localkey = key;
			return TRUE;
		}
	}

	return FALSE;
}

#define KEYSEPS		" \t\n"

t_bool
read_keymap_file (
	void)
{
	FILE *fp = (FILE *) 0;
	char *line, *keydef, *kname;
	char *ptr;
	const char *ptr2, *map;
	char buf[LEN], buff[LEN];
	char key;
	int i;
	t_bool ret = TRUE;

	if (!batch_mode)
		wait_message (0, _(txt_reading_keymap_file));

	/*
	 * checks TIN_HOMEDIR/HOME/TIN_DEFAULTS_DIR
	 * for keymap."locale" or keymap
	 *
	 * locale is first match from LC_CYTYPE, LC_MESSAGES, LC_ALL or LANG
	 */
	ptr2 = get_val("TIN_HOMEDIR", get_val("HOME", homedir));
	/* get locale suffix */
	map = get_val("LC_CTYPE", get_val("LC_MESSAGES", get_val("LC_ALL", get_val("LANG", ""))));
	if (strlen(map)) {
		if ((ptr = strchr (map, '.')))
				*ptr = '\0';
		snprintf(buff, sizeof(buf) - 1, "%s/.tin/keymap.%s", ptr2, map);
		if (strfpath (buff, buf, sizeof(buf), &CURR_GROUP))
			fp = fopen (buf, "r");
	}
	if (!fp) {
		snprintf(buff, sizeof(buf) - 1, "%s/.tin/keymap", ptr2);
		if (strfpath (buff, buf, sizeof(buf), &CURR_GROUP))
			fp = fopen (buf, "r");
	}
#ifdef TIN_DEFAULTS_DIR
	if (strlen(map) && !fp) {
		snprintf(buff, sizeof(buf) - 1, "%s/keymap.%s", TIN_DEFAULTS_DIR, map);
		if (strfpath (buff, buf, sizeof(buf), &CURR_GROUP))
			fp = fopen (buf, "r");
	}
	if (!fp) {
		snprintf(buff, sizeof(buf) - 1, "%s/keymap", TIN_DEFAULTS_DIR);
		if (strfpath (buff, buf, sizeof(buf), &CURR_GROUP))
			fp = fopen (buf, "r");
	}
#endif /* TIN_DEFAULTS_DIR */

	if (!fp)
		return FALSE;

	while ((line = fgets (buf, sizeof(buf), fp)) != NULL) {
		/*
		 * Ignore blank and comment lines
		 */
		if (line[0] == '#' || line[0] == '\n')
			continue;

		kname = strtok (line, KEYSEPS);
		keydef = strtok (NULL, KEYSEPS);

		/*
		 * Warn about basic syntax errors
		 */
		if (keydef == NULL) {
			fprintf (stderr, _(txt_keymap_missing_key), kname);
			ret = FALSE;
			continue;
		}

		/*
		 * Parse the key sequence into 'key'
		 * Special sequences are:
		 * ^A -> control chars
		 * TAB -> ^I
		 * SPACE -> ' '
		 */
		if (strlen (keydef) > 1)
			switch (keydef[0]) {
				case '^':
					if (!(isupper ((int)keydef[1]))) {
						fprintf (stderr, _(txt_keymap_invalid_key), keydef);
						ret = FALSE;
						continue;
					}
					key = ctrl (keydef[1]);
					break;
				case 'S':		/* Only test 1st char - crude but effective */
					key = ' ';
					break;
				case 'T':
					key = ctrl ('I');
					break;
				default:
					fprintf (stderr, _(txt_keymap_invalid_key), keydef);
					ret = FALSE;
					continue;
			}
		else
			key = keydef[0];

		/*
		 * Try and locate the tagline in each keygroup
		 */
		for (i = 0; keygroups[i] != NULL; ++i) {
			if (processkey (keygroups[i], kname, key))
				break;
		}

		/*
		 * TODO: usefull? shared keymaps (NFS-Home) may differ
		 * depending on the OS (i.e. on tin has colr the other has not)
		 */
		if (keygroups[i] == NULL) {
			fprintf (stderr, _(txt_keymap_invalid_name), kname);
			ret = FALSE;
			continue;
		}

	}

	/*
	 * Check for duplicates in each keygroup and vs. the global group
	 */
	for (i = 0; keygroups[i] != NULL; ++i) {
		if (!(check_duplicates (keygroups[i], keygroups[i])))
			ret = FALSE;
		if (i != 0 && !(check_duplicates (keygroups[i], keygroups[0])))
			ret = FALSE;
	}

	fclose (fp);
	build_keymaps ();
	return ret;
}
