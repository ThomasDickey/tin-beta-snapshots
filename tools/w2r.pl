#! /usr/bin/perl -w
#
# reads a tin filter file with wildmat filters on STDIN, converts it to
# regexp filters and returns it on STDOUT
#
# 2000-04-27 <urs@tin.org>
#
# NOTE: don't use w2r.pl on regexp filters
#
# for case optimization of your regexp filters use opt-case.pl, i.e.:
# w2r.pl < wildmat-filter-file | opt-case.pl > regexp-filter-file
#
# for joining regexp filters with the same group= and score= use
# joinf.pl (not written yet)

# perl 5 is needed for lookahead assertions and perl < 5.004 is know
# to be buggy
require 5.004;

# version Number
# $VERSION = "0.2.5";

while (defined($line = <>)) {
	chomp $line;

	# ignore comments etc.
	if ($line =~ m/^(?:[#\s]|$)/o) {
		print "$line\n";
		next;
	}

	# skip 'empty' patterns, they are nonsense
	next if ($line =~ m/^[^=]+=$/o);

	# lines which needs to be translated
	if ($line =~ m/^(subj|from|msgid(?:|_last|_only)|refs_only|xref)=(.*)$/o) {
		printf ("$1=%s\n", w2p($2));
		next;
	}

	# other lines don't need to be translated
	print "$line\n";
}


# turns a wildmat into a regexp
sub w2p {
	local ($wild)  = @_;	# input line
	my $cchar = "";		# current char
	my $lchar = "";		# last char
	my $reg = "";		# translated char
	$bmode = 0;		# inside [] ?
	$rval = "";		# output line

	# break line into chars
	while ($wild =~ s/(.)//) {
		$cchar = $1;

		# if char is a [, and we arn't allreay in []
		if ($lchar !~ m/\\/o && $cchar =~ m/\[/o) {
			$bmode++;
			$reg = $cchar;
		}

		# if char is a ], and we were in []
		if ($lchar !~ m/\\/o && $cchar =~ m/\]/o) {
			$bmode--;
			$reg = $cchar;
		}

		# usual cases
		if ($bmode == 0 && $lchar !~ m/\\/o) {
			$reg = $cchar;
			$reg =~ s/\t/\\t/o;	# translate tabs
			$reg =~ s/\./\\./o;	# quote .
			$reg =~ s/\)/\\)/o;	# quote )
			$reg =~ s/\(/\\(/o;	# quote (
			$reg =~ s/\*/\.*/o;	# translate *
			$reg =~ s/\?/\./o;	# translate ?
			$reg =~ s/\^/\\^/o;	# quote ^
			$reg =~ s/\$/\\\$/o;	# quote $
		}

		# if last char was a qute, current char can't be a meta
		if ($lchar =~ m/\\/o || $bmode != 0) {
			$reg = $cchar;
			$cchar =~ s/\\//o;	# skip 2nd \\ inside []
		}

		$lchar = $cchar;	# store last char
		$rval = $rval.$reg;	# build return string
	}

	# common abbreviations
	#
	# replace [0-9] with [\d] in the first []
	# TODO: make this global
	$rval =~ s/^([^\[]*)\[0-9\]/$1\[\\d\]/o;
	# replace [a-zA-Z_] with [\w] in the first []
	# TODO: make this global
	$rval =~ s/([^\[]*)\[a-za-z0-9_\]/$1\[\\w\]/io;

	# optimizations
	#
	# add ^-anchor if needed
	$rval =~ s/^(?!\.\*)(.*)/\^$1/o;
	# add $-anchor if needed
	$rval =~ s/^((?:.*)(?:[^.][^*]))$/$1\$/o;
	# remove leading .* if allowed
	$rval =~ s/^\.\*(?!$)//o;
	# remove tailing .* if allowed
	$rval =~ s/(.+)\.\*$/$1/o;

	return $rval;
}

__END__

=head1 NAME

w2r.pl - Convert tin wildmat filters to tin regexp filters

=head1 SYNOPSIS

B<w2r.pl> < I<input> [> I<output>]

=head1 DESCRIPTION

B<w2r.pl> reads a B<tin>(1) filter file with wildmat filters on STDIN,
converts it to regexp filters and returns it on STDOUT.

=head1 NOTES

Don't use B<w2r.pl> on regexp filter files

=head1 AUTHOR

Urs Janssen E<lt>urs@tin.orgE<gt>

=head1 SEE ALSO

B<tin>(1), B<tin>(5)

=cut
