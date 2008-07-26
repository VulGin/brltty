/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2008 by The BRLTTY Developers.
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

#ifndef BRLTTY_INCLUDED_TBL
#define BRLTTY_INCLUDED_TBL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "brl.h"

#define TBL_UNDEFINED 0X1
#define TBL_DUPLICATE 0X2
#define TBL_UNUSED    0X4

extern int loadTranslationTable (
  const char *path,
  FILE *file,
  TranslationTable table,
  int options
);

extern unsigned char convertWcharToDots (TranslationTable table, wchar_t character);

extern void reverseTranslationTable (TranslationTable from, TranslationTable to);

extern void fixTextTablePath (char **path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_TBL */
