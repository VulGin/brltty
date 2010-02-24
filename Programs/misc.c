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

#include "prologue.h"

#include <string.h>
#include <errno.h>

#include "misc.h"
#include "log.h"

static void
noMemory (void) {
  LogPrint(LOG_CRIT, "insufficient memory: %s", strerror(errno));
  exit(3);
}

void *
mallocWrapper (size_t size) {
  void *address = malloc(size);
  if (!address && size) noMemory();
  return address;
}

void *
reallocWrapper (void *address, size_t size) {
  if (!(address = realloc(address, size)) && size) noMemory();
  return address;
}

char *
strdupWrapper (const char *string) {
  char *address = strdup(string);
  if (!address) noMemory();
  return address;
}
