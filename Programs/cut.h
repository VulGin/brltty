/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2004 by The BRLTTY Team. All rights reserved.
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

#ifndef BRLTTY_INCLUDED_CUT
#define BRLTTY_INCLUDED_CUT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

unsigned char *cut_buffer; /* for CMD_NXSEARCH */
extern void cut_begin (int column, int row);
extern void cut_append (int column, int row);
extern int cut_rectangle (int column, int row);
extern int cut_line (int column, int row);
extern int cut_paste (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_CUT */
