#!/bin/sh

if [ $# -eq 0 ] ;  then
    echo "No arguments supplied;, usage: genfont.sh <location of font> <output point size>"
fi

BIGFONT=false

if [ -z $LIST ] ; then
    LIST=`seq 0 9`
    BIGFONT=true
fi

FONT=$1
SIZE=$2

if [ $BIGFONT == "true" ] ; then
    cat <<EOF
/********************************
 * adapted from font_sun12x22.c *
 * by Jurriaan Kalkman 06-2005  *
 ********************************/

#include <stdint.h>
#include <Font_Data.h>

// There was much suffering to make something that works right
// Hours and hours of pain and suffering
//
//
// How to make:
// convert -font Dejavu-Sans-Mono -pointsize 24 -rotate 270 -flip -endian LSB label:4 4.xbm
// In the file that outputs, flip the height and width vars that it tells you.
// see \$EVICSDK/tools/genfont.sh

const uint8_t fontdata_10x18[] = {
EOF
else
    echo "bitmap data\n\n"
fi

for i in $LIST; do
    echo "//$i"
    # at the end, we remove the last 4 bytes to clear out the extra curly brace
    convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep -o '0x\w\w' | awk '{ if (NR%8==0) printf $1 ", \n"; else printf $1 ", "; }'
    echo "\n";
done

if [ $BIGFONT == "true" ] ; then
    echo "};\n"
    echo "const Font_CharInfo_t fontdata_10x18_Descriptors[] = {\n"
else
    echo "size table\n\n"
fi

COUNT=0
for i in $LIST; do
    WIDTH=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep "height" | cut -d ' ' -f3`
    HEIGHT=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep "width" | cut -d ' ' -f3`
    BYTESIZE=`convert -font $FONT -pointsize $SIZE -rotate 270 -flip -endian LSB label:$i xbm:- | grep -o "," | wc -l`

    echo "{$WIDTH, $COUNT}, /* $i w: $WIDTH h: $HEIGHT */"
    COUNT=$(( $COUNT + $BYTESIZE ))
done

if [ $BIGFONT == "true" ] ; then
    cat <<EOF
};

const Font_Info_t fontdata_10x18_fontinfo = {
    $HEIGHT, /* Character height */
    '0', /* Start character */
    '9', /* End character */
    15, /* Width, in pixels, of space character */
    fontdata_10x18_Descriptors, /* Character descriptor array */
    fontdata_10x18, /* Character bitmap array */
};
EOF
fi
