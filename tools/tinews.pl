#! /usr/bin/perl -w
#
# reads an article on STDIN, mails any copies if requied,
# signs the article and posts it.
#
# 2002-04-05 <urs@tin.org>
#            <marc@marcbrockschmidt.de>
#
# TODO: - add pgp6 support
#       - add debug/verbose mode which doesn't dele tmp-files and is verbose
#       - reorder posting/mailing, do mailing after successfull post
#         (if Newsgroups header was prestent, otherwise order is unimportant)
#       - cleanup, remove dublicated code
#       - turn off echoing if prompting for password
#	- check for ~/.newsauth and use username/password if found
#
# version Number
my $version = "0.8.7";

# TODO: put into a "my %config('NNTPServer' => 'news', ... );" array
my $NNTPServer	= 'news';		# you NNTP servers name
my $NNTPUser	= '';
my $NNTPPass	= '';
my $PGPSigner	= '';			# sign as who?
my $PGPPass	= '';			# aBiGseCreT

my $pgp		= '/usr/bin/pgp';	# path to pgp
my $PGPVersion	= '2';			# Use 2 for 2.X, 5 for PGP > 2.X and GPG for GPG

my $Interactive = 1;			# allow interactive usage

my $sendmail	= '| /usr/sbin/sendmail -t'; # set to '' to disable mail-actions

my @PGPSignHeaders = ('From', 'Newsgroups', 'Subject', 'Followup-To',
	'Control', 'Approved', 'Message-ID', 'Sender', 'Date', 'Cancel-Key',
	'Cancel-Lock');
my @PGPorderheaders = ('From', 'Newsgroups', 'Subject', 'Followup-To',
	'Control', 'Approved', 'Message-ID', 'Sender', 'Date', 'Cancel-Key',
	'Cancel-Lock', 'Organization', 'Mime-Version', 'Content-Type',
	'Content-Transfer-Encoding',);

my $pgptmpf	= 'pgptmp';		# temporary file for PGP.

my $pgpheader	= 'X-PGP-Sig';
my $pgpbegin	= '-----BEGIN PGP SIGNATURE-----';	# Begin of PGP-Signature
my $pgpend	= '-----END PGP SIGNATURE-----';	# End of PGP-Signature

################################################################################

use Net::NNTP;		# only used if (defined(Header{'Newsgroups'}))
use Time::Local;	# only used for Date/Message-ID generation
if ($Interactive) {
	use Term::ReadLine;
}
use strict;

my $term = '';
my $pname = $0;
$pname =~ s#^.*/##;

my $in_header = 1;
my (@Header, %Header, @Body, $PGPCommand, $PathtoPGPPass);

$NNTPServer = $ENV{'NNTPSERVER'} if ($ENV{'NNTPSERVER'});
$PGPSigner = $ENV{'SIGNER'} if ($ENV{'SIGNER'});

if ($PGPVersion eq '2') {
	$PGPPass = $ENV{'PGPPASS'} if ($ENV{'PGPPASS'});
} else {
	$PathtoPGPPass = $ENV{'PGPPASSFILE'} if ($ENV{'PGPPASSFILE'});
	open (PGPPass, "$PathtoPGPPass");
	if (!PGPPass) {
		if (!$Interactive) {
			die ("$0: Can't open $PathtoPGPPass: $!");
		} else {
			# warn ("$0: Can't open $PathtoPGPPass: $!") if ($verbose);
		}
	} else {
		$PGPPass = <PGPPass>;
		close(PGPPass);
	}
}


# Read the message and split the header:
while (defined($_ = <>)) {
	if ($in_header) {
		if (m/^$/) { #end of header
			$in_header = 0;
			next;
		}
		push @Header, $_;
	} else {
		push @Body, $_;
	}
}

my $currentheader;
foreach (@Header) {
	if (m/^(\S+): (.*)$/s) {
		$currentheader = $1;
		$Header{$currentheader} = $2;
	} else {
		$Header{$currentheader} .= $_;
	}
}


# verify/add/remove headers
my ($i);
foreach $i ('From', 'Subject') {
	if (!defined($Header{$i})) {
		die "$0: No $i:-Header defined.";
	}
}

if (!defined($Header{'Date'})) {
	my @time = localtime;
	my $ss = ($time[0]<10) ? "0".$time[0] : $time[0];
	my $mm = ($time[1]<10) ? "0".$time[1] : $time[1];
	my $hh = ($time[2]<10) ? "0".$time[2] : $time[2];
	my $day = $time[3];
	my $month = ($time[4]+1 < 10) ? "0".($time[4]+1) : $time[4]+1;
	my $monthN = ("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec")[$time[4]];
	my $wday = ("Sun","Mon","Tue","Wed","Thu","Fri","Sat")[$time[6]];
	my $year = $time[5] + 1900;
	my $offset = timelocal(localtime) - timelocal(gmtime);
	my $sign ="+";
	if ($offset <= 0) {
		$sign ="-";
		$offset *= -1;
	}
	my $offseth = int($offset/3600);
	my $offsetm = int(($offset - $offseth*3600)/60);
	my $tz = sprintf ("%s%0.2d%0.2d", $sign, $offseth, $offsetm);
	my $rfcdate = "$wday, $day $monthN $year $hh:$mm:$ss $tz";
	$Header{'Date'} = $rfcdate."\n";
}

if (defined($Header{'Newsgroups'}) && !defined($Header{'Message-ID'})) {
	my $Server = Net::NNTP->new($NNTPServer, Reader => 1, Debug => 0) or die "$0: Can't connect to news server $NNTPServer!\n";
	my $ServerMsg = "";
	my $ServerCod = $Server->code();
	if ($ServerCod < 200 || $ServerCod > 201) {
		$ServerMsg = $Server->message();
		$Server->quit();
		die ($0.": ".$ServerCod." ".$ServerMsg."\n");
	}
	if ($ServerCod == 201) {
		if ($NNTPPass ne "") {
			$Server->authinfo($NNTPUser, $NNTPPass);
			$ServerCod = $Server->code();
			$ServerMsg = $Server->message();
		} elsif ($Interactive) {
			$term = new Term::ReadLine 'tinews' if (!$term);
			$NNTPUser = $term->readline("Your Username at $NNTPServer: ");
			chomp $NNTPUser;
			$NNTPPass = $term->readline("Password for $NNTPUser at $NNTPServer:");
			chomp $NNTPPass;
			$Server->authinfo($NNTPUser, $NNTPPass);
			$ServerCod = $Server->code();
			$ServerMsg = $Server->message();
		}
		if ($ServerCod != 281) {
			$Server->quit();
			die $0.": ".$ServerCod." ".$ServerMsg."\n";
		}
	}
	$Server->post();
	$ServerCod = $Server->code();
	if ($ServerCod == 480) {
		if ($NNTPPass ne "") {
			$Server->authinfo($NNTPUser, $NNTPPass);
		} elsif ($Interactive) {
			$term = new Term::ReadLine 'tinews' if (!$term);
			$NNTPUser = $term->readline("Your Username at $NNTPServer: ");
			chomp $NNTPUser;
			$NNTPPass = $term->readline("Password for $NNTPUser at $NNTPServer: ");
			chomp $NNTPPass;
			$Server->authinfo($NNTPUser, $NNTPPass);
		} else {
			$ServerMsg = $Server->message();
			$Server->quit();
			die ($0.": ".$ServerCod." ".$ServerMsg."\n");
		}
		$Server->post();
	}
	$ServerMsg = $Server->message();
	$Server->datasend('.');
	$Server->dataend();
	$Server->quit();
	if ($ServerMsg =~ /(<\S+\@\S+>)/) {
		$Header{'Message-ID'} = "$1\n";
	}
}
if (!defined($Header{'Message-ID'})) {
	my $hname = `hostname`;
	chomp $hname;
	my ($hostname,) = gethostbyname($hname);
	$Header{'Message-ID'} = sprintf ("<N%xI%xT%x@%s>\n", $>, timelocal(localtime), $$, $hostname);
}

if (defined($Header{'User-Agent'})) {
	chomp $Header{'User-Agent'};
	$Header{'User-Agent'} = $Header{'User-Agent'}." ".$pname."/".$version."\n";
}

if (defined($Header{'X-PGP-Key'})) {
	undef $Header{'X-PGP-Key'};
}

# set Posted-And-Mailed if we send a mailcopy to someone else
if ($sendmail && defined($Header{'Newsgroups'}) && (defined($Header{'To'}) || defined($Header{'Cc'}) || defined($Header{'Bcc'}))) {
	my $foo = "";
	foreach ('To', 'Bcc', 'Cc') {
		$foo .= $Header{$_} if (defined($Header{$_}));
	}
	if (length($foo) && $foo !~ m/$Header{'From'}/) {
		$Header{'Posted-And-Mailed'} = "yes\n";
	}
}

if (!$PGPSigner) {
	$PGPSigner = $Header{'From'};
	chomp $PGPSigner;
}

if ($PGPVersion eq '2') {
	if ($PGPPass) {
		$PGPCommand = $pgp." -z \"".$PGPPass."\" -u \"".$PGPSigner."\" +verbose=0 language='en' -saft <".$pgptmpf.".txt >".$pgptmpf.".txt.asc";
	} elsif ($Interactive) {
		$PGPCommand = $pgp." -z -u \"".$PGPSigner."\" +verbose=0 language='en' -saft <".$pgptmpf.".txt >".$pgptmpf.".txt.asc";
	} else {
		die "$0: Passphrase is unknown!\n";
	}
} elsif ($PGPVersion eq '5') {
	if (defined($PathtoPGPPass)) {
		$PGPCommand = "PGPPASSFD=2 ".$pgp."s -u \"".$PGPSigner."\" -t --armor -o ".$pgptmpf.".txt.asc -z -f < ".$pgptmpf.".txt 2< ".$PathtoPGPPass;
	} elsif ($Interactive) {
		$PGPCommand = $pgp."s -u \"".$PGPSigner."\" -t --armor -o ".$pgptmpf.".txt.asc -z -f < ".$pgptmpf.".txt";
	} else {
		die "$0: Passphrase is unknown!\n";
	}
} elsif ($PGPVersion eq 'GPG') {
	if (defined($PathtoPGPPass)) {
		$PGPCommand = $pgp." -a -z 0 -u \"".$PGPSigner."\" -o ".$pgptmpf.".txt.asc --no-tty --batch --passphrase-fd 2 2<".$PathtoPGPPass." --clearsign ".$pgptmpf.".txt";
	} elsif ($Interactive) {
		$PGPCommand = $pgp." -a -z 0 -u \"".$PGPSigner."\" -o ".$pgptmpf.".txt.asc --quiet --no-secmem-warning --no-batch --clearsign ".$pgptmpf.".txt";
	} else {
		die "$0: Passphrase is unknown!\n";
	}
} else {
	die "$0: Unknown PGP-Version $PGPVersion!";
}


# backup headers
my %OHeader = %Header;


# sign article
my $SignedMessageR = signarticle(\%Header, \@Body);


# mail article
if ($sendmail && (defined($OHeader{'To'}) || defined($OHeader{'Cc'}) || defined($OHeader{'Bcc'}))) {
	open(MAIL, $sendmail) || die "$!";
	print(MAIL @$SignedMessageR);
	close(MAIL);
}


# remove mail headers
$in_header = 0;
my ($Delete, @NewSignedMessage);
foreach (@$SignedMessageR) {
	if (!$in_header) {
		if (m/^$/) {
			$in_header++;
			redo;
		} elsif (m/^(To|Bcc|Cc): /){
			$Delete++;
		} elsif (m/^\s+/){
			# Don't use them in @SignedHeader
		} else {
			push @NewSignedMessage, $_;
		}
	} else {
		push @NewSignedMessage, $_;
	}
}


# post article
if (defined($OHeader{'Newsgroups'})) {
	postarticle(\@NewSignedMessage);
}


# Game over. Insert new coin.
exit;


#-------- sub postarticle
# postarticle posts your article to your Newsserver.
#
# Receives:
# 	- $ArticleR: A reference to an array containing the article
sub postarticle {
	my ($ArticleR) = @_;

	my $Server = Net::NNTP->new($NNTPServer, Reader => 1, Debug => 0) or die "$0: Can't connect to news server $NNTPServer!\n";
	my $ServerMsg ="";
	my $ServerCod = $Server->code();
	if ($ServerCod < 200 || $ServerCod > 201) {
		$ServerMsg = $Server->message();
		$Server->quit();
		die ($0.": ".$ServerCod." ".$ServerMsg."\n");
	}
	if ($ServerCod == 201) {
		if ($NNTPPass ne "") {
			$Server->authinfo($NNTPUser, $NNTPPass);
			$ServerCod = $Server->code();
			$ServerMsg = $Server->message();
		} elsif ($Interactive) {
			$term = new Term::ReadLine 'tinews' if (!$term);
			$NNTPUser = $term->readline("Your Username at $NNTPServer: ");
			chomp $NNTPUser;
			$NNTPPass = $term->readline("Password for $NNTPUser at $NNTPServer:");
			chomp $NNTPPass;
			$Server->authinfo($NNTPUser, $NNTPPass);
			$ServerCod = $Server->code();
			$ServerMsg = $Server->message();
		}
		if ($ServerCod != 281) {
			$Server->quit();
			die $0.": ".$ServerCod." ".$ServerMsg."\n";
		}
	}
	$Server->post();
	$ServerCod = $Server->code();
	if ($ServerCod == 480) {
		if ($NNTPPass ne "") {
			$Server->authinfo($NNTPUser, $NNTPPass);
		} elsif ($Interactive) {
			$term = new Term::ReadLine 'tinews' if (!$term);
			$NNTPUser = $term->readline("Your Username at $NNTPServer: ");
			chomp $NNTPUser;
			$NNTPPass = $term->readline("Password for $NNTPUser at $NNTPServer:");
			chomp $NNTPPass;
			$Server->authinfo($NNTPUser, $NNTPPass);
		} else {
			$ServerMsg = $Server->message();
			$Server->quit();
			die ($0.": ".$ServerCod." ".$ServerMsg."\n");
		}
		$Server->post();
	}
	$ServerCod = $Server->code();
	if ($ServerCod == 340) {
		$Server->datasend(@$ArticleR);
		$Server->dataend();
		if (!$Server->ok()) {
			$ServerMsg = $Server->message();
			$Server->quit();
			die ("\n$0: Posting failed! Response from news server:\n", $Server->code(), ' ', $ServerMsg);
		}
		$Server->quit();
	} else {
		die "\n".$0.": Posting failed!\n";
	}
}


#-------- sub signarticle
# signarticle signs an articel and returns a reference to an array
# 	containing the whole signed Message.
#
# Receives:
# 	- $HeaderR: A reference to a hash containing the articles headers.
# 	- $BodyR: A reference to an array containing the body.
#
# Returns:
# 	- $MessageRef: A reference to an array containing the whole message.
sub signarticle {
	my ($HeaderR, $BodyR) = @_;

	my (@pgphead, @pgpbody, $pgphead, $pgpbody, $header, $signheaders, @signheaders);

	foreach (@PGPSignHeaders) {
		push @signheaders, $_ if ($$HeaderR{$_});
	}

	$pgpbody = join ("", @$BodyR);

	# Delete and create the temporary pgp-Files
	unlink "$pgptmpf.txt";
	unlink "$pgptmpf.txt.asc";
	$signheaders = join(",", @signheaders);

	$pgphead = "X-Signed-Headers: $signheaders\n";
	foreach $header (@signheaders) {
		$pgphead .= "$header: $$HeaderR{$header}";
	}

	open(FH, ">" . $pgptmpf . ".txt") or die "$0: can't open $pgptmpf: $!\n";
	print FH $pgphead, "\n", $pgpbody;
	close(FH) or warn "$0: Couldn't close TMP: $!\n";

	# Start PGP, then read the signature;
	`$PGPCommand`;

	open (FH, "<" . $pgptmpf . ".txt.asc") or die "$0: can't open ".$pgptmpf.".txt.asc: $!\n";
	$/ = "$pgpbegin\n";
	$_ = <FH>;
	unless (m/\Q$pgpbegin\E$/) {
		unlink $pgptmpf . ".txt";
		unlink $pgptmpf . ".txt.asc";
		die "$0: $pgpbegin not found in ".$pgptmpf.".txt.asc\n"
	}
	unlink($pgptmpf . ".txt") or warn "$0: Couldn't unlink $pgptmpf.txt: $!\n";

	$/ = "\n";
	$_ = <FH>;
	unless (m/^Version: (\S+)/) {
		unlink $pgptmpf . ".txt";
		unlink $pgptmpf . ".txt.asc";
		die "$0: didn't find PGP Version line where expected.\n";
	}
	$$HeaderR{$pgpheader} = "$1 $signheaders";
	do {			# skip other pgp headers like
		$_ = <FH>;	# "charset:"||"comment:" until empty line
	} while ! /^$/;

	while (<FH>) {
		chomp;
		last if /^\Q$pgpend\E$/;
		$$HeaderR{$pgpheader} .= "\n\t$_";
	}
	$$HeaderR{$pgpheader} .= "\n" unless ($$HeaderR{$pgpheader} =~ /\n$/s);

	$_ = <FH>;
	unless (eof(FH)) {
		unlink $pgptmpf . ".txt";
		unlink $pgptmpf . ".txt.asc";
		die "$0: unexpected data following $pgpend\n";
	}
	close(FH);

	my $tmppgpheader = $pgpheader . ": " . $$HeaderR{$pgpheader};
	delete $$HeaderR{$pgpheader};

	@pgphead = ();
	foreach $header (@PGPorderheaders) {
		if ($$HeaderR{$header} && $$HeaderR{$header} ne "\n") {
			push(@pgphead, "$header: $$HeaderR{$header}");
			delete $$HeaderR{$header};
		}
	}

	foreach $header (keys %$HeaderR) {
		if ($$HeaderR{$header} && $$HeaderR{$header} ne "\n") {
			push(@pgphead, "$header: $$HeaderR{$header}");
			delete $$HeaderR{$header};
		}
	}

	push @pgphead, ("X-PGP-Key: " . $PGPSigner . "\n"), $tmppgpheader;
	undef $tmppgpheader;
	unlink "$pgptmpf.txt.asc";

	@pgpbody = split /$/m, $pgpbody;
	my @pgpmessage = (@pgphead, "\n", @pgpbody);
	return \@pgpmessage;
}

__END__

=head1 NAME

tinews.pl - Post and sign an article via NNTP

=head1 SYNOPSIS

B<tinews.pl> < I<input>

=head1 DESCRIPTION

B<tinews.pl> reads an article on STDIN, signs it via B<pgp>(1) or
B<gpg>(1) and posts it to a newsserver.

If the article contains To:, Cc: or Bcc: headers and mail-actions are
configured it will automatically add a "Posted-And-Mailed: yes" header
to the article and send out the mail-copies.

=head1 OPTIONS

None.

=head1 EXIT STATUS

The following exit values are returned:

=over 4

=item 0

Successful completion.

=item >0

An error occurred.

=back

=head1 ENVIRONMENT

=over 4

=item B<$NNTPSERVER>

Set to override the NNTP server configured in the source.

=item B<$PGPPASS>

Set to override the passphrase configured in the source (used for
B<pgp>(1)-2.6.3).

=item B<$PGPPASSFILE>

Set to override passphrase file configured in the source (used for
B<pgp>(1)-5 or B<gpg>(1)).

=item B<$SIGNER>

Set to override the user-id for signing configured in the source. If you
neither set B<$SIGNER> nor configure it in the source the contents of the
From:-field will be used.

=back

=head1 FILES

=over 4

=item F<pgptmp.txt>

Temporary file used to store the reformated article

=item F<pgptmp.txt.asc>

Temporary file used to store the reformated and signed article

=item F<$PGPPASSFILE>

The passphrase file to be used for B<pgp>(1) 5 or B<gpg>(1).

=back

=head1 SECURITY

If interactive usage is configured and B<tinews.pl> prompts for the
NNTP-password the input is echoed to the terminal.

If you've configured or entered a password, even if the variable that
contained that password has been erased, it may be possible for someone to
find that password, in plaintext, in a core dump. In short, if serious
security is an issue, don't use this script.

=head1 NOTES

B<tinews.pl> is designed to be used with B<pgp>(1)-2.6.3.
B<pgp>(1)-5 and B<gpg>(1) may also work.

B<tinews.pl> requires the following standard modules to be installed:
B<Net::NNTP>(3pm), B<Time::Local>(3pm).

If interactive usage is configured also B<Term::Readline>(3pm) is required.

=head1 AUTHOR

Urs Janssen E<lt>urs@tin.orgE<gt>,
Marc Brockschmidt E<lt>marc@marcbrockschmidt.deE<gt>

=head1 SEE ALSO

B<pgp>(1), B<gpg>(1), B<pgps>(1), B<Net::NNTP>(3pm), B<Time::Local>(3pm),
B<Term::Readline>(3pm)

=cut
