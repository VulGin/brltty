# Dependencies for braille.o:
braille.o: $(SRC_DIR)/braille.c
braille.o: $(BLD_TOP)config.h
braille.o: $(SRC_TOP)Programs/brl.h
braille.o: $(SRC_TOP)Programs/misc.h
braille.o: $(SRC_DIR)/brlconf.h
braille.o: $(SRC_TOP)Programs/brl_driver.h
braille.o: $(SRC_DIR)/input.h
braille.o: $(SRC_DIR)/output.h

