#!/bin/sh
# Hazard check for detours: lists hooks whose stolen bytes contain a branch target (a loop back-edge into the
# patched region breaks the trampoline). Usage: check_hooks.sh  (needs /tmp/sqz.dis = objdump -d -M intel of SQZ.EXE)
# For a hit, steal the WHOLE function (if it is tiny and has no outside calls) so internal branches stay valid.
for spec in "$@"; do a=${spec%%:*}; n=${spec##*:}; lo=$((0x$a+1)); hi=$((0x$a+n-1))
  grep -E '^ +[0-9a-f]+:.*\b(j[a-z]+|loop[a-z]*) +0x[0-9a-f]+$' /tmp/sqz.dis | awk -v lo=$lo -v hi=$hi -v base=$((0x$a)) '{ a=strtonum("0x" substr($1,1,length($1)-1)); if (a<base || a>base+0x400) next; t=strtonum($NF); if (t>=lo && t<=hi) print "'$a'", "HAZARD", $1, $NF }'
done
