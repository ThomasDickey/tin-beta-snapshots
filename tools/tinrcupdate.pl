#! /usr/bin/perl -w
#
# quick and dirty hack which reads an old tinrc-file on STDIN,
# updates it and returns the result on STDOUT
#
# 2002-05-15 <urs@tin.org>
#
# TODO: - add missing updates (full_page_scroll, show_xcommentto)
#       - add documentation
#
# version Number
# $VERSION = "0.0.6";

# current tinrc version number
my $rc_version="1.3.0";

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

my $thread_articles = -1;		# bool -> int change

my $quote_style = 5;			# default=quote_empty_lines|compress_quotes
# denioj ,devomer ,detadpu eb ot seulav cr #

while (defined($line = <>)) {
	chomp $line;

	# update version number
	if ($line =~ m/# tin configuration file V(.*)$/o) {
		die "Nothing to convert" if ($1 eq $rc_version);
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
		$thread_articles = 3 if ($1 =~ m/on/oi);
		$thread_articles = 0 if ($1 =~ m/off/oi);
		$thread_articles = $1 if ($thread_articles < 0);
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

print "quote_style=".$quote_style"\n";
