#! /usr/bin/perl -w
#
# quick and dirty hack which reads an old tinrc-file on STDIN,
# updates it and returns the result on STDOUT
#
# 2002-12-09 <urs@tin.org>
#
# TODO: - add missing updates (full_page_scroll, show_xcommentto)
#       - rewrite in C and integrate into config.c:check_upgrade()
#
# NOPS: - word_h_display_marks
#
# suggested usage:
#   ./tinrcupdate.pl < ${TIN_HOMEDIR-"$HOME"}/.tin/tinrc > \
#   ${TIN_HOMEDIR-"$HOME"}/.tin/tinrc.$$ && \
#   mv -f ${TIN_HOMEDIR-"$HOME"}/.tin/tinrc.$$ \
#   ${TIN_HOMEDIR-"$HOME"}/.tin/tinrc || \
#   rm ${TIN_HOMEDIR-"$HOME"}/.tin/tinrc.$$
#
# version Number
# $VERSION = "0.1.6";

# tinrc version number this script creates
my $rc_version = "1.3.6";
my $m_rc_version = 0;

# version number in input tinrc: x.y.z -> 10000x+100y+z
my $o_rc_version = 0;

# rc values to be updated, removed, joined #
my $use_getart_limit = 0;		# replaced by getart_limit
my $getart_limit = 0;			# default=0

my $confirm_to_quit = 0;		# replaced by confirm_choice
my $confirm_action = 0;			# replaced by confirm_choice
my $confirm_choice = "";		# default=commands & quit

my $show_last_line_prev_page = 0;	# replaced by scroll_lines
my $scroll_lines = -3;			# default=1

my $use_builtin_inews = 0;		# replaced by inews_prog
my $inews_prog = "";			# default=--internal

my $save_to_mmdf_mailbox = 0;		# replaced by mailbox_format
my $mailbox_format = "";		# default=MBOXO

my $thread_articles = 3;		# bool -> int change

my $quote_style = 5;			# default=quote_empty_lines|compress_quotes

my $show_info = 1;			# replaces show_score and show_lines

my $default_regex_pattern = "";		# misnomer, renamed to default_pattern

my $keep_posted_articles = 0;		# replaced by posted_articles_file
my $posted_articles_file = "posted";	# replaces keep_posted_articles_file

my $metamail_prog = "";			# replaces use_metamail

my $hide_uue = 0;			# bool -> int change
# denioj ,devomer ,detadpu eb ot seulav cr #

while (defined($line = <>)) {
	chomp $line;

	# update version number
	if ($line =~ m/# tin configuration file V(.*)$/o) {
		die "Nothing to convert" if ($1 eq $rc_version);
		$line = $1;
		if ($line =~ m/(\d+)\.(\d+)\.(\d+)/o) {
			$o_rc_version = $3 + 100 * $2 + 10000 * $1;
		} else {
			$o_rc_version = 100 * $2 + 10000 * $1 if ($line =~ m/(\d+)\.(\d+)/o);
		}
		$m_rc_version = $3 + 100 * $2 + 10000 * $1 if ($rc_version =~ m/(\d+)\.(\d+)\.(\d+)/o);
		die "Downgrade not supported" if ($o_rc_version > $m_rc_version);
		print "# tin configuration file V".$rc_version."\n";
		next;
	}

	# use_getart_limit/getart_limit
	if ($line =~ m/^use_getart_limit=(.*)$/o) {
		$use_getart_limit = 0 if ($1 =~ m/off/oi);
		next;
	}
	if ($line =~ m/^getart_limit=(.*)$/o) {
		$getart_limit = $1;
		next;
	}

	# confirm_to_quit/confirm_action
	if ($line =~ m/^confirm_to_quit=(.*)/o) {
		$confirm_to_quit = 1 if ($1 =~ m/on/oi);
		next;
	}
	if ($line =~ m/^confirm_action=(.*)/o) {
		$confirm_action = 1 if ($1 =~ m/on/oi);
		next;
	}
	if ($line =~ m/^confirm_choice=(.*)/o) {
		$confirm_choice = $1;
		next;
	}

	# show_last_line_prev_page
	if ($line =~ m/^show_last_line_prev_page=(.*)/o) {
		$show_last_line_prev_page = 1 if ($1 =~ m/on/oi);
		next;
	}
	if ($line =~ m/^scroll_lines=(.*)/o) {
		$scroll_lines = $1;
		next;
	}

	# use_builtin_inews
	if ($line =~ m/^use_builtin_inews=(.*)/o) {
		$use_builtin_inews = 1 if ($1 =~ m/on/oi);
		next;
	}
	if ($line =~ m/^inews_prog=(.*)/o) {
		$inews_prog = $1;
		next;
	}

	# save_to_mmdf_mailbox
	if ($line =~ m/^save_to_mmdf_mailbox=(.*)/o) {
		$save_to_mmdf_mailbox = 1 if ($1 =~ m/on/oi);
		next;
	}
	if ($line =~ m/^mailbox_format=(.*)/o) {
		$mailbox_format = $1;
		next;
	}

	# thread_articles
	if ($line =~ m/^thread_articles=(.*)/o) {
		if ($1 =~ m/off/oi) {
			$thread_articles = 0;
		} else {
			if ($1 =~ m/on/oi) {
				$thread_articles = 3;
			} else {
				$thread_articles = $1;
			}
		}
		next;
	}

	# hide_uue
	if ($line =~ m/^hide_uue=(.*)/o) {
		if ($1 =~ m/on/oi) {
			$hide_uue = 1;
		} else {
			if ($1 =~ m/off/oi) {
				$hide_uue = 0;
			} else {
				$hide_uue = $1;
			}
		}
		next;
	}

	# quote_style
	if ($line =~ m/^compress_quotes=(?i)on/o) {
		$quote_style |= 1;
		next;
	}
	if ($line =~ m/^quote_signatures=(?i)on/o) {
		$quote_style |= 2;
		next;
	}
	if ($line =~ m/^quote_empty_lines=(?i)on/o) {
		$quote_style |= 4;
		next;
	}
	if ($line =~ m/^quote_style=(.*)/o) {
		$quote_style = $1;
		next;
	}

	# default_regex_pattern was a misnomer
	if ($line =~ m/^default_regex_pattern=(.*)/o) {
		$default_regex_pattern = $1;
		next;
	}

	# show_lines/show_score
	if ($line =~ m/^show_lines=(.*)/o) {
		$show_info -= 1 if ($1 =~ m/(?i)off/o);
		next;
	}
	if ($line =~ m/^show_score=(.*)/o) {
		$show_info += 2 if ($1 =~ m/(?i)on/o);
		next;
	}

	# keep_posted_articles/keep_posted_articles_file
	if ($line =~ m/^keep_posted_articles=(?i)on/o) {
		$keep_posted_articles++;
		next;
	}
	if ($line =~ m/^keep_posted_articles_file=(.*)/o) {
		$posted_articles_file=$1 if ($1 ne "");
		next;
	}

	# use_metamail
	if ($line =~ m/^use_metamail=(.*)/o) {
		$metamail_prog="--internal" if ($1 =~ m/(?i)on/o);
		next;
	}

	# other lines don't need to be translated
	print "$line\n";
}

if ($use_getart_limit) {
	print "getart_limit=".$getart_limit."\n";
} else {
	print "getart_limit=0\n";
}

if ($confirm_choice eq "") {
	if ($confirm_action && $confirm_to_quit) {
		print "confirm_choice=commands & quit\n";
	} else {
		if ($confirm_action) {
			print "confirm_choice=commands\n";
		} else {
			print "confirm_choice=quit\n" if ($confirm_to_quit);
		}
	}
} else {
	print "confirm_choice=".$confirm_choice."\n";
}

if ($scroll_lines eq -3) {
	print "scroll_lines=-1\n" if ($show_last_line_prev_page);
} else {
	print "scroll_lines=".$scroll_lines."\n";
}

if ($inews_prog ne "") {
	print "inews_prog=".$inews_prog."\n";
} else {
	print "inews_prog=--internal\n" if ($use_builtin_inews);
}

if ($mailbox_format ne "") {
	print "mailbox_format=".$mailbox_format."\n";
} else {
	if ($save_to_mmdf_mailbox) {
		print "mailbox_format=MMDF\n";
	} else {
		print "mailbox_format=MBOXO\n";
	}
}

print "thread_articles=".$thread_articles."\n";

print "hide_uue=".$hide_uue."\n";

print "quote_style=".$quote_style."\n";

print "default_pattern=".$default_regex_pattern."\n";

print "show_info=".$show_info."\n";

if ($keep_posted_articles) {
	print "posted_articles_file=".$posted_articles_file."\n";
} else {
	print "posted_articles_file=\n";
}

$metamail = $ENV{'METAMAIL'} if ($ENV{'METAMAIL'});
if ($metamail) {
	if ($metamail =~ m/\(internal\)/o) {
		$metamail_prog="--internal";
#	} else {
#		$metamail_prog=$metamail;
	}
}
print "metamail_prog=".$metamail_prog."\n";
