#! /usr/bin/perl -w
# example of how to call the appropriate viewer
#
# URLs must start with a scheme and shell metas must be allready quoted
# (tin doesn't recognize URLs without a scheme and it quotes the metas)

use strict;

(my $pname = $0) =~ s#^.*/##;
die "Usage: $pname URL" if $#ARGV != 0;

# version Number
my $version = "0.0.9";

my ($method, $url, $browser, $match, @try);
$method = $url = $ARGV[0];
$method =~ s#^([^:]+):.*#$1#io;

if ($ENV{BROWSER}) {
	push(@try, split(/:/, $ENV{BROWSER}));
} else { # set some defaults
	push(@try, 'firefox -a firefox -remote openURL\(%s\)');
	push(@try, 'mozilla -remote openURL\(%s\)');
	push(@try, 'opera -remote openURL\(%s\)');
	push(@try, 'galeon -n');
	push(@try, 'lynx');	# prefer lynx over links as it can handle news:-urls
	push(@try, qw('links2 -g' links w3m));
	push(@try, 'kfmclient newTab'); # has no usefull return-value on error
}

if ($ENV{DISPLAY}) { # X running
	# try $BROWSER for http, https, gopher, ftp, news, snews
	if ($method =~ m/^(?:https?|gopher|ftp|s?news)$/io) {
		for $browser (@try) {
			# ignore empty parts
			next if ($browser =~ m/^$/o);
			# expand %s if not preceded by odd number of %
			$match = $browser =~ s/(?<!%)((?:%%)*)%s/$1$url/og;
			# expand %c if not preceded by odd number of %
			$browser =~ s/(?<!%)((?:%%)*)%c/$1:/og;
			# reduce dubble %
			$browser =~ s/%%/%/og;
			# append URL if no %s expansion took place
			$browser .= " ".$url if (!$match);
			# leave loop if $browser was started successfull
			last if (system("$browser 2>/dev/null") == 0);
		}
		exit 0;
    }
} else { # no X running
	# try $BROWSER for http, https, gopher, news, snews
	if ($method =~ m/^(?:https?|gopher|s?news)$/io) {
		for $browser (@try) {
			next if ($browser =~ m/^$/o);
			$match = $browser =~ s/(?<!%)((?:%%)*)%s/$1$url/og;
			$browser =~ s/(?<!%)((?:%%)*)%c/$1:/og;
			$browser =~ s/%%/%/og;
			$browser .= " ".$url if (!$match);
			last if (system("$browser 2>/dev/null") == 0);
		}
		exit 0;
	}
	# use ncftp for ftp
	if ($method =~ m/^ftp$/io) {
		system ("ncftp $url") || exit 1;
		exit 0;
	}
}

# no matter if we're using X or not

# use lynx for nntp (as e.g. Netscape can't handle it)
if ($method =~ m/^nntp$/io) {
	system ("lynx $url") || exit 1;
	exit 0;
}

# use mutt for mailto
if ($method =~ m/^mailto$/io) {
	system ("mutt $url") || exit 1;
	# for pine users:
	# system ("pine -url $url") || exit 1;
	exit 0;
}

die "unsupported URL-scheme";

__END__

=head1 NAME

url_handler.pl - Spawn appropriate viewer for a given URL

=head1 SYNOPSIS

B<url_handler.pl> I<URL>

=head1 DESCRIPTION

B<url_handler.pl> takes a URL as argument and spawns the appropriate
viewer with the URL. When running under X11 it follows B<$BROWSER> for
the following schemes: HTTP, HTTPS, GOPHER, FTP, NEWS and SNEWS; when
not running under X11 B<$BROWSER> is considered for HTTP, HTTPS, GOPHER,
NEWS and SNEWS. The schemes NNTP and MAILTO (and FTP when not running
under X11) are handled separately.

=head1 ENVIRONMENT

=over 4

=item B<$BROWSER>

The user's preferred utility to browse URLs. May actually consist of a
sequence of colon-separated browser commands to be tried in order until one
succeeds. If a command part contains %s, the URL is substituted there,
otherwise the browser command is simply called with the URL as its last
argument. %% is replaced by a single percent sign (%), and %c is replaced
by a colon (:).

=back

=head1 SECURITY

B<url_handler.pl> was designed to work together with B<tin>(1) which
only issues shell escaped absolute URLs thus B<url_handler.pl> does not
shell escape it's input nor does it convert relative URLs into absolute
ones! If you use B<url_handler.pl> from other applications be sure to at
least shell escaped it's input!

=head1 AUTHOR

Urs Janssen E<lt>urs@tin.orgE<gt>

=head1 SEE ALSO

http://www.catb.org/~esr/BROWSER/
http://www.dwheeler.com/browse/secure_browser.html

=cut
