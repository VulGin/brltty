/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2005 by The BRLTTY Team. All rights reserved.
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

/* tbl2hex.c - filter to compile 256-byte table file into C code
 * $Id: tbl2hex.c,v 1.3 1996/09/24 01:04:25 nn201 Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "options.h"
#include "misc.h"
#include "brl.h"
#include "ctb.h"
#include "tbl.h"

static char *opt_textTable;
static char *opt_dataDirectory;

BEGIN_OPTION_TABLE
  {"text-table", "file", 't', 0, 0,
   &opt_textTable, TEXT_TABLE,
   "Text translation table."},

  {"data-directory", "file", 'D', 0, OPT_Hidden,
   &opt_dataDirectory, DATA_DIRECTORY,
   "Path to directory for configuration files."},
END_OPTION_TABLE

TranslationTable textTable;
TranslationTable untextTable;
void *contractionTable;

static void
reportTextTableMessage (const char *message) {
  fprintf(stderr, "%s: %s\n", programName, message);
}

typedef struct {
  unsigned char status;
} LineProcessingData;
static int
processLine (char *line, void *data) {
  LineProcessingData *lpd = data;
  unsigned char buffer[0X100];
  int bufferLength = sizeof(buffer);
  int lineLength = strlen(line);
  int offsets[0X100];
  if (contractText(contractionTable, (unsigned char *)line, &lineLength, buffer, &bufferLength, offsets, -1)) {
    int index;
    for (index=0; index<bufferLength; ++index)
      buffer[index] = untextTable[buffer[index]];
    fwrite(buffer, 1, bufferLength, stdout);
    if (!ferror(stdout)) {
      fputc('\n', stdout);
      if (!ferror(stdout)) {
        return 1;
      } else {
        lpd->status = 6;
      }
    } else {
      lpd->status = 6;
    }
  } else {
    lpd->status = 7;
  }
  return 0;
}

int
main (int argc, char *argv[]) {
  int status = 3;
  char *contractionTablePath;

  processOptions(optionTable, optionCount,
                 "ctbtest", &argc, &argv,
                 NULL, NULL, NULL,
                 "contraction-table");

  if (argc == 0) {
    fprintf(stderr, "%s: missing contraction table.\n", programName);
    exit(2);
  }
  contractionTablePath = *argv++, argc--;

  if ((contractionTablePath = makePath(opt_dataDirectory, contractionTablePath))) {
    if ((contractionTable = compileContractionTable(contractionTablePath))) {
      char *textTablePath;
      if ((textTablePath = makePath(opt_dataDirectory, opt_textTable))) {
        if (loadTranslationTable(textTablePath, &textTable, reportTextTableMessage, 0)) {
          LineProcessingData lpd;
          reverseTranslationTable(&textTable, &untextTable);
          lpd.status = 0;
          if (processLines(stdin, processLine, &lpd)) {
            status = lpd.status;
          } else {
            status = 5;
          }
        } else {
          status = 4;
        }
        free(textTablePath);
      } else {
        status = 4;
      }
      destroyContractionTable(contractionTable);
    } else {
      status = 3;
    }
    free(contractionTablePath);
  } else {
    status = 3;
  }
  return status;
}
