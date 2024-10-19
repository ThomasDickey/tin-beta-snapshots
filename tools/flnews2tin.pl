#! /usr/bin/perl
#
# WARNING: THIS SCRIPT IS UNTESTED
#
# turn a flnews(1) score file into a tin(1) regexp filter file.
#
# flnews2tin.pl < ~/.config/flnews/scorefile > checkme.tmp
# cat checkme.tmp >> ~/.tin/filter
#
# 2024-09-30 <urs@tin.org>

require 5.008;

use strict;
use warnings;

# version Number
# my $version = "0.0.7";

my ($line, $target, $type, $score, $string, $prefix, $suffix, @comments);

# max score value (tin limit)
use constant SCORE_MAX => 10000;

while (defined($line = <>)) {
    chomp $line;

    # ignore empty lines
    next if ($line =~ m/^(?:\s|$)/);

    # save comments
    if ($line =~ s/^#\s*//) {
        push @comments, $line;
        next;
    }

    # skip lines with less than 3 colons
    if ($line !~ m/^([^:]+):([^:]+):([^:]+):(.*)/) {
        next;
    } else {
        $target = $1;
        $type = $2;
        $score = $3;
        $string = $4;
    }

    # skip non target
    next if ($target =~ m/^\s*$/);

    # skip non types
    next if ($type !~ m/^(?:from|from_ere|group|msgid_ere|subject|subject_ere)$/);

    # skip non scores
    next if ($score !~ m/^\s*[+-]?\d+\s*$/);

    # skip non string
    next if ($string =~ m/^$/);

    # looks like we have a valid line, here we go
    unshift(@comments, "converted from: ".$line);
    $prefix = $suffix = "";

    # stuff which needs to be anchored and quoted
    if ($type =~ m/^(?:from|group|subject)$/) {
        $suffix = "\$";
        $string = quotemeta($string);
        $string =~ s/(?<!\\)\\([ !@,;:#<>-])/$1/g; # undo excessive quoting
    }

    $prefix = "from=^" if ($type eq "from");
    $prefix = "from=" if ($type eq "from_ere");

    if ($type eq "group") {
        $prefix = "xref=(?:^|,)";
        $suffix = "(?:,|\$)";
    }

    $prefix = "msgid_only=" if ($type eq "msgid_ere");

    $prefix = "subj=^" if ($type eq "subject");
    $prefix = "subj=" if ($type eq "subject_ere");

    # cosmetics
    $score =~ s/\s+//g;

    # check limits
    $score = SCORE_MAX if ($score > SCORE_MAX);
    $score = -SCORE_MAX if ($score < -SCORE_MAX);

    # print rule
    map{print "\ncomment=$_"} @comments;
    print "\ngroup=".$target."\n";
    print "case=0\n";
    print "score=".$score."\n";
    print $prefix.$string.$suffix."\n";
    @comments = ();
}
