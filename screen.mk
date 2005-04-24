###############################################################################
# BRLTTY - A background process providing access to the console screen (when in
#          text mode) for a blind person using a refreshable braille display.
#
# Copyright (C) 1995-2005 by The BRLTTY Team. All rights reserved.
#
# BRLTTY comes with ABSOLUTELY NO WARRANTY.
#
# This is free software, placed under the terms of the
# GNU General Public License, as published by the Free Software
# Foundation.  Please see the file COPYING for details.
#
# Web Page: http://mielke.cc/brltty/
#
# This software is maintained by Dave Mielke <dave@mielke.cc>.
###############################################################################

SCR_DEFS ='-DSCRNAME=$(DRIVER_NAME)' '-DSCRCODE=$(DRIVER_CODE)' '-DSCRCOMMENT="$(DRIVER_COMMENT)"'
SCR_CFLAGS = $(LIBCFLAGS) $(SCR_DEFS)
SCR_CXXFLAGS = $(LIBCXXFLAGS) $(SCR_DEFS)
SCR_MOD_NAME = $(BLD_TOP)$(DRV_DIR)/$(MOD_NAME)x$(DRIVER_CODE)
SCR_MOD_FILE = $(SCR_MOD_NAME).$(MOD_EXT)
$(SCR_MOD_FILE): screen.$O
	$(INSTALL_DIRECTORY) $(@D)
	$(MKMOD) $(@) screen.$O $(SCR_OBJS)
screen-driver: $(SCR_MOD_FILE)

install::

uninstall::

clean::
	-rm -f $(SCR_MOD_NAME).*
