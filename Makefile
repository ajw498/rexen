#
# $Id$
#

all: rexen rexen-cgi

rexen: rexen.c
	gcc -Wall rexen.c -o rexen

rexen-cgi: rexen-cgi.c
	gcc -Wall rexen-cgi.c -o rexen-cgi

setup: /proc/sys/fs/binfmt_misc/register
	echo ':riscosaof:M::\x00\x00\xa0\xe1\x00\x00\xa0\xe1::$(PWD)/rexen:' > /proc/sys/fs/binfmt_misc/register

/proc/sys/fs/binfmt_misc/register:
	mount binfmt_misc -t binfmt_misc /proc/sys/fs/binfmt_misc

release:
	-rm -f rexen.zip
	zip -r -, -9 rexen.zip rexen.c rexen-cgi.c COPYING Makefile README -x *CVS* *svn*
