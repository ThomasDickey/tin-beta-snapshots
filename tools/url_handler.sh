#! /bin/sh
# example of how to call the appropriate viewer
# based on a script by Michael Elkins <me@cs.hmc.edu>
# 2001-01-31 <urs@tin.org>
#
# URLs must start with a scheme and shell metas must be allready quoted
# (tin doesn't recognize URLs withou a scheme and it quotes the metas)

if test $# -ne 1; then
	echo "Usage: `basename $0` URL" >&2
	exit 1
fi

url=$1
method=`echo $url | sed 's,^\([^:]*\):.*,\1,' | tr 'A-Z' 'a-z'`

case $method in
	http|https|gopher)
		if test x$DISPLAY = x; then
			lynx $url || exit 1
		else
			( netscape -remote openURL\($url\) || netscape $url ) || exit 1
		fi
		;;
	ftp)
		if test x$DISPLAY = x; then
			target=`echo $url | sed 's;^.*://\([^/]*\)/*\(.*\);\1:/\2;'`
			( ncftp $target || ncftp $target"/" ) || exit 1
		else
			( netscape -remote openURL\($url\) || netscape $url ) || exit 1
		fi
		;;
	mailto)
		( mutt `echo $url | sed 's;^[^:]*:\(.*\);\1;'` ) || exit 1
		;;
esac

exit 0
