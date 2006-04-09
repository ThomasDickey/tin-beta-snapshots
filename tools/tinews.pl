#! /usr/bin/perl -w
#
# reads an article on STDIN, mails any copies if requied,
# signs the article and posts it.
#
#
# Copyright (c) 2002-2006 Urs Janssen <urs@tin.org>,
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
# TODO: - FIXME add debug mode which doesn't delete tmp-files and is verbose
#         (e.g. warns about missing mime-headers if body contains 8bit
#          chars)
#       - add pgp6 support
#       - check for ~/.newsauth (and ~/.nntpauth?) and use username/password
#         if found
#       - check for /etc/nntpserver (and /etc/news/server)
#       - allow config in ~/.tinewsrc
#       - add $PGPOPTS, $PGPPATH, $GNUPGHOME support
#       - cleanup, remove duplicated code
#
# version Number
my $version = "1.1.10";

my %config;

$config{'NNTPServer'}	= 'news';	# your NNTP servers name
$config{'NNTPPort'}	= 119;		# NNTP-port
$config{'NNTPUser'}	= '';
$config{'NNTPPass'}	= '';

$config{'PGPSigner'}	= '';		# sign as who?
$config{'PGPPass'}	= '';		# pgp2 only
$config{'PathtoPGPPass'}= '';		# pgp2, pgp5 and gpg

$config{'pgp'}		= '/usr/bin/pgp';# path to pgp
$config{'PGPVersion'}	= '2';		# Use 2 for 2.X, 5 for PGP > 2.X and GPG for GPG
$config{'digest-algo'}	= 'MD5';	# Digest Algorithm for GPG -- Must be supported by your installation

$config{'Interactive'}	= "yes";	# allow interactive usage

$config{'add_signature'}= "yes";	# Add ~/.signature to posting if there is no sig

$config{'sendmail'}	= '| /usr/sbin/sendmail -i -t'; # set to '' to disable mail-actions

$config{'PGPSignHeaders'} = ['From', 'Newsgroups', 'Subject', 'Control',
	'Supersedes', 'Followup-To', 'Date', 'Sender', 'Approved',
	'Message-ID', 'Reply-To', 'Cancel-Key', 'Also-Control',
	'Distribution'];
$config{'PGPorderheaders'} = ['from', 'newsgroups', 'subject', 'control',
	'supersedes', 'followup-To', 'date', 'organization', 'lines',
	'sender', 'approved', 'distribution', 'message-id',
	'references', 'reply-to', 'mime-version', 'content-type',
	'content-transfer-encoding', 'summary', 'keywords', 'cancel-lock',
	'cancel-key', 'also-control', 'x-pgp', 'user-agent'];

$config{'pgptmpf'}	= 'pgptmp';	# temporary file for PGP.

$config{'pgpheader'}	= 'X-PGP-Sig';
$config{'pgpbegin'}	= '-----BEGIN PGP SIGNATURE-----';	# Begin of PGP-Signature
$config{'pgpend'}	= '-----END PGP SIGNATURE-----';	# End of PGP-Signature

################################################################################

use strict;
use Getopt::Long qw(GetOptions);
use Net::NNTP;
use Time::Local;
use Term::ReadLine;

my $pname = $0;
$pname =~ s#^.*/##;

my %cli_headers;

$config{'NNTPServer'} = $ENV{'NNTPSERVER'} if ($ENV{'NNTPSERVER'});
$config{'NNTPPort'} = $ENV{'NNTPPORT'} if ($ENV{'NNTPPORT'});

# Get options:
$Getopt::Long::ignorecase=0;
GetOptions('A|V|W|O|no-organization|h|headers' => [], #do nothing
	'debug|D|N'	=> \$config{'debug'}, #XXX
	'port|p=i'	=> \$config{'NNTPPort'},
	'no-sign|X'	=> \$config{'no_sign'},
	'no-control|R'	=> \$config{'no_control'},
	'no-signature|S'	=> \$config{'no_signature'},
	'approved|a=s'	=> \$config{'approved'},
	'control|c=s'	=> \$config{'control'},
	'distribution|d=s'	=> \$config{'distribution'},
	'expires|e=s'	=> \$config{'expires'},
	'from|f=s'	=> \$config{'from'},
	'followupto|w=s'	=> \$config{'followup-to'},
	'newsgroups|n=s'	=> \$config{'newsgroups'},
	'replyto|r=s'	=> \$config{'reply-to'},
	'subject|t=s'	=> \$config{'subject'},
	'references|F=s'	=> \$config{'references'},
	'organization|o=s'	=> \$config{'organization'},
	'path|x=s'	=> \$config{'path'},
	'help|H'	=> \$config{'help'}
);

foreach (@ARGV) {
	print STDERR "Unknown argument $_.";
	usage();
}

usage() if($config{'help'});

my $term = new Term::ReadLine 'tinews';
my $attribs = $term->Attribs;
my $in_header = 1;
my (@Header, %Header, @Body, $PGPCommand);

if (! $config{'no_sign'}) {
	$config{'PGPSigner'} = $ENV{'SIGNER'} if ($ENV{'SIGNER'});

	$config{'PathtoPGPPass'} = $ENV{'PGPPASSFILE'} if ($ENV{'PGPPASSFILE'});
	if ($config{'PathtoPGPPass'}) {
		open (PGPPass, $config{'PathtoPGPPass'}) or
			$config{'Interactive'} && die ("$0: Can't open ".$config{'PathtoPGPPass'}.": $!");
		chomp ($config{'PGPPass'} = <PGPPass>);
		close(PGPPass);
	}
	if ($config{'PGPVersion'} eq '2') {
		$config{'PGPPass'} = $ENV{'PGPPASS'} if ($ENV{'PGPPASS'});
	}
}


# Read the message and split the header
readarticle(\%Header, \@Body);

# Add signature if there is none
if (!$config{'no_signature'}) {
	if ($config{'add_signature'} && !grep /^-- /, @Body) {
		if (-r (glob("~/.signature"))[0]) {
			my $l = 0;
			push @Body, "-- \n";
			open SIGNATURE, (glob("~/.signature"))[0] or die "Can't open ~/.signature: $!";
			while (<SIGNATURE>){
				die "~/.signature longer than 4 lines!" if (++$l > 4);
				push @Body, $_;
			}
			close SIGNATURE;
		} else {
			warn "Tried to add ~/.signature, but ~/.signature is not readable";
		}
	}
}

# import headers set in the environment
if (!defined($Header{'reply-to'})) {
	if ($ENV{'REPLYTO'}) {
		chomp ($Header{'reply-to'} = "Reply-To: " . $ENV{'REPLYTO'});
		$Header{'reply-to'} .= "\n";
	}
}
foreach ('DISTRIBUTION', 'ORGANIZATION') {
	if (!defined($Header{lc($_)}) && $ENV{$_}) {
		chomp ($Header{lc($_)} = ucfirst($_).": " . $ENV{$_});
		$Header{lc($_)} .= "\n";
	}
}

# overwrite headers if specified via cmd-line
foreach ('Approved', 'Control', 'Distribution', 'Expires',
	'From', 'Followup-To', 'Newsgroups',' Reply-To', 'Subject',
	'References', 'Organization', 'Path') {
	next if (!defined($config{lc($_)}));
	chomp ($Header{lc($_)} = $_ . ": " . $config{lc($_)});
	$Header{lc($_)} .= "\n";
}

# verify/add/remove headers
foreach ('From', 'Subject') {
	die "$0: No $_:-header defined." if (!defined($Header{lc($_)}));
}

$Header{'date'} = "Date: ".getdate()."\n" if (!defined($Header{'date'}) || $Header{'date'} !~ m/^[^\s:]+: .+/o);

if (defined($Header{'user-agent'})) {
	chomp $Header{'user-agent'};
	$Header{'user-agent'} = $Header{'user-agent'}." ".$pname."/".$version."\n";
}

delete $Header{'x-pgp-key'} if (!$config{'no_sign'} && defined($Header{'x-pgp-key'}));


# No control messages allowed when using -R|--no-control
if ($config{'no_control'} and $Header{control}) {
	print STDERR "No control messages allowed.\n";
	exit 1;
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
if ($config{'sendmail'} && defined($Header{'newsgroups'}) && (defined($Header{'to'}) || defined($Header{'cc'}) || defined($Header{'bcc'}))) {
	foreach ('to', 'bcc', 'cc') {
		if (defined($Header{$_}) && $Header{$_} ne $Header{'from'}) {
			$Header{'posted-and-mailed'} = "Posted-And-Mailed: yes\n";
			last;
		}
	}
}

if (! $config{'no_sign'}) {
	if (!$config{'PGPSigner'}) {
		chomp ($config{'PGPSigner'} = $Header{'from'});
		$config{'PGPSigner'} =~ s/^[^\s:]+: (.*)/$1/;
	}
	$PGPCommand = getpgpcommand($config{'PGPVersion'});
}

# (re)move mail-headers
my ($To, $Cc, $Bcc, $Newsgroups) = '';
$To = $Header{'to'} if (defined($Header{'to'}));
$Cc = $Header{'cc'} if (defined($Header{'cc'}));
$Bcc = $Header{'bcc'} if (defined($Header{'bcc'}));
delete $Header{$_} foreach ('to', 'cc', 'bcc');
$Newsgroups = $Header{'newsgroups'} if (defined($Header{'newsgroups'}));

my $MessageR = [];

if ($config{'no_sign'}) {
	# don't sign article
	push @$MessageR, $Header{$_} for (keys %Header);
	push @$MessageR, "\n", @Body;
} else {
	# sign article
	$MessageR = signarticle(\%Header, \@Body);
}

# post article
postarticle($MessageR) if ($Newsgroups);

# mail article
if (($To || $Cc || $Bcc) && $config{'sendmail'}) {
	open(MAIL, $config{'sendmail'}) || die "$!";
	unshift @$MessageR, "$To" if ($To);
	unshift @$MessageR, "$Cc" if ($Cc);
	unshift @$MessageR, "$Bcc" if ($Bcc);
	print(MAIL @$MessageR);
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
# User, Password and Server are defined before as elements
# of the global hash %config. If no values for user or password
# are defined, the sub will try to ask the user (only if
# $config{'Interactive'} is != 0).
sub AuthonNNTP {
	my $Server = Net::NNTP->new($config{'NNTPServer'}, Reader => 1, Debug => 0, Port => $config{'NNTPPort'})
		or die "$0: Can't connect to ".$config{'NNTPServer'}.":".$config{'NNTPPort'}."!\n";
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
		if ($config{'NNTPPass'} eq "") {
			if ($config{'Interactive'}) {
				$config{'NNTPUser'} = $term->readline("Your Username at ".$config{'NNTPServer'}.": ");
				$attribs->{redisplay_function} = $attribs->{shadow_redisplay};
				$config{'NNTPPass'} = $term->readline("Password for ".$config{'NNTPUser'}." at ".$config{'NNTPServer'}.": ");
			} else {
				$ServerMsg = $Server->message();
				$Server->quit();
				die ($0.": ".$ServerCod." ".$ServerMsg."\n");
			}
		}
		$Server->authinfo($config{'NNTPUser'}, $config{'NNTPPass'});
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
		if ($config{'NNTPPass'} eq "") {
			if ($config{'Interactive'}) {
				$config{'NNTPUser'} = $term->readline("Your Username at ".$config{'NNTPServer'}.": ");
				$attribs->{redisplay_function} = $attribs->{shadow_redisplay};
				$config{'NNTPPass'} = $term->readline("Password for ".$config{'NNTPUser'}." at ".$config{'NNTPServer'}.": ");
			} else {
				$ServerMsg = $Server->message();
				$Server->quit();
				die ($0.": ".$ServerCod." ".$ServerMsg."\n");
			}
		}
		$Server->authinfo($config{'NNTPUser'}, $config{'NNTPPass'});
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
		if ($config{'PGPPass'}) {
			$PGPCommand = "PGPPASS=\"".$config{'PGPPass'}."\" ".$config{'pgp'}." -z -u \"".$config{'PGPSigner'}."\" +verbose=0 language='en' -saft <".$config{'pgptmpf'}.".txt >".$config{'pgptmpf'}.".txt.asc";
		} elsif ($config{'Interactive'}) {
			$PGPCommand = $config{'pgp'}." -z -u \"".$config{'PGPSigner'}."\" +verbose=0 language='en' -saft <".$config{'pgptmpf'}.".txt >".$config{'pgptmpf'}.".txt.asc";
		} else {
			die "$0: Passphrase is unknown!\n";
		}
	} elsif ($PGPVersion eq '5') {
		if ($config{'PathtoPGPPass'}) {
			$PGPCommand = "PGPPASSFD=42 ".$config{'pgp'}."s -u \"".$config{'PGPSigner'}."\" -t --armor -o ".$config{'pgptmpf'}.".txt.asc -z -f < ".$config{'pgptmpf'}.".txt 42<".$config{'PathtoPGPPass'};
		} elsif ($config{'Interactive'}) {
			$PGPCommand = $config{'pgp'}."s -u \"".$config{'PGPSigner'}."\" -t --armor -o ".$config{'pgptmpf'}.".txt.asc -z -f < ".$config{'pgptmpf'}.".txt";
		} else {
			die "$0: Passphrase is unknown!\n";
		}
	} elsif ($PGPVersion =~ m/GPG/io) {
		if ($config{'PathtoPGPPass'}) {
			$PGPCommand = $config{'pgp'}." --digest-algo $config{'digest-algo'} -a -u \"".$config{'PGPSigner'}."\" -o ".$config{'pgptmpf'}.".txt.asc --no-tty --batch --passphrase-fd 42 42<".$config{'PathtoPGPPass'}." --clearsign ".$config{'pgptmpf'}.".txt";
		} elsif ($config{'Interactive'}) {
			$PGPCommand = $config{'pgp'}." --digest-algo $config{'digest-algo'} -a -u \"".$config{'PGPSigner'}."\" -o ".$config{'pgptmpf'}.".txt.asc --no-secmem-warning --no-batch --clearsign ".$config{'pgptmpf'}.".txt";
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

	foreach (@{$config{'PGPSignHeaders'}}) {
		if (defined($$HeaderR{lc($_)}) && $$HeaderR{lc($_)} =~ m/^[^\s:]+: .+/o) {
			push @signheaders, $_;
		}
	}

	$pgpbody = join ("", @$BodyR);

	# Delete and create the temporary pgp-Files
	unlink $config{'pgptmpf'}.".txt";
	unlink $config{'pgptmpf'}.".txt.asc";
	$signheaders = join(",", @signheaders);

	$pgphead = "X-Signed-Headers: $signheaders\n";
	foreach $header (@signheaders) {
		if ($$HeaderR{lc($header)} =~ m/^[^\s:]+: (.+?)\n?$/so) {
			$pgphead .= $header.": ".$1."\n";
		}
	}

	unless (substr($pgpbody,-1,1)=~ /\n/ ) {$pgpbody.="\n"};
	open(FH, ">" . $config{'pgptmpf'} . ".txt") or die "$0: can't open ".$config{'pgptmpf'}.": $!\n";
	print FH $pgphead, "\n", $pgpbody;
	print FH "\n" if ($config{'PGPVersion'} =~ m/GPG/io);	# workaround a pgp/gpg incompatibility - should IMHO be fixed in pgpverify
	close(FH) or warn "$0: Couldn't close TMP: $!\n";

	# Start PGP, then read the signature;
	`$PGPCommand`;

	open (FH, "<" . $config{'pgptmpf'} . ".txt.asc") or die "$0: can't open ".$config{'pgptmpf'}.".txt.asc: $!\n";
	$/ = "\n".$config{'pgpbegin'}."\n";
	$_ = <FH>;
	unless (m/\Q$config{'pgpbegin'}\E$/o) {
		unlink $config{'pgptmpf'} . ".txt";
		unlink $config{'pgptmpf'} . ".txt.asc";
		die "$0: ".$config{'pgpbegin'}." not found in ".$config{'pgptmpf'}.".txt.asc\n"
	}
	unlink($config{'pgptmpf'} . ".txt") or warn "$0: Couldn't unlink ".$config{'pgptmpf'}.".txt: $!\n";

	$/ = "\n";
	$_ = <FH>;
	unless (m/^Version: (\S+)(?:\s(\S+))?/o) {
		unlink $config{'pgptmpf'} . ".txt.asc";
		die "$0: didn't find PGP Version line where expected.\n";
	}
	if (defined($2)) {
		$$HeaderR{$config{'pgpheader'}} = $1."-".$2." ".$signheaders;
	} else {
		$$HeaderR{$config{'pgpheader'}} = $1." ".$signheaders;
	}
	do {			# skip other pgp headers like
		$_ = <FH>;	# "charset:"||"comment:" until empty line
	} while ! /^$/;

	while (<FH>) {
		chomp;
		last if /^\Q$config{'pgpend'}\E$/;
		$$HeaderR{$config{'pgpheader'}} .= "\n\t$_";
	}
	$$HeaderR{$config{'pgpheader'}} .= "\n" unless ($$HeaderR{$config{'pgpheader'}} =~ /\n$/s);

	$_ = <FH>;
	unless (eof(FH)) {
		unlink $config{'pgptmpf'} . ".txt.asc";
		die "$0: unexpected data following ".$config{'pgpend'}."\n";
	}
	close(FH);
	unlink $config{'pgptmpf'} . ".txt.asc";

	my $tmppgpheader = $config{'pgpheader'} . ": " . $$HeaderR{$config{'pgpheader'}};
	delete $$HeaderR{$config{'pgpheader'}};

	@pgphead = ();
	foreach $header (@{$config{PGPorderheaders}}) {
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

	push @pgphead, ("X-PGP-Hash: " . $config{'digest-algo'} . "\n");
	push @pgphead, ("X-PGP-Key: " . $config{'PGPSigner'} . "\n"), $tmppgpheader;
	undef $tmppgpheader;

	@pgpbody = split /$/m, $pgpbody;
	my @pgpmessage = (@pgphead, "\n", @pgpbody);
	return \@pgpmessage;
}


sub usage {
	print $pname." ".$version."\n";
	print "Usage: ".$pname." [OPTS] < article\n";
	print "  -a string  set Approved:-header to string\n";
	print "  -c string  set Control:-header to string\n";
	print "  -d string  set Distribution:-header to string\n";
	print "  -e string  set Expires:-header to string\n";
	print "  -f string  set From:-header to string\n";
	print "  -n string  set Newsgroups:-header to string\n";
	print "  -o string  set Organization:-header to string\n";
	print "  -p port    use port as NNTP port [default=".$config{'NNTPPort'}."]\n";
	print "  -r string  set Reply-To:-header to string\n";
	print "  -t string  set Subject:-header to string\n";
	print "  -w string  set Followup-To:-header to string\n";
	print "  -x string  set Path:-header to string\n";
	print "  -H         show help\n";
	print "  -R         disallow control messages\n";
	print "  -S         do not append \$HOME/.signature\n";
	print "  -X         do not sign article\n";
	exit 0;
}

__END__

=head1 NAME

tinews.pl - Post and sign an article via NNTP

=head1 SYNOPSIS

B<tinews.pl> [B<OPTIONS>] E<lt> I<input>

=head1 DESCRIPTION

B<tinews.pl> reads an article on STDIN, signs it via B<pgp>(1) or
B<gpg>(1) and posts it to a newsserver.

If the article contains To:, Cc: or Bcc: headers and mail-actions are
configured it will automatically add a "Posted-And-Mailed: yes" header
to the article and send out the mail-copies.

=head1 OPTIONS

=over 4

=item -B<a> C<Approved> | --B<approved> C<Approved>

Set the article header field Approved: to the given value.

=item -B<c> C<Control> | --B<control> C<Control>

Set the article header field Control: to the given value.

=item -B<d> C<Distribution> | --B<distribution> C<Distribution>

Set the article header field Distribution: to the given value.

=item -B<e> C<Expires> | --B<expires> C<Expires>

Set the article header field Expires: to the given value.

=item -B<f> C<From> | --B<from> C<From>

Set the article header field From: to the given value.

=item -B<n> C<Newsgroups> | --B<newsgroups> C<Newsgroups>

Set the article header field Newsgroups: to the given value.

=item -B<o> C<Organization> | --B<organization> C<Organization>

Set the article header field Organization: to the given value.

=item -B<p> C<port> | --B<port> C<port>

use C<port> as NNTP-port

=item -B<r> C<Reply-To> | --B<replyto> C<Reply-To>

Set the article header field Reply-To: to the given value.

=item -B<t> C<Subject> | --B<subject> C<Subject>

Set the article header field Subject: to the given value.

=item -B<w> C<Followup-To> | --B<followupto> C<Followup-To>

Set the article header field Followup-To: to the given value.

=item -B<x> C<Path> | --B<path> C<Path>

Set the article header field Path: to the given value.

=item -B<H> | --B<help>

Show help-page.

=item -B<R> | --B<no-control>

Restricted mode, disallow control-messages.

=item -B<S> | --B<no-signature>

Do not append F<$HOME/.signature>

=item -B<X> | --B<no-sign>

Do not sign the article.

=item -B<A> -B<V> -B<W>

These options are accepted for compatibility reasons but ignored.

=item -B<h> | --B<headers>

These options are accepted for compatibility reasons but ignored.

=item -B<O> | --B<no-organization>

These options are accepted for compatibility reasons but ignored.

=item -B<D> | -B<N> | --B<debug>

These options are accepted but do not have any functionality yet.

=back

=head1 EXIT STATUS

The following exit values are returned:

=over 4

=item S< 0>

Successful completion.

=item S<!=0>

An error occurred.

=back

=head1 ENVIRONMENT

=over 4

=item B<$NNTPSERVER>

Set to override the NNTP server configured in the source.

=item B<$NNTPPORT>

The NNTP TCP-port to post news to. This variable only needs to be set if the
TCP-port is not 119 (the default). The '-B<p>' command-line option overrides
B<$NNTPPORT>.

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
the variable if there isn't already a Reply-To: header in the article.
The '-B<r>' command-line option overrides B<$REPLYTO>.

=item B<$ORGANIZATION>

Set the article header field Organization: to the contents of the variable
if there isn't already a Organization: header in the article. The '-B<o>'
command-line option overrides B<$ORGANIZATION>.

=item B<$DISTRIBUTION>

Set the article header field Distribution: to the contents of the variable
if there isn't already a Distribution: header in the article. The '-B<d>'
command-line option overrides B<$DISTRIBUTION>.

=back

=head1 FILES

=over 4

=item F<pgptmp.txt>

Temporary file used to store the reformatted article

=item F<pgptmp.txt.asc>

Temporary file used to store the reformatted and signed article

=item F<$PGPPASSFILE>

The passphrase file to be used for B<pgp>(1) or B<gpg>(1).

=item F<$HOME/.signature>

Signature-file which will be automatically included.

=back

=head1 SECURITY

If you've configured or entered a password, even if the variable that
contained that password has been erased, it may be possible for someone to
find that password, in plaintext, in a core dump. In short, if serious
security is an issue, don't use this script.

=head1 NOTES

B<tinews.pl> is designed to be used with B<pgp>(1)-2.6.3,
B<pgp>(1)-5 and B<gpg>(1).

B<tinews.pl> requires the following standard modules to be installed:
B<Getopt::Long>(3pm), B<Net::NNTP>(3pm), B<Time::Local>(3pm) and
B<Term::Readline>(3pm).

=head1 AUTHOR

Urs Janssen E<lt>urs@tin.orgE<gt>,
Marc Brockschmidt E<lt>marc@marcbrockschmidt.deE<gt>

=head1 SEE ALSO

B<pgp>(1), B<gpg>(1), B<pgps>(1), B<Getopt::Long>(3pm), B<Net::NNTP>(3pm),
B<Time::Local>(3pm), B<Term::Readline>(3pm)

=cut
