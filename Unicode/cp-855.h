/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2002 by The BRLTTY Team. All rights reserved.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

/*
#    Name:     cp855_DOSCyrillic to Unicode table
#    Unicode version: 2.0
#    Table version: 2.00
#    Table format:  Format A
#    Date:          04/24/96
#    Authors:       Lori Brownell <loribr@microsoft.com>
#                   K.D. Chang    <a-kchang@microsoft.com>
#    General notes: none
#
#    Format: Three tab-separated columns
#        Column #1 is the cp855_DOSCyrillic code (in hex)
#        Column #2 is the Unicode (in hex as 0xXXXX)
#        Column #3 is the Unicode name (follows a comment sign, '#')
#
#    The entries are in cp855_DOSCyrillic order
*/
[0x00]=0x0000, //	NULL
[0x01]=0x0001, //	START OF HEADING
[0x02]=0x0002, //	START OF TEXT
[0x03]=0x0003, //	END OF TEXT
[0x04]=0x0004, //	END OF TRANSMISSION
[0x05]=0x0005, //	ENQUIRY
[0x06]=0x0006, //	ACKNOWLEDGE
[0x07]=0x0007, //	BELL
[0x08]=0x0008, //	BACKSPACE
[0x09]=0x0009, //	HORIZONTAL TABULATION
[0x0a]=0x000a, //	LINE FEED
[0x0b]=0x000b, //	VERTICAL TABULATION
[0x0c]=0x000c, //	FORM FEED
[0x0d]=0x000d, //	CARRIAGE RETURN
[0x0e]=0x000e, //	SHIFT OUT
[0x0f]=0x000f, //	SHIFT IN
[0x10]=0x0010, //	DATA LINK ESCAPE
[0x11]=0x0011, //	DEVICE CONTROL ONE
[0x12]=0x0012, //	DEVICE CONTROL TWO
[0x13]=0x0013, //	DEVICE CONTROL THREE
[0x14]=0x0014, //	DEVICE CONTROL FOUR
[0x15]=0x0015, //	NEGATIVE ACKNOWLEDGE
[0x16]=0x0016, //	SYNCHRONOUS IDLE
[0x17]=0x0017, //	END OF TRANSMISSION BLOCK
[0x18]=0x0018, //	CANCEL
[0x19]=0x0019, //	END OF MEDIUM
[0x1a]=0x001a, //	SUBSTITUTE
[0x1b]=0x001b, //	ESCAPE
[0x1c]=0x001c, //	FILE SEPARATOR
[0x1d]=0x001d, //	GROUP SEPARATOR
[0x1e]=0x001e, //	RECORD SEPARATOR
[0x1f]=0x001f, //	UNIT SEPARATOR
[0x20]=0x0020, //	SPACE
[0x21]=0x0021, //	EXCLAMATION MARK
[0x22]=0x0022, //	QUOTATION MARK
[0x23]=0x0023, //	NUMBER SIGN
[0x24]=0x0024, //	DOLLAR SIGN
[0x25]=0x0025, //	PERCENT SIGN
[0x26]=0x0026, //	AMPERSAND
[0x27]=0x0027, //	APOSTROPHE
[0x28]=0x0028, //	LEFT PARENTHESIS
[0x29]=0x0029, //	RIGHT PARENTHESIS
[0x2a]=0x002a, //	ASTERISK
[0x2b]=0x002b, //	PLUS SIGN
[0x2c]=0x002c, //	COMMA
[0x2d]=0x002d, //	HYPHEN-MINUS
[0x2e]=0x002e, //	FULL STOP
[0x2f]=0x002f, //	SOLIDUS
[0x30]=0x0030, //	DIGIT ZERO
[0x31]=0x0031, //	DIGIT ONE
[0x32]=0x0032, //	DIGIT TWO
[0x33]=0x0033, //	DIGIT THREE
[0x34]=0x0034, //	DIGIT FOUR
[0x35]=0x0035, //	DIGIT FIVE
[0x36]=0x0036, //	DIGIT SIX
[0x37]=0x0037, //	DIGIT SEVEN
[0x38]=0x0038, //	DIGIT EIGHT
[0x39]=0x0039, //	DIGIT NINE
[0x3a]=0x003a, //	COLON
[0x3b]=0x003b, //	SEMICOLON
[0x3c]=0x003c, //	LESS-THAN SIGN
[0x3d]=0x003d, //	EQUALS SIGN
[0x3e]=0x003e, //	GREATER-THAN SIGN
[0x3f]=0x003f, //	QUESTION MARK
[0x40]=0x0040, //	COMMERCIAL AT
[0x41]=0x0041, //	LATIN CAPITAL LETTER A
[0x42]=0x0042, //	LATIN CAPITAL LETTER B
[0x43]=0x0043, //	LATIN CAPITAL LETTER C
[0x44]=0x0044, //	LATIN CAPITAL LETTER D
[0x45]=0x0045, //	LATIN CAPITAL LETTER E
[0x46]=0x0046, //	LATIN CAPITAL LETTER F
[0x47]=0x0047, //	LATIN CAPITAL LETTER G
[0x48]=0x0048, //	LATIN CAPITAL LETTER H
[0x49]=0x0049, //	LATIN CAPITAL LETTER I
[0x4a]=0x004a, //	LATIN CAPITAL LETTER J
[0x4b]=0x004b, //	LATIN CAPITAL LETTER K
[0x4c]=0x004c, //	LATIN CAPITAL LETTER L
[0x4d]=0x004d, //	LATIN CAPITAL LETTER M
[0x4e]=0x004e, //	LATIN CAPITAL LETTER N
[0x4f]=0x004f, //	LATIN CAPITAL LETTER O
[0x50]=0x0050, //	LATIN CAPITAL LETTER P
[0x51]=0x0051, //	LATIN CAPITAL LETTER Q
[0x52]=0x0052, //	LATIN CAPITAL LETTER R
[0x53]=0x0053, //	LATIN CAPITAL LETTER S
[0x54]=0x0054, //	LATIN CAPITAL LETTER T
[0x55]=0x0055, //	LATIN CAPITAL LETTER U
[0x56]=0x0056, //	LATIN CAPITAL LETTER V
[0x57]=0x0057, //	LATIN CAPITAL LETTER W
[0x58]=0x0058, //	LATIN CAPITAL LETTER X
[0x59]=0x0059, //	LATIN CAPITAL LETTER Y
[0x5a]=0x005a, //	LATIN CAPITAL LETTER Z
[0x5b]=0x005b, //	LEFT SQUARE BRACKET
[0x5c]=0x005c, //	REVERSE SOLIDUS
[0x5d]=0x005d, //	RIGHT SQUARE BRACKET
[0x5e]=0x005e, //	CIRCUMFLEX ACCENT
[0x5f]=0x005f, //	LOW LINE
[0x60]=0x0060, //	GRAVE ACCENT
[0x61]=0x0061, //	LATIN SMALL LETTER A
[0x62]=0x0062, //	LATIN SMALL LETTER B
[0x63]=0x0063, //	LATIN SMALL LETTER C
[0x64]=0x0064, //	LATIN SMALL LETTER D
[0x65]=0x0065, //	LATIN SMALL LETTER E
[0x66]=0x0066, //	LATIN SMALL LETTER F
[0x67]=0x0067, //	LATIN SMALL LETTER G
[0x68]=0x0068, //	LATIN SMALL LETTER H
[0x69]=0x0069, //	LATIN SMALL LETTER I
[0x6a]=0x006a, //	LATIN SMALL LETTER J
[0x6b]=0x006b, //	LATIN SMALL LETTER K
[0x6c]=0x006c, //	LATIN SMALL LETTER L
[0x6d]=0x006d, //	LATIN SMALL LETTER M
[0x6e]=0x006e, //	LATIN SMALL LETTER N
[0x6f]=0x006f, //	LATIN SMALL LETTER O
[0x70]=0x0070, //	LATIN SMALL LETTER P
[0x71]=0x0071, //	LATIN SMALL LETTER Q
[0x72]=0x0072, //	LATIN SMALL LETTER R
[0x73]=0x0073, //	LATIN SMALL LETTER S
[0x74]=0x0074, //	LATIN SMALL LETTER T
[0x75]=0x0075, //	LATIN SMALL LETTER U
[0x76]=0x0076, //	LATIN SMALL LETTER V
[0x77]=0x0077, //	LATIN SMALL LETTER W
[0x78]=0x0078, //	LATIN SMALL LETTER X
[0x79]=0x0079, //	LATIN SMALL LETTER Y
[0x7a]=0x007a, //	LATIN SMALL LETTER Z
[0x7b]=0x007b, //	LEFT CURLY BRACKET
[0x7c]=0x007c, //	VERTICAL LINE
[0x7d]=0x007d, //	RIGHT CURLY BRACKET
[0x7e]=0x007e, //	TILDE
[0x7f]=0x007f, //	DELETE
[0x80]=0x0452, //	CYRILLIC SMALL LETTER DJE
[0x81]=0x0402, //	CYRILLIC CAPITAL LETTER DJE
[0x82]=0x0453, //	CYRILLIC SMALL LETTER GJE
[0x83]=0x0403, //	CYRILLIC CAPITAL LETTER GJE
[0x84]=0x0451, //	CYRILLIC SMALL LETTER IO
[0x85]=0x0401, //	CYRILLIC CAPITAL LETTER IO
[0x86]=0x0454, //	CYRILLIC SMALL LETTER UKRAINIAN IE
[0x87]=0x0404, //	CYRILLIC CAPITAL LETTER UKRAINIAN IE
[0x88]=0x0455, //	CYRILLIC SMALL LETTER DZE
[0x89]=0x0405, //	CYRILLIC CAPITAL LETTER DZE
[0x8a]=0x0456, //	CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
[0x8b]=0x0406, //	CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
[0x8c]=0x0457, //	CYRILLIC SMALL LETTER YI
[0x8d]=0x0407, //	CYRILLIC CAPITAL LETTER YI
[0x8e]=0x0458, //	CYRILLIC SMALL LETTER JE
[0x8f]=0x0408, //	CYRILLIC CAPITAL LETTER JE
[0x90]=0x0459, //	CYRILLIC SMALL LETTER LJE
[0x91]=0x0409, //	CYRILLIC CAPITAL LETTER LJE
[0x92]=0x045a, //	CYRILLIC SMALL LETTER NJE
[0x93]=0x040a, //	CYRILLIC CAPITAL LETTER NJE
[0x94]=0x045b, //	CYRILLIC SMALL LETTER TSHE
[0x95]=0x040b, //	CYRILLIC CAPITAL LETTER TSHE
[0x96]=0x045c, //	CYRILLIC SMALL LETTER KJE
[0x97]=0x040c, //	CYRILLIC CAPITAL LETTER KJE
[0x98]=0x045e, //	CYRILLIC SMALL LETTER SHORT U
[0x99]=0x040e, //	CYRILLIC CAPITAL LETTER SHORT U
[0x9a]=0x045f, //	CYRILLIC SMALL LETTER DZHE
[0x9b]=0x040f, //	CYRILLIC CAPITAL LETTER DZHE
[0x9c]=0x044e, //	CYRILLIC SMALL LETTER YU
[0x9d]=0x042e, //	CYRILLIC CAPITAL LETTER YU
[0x9e]=0x044a, //	CYRILLIC SMALL LETTER HARD SIGN
[0x9f]=0x042a, //	CYRILLIC CAPITAL LETTER HARD SIGN
[0xa0]=0x0430, //	CYRILLIC SMALL LETTER A
[0xa1]=0x0410, //	CYRILLIC CAPITAL LETTER A
[0xa2]=0x0431, //	CYRILLIC SMALL LETTER BE
[0xa3]=0x0411, //	CYRILLIC CAPITAL LETTER BE
[0xa4]=0x0446, //	CYRILLIC SMALL LETTER TSE
[0xa5]=0x0426, //	CYRILLIC CAPITAL LETTER TSE
[0xa6]=0x0434, //	CYRILLIC SMALL LETTER DE
[0xa7]=0x0414, //	CYRILLIC CAPITAL LETTER DE
[0xa8]=0x0435, //	CYRILLIC SMALL LETTER IE
[0xa9]=0x0415, //	CYRILLIC CAPITAL LETTER IE
[0xaa]=0x0444, //	CYRILLIC SMALL LETTER EF
[0xab]=0x0424, //	CYRILLIC CAPITAL LETTER EF
[0xac]=0x0433, //	CYRILLIC SMALL LETTER GHE
[0xad]=0x0413, //	CYRILLIC CAPITAL LETTER GHE
[0xae]=0x00ab, //	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
[0xaf]=0x00bb, //	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
[0xb0]=0x2591, //	LIGHT SHADE
[0xb1]=0x2592, //	MEDIUM SHADE
[0xb2]=0x2593, //	DARK SHADE
[0xb3]=0x2502, //	BOX DRAWINGS LIGHT VERTICAL
[0xb4]=0x2524, //	BOX DRAWINGS LIGHT VERTICAL AND LEFT
[0xb5]=0x0445, //	CYRILLIC SMALL LETTER HA
[0xb6]=0x0425, //	CYRILLIC CAPITAL LETTER HA
[0xb7]=0x0438, //	CYRILLIC SMALL LETTER I
[0xb8]=0x0418, //	CYRILLIC CAPITAL LETTER I
[0xb9]=0x2563, //	BOX DRAWINGS DOUBLE VERTICAL AND LEFT
[0xba]=0x2551, //	BOX DRAWINGS DOUBLE VERTICAL
[0xbb]=0x2557, //	BOX DRAWINGS DOUBLE DOWN AND LEFT
[0xbc]=0x255d, //	BOX DRAWINGS DOUBLE UP AND LEFT
[0xbd]=0x0439, //	CYRILLIC SMALL LETTER SHORT I
[0xbe]=0x0419, //	CYRILLIC CAPITAL LETTER SHORT I
[0xbf]=0x2510, //	BOX DRAWINGS LIGHT DOWN AND LEFT
[0xc0]=0x2514, //	BOX DRAWINGS LIGHT UP AND RIGHT
[0xc1]=0x2534, //	BOX DRAWINGS LIGHT UP AND HORIZONTAL
[0xc2]=0x252c, //	BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
[0xc3]=0x251c, //	BOX DRAWINGS LIGHT VERTICAL AND RIGHT
[0xc4]=0x2500, //	BOX DRAWINGS LIGHT HORIZONTAL
[0xc5]=0x253c, //	BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
[0xc6]=0x043a, //	CYRILLIC SMALL LETTER KA
[0xc7]=0x041a, //	CYRILLIC CAPITAL LETTER KA
[0xc8]=0x255a, //	BOX DRAWINGS DOUBLE UP AND RIGHT
[0xc9]=0x2554, //	BOX DRAWINGS DOUBLE DOWN AND RIGHT
[0xca]=0x2569, //	BOX DRAWINGS DOUBLE UP AND HORIZONTAL
[0xcb]=0x2566, //	BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
[0xcc]=0x2560, //	BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
[0xcd]=0x2550, //	BOX DRAWINGS DOUBLE HORIZONTAL
[0xce]=0x256c, //	BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
[0xcf]=0x00a4, //	CURRENCY SIGN
[0xd0]=0x043b, //	CYRILLIC SMALL LETTER EL
[0xd1]=0x041b, //	CYRILLIC CAPITAL LETTER EL
[0xd2]=0x043c, //	CYRILLIC SMALL LETTER EM
[0xd3]=0x041c, //	CYRILLIC CAPITAL LETTER EM
[0xd4]=0x043d, //	CYRILLIC SMALL LETTER EN
[0xd5]=0x041d, //	CYRILLIC CAPITAL LETTER EN
[0xd6]=0x043e, //	CYRILLIC SMALL LETTER O
[0xd7]=0x041e, //	CYRILLIC CAPITAL LETTER O
[0xd8]=0x043f, //	CYRILLIC SMALL LETTER PE
[0xd9]=0x2518, //	BOX DRAWINGS LIGHT UP AND LEFT
[0xda]=0x250c, //	BOX DRAWINGS LIGHT DOWN AND RIGHT
[0xdb]=0x2588, //	FULL BLOCK
[0xdc]=0x2584, //	LOWER HALF BLOCK
[0xdd]=0x041f, //	CYRILLIC CAPITAL LETTER PE
[0xde]=0x044f, //	CYRILLIC SMALL LETTER YA
[0xdf]=0x2580, //	UPPER HALF BLOCK
[0xe0]=0x042f, //	CYRILLIC CAPITAL LETTER YA
[0xe1]=0x0440, //	CYRILLIC SMALL LETTER ER
[0xe2]=0x0420, //	CYRILLIC CAPITAL LETTER ER
[0xe3]=0x0441, //	CYRILLIC SMALL LETTER ES
[0xe4]=0x0421, //	CYRILLIC CAPITAL LETTER ES
[0xe5]=0x0442, //	CYRILLIC SMALL LETTER TE
[0xe6]=0x0422, //	CYRILLIC CAPITAL LETTER TE
[0xe7]=0x0443, //	CYRILLIC SMALL LETTER U
[0xe8]=0x0423, //	CYRILLIC CAPITAL LETTER U
[0xe9]=0x0436, //	CYRILLIC SMALL LETTER ZHE
[0xea]=0x0416, //	CYRILLIC CAPITAL LETTER ZHE
[0xeb]=0x0432, //	CYRILLIC SMALL LETTER VE
[0xec]=0x0412, //	CYRILLIC CAPITAL LETTER VE
[0xed]=0x044c, //	CYRILLIC SMALL LETTER SOFT SIGN
[0xee]=0x042c, //	CYRILLIC CAPITAL LETTER SOFT SIGN
[0xef]=0x2116, //	NUMERO SIGN
[0xf0]=0x00ad, //	SOFT HYPHEN
[0xf1]=0x044b, //	CYRILLIC SMALL LETTER YERU
[0xf2]=0x042b, //	CYRILLIC CAPITAL LETTER YERU
[0xf3]=0x0437, //	CYRILLIC SMALL LETTER ZE
[0xf4]=0x0417, //	CYRILLIC CAPITAL LETTER ZE
[0xf5]=0x0448, //	CYRILLIC SMALL LETTER SHA
[0xf6]=0x0428, //	CYRILLIC CAPITAL LETTER SHA
[0xf7]=0x044d, //	CYRILLIC SMALL LETTER E
[0xf8]=0x042d, //	CYRILLIC CAPITAL LETTER E
[0xf9]=0x0449, //	CYRILLIC SMALL LETTER SHCHA
[0xfa]=0x0429, //	CYRILLIC CAPITAL LETTER SHCHA
[0xfb]=0x0447, //	CYRILLIC SMALL LETTER CHE
[0xfc]=0x0427, //	CYRILLIC CAPITAL LETTER CHE
[0xfd]=0x00a7, //	SECTION SIGN
[0xfe]=0x25a0, //	BLACK SQUARE
[0xff]=0x00a0  //	NO-BREAK SPACE
