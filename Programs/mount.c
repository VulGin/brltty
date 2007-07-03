/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2007 by The BRLTTY Developers.
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

#include "prologue.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>
#include <sys/mount.h>

#include "misc.h"
#include "async.h"
#include "mount.h"

static FILE *
openMountsTable (int update) {
  FILE *table = setmntent(MOUNTED, (update? "a": "r"));
  if (!table)
    LogPrint((errno == ENOENT)? LOG_WARNING: LOG_ERR,
             "mounted file systems table open erorr: %s: %s",
             MOUNTED, strerror(errno));
  return table;
}

char *
getMountPoint (int (*test) (const char *path, const char *type)) {
  char *path = NULL;
  FILE *table;

  if ((table = openMountsTable(0))) {
    struct mntent *entry;

    while ((entry = getmntent(table))) {
      if (test(entry->mnt_dir, entry->mnt_type)) {
        path = strdupWrapper(entry->mnt_dir);
        break;
      }
    }

    endmntent(table);
  }

  return path;
}

static int
addMountEntry (FILE *table, struct mntent *entry) {
  if (!addmntent(table, entry)) return 1;
  LogPrint(LOG_ERR, "mounts table entry add error: %s[%s] -> %s: %s",
           entry->mnt_type, entry->mnt_fsname, entry->mnt_dir, strerror(errno));
  return 0;
}

static void updateMountsTable (struct mntent *entry);

static void
retryMountsTableUpdate (void *data) {
  struct mntent *entry = data;
  updateMountsTable(entry);
}

static void
updateMountsTable (struct mntent *entry) {
  int retry = 0;

  {
    FILE *table;

    if ((table = openMountsTable(1))) {
      addMountEntry(table, entry);
      endmntent(table);
    } else if ((errno == EROFS) || (errno == EACCES)) {
      retry = 1;
    }
  }

  if (retry) {
    asyncRelativeAlarm(5000, retryMountsTableUpdate, entry);
  } else {
    if (entry->mnt_dir) free(entry->mnt_dir);
    if (entry->mnt_fsname) free(entry->mnt_fsname);
    if (entry->mnt_type) free(entry->mnt_type);
    if (entry->mnt_opts) free(entry->mnt_opts);
    free(entry);
  }
}

int
mountFileSystem (const char *path, const char *reference, const char *type) {
  if (mount(reference, path, type, 0, NULL) != -1) {
    LogPrint(LOG_NOTICE, "file system mounted: %s[%s] -> %s",
             type, reference, path);

    {
      struct mntent *entry = mallocWrapper(sizeof(*entry));
      memset(entry, 0, sizeof(*entry));
      entry->mnt_dir = strdupWrapper(path);
      entry->mnt_fsname = strdupWrapper(reference);
      entry->mnt_type = strdupWrapper(type);
      entry->mnt_opts = strdupWrapper(MNTOPT_RW);
      updateMountsTable(entry);
    }

    return 1;
  } else {
    LogPrint(LOG_ERR, "file system mount error: %s[%s] -> %s: %s",
             type, reference, path, strerror(errno));
  }

  return 0;
}
