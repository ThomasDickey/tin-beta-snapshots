#! /usr/bin/perl -w
#
# reads an article on STDIN, mails any copies if requied,
# signs the article and posts it.
#
#
# Copyright (c) 2002-2003 Urs Janssen <urs@tin.org>,
#                         Marc Brockschmidt <marc@marcbrockschmidt.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# TODO: - add pgp6 support
#       - add debug mode which doesn't dele tmp-files and is verbose
#       - cleanup, remove duplicated code
#       - turn off echoing if prompting for password
#       - check for ~/.newsauth and use username/password if found
#
# version Number
my $version = "1.0.9";

# TODO: put into a "my %config('NNTPServer' => 'news', ... );" array
my $NNTPServer	= 'news';		# your NNTP servers name
my $NNTPUser	= '';
my $NNTPPass	= '';
my $PGPSigner	= '';			# sign as who?
my $PGPPass	= '';			# pgp2 only
my $PathtoPGPPass = '';			# pgp2, pgp5 and gpg

my $pgp		= '/usr/bin/pgp';	# path to pgp
my $PGPVersion	= '2';			# Use 2 for 2.X, 5 for PGP > 2.X and GPG for GPG

my $Interactive = 1;			# allow interactive usage

my $sendmail	= '| /usr/sbin/sendmail -i -t'; # set to '' to disable mail-actions

my @PGPSignHeaders = ('From', 'Newsgroups', 'Subject', 'Control',
	'Supersedes', 'Followup-To', 'Date', 'Sender', 'Approved',
	'Message-ID', 'Reply-To', 'Cancel-Key', 'Also-Control',
	'Distribution');
my @PGPorderheaders = ('from', 'newsgroups', 'subject', 'control',
	'supersedes', 'followup-To', 'date', 'organization', 'lines',
	'sender', 'approved', 'distribution', 'message-id',
	'references', 'reply-to', 'mime-version', 'content-type',
	'content-transfer-encoding', 'summary', 'keywords', 'cancel-lock',
	'cancel-key', 'also-control', 'x-pgp', 'user-agent');

my $pgptmpf	= 'pgptmp';		# temporary file for PGP.

my $pgpheader	= 'X-PGP-Sig';
my $pgpbegin	= '-----BEGIN PGP SIGNATURE-----';	# Begin of PGP-Signature
my $pgpend	= '-----END PGP SIGNATURE-----';	# End of PGP-Signature

################################################################################

use strict;
use Net::NNTP;
use Time::Local;
use Term::ReadLine;

my $pname = $0;
$pname =~ s#^.*/##;

my $term = new Term::ReadLine 'tinews';
my $in_header = 1;
my (@Header, %Header, @Body, $PGPCommand);

$NNTPServer = $ENV{'NNTPSERVER'} if ($ENV{'NNTPSERVER'});
$PGPSigner = $ENV{'SIGNER'} if ($ENV{'SIGNER'});

$PathtoPGPPass = $ENV{'PGPPASSFILE'} if ($ENV{'PGPPASSFILE'});
if ($PathtoPGPPass) {
	open (PGPPass, "$PathtoPGPPass") or die ("$0: Can't open $PathtoPGPPass: $!");
	# TODO: ignore any errors if running $Interactive as we can prompt
	#       the user later?
	chomp ($PGPPass = <PGPPass>);
	close(PGPPass);
}
if ($PGPVersion eq '2') {
	$PGPPass = $ENV{'PGPPASS'} if ($ENV{'PGPPASS'});
}


# Read the message and split the header
readarticle(\%Header, \@Body);


# verify/add/remove headers
foreach ('From', 'Subject') {
	die "$0: No $_:-header defined." if (!defined($Header{lc($_)}));
}

$Header{'date'} = "Date: ".getdate()."\n" if (!defined($Header{'date'}) || $Header{'date'} !~ m/^[^\s:]+: .+/o);

if (defined($Header{'user-agent'})) {
	chomp $Header{'user-agent'};
	$Header{'user-agent'} = $Header{'user-agent'}." ".$pname."/".$version."\n";
}

delete $Header{'x-pgp-key'} if (defined($Header{'x-pgp-key'}));

if (!defined($Header{'organization'})) {
	if ($ENV{'ORGANIZATION'}) {
		chomp ($Header{'organization'} = "Organization: " . $ENV{'ORGANIZATION'});
		$Header{'organization'} .= "\n";
	}
}

if (!defined($Header{'reply-to'})) {
	if ($ENV{'REPLYTO'}) {
		chomp ($Header{'reply-to'} = "Reply-To: " . $ENV{'REPLYTO'});
		$Header{'reply-to'} .= "\n";
	}
}

if (defined($Header{'newsgroups'}) && !defined($Header{'message-id'})) {
	my $Server = AuthonNNTP();
	my $ServerMsg = $Server->message();
	$Server->datasend('.');
	$Server->dataend();
	$Server->quit();
	$Header{'message-id'} = "Message-ID: $1\n" if ($ServerMsg =~ m/(<\S+\@\S+>)/o);
}

if (!defined($Header{'message-id'})) {
	chomp (my $hname = `hostname`);
	my ($hostname,) = gethostbyname($hname);
	$Header{'message-id'} = "Message-ID: " . sprintf ("<N%xI%xT%x@%s>\n", $>, timelocal(localtime), $$, $hostname);
}

# set Posted-And-Mailed if we send a mailcopy to someone else
if ($sendmail && defined($Header{'newsgroups'}) && (defined($Header{'to'}) || defined($Header{'cc'}) || defined($Header{'bcc'}))) {
	foreach ('to', 'bcc', 'cc') {
		if (defined($Header{$_}) && $Header{$_} ne $Header{'from'}) {
			$Header{'posted-and-mailed'} = "Posted-And-Mailed: yes\n";
			last;
		}
	}
}

if (!$PGPSigner) {
	chomp ($PGPSigner = $Header{'from'});
	$PGPSigner =~ s/^[^\s:]+: (.*)/$1/;
}
$PGPCommand = getpgpcommand($PGPVersion);

# (re)move mail-headers
my ($To, $Cc, $Bcc, $Newsgroups) = '';
$To = $Header{'to'} if (defined($Header{'to'}));
$Cc = $Header{'cc'} if (defined($Header{'cc'}));
$Bcc = $Header{'bcc'} if (defined($Header{'bcc'}));
delete $Header{$_} foreach ('to', 'cc', 'bcc');
$Newsgroups = $Header{'newsgroups'} if (defined($Header{'newsgroups'}));

# sign article
my $SignedMessageR = signarticle(\%Header, \@Body);

# post article
postarticle($SignedMessageR) if ($Newsgroups);

# mail article
if (($To || $Cc || $Bcc) && $sendmail) {
	open(MAIL, $sendmail) || die "$!";
	unshift @$SignedMessageR, "$To" if ($To);
	unshift @$SignedMessageR, "$Cc" if ($Cc);
	unshift @$SignedMessageR, "$Bcc" if ($Bcc);
	print(MAIL @$SignedMessageR);
	close(MAIL);
}

# Game over. Insert new coin.
exit;


#-------- sub readarticle
#
sub readarticle {
	my ($HeaderR, $BodyR) = @_;
	my $currentheader;
	while (defined($_ = <>)) {
		if ($in_header) {
			if (m/^$/o) { #end of header
				$in_header = 0;
			} elsif (m/^([^\s:]+): (.*)$/s) {
				$currentheader = lc($1);
				$$HeaderR{$currentheader} = "$1: $2";
			} elsif (m/^[ \t]/o) {
				$$HeaderR{$currentheader} .= $_;
			} else {
				chomp($_);
				die ("'$_' is not a correct header-line");
			}
		} else {
			push @$BodyR, $_;
		}
	}
}

#-------- sub askuser
# askuser uses Term::Readline to ask the user a question and returns his
# answer(s).
#
# Receives:
# 	- $AnsRef: A reference to a scalar which will hold the answer.
# 	- $Question: A scalar containing the question.
sub askuser {
	my ($AnsRef, $Question) = @_;
	$$AnsRef = $term->readline($Question);
}


#-------- sub getdate
# getdate generates a date and returns it.
#
sub getdate {
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
	if ($offset < 0) {
		$sign ="-";
		$offset *= -1;
	}
	my $offseth = int($offset/3600);
	my $offsetm = int(($offset - $offseth*3600)/60);
	my $tz = sprintf ("%s%0.2d%0.2d", $sign, $offseth, $offsetm);
	return "$wday, $day $monthN $year $hh:$mm:$ss $tz";
}


#-------- sub AuthonNNTP
# AuthonNNTP opens the connection to a Server and returns a Net::NNTP-Object.
#
# User, Password and Server are defined before as global
# scalars ($NNTPServer, $NNTPUser, $NNTPPass). If no values
# for user or password are defined, the sub will try to
# ask the user (only if $Interactive is != 0).
sub AuthonNNTP {
	my $Server = Net::NNTP->new($NNTPServer, Reader => 1, Debug => 0) or die "$0: Can't connect to $NNTPServer!\n";
	my $ServerMsg = "";
	my $ServerCod = $Server->code();

	# no read and/or write access - give up
	if ($ServerCod < 200 || $ServerCod > 201) {
		$ServerMsg = $Server->message();
		$Server->quit();
		die ($0.": ".$ServerCod." ".$ServerMsg."\n");
	}

	# read access - try auth
	if ($ServerCod == 201) {
		if ($NNTPPass eq "") {
			if ($Interactive) {
				askuser(\$NNTPUser, "Your Username at $NNTPServer: ");
				askuser(\$NNTPPass, "Password for $NNTPUser at $NNTPServer: ");
			} else {
				$ServerMsg = $Server->message();
				$Server->quit();
				die ($0.": ".$ServerCod." ".$ServerMsg."\n");
			}
		}
		$Server->authinfo($NNTPUser, $NNTPPass);
		$ServerCod = $Server->code();
		$ServerMsg = $Server->message();
		if ($ServerCod != 281) { # auth failed
			$Server->quit();
			die $0.": ".$ServerCod." ".$ServerMsg."\n";
		}
	}

	$Server->post();
	$ServerCod = $Server->code();
	if ($ServerCod == 480) {
		if ($NNTPPass eq "") {
			if ($Interactive) {
				askuser(\$NNTPUser, "Your Username at $NNTPServer: ");
				askuser(\$NNTPPass, "Password for $NNTPUser at $NNTPServer: ");
			} else {
				$ServerMsg = $Server->message();
				$Server->quit();
				die ($0.": ".$ServerCod." ".$ServerMsg."\n");
			}
		}
		$Server->authinfo($NNTPUser, $NNTPPass);
		$Server->post();
	}
	return $Server;
}


#-------- sub getpgpcommand
# getpgpcommand generates the command to sign the message and returns it.
#
# Receives:
# 	- $PGPVersion: A scalar holding the PGPVersion
sub getpgpcommand {
	my ($PGPVersion) = @_;
	my $PGPCommand;

	if ($PGPVersion eq '2') {
		if ($PGPPass) {
			$PGPCommand = "PGPPASS=\"".$PGPPass."\" ".$pgp." -z -u \"".$PGPSigner."\" +verbose=0 language='en' -saft <".$pgptmpf.".txt >".$pgptmpf.".txt.asc";
		} elsif ($Interactive) {
			$PGPCommand = $pgp." -z -u \"".$PGPSigner."\" +verbose=0 language='en' -saft <".$pgptmpf.".txt >".$pgptmpf.".txt.asc";
		} else {
			die "$0: Passphrase is unknown!\n";
		}
	} elsif ($PGPVersion eq '5') {
		if ($PathtoPGPPass) {
			$PGPCommand = "PGPPASSFD=42 ".$pgp."s -u \"".$PGPSigner."\" -t --armor -o ".$pgptmpf.".txt.asc -z -f < ".$pgptmpf.".txt 42<".$PathtoPGPPass;
		} elsif ($Interactive) {
			$PGPCommand = $pgp."s -u \"".$PGPSigner."\" -t --armor -o ".$pgptmpf.".txt.asc -z -f < ".$pgptmpf.".txt";
		} else {
			die "$0: Passphrase is unknown!\n";
		}
	} elsif ($PGPVersion =~ m/GPG/io) {
		if ($PathtoPGPPass) {
			$PGPCommand = $pgp." --digest-algo MD5 -a -u \"".$PGPSigner."\" -o ".$pgptmpf.".txt.asc --no-tty --batch --passphrase-fd 42 42<".$PathtoPGPPass." --clearsign ".$pgptmpf.".txt";
		} elsif ($Interactive) {
			$PGPCommand = $pgp." --digest-algo MD5 -a -u \"".$PGPSigner."\" -o ".$pgptmpf.".txt.asc --no-secmem-warning --no-batch --clearsign ".$pgptmpf.".txt";
		} else {
			die "$0: Passphrase is unknown!\n";
		}
	} else {
		die "$0: Unknown PGP-Version $PGPVersion!";
	}
	return $PGPCommand;
}


#-------- sub postarticle
# postarticle posts your article to your Newsserver.
#
# Receives:
# 	- $ArticleR: A reference to an array containing the article
sub postarticle {
	my ($ArticleR) = @_;

	my $Server = AuthonNNTP();
	my $ServerCod = $Server->code();
	if ($ServerCod == 340) {
		$Server->datasend(@$ArticleR);
		$Server->dataend();
		if (!$Server->ok()) {
			my $ServerMsg = $Server->message();
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
		if (defined($$HeaderR{lc($_)}) && $$HeaderR{lc($_)} =~ m/^[^\s:]+: .+/o) {
			push @signheaders, $_;
		}
	}

	$pgpbody = join ("", @$BodyR);

	# Delete and create the temporary pgp-Files
	unlink "$pgptmpf.txt";
	unlink "$pgptmpf.txt.asc";
	$signheaders = join(",", @signheaders);

	$pgphead = "X-Signed-Headers: $signheaders\n";
	foreach $header (@signheaders) {
		if ($$HeaderR{lc($header)} =~ m/^[^\s:]+: (.+?)\n?$/so) {
			$pgphead .= $header.": ".$1."\n";
		}
	}

	open(FH, ">" . $pgptmpf . ".txt") or die "$0: can't open $pgptmpf: $!\n";
	print FH $pgphead, "\n", $pgpbody;
	print FH "\n" if ($PGPVersion =~ m/GPG/io);	# workaround a pgp/gpg incompatibility - should IMHO be fixed in pgpverify
	close(FH) or warn "$0: Couldn't close TMP: $!\n";

	# Start PGP, then read the signature;
	`$PGPCommand`;

	open (FH, "<" . $pgptmpf . ".txt.asc") or die "$0: can't open ".$pgptmpf.".txt.asc: $!\n";
	$/ = "$pgpbegin\n";
	$_ = <FH>;
	unless (m/\Q$pgpbegin\E$/o) {
		unlink $pgptmpf . ".txt";
		unlink $pgptmpf . ".txt.asc";
		die "$0: $pgpbegin not found in ".$pgptmpf.".txt.asc\n"
	}
	unlink($pgptmpf . ".txt") or warn "$0: Couldn't unlink $pgptmpf.txt: $!\n";

	$/ = "\n";
	$_ = <FH>;
	unless (m/^Version: (\S+)(?:\s(\S+))?/o) {
		unlink $pgptmpf . ".txt";
		unlink $pgptmpf . ".txt.asc";
		die "$0: didn't find PGP Version line where expected.\n";
	}
	if (defined($2)) {
		$$HeaderR{$pgpheader} = $1."-".$2." ".$signheaders;
	} else {
		$$HeaderR{$pgpheader} = $1." ".$signheaders;
	}
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
	unlink "$pgptmpf.txt.asc";

	my $tmppgpheader = $pgpheader . ": " . $$HeaderR{$pgpheader};
	delete $$HeaderR{$pgpheader};

	@pgphead = ();
	foreach $header (@PGPorderheaders) {
		if ($$HeaderR{$header} && $$HeaderR{$header} ne "\n") {
			push(@pgphead, "$$HeaderR{$header}");
			delete $$HeaderR{$header};
		}
	}

	foreach $header (keys %$HeaderR) {
		if ($$HeaderR{$header} && $$HeaderR{$header} ne "\n") {
			push(@pgphead, "$$HeaderR{$header}");
			delete $$HeaderR{$header};
		}
	}

	push @pgphead, ("X-PGP-Key: " . $PGPSigner . "\n"), $tmppgpheader;
	undef $tmppgpheader;

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

Passphrase file used for B<pgp>(1) or B<gpg>(1).

=item B<$SIGNER>

Set to override the user-id for signing configured in the source. If you
neither set B<$SIGNER> nor configure it in the source the contents of the
From:-field will be used.

=item B<$REPLYTO>

Set the article header field Reply-To: to the return address specified by
the variable if there isn't allready a Reply-To: header in the article.

=item B<$ORGANIZATION>

Set the article header field Organization: to the contents of the variable
if there isn't allready a Organization: header in the article.

=back

=head1 FILES

=over 4

=item F<pgptmp.txt>

Temporary file used to store the reformated article

=item F<pgptmp.txt.asc>

Temporary file used to store the reformated and signed article

=item F<$PGPPASSFILE>

The passphrase file to be used for B<pgp>(1) or B<gpg>(1).

=back

=head1 SECURITY

If interactive usage is configured and B<tinews.pl> prompts for the
NNTP-password the input is echoed to the terminal.

If you've configured or entered a password, even if the variable that
contained that password has been erased, it may be possible for someone to
find that password, in plaintext, in a core dump. In short, if serious
security is an issue, don't use this script.

=head1 NOTES

B<tinews.pl> is designed to be used with B<pgp>(1)-2.6.3,
B<pgp>(1)-5 and B<gpg>(1).

B<tinews.pl> requires the following standard modules to be installed:
B<Net::NNTP>(3pm), B<Time::Local>(3pm) and B<Term::Readline>(3pm).

=head1 AUTHOR

Urs Janssen E<lt>urs@tin.orgE<gt>,
Marc Brockschmidt E<lt>marc@marcbrockschmidt.deE<gt>

=head1 SEE ALSO

B<pgp>(1), B<gpg>(1), B<pgps>(1), B<Net::NNTP>(3pm), B<Time::Local>(3pm),
B<Term::Readline>(3pm)

=cut
