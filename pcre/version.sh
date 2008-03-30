#! /bin/sh
#
# Provide the current PCRE version information. Do not use numbers
# with leading zeros for the minor version, as they end up in a C
# macro, and may be treated as octal constants. Stick to single
# digits for minor numbers less than 10. There are unlikely to be
# that many releases anyway.

PCRE_MAJOR=7
PCRE_MINOR=0
PCRE_DATE=18-Dec-2006
PCRE_VERSION=${PCRE_MAJOR}.${PCRE_MINOR}
