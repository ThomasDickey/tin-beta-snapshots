# Top level Makefile for tin
# - for configuration options read the doc/INSTALL file.
#
# Updated: 2002-03-11
#

PROJECT	= tin
LVER	= 1
PVER	= 5
SVER	= 17
VER	= $(LVER).$(PVER).$(SVER)
DVER	= 20030306
EXE	= tin

# directory structure
TOPDIR	= .
DOCDIR	= ./doc
INCDIR	= ./include
OBJDIR	= ./src
SRCDIR	= ./src
AMGDIR	= ./amiga
VMSDIR	= ./vms
PCREDIR	= ./pcre
CANDIR	= ./libcanlock
TOLDIR	= ./tools
PODIR	= ./po
INTLDIR	= ./intl

HFILES	= \
	$(INCDIR)/bool.h \
	$(INCDIR)/bugrep.h \
	$(INCDIR)/oldconfig.h \
	$(INCDIR)/extern.h \
	$(INCDIR)/keymap.h \
	$(INCDIR)/menukeys.h \
	$(INCDIR)/nntplib.h \
	$(INCDIR)/plp_snprintf.h \
	$(INCDIR)/policy.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/rfc2046.h \
	$(INCDIR)/stpwatch.h \
	$(INCDIR)/tcurses.h \
	$(INCDIR)/tin.h \
	$(INCDIR)/tinrc.h \
	$(INCDIR)/tnntp.h \
	$(INCDIR)/trace.h \
	$(INCDIR)/version.h

CFILES	= \
	$(SRCDIR)/active.c \
	$(SRCDIR)/art.c \
	$(SRCDIR)/attrib.c \
	$(SRCDIR)/auth.c \
	$(SRCDIR)/charset.c \
	$(SRCDIR)/color.c \
	$(SRCDIR)/config.c \
	$(SRCDIR)/cook.c \
	$(SRCDIR)/curses.c \
	$(SRCDIR)/debug.c\
	$(SRCDIR)/envarg.c \
	$(SRCDIR)/feed.c \
	$(SRCDIR)/filter.c \
	$(SRCDIR)/getline.c \
	$(SRCDIR)/global.c \
	$(SRCDIR)/group.c \
	$(SRCDIR)/hashstr.c \
	$(SRCDIR)/header.c \
	$(SRCDIR)/help.c\
	$(SRCDIR)/inews.c \
	$(SRCDIR)/init.c \
	$(SRCDIR)/joinpath.c \
	$(SRCDIR)/keymap.c \
	$(SRCDIR)/lang.c \
	$(SRCDIR)/langinfo.c \
	$(SRCDIR)/list.c \
	$(SRCDIR)/lock.c \
	$(SRCDIR)/mail.c \
	$(SRCDIR)/main.c \
	$(SRCDIR)/makecfg.c \
	$(SRCDIR)/memory.c \
	$(SRCDIR)/mimetypes.c \
	$(SRCDIR)/misc.c \
	$(SRCDIR)/newsrc.c\
	$(SRCDIR)/nntplib.c \
	$(SRCDIR)/nrctbl.c \
	$(SRCDIR)/open.c \
	$(SRCDIR)/page.c \
	$(SRCDIR)/parsdate.y \
	$(SRCDIR)/plp_snprintf.c \
	$(SRCDIR)/pgp.c \
	$(SRCDIR)/post.c \
	$(SRCDIR)/prompt.c \
	$(SRCDIR)/read.c \
	$(SRCDIR)/refs.c \
	$(SRCDIR)/regex.c \
	$(SRCDIR)/rfc1524.c \
	$(SRCDIR)/rfc2045.c \
	$(SRCDIR)/rfc2046.c \
	$(SRCDIR)/rfc2047.c \
	$(SRCDIR)/save.c \
	$(SRCDIR)/screen.c \
	$(SRCDIR)/search.c \
	$(SRCDIR)/select.c \
	$(SRCDIR)/sigfile.c \
	$(SRCDIR)/signal.c \
	$(SRCDIR)/strftime.c \
	$(SRCDIR)/string.c \
	$(SRCDIR)/tags.c \
	$(SRCDIR)/tcurses.c \
	$(SRCDIR)/tmpfile.c \
	$(SRCDIR)/my_tmpfile.c \
	$(SRCDIR)/thread.c \
	$(SRCDIR)/trace.c \
	$(SRCDIR)/wildmat.c \
	$(SRCDIR)/xref.c

AMIGA	= \
	$(AMGDIR)/README \
	$(AMGDIR)/tin.readme \
	$(AMGDIR)/smakefile \
	$(AMGDIR)/actived.c \
	$(AMGDIR)/amiga.c \
	$(AMGDIR)/amigatcp.c \
	$(AMGDIR)/amiga.h \
	$(AMGDIR)/amigatcp.h

VMS	= \
	$(VMSDIR)/dir.h \
	$(VMSDIR)/filetypes.h \
	$(VMSDIR)/getopt.c \
	$(VMSDIR)/getopt.h \
	$(VMSDIR)/getopt1.c \
	$(VMSDIR)/getpass.c \
	$(VMSDIR)/isxterm.c \
	$(VMSDIR)/libvms.mms \
	$(VMSDIR)/ndir.h \
	$(VMSDIR)/parsdate.c \
	$(VMSDIR)/parse.c \
	$(VMSDIR)/parse.h \
	$(VMSDIR)/pwd.h \
	$(VMSDIR)/qsort.c \
	$(VMSDIR)/select.h \
	$(VMSDIR)/strings.h \
	$(VMSDIR)/uaf.h \
	$(VMSDIR)/vms.c \
	$(VMSDIR)/vmsdir.c \
	$(VMSDIR)/vmsfile.c \
	$(VMSDIR)/vmspwd.c \
	$(VMSDIR)/vmstimval.h

DOC	= \
	$(DOCDIR)/ABOUT-NLS \
	$(DOCDIR)/CHANGES \
	$(DOCDIR)/CHANGES.old \
	$(DOCDIR)/DEBUG_REFS \
	$(DOCDIR)/INSTALL \
	$(DOCDIR)/TODO \
	$(DOCDIR)/WHATSNEW \
	$(DOCDIR)/art_handling.txt \
	$(DOCDIR)/auth.txt \
	$(DOCDIR)/config-anomalies \
	$(DOCDIR)/filtering \
	$(DOCDIR)/good-netkeeping-seal \
	$(DOCDIR)/internals.txt \
	$(DOCDIR)/iso2asc.txt \
	$(DOCDIR)/keymap.sample \
	$(DOCDIR)/mailcap.sample \
	$(DOCDIR)/mime.types \
	$(DOCDIR)/opt-case.1 \
	$(DOCDIR)/pgp.txt \
	$(DOCDIR)/rcvars.txt \
	$(DOCDIR)/reading-mail.txt \
	$(DOCDIR)/umlaute.txt \
	$(DOCDIR)/umlauts.txt \
	$(DOCDIR)/tin.defaults \
	$(DOCDIR)/tools.txt \
	$(DOCDIR)/mbox.5 \
	$(DOCDIR)/mmdf.5 \
	$(DOCDIR)/newsoverview.5 \
	$(DOCDIR)/plp_snprintf.3 \
	$(DOCDIR)/tin.1 \
	$(DOCDIR)/tin.5 \
	$(DOCDIR)/w2r.1 \
	$(DOCDIR)/wildmat.3

TOL	= \
	$(TOLDIR)/expiretover \
	$(TOLDIR)/metamutt \
	$(TOLDIR)/opt-case.pl \
	$(TOLDIR)/tinlock \
	$(TOLDIR)/tinews.pl \
	$(TOLDIR)/tinrcupdate.pl \
	$(TOLDIR)/url_handler.sh \
	$(TOLDIR)/w2r.pl \
	$(TOLDIR)/expand_aliases.tgz

TOP	= \
	$(TOPDIR)/Makefile \
	$(TOPDIR)/MANIFEST \
	$(TOPDIR)/README \
	$(TOPDIR)/README.MAC \
	$(TOPDIR)/README.VMS \
	$(TOPDIR)/README.WIN \
	$(TOPDIR)/aclocal.m4 \
	$(TOPDIR)/conf-tin \
	$(TOPDIR)/config.guess \
	$(TOPDIR)/config.sub \
	$(TOPDIR)/configure \
	$(TOPDIR)/configure.in \
	$(TOPDIR)/install.sh \
	$(TOPDIR)/mkdirs.sh

PCRE	= \
	$(PCREDIR)/AUTHORS \
	$(PCREDIR)/COPYING \
	$(PCREDIR)/ChangeLog \
	$(PCREDIR)/INSTALL \
	$(PCREDIR)/LICENCE \
	$(PCREDIR)/Makefile.in \
	$(PCREDIR)/Makefile.in-old \
	$(PCREDIR)/NEWS \
	$(PCREDIR)/NON-UNIX-USE \
	$(PCREDIR)/README \
	$(PCREDIR)/RunTest \
	$(PCREDIR)/config.h \
	$(PCREDIR)/configure.in \
	$(PCREDIR)/dftables.c \
	$(PCREDIR)/dll.mk \
	$(PCREDIR)/get.c \
	$(PCREDIR)/internal.h \
	$(PCREDIR)/maketables.c \
	$(PCREDIR)/pcre-config.in \
	$(PCREDIR)/pcre.c \
	$(PCREDIR)/pcre.def \
	$(PCREDIR)/pcre.in \
	$(PCREDIR)/pcredemo.c \
	$(PCREDIR)/pcregrep.c \
	$(PCREDIR)/pcreposix.c \
	$(PCREDIR)/pcreposix.h \
	$(PCREDIR)/pcretest.c \
	$(PCREDIR)/perltest \
	$(PCREDIR)/study.c \
	$(PCREDIR)/version.sh \
	$(PCREDIR)/doc/Tech.Notes \
	$(PCREDIR)/doc/pcre.3 \
	$(PCREDIR)/doc/pcre.html \
	$(PCREDIR)/doc/pcre.txt \
	$(PCREDIR)/doc/pcregrep.1 \
	$(PCREDIR)/doc/pcregrep.html \
	$(PCREDIR)/doc/pcregrep.txt \
	$(PCREDIR)/doc/pcreposix.3 \
	$(PCREDIR)/doc/pcreposix.html \
	$(PCREDIR)/doc/pcreposix.txt \
	$(PCREDIR)/doc/pcretest.1 \
	$(PCREDIR)/doc/pcretest.html \
	$(PCREDIR)/doc/pcretest.txt \
	$(PCREDIR)/doc/perltest.txt \
	$(PCREDIR)/testdata/testinput1 \
	$(PCREDIR)/testdata/testinput2 \
	$(PCREDIR)/testdata/testinput3 \
	$(PCREDIR)/testdata/testinput4 \
	$(PCREDIR)/testdata/testinput5 \
	$(PCREDIR)/testdata/testinput6 \
	$(PCREDIR)/testdata/testoutput1 \
	$(PCREDIR)/testdata/testoutput2 \
	$(PCREDIR)/testdata/testoutput3 \
	$(PCREDIR)/testdata/testoutput4 \
	$(PCREDIR)/testdata/testoutput5 \
	$(PCREDIR)/testdata/testoutput6

CAN	= \
	$(CANDIR)/Build \
	$(CANDIR)/CHANGES \
	$(CANDIR)/MANIFEST \
	$(CANDIR)/README \
	$(CANDIR)/base64.c \
	$(CANDIR)/base64.h \
	$(CANDIR)/canlock.h \
	$(CANDIR)/canlock_md5.c \
	$(CANDIR)/canlock_misc.c \
	$(CANDIR)/canlock_sha1.c \
	$(CANDIR)/canlocktest.c \
	$(CANDIR)/endian.c \
	$(CANDIR)/hmac_md5.c \
	$(CANDIR)/hmac_md5.h \
	$(CANDIR)/hmac_sha1.c \
	$(CANDIR)/hmac_sha1.h \
	$(CANDIR)/hmactest.c \
	$(CANDIR)/main.c \
	$(CANDIR)/md5.c \
	$(CANDIR)/md5.h \
	$(CANDIR)/sha1.c \
	$(CANDIR)/sha1.h \
	$(CANDIR)/doc/HOWTO \
	$(CANDIR)/doc/draft-ietf-usefor-cancel-lock-01.txt \
	$(CANDIR)/doc/rfc2104.txt \
	$(CANDIR)/doc/rfc2202.txt \
	$(CANDIR)/doc/rfc2286.txt

MISC	= \
	$(INCDIR)/autoconf.hin \
	$(PCREDIR)/pcre.mms \
	$(SRCDIR)/Makefile.in \
	$(SRCDIR)/ibm437_l1.tab \
	$(SRCDIR)/ibm850_l1.tab \
	$(SRCDIR)/l1_ibm437.tab \
	$(SRCDIR)/l1_ibm850.tab \
	$(SRCDIR)/l1_next.tab \
	$(SRCDIR)/next_l1.tab \
	$(SRCDIR)/tincfg.tbl \
	$(SRCDIR)/descrip.mms

INTLFILES = \
	$(INTLDIR)/bindtextdom.c \
	$(INTLDIR)/ChangeLog \
	$(INTLDIR)/config.charset \
	$(INTLDIR)/dcgettext.c \
	$(INTLDIR)/dcigettext.c \
	$(INTLDIR)/dcngettext.c \
	$(INTLDIR)/dgettext.c \
	$(INTLDIR)/dngettext.c \
	$(INTLDIR)/explodename.c \
	$(INTLDIR)/finddomain.c \
	$(INTLDIR)/gettext.c \
	$(INTLDIR)/gettext.h \
	$(INTLDIR)/gettextP.h \
	$(INTLDIR)/hash-string.h \
	$(INTLDIR)/intl-compat.c \
	$(INTLDIR)/l10nflist.c \
	$(INTLDIR)/libgettext.h \
	$(INTLDIR)/libgnuintl.h \
	$(INTLDIR)/loadinfo.h \
	$(INTLDIR)/loadmsgcat.c \
	$(INTLDIR)/localcharset.c \
	$(INTLDIR)/locale.alias \
	$(INTLDIR)/localealias.c \
	$(INTLDIR)/Makefile.in \
	$(INTLDIR)/ngettext.c \
	$(INTLDIR)/plural.c \
	$(INTLDIR)/plural.y \
	$(INTLDIR)/ref-add.sin \
	$(INTLDIR)/ref-del.sin \
	$(INTLDIR)/textdomain.c \
	$(INTLDIR)/VERSION

POFILES = \
	$(PODIR)/Makefile.inn \
	$(PODIR)/POTFILES.in \
	$(PODIR)/$(PROJECT).pot \
	$(PODIR)/de.gmo \
	$(PODIR)/de.po \
	$(PODIR)/et.gmo \
	$(PODIR)/et.po \
	$(PODIR)/en_GB.gmo \
	$(PODIR)/en_GB.po


ALL_FILES = $(TOP) $(DOC) $(TOL) $(HFILES) $(CFILES) $(AMIGA) $(VMS) $(PCRE) $(MISC) $(CAN) $(INTLFILES) $(POFILES)

ALL_DIRS = $(TOPDIR) $(DOCDIR) $(SRCDIR) $(INCDIR) $(AMGDIR) $(VMSDIR) $(PCREDIR) $(PCREDIR)/doc $(PCREDIR)/testdata $(CANDIR) $(CANDIR)/doc $(INTLDIR) $(PODIR)

# standard commands
CD	= cd
CHMOD	= chmod
CP	= cp -p
ECHO	= echo
LS	= ls
MAKE	= make
MV	= mv
NROFF	= groff -Tascii
RM	= rm
SHELL	= /bin/sh
TAR	= tar
GZIP	= gzip
BZIP2	= bzip2
WC	= wc
SED	= sed
TR	= tr
TEST	= test


all:
	@$(ECHO) "Top level Makefile for the $(PROJECT) v$(VER) Usenet newsreader."
	@$(ECHO) " "
	@$(ECHO) "To compile the source code type 'make build' or change to the"
	@$(ECHO) "source directory by typing 'cd src' and then type 'make'."
	@$(ECHO) " "
	@$(ECHO) "This Makefile offers the following general purpose options:"
	@$(ECHO) " "
	@$(ECHO) "    make build           [ Compile $(PROJECT) ]"
	@$(ECHO) "    make clean           [ Delete all object and backup files ]"
	@$(ECHO) "    make dist            [ Create a gzipped & bzipped distribution tar file ]"
	@$(ECHO) "    make distclean       [ Delete all config, object and backup files ]"
	@$(ECHO) "    make install         [ Install the binary and the manual page ]"
	@$(ECHO) "    make install_sysdefs [ Install the system-wide-defaults file ]"
	@$(ECHO) "    make manpage         [ Create nroff version of manual page ]"
	@$(ECHO) "    make manifest        [ Create MANIFEST ]"
	@$(ECHO) " "

build:
	@-if $(TEST) -r $(SRCDIR)/Makefile ; then $(CD) $(SRCDIR) && $(MAKE) ; else $(ECHO) "You need to run configure first - didn't you read README?" ; fi

install:
	@$(CD) $(SRCDIR) && $(MAKE) install

install_sysdefs:
	@$(CD) $(SRCDIR) && $(MAKE) install_sysdefs

clean:
	@-$(RM) -f \
	*~ \
	$(DOCDIR)/*~ \
	$(INCDIR)/*~ \
	$(SRCDIR)/*~ \
	$(PCREDIR)/*~
	@-if $(TEST) -r $(PCREDIR)/Makefile ; then $(CD) $(PCREDIR) && $(MAKE) clean ; fi
	@-if $(TEST) -r $(INTLDIR)/Makefile ; then $(CD) $(INTLDIR) && $(MAKE) clean ; fi
	@-if $(TEST) -r $(PODIR)/Makefile ; then $(CD) $(PODIR) && $(MAKE) clean ; fi
	@-if $(TEST) -r $(SRCDIR)/Makefile ; then $(CD) $(SRCDIR) && $(MAKE) clean ; fi

man:
	@$(MAKE) manpage

manpage:
	@$(ECHO) "Creating $(NROFF) man page for $(EXE)-$(VER)..."
	@$(NROFF) -man $(DOCDIR)/tin.1 > $(DOCDIR)/$(EXE).nrf

# Use 2 passes for creating MANIFEST because its size changes (it's not likely
# that we'll need 3 passes, since that'll happen only when the grand total's
# digits change).
manifest:
	@$(ECHO) "Creating MANIFEST..."
	@$(ECHO) "MANIFEST for $(PROJECT)-$(VER) (`date`)" > MANIFEST.tmp
	@$(ECHO) "----------------------------------------------------" >> MANIFEST.tmp
	@$(CP) MANIFEST.tmp MANIFEST
	@$(WC) -c $(ALL_FILES) >> MANIFEST
	@$(WC) -c $(ALL_FILES) >> MANIFEST.tmp
	@$(MV) MANIFEST.tmp MANIFEST

chmod:
	@$(ECHO) "Setting the file permissions..."
	@$(CHMOD) 644 $(ALL_FILES)
	@$(CHMOD) 755 \
	$(ALL_DIRS) \
	./conf-tin \
	./config.guess \
	./config.sub \
	./configure \
	./install.sh \
	./mkdirs.sh \
	$(TOLDIR)/expiretover \
	$(TOLDIR)/metamutt \
	$(TOLDIR)/opt-case.pl \
	$(TOLDIR)/tinlock \
	$(TOLDIR)/tinews.pl \
	$(TOLDIR)/tinrcupdate.pl \
	$(TOLDIR)/url_handler.sh \
	$(TOLDIR)/w2r.pl \
	$(PCREDIR)/RunTest \
	$(PCREDIR)/perltest \
	$(PCREDIR)/version.sh \
	$(CANDIR)/Build

tar:
	@$(ECHO) "Generating gzipped tar file..."
	@-$(RM) -f $(PROJECT)-$(VER).tar.gz
	@$(TAR) cvf $(PROJECT)-$(VER).tar -C ../ \
	`$(ECHO) $(ALL_FILES) \
	| $(TR) -s '[[:space:]]' "[\012*]" \
	| $(SED) 's,^\./,$(PROJECT)-$(VER)/,' \
	| $(TR) "[\012]" " "`
	@$(GZIP) -9 $(PROJECT)-$(VER).tar
	@$(CHMOD) 644 $(PROJECT)-$(VER).tar.gz
	@$(LS) -l $(PROJECT)-$(VER).tar.gz

bzip2:
	@$(ECHO) "Generating bzipped tar file..."
	@-$(RM) -f $(PROJECT)-$(VER).tar.bz2
	@$(TAR) cvf $(PROJECT)-$(VER).tar -C ../ \
	`$(ECHO) $(ALL_FILES) \
	| $(TR) -s '[[:space:]]' "[\012*]" \
	| $(SED) 's,^\./,$(PROJECT)-$(VER)/,' \
	| $(TR) "[\012]" " "`
	@$(BZIP2) -9 $(PROJECT)-$(VER).tar
	@$(CHMOD) 644 $(PROJECT)-$(VER).tar.bz2
	@$(LS) -l $(PROJECT)-$(VER).tar.bz2

#
# I know it's ugly, but it works
#
name:
	@DATE=`date +%Y%m%d` ; NAME=`basename \`pwd\`` ;\
	if $(TEST) $$NAME != "$(PROJECT)-$(VER)" ; then \
		$(MV) ../$$NAME ../$(PROJECT)-$(VER) ;\
	fi ;\
	$(SED) "s,^PACKAGE=[[:print:]]*,PACKAGE=$(PROJECT)," ./configure.in > ./configure.in.out && \
	$(SED) "s,^VERSION=[[:print:]]*,VERSION=$(VER)," ./configure.in.out > ./configure.in && \
	$(RM) ./configure.in.out ;\
	$(SED) "s,^DVER[[:space:]]*=[[:print:]]*,DVER	= $$DATE," ./Makefile > ./Makefile.tmp && \
	$(MV) ./Makefile.tmp ./Makefile ;\
	$(SED) "s,RELEASEDATE[[:space:]]*\"[[:print:]]*\",RELEASEDATE	\"$$DATE\"," $(INCDIR)/version.h > $(INCDIR)/version.h.tmp && \
	$(SED) "s, VERSION[[:space:]]*\"[[:print:]]*\", VERSION		\"$(VER)\"," $(INCDIR)/version.h.tmp > $(INCDIR)/version.h && \
	$(RM) $(INCDIR)/version.h.tmp ;\
	$(MAKE) configure

dist:
	@$(MAKE) name
	@-if $(TEST) -r $(PODIR)/Makefile ; then $(CD) $(PODIR) && $(MAKE) ; fi
	@$(MAKE) manifest
	@$(MAKE) chmod
	@$(MAKE) tar
	@$(MAKE) bzip2

version:
	@$(ECHO) "$(PROJECT)-$(VER)"

distclean:
	@-$(MAKE) clean
	@-if $(TEST) -r $(PODIR)/Makefile ; then $(CD) $(PODIR) && $(MAKE) distclean ; fi
	@-if $(TEST) -r $(INTLDIR)/Makefile ; then $(CD) $(INTLDIR) && $(MAKE) distclean ; fi
	@-$(RM) -f \
	config.cache \
	config.log \
	config.status \
	td-conf.out \
	$(INCDIR)/autoconf.h \
	$(PCREDIR)/chartables.c \
	$(PCREDIR)/dftables \
	$(PCREDIR)/pcre.h \
	$(PCREDIR)/Makefile \
	$(SRCDIR)/Makefile \
	$(INTLDIR)/po2tbl.sed \
	$(CANDIR)/*.[oa] \
	$(CANDIR)/endian.h \
	$(CANDIR)/canlocktest \
	$(CANDIR)/endian \
	$(CANDIR)/hmactest

configure: configure.in aclocal.m4
	autoconf

config.status: configure
	./config.status --recheck
