/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2010 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

/* brltest.c - Test progrm for the Braille display library
 * $Id: brltest.c,v 1.3 1996/09/24 01:04:24 nn201 Exp $
 */

#include "prologue.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "program.h"
#include "options.h"
#include "brl.h"
#include "log.h"
#include "file.h"
#include "timing.h"
#include "charset.h"
#include "message.h"
#include "defaults.h"

int updateInterval = DEFAULT_UPDATE_INTERVAL;
static BrailleDisplay brl;

static char *opt_brailleDevice;
static char *opt_libraryDirectory;
static char *opt_writableDirectory;
static char *opt_dataDirectory;

BEGIN_OPTION_TABLE(programOptions)
  { .letter = 'D',
    .word = "data-directory",
    .flags = OPT_Hidden,
    .argument = "directory",
    .setting.string = &opt_dataDirectory,
    .defaultSetting = DATA_DIRECTORY,
    .description = "Path to directory for driver help and configuration files."
  },

  { .letter = 'L',
    .word = "library-directory",
    .flags = OPT_Hidden,
    .argument = "directory",
    .setting.string = &opt_libraryDirectory,
    .defaultSetting = LIBRARY_DIRECTORY,
    .description = "Path to directory for loading drivers."
  },

  { .letter = 'W',
    .word = "writable-directory",
    .flags = OPT_Hidden,
    .argument = strtext("directory"),
    .setting.string = &opt_writableDirectory,
    .defaultSetting = WRITABLE_DIRECTORY,
    .description = strtext("Path to directory which can be written to.")
  },

  { .letter = 'd',
    .word = "device",
    .argument = "device",
    .setting.string = &opt_brailleDevice,
    .defaultSetting = BRAILLE_DEVICE,
    .description = "Path to device for accessing braille display."
  },
END_OPTION_TABLE

int
message (const char *mode, const char *string, short flags) {
  size_t length = strlen(string);
  size_t size = brl.textColumns * brl.textRows;
  char buffer[size];

  clearStatusCells(&brl);

  memset(buffer, ' ', size);
  while (length) {
    int count = (length <= size)? length: (size - 1);
    int index;

    for (index=0; index<count; buffer[index++]=*string++);
    if (length -= count) {
      buffer[(count = size) - 1] = WC_C('-');
    }

    {
      wchar_t characters[count];
      wchar_t text[brl.textColumns * brl.textRows];
      convertCharsToWchars(buffer, characters, count);
      fillTextRegion(text, brl.buffer, 0, brl.textColumns, brl.textColumns, brl.textRows, characters, count);
      if (!braille->writeWindow(&brl, text)) return 0;
    }

    if (length) {
      int timer = 0;
      while (braille->readCommand(&brl, BRL_CTX_WAITING) == EOF) {
        if (timer > 4000) break;
        approximateDelay(updateInterval);
        timer += updateInterval;
      }
    }
  }

  return 1;
}

int
main (int argc, char *argv[]) {
  int status;
  const char *driver = NULL;
  void *object;

  {
    static const OptionsDescriptor descriptor = {
      OPTION_TABLE(programOptions),
      .applicationName = "brltest",
      .argumentsSummary = "[driver [parameter=value ...]]"
    };
    processOptions(&descriptor, &argc, &argv);
  }

  {
    char **const paths[] = {
      &opt_libraryDirectory,
      &opt_writableDirectory,
      &opt_dataDirectory,
      NULL
    };
    fixInstallPaths(paths);
  }

  writableDirectory = opt_writableDirectory;

  if (argc) {
    driver = *argv++;
    --argc;
  }
  if (!opt_brailleDevice) opt_brailleDevice = BRAILLE_DEVICE;

  if ((braille = loadBrailleDriver(driver, &object, opt_libraryDirectory))) {
    const char *const *parameterNames = braille->parameters;
    char **parameterSettings;
    if (!parameterNames) {
      static const char *const noNames[] = {NULL};
      parameterNames = noNames;
    }
    {
      const char *const *name = parameterNames;
      unsigned int count;
      char **setting;
      while (*name) ++name;
      count = name - parameterNames;
      if (!(parameterSettings = malloc((count + 1) * sizeof(*parameterSettings)))) {
        LogPrint(LOG_ERR, "insufficient memory.");
        exit(9);
      }
      setting = parameterSettings;
      while (count--) *setting++ = "";
      *setting = NULL;
    }
    while (argc) {
      char *assignment = *argv++;
      int ok = 0;
      char *delimiter = strchr(assignment, '=');
      if (!delimiter) {
        LogPrint(LOG_ERR, "missing braille driver parameter value: %s", assignment);
      } else if (delimiter == assignment) {
        LogPrint(LOG_ERR, "missing braille driver parameter name: %s", assignment);
      } else {
        size_t nameLength = delimiter - assignment;
        const char *const *name = parameterNames;
        while (*name) {
          if (strncasecmp(assignment, *name, nameLength) == 0) {
            parameterSettings[name - parameterNames] = delimiter + 1;
            ok = 1;
            break;
          }
          ++name;
        }
        if (!ok) LogPrint(LOG_ERR, "invalid braille driver parameter: %s", assignment);
      }
      if (!ok) exit(2);
      --argc;
    }

    initializeBrailleDisplay(&brl);
    identifyBrailleDriver(braille, 0);		/* start-up messages */
    if (braille->construct(&brl, parameterSettings, opt_brailleDevice)) {
      if (ensureBrailleBuffer(&brl, LOG_INFO)) {
#ifdef ENABLE_LEARN_MODE
        learnMode(&brl, updateInterval, 10000);
#else /* ENABLE_LEARN_MODE */
        message("braille test", 0);
#endif /* ENABLE_LEARN_MODE */
        braille->destruct(&brl);		/* finish with the display */
        status = 0;
      } else {
        LogPrint(LOG_ERR, "can't allocate braille buffer.");
        status = 6;
      }
    } else {
      LogPrint(LOG_ERR, "can't initialize braille driver.");
      status = 5;
    }
  } else {
    LogPrint(LOG_ERR, "can't load braille driver.");
    status = 3;
  }
  return status;
}

/* dummy functions to allow drivers to link... */
unsigned char
cursorDots (void) {
  return (BRL_DOT7 | BRL_DOT8);
}

int
currentVirtualTerminal (void) {
  return 0;
}
