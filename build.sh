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

# echo "Building Windows exe..."
# pcc -DSNORE_VERSION="$CALNUM" "-o$WINEXE" -Ze -Tx86-coff snore.c -LIBPATH:/c/PellesC/Lib/Win
# rm snore.obj
# echo

# exit

# echo "Building Linux exe... hashes must match!"
# rm -f "$LINEXE"
# ssh <snore.c fedora /usr/local/musl/bin/musl-gcc -DSNORE_VERSION="$CALNUM" -static -Os -x c -std=c89 - -o "/tmp/$LINEXE" '&&' sha256sum "/tmp/$LINEXE" '>/dev/stderr' '&&' cat "/tmp/$LINEXE" >"$LINEXE"
# sha256sum "$LINEXE"
# echo

if [ "$1" == "install" ]; then
    echo "installing"
    cp "$WINEXE" /c/mybin/snore.exe
    # sha256sum "$WINEXE" /c/mybin/snore.exe
fi
