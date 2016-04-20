#!/bin/sh

if [[ $# -eq 0 ]]
  then
    echo "No arguments supplied;, usage: genfont.sh <location of font> <output point size>"
fi

if [[ -z $LIST ]] ; then
    echo "defaulting to 0-9 font list"
    LIST=`seq 0 9`
fi

FONT=$1
SIZE=$2

echo "bitmap data\n\n"

for i in $LIST; do
    echo "  //$i"
    # at the end, we remove the last 4 bytes to clear out the extra curly brace
    convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep '0x\w\w' | head -c-4
    echo "\n";
done

echo "size table\n\n"
COUNT=0
for i in $LIST; do
    WIDTH=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep "height" | cut -d ' ' -f3`
    HEIGHT=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep "width" | cut -d ' ' -f3`
    BYTESIZE=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep -o "," | wc -l`

    echo "{$WIDTH, $COUNT}, /* $i w: $WIDTH h: $HEIGHT */"
    COUNT=$(( $COUNT + $BYTESIZE ))
done
