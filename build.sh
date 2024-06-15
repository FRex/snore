#!/bin/bash

CALVER=$(date '+v%Y_%m_%d')
CALNUM=$(date '+%Y%m%d')

WINEXE="snore32${CALVER}.exe"
PDBFILE="snore32${CALVER}.pdb"

LINEXE="snore32${CALVER}.linux"

echo "Building $WINEXE and $LINEXE"
zig cc -o "$WINEXE" snore.c -O3 -DSNORE_VERSION="$CALNUM" -target x86-windows-gnu &
zig cc -o "$LINEXE" snore.c -O3 -DSNORE_VERSION="$CALNUM" -target x86-linux-musl &
wait
# strip "$LINEXE"
rm "$PDBFILE"

sha1sum "$WINEXE" "$LINEXE"

if [ "$1" == "install" ]; then
    echo "installing"
    cp "$WINEXE" /c/mybin/snore.exe
fi
