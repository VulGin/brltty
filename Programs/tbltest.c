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
#include <wchar.h>
#include <ctype.h>

#include "program.h"
#include "options.h"
#include "misc.h"
#include "brl.h"
#include "charset.h"
#include "tbl.h"
#include "tbl_internal.h"

static char *opt_characterSet;
static char *opt_inputFormat;
static char *opt_outputFormat;
static int opt_translate;
static char *opt_dataDirectory;

BEGIN_OPTION_TABLE(programOptions)
  { .letter = 'c',
    .word = "character-set",
    .argument = "charset",
    .setting.string = &opt_characterSet,
    .description = "8-bit character set to use."
  },

  { .letter = 'i',
    .word = "input-format",
    .argument = "format",
    .setting.string = &opt_inputFormat,
    .description = "Format of input file."
  },

  { .letter = 'o',
    .word = "output-format",
    .argument = "format",
    .setting.string = &opt_outputFormat,
    .description = "Format of output file."
  },

  { .letter = 't',
    .word = "translate",
    .setting.flag = &opt_translate,
    .description = "Translate."
  },

  { .letter = 'D',
    .word = "data-directory",
    .flags = OPT_Hidden,
    .argument = "file",
    .setting.string = &opt_dataDirectory,
    .defaultSetting = DATA_DIRECTORY,
    .description = "Path to directory for configuration files."
  },
END_OPTION_TABLE

static const DotsTable dotsInternal = {
  BRL_DOT1, BRL_DOT2, BRL_DOT3, BRL_DOT4,
  BRL_DOT5, BRL_DOT6, BRL_DOT7, BRL_DOT8
};

static const DotsTable dots12345678 = {
  0X01, 0X02, 0X04, 0X08, 0X10, 0X20, 0X40, 0X80
};

static const DotsTable dots14253678 = {
  0X01, 0X04, 0X10, 0X02, 0X08, 0X20, 0X40, 0X80
};

static unsigned char
mapDots (unsigned char input, const DotsTable from, const DotsTable to) {
  unsigned char output = 0;
  {
    int dot;
    for (dot=0; dot<DOTS_TABLE_SIZE; ++dot) {
      if (input & from[dot]) output |= to[dot];
    }
  }
  return output;
}

static int
readTable_Native (const char *path, FILE *file, TranslationTable table, void *data) {
  return tblLoad_Native(path, file, table,
                        TBL_UNDEFINED | TBL_DUPLICATE | TBL_UNUSED);
}

static int
writeTable_Native (const char *path, FILE *file, const TranslationTable table, void *data) {
  int index;

  if (fprintf(file, "# generated by %s\n", programName) == EOF) goto error;

  {
    const char *charset = getCharset();
    if (charset)
      if (fprintf(file, "# charset: %s\n", charset) == EOF)
        goto error;
  }

  for (index=0; index<TRANSLATION_TABLE_SIZE; ++index) {
    unsigned char cell = table[index];
    if (fprintf(file, "\\X%02X (", index) == EOF) goto error;

#define DOT(dot) if (fputs(((cell & BRL_DOT##dot)? #dot: " "), file) == EOF) goto error
    DOT(1);
    DOT(2);
    DOT(3);
    DOT(4);
    DOT(5);
    DOT(6);
    DOT(7);
    DOT(8);
#undef DOT

    if (fprintf(file, ")\n") == EOF) goto error;
  }
  return 1;

error:
  return 0;
}

static int
readTable_Binary (const char *path, FILE *file, TranslationTable table, void *data) {
  {
    int character;
    for (character=0; character<TRANSLATION_TABLE_SIZE; ++character) {
      int cell = fgetc(file);

      if (cell == EOF) {
        if (ferror(file)) {
          LogPrint(LOG_ERR, "input error: %s: %s", path, strerror(errno));
        } else {
          LogPrint(LOG_ERR, "table too short: %s", path);
        }
        return 0;
      }

      if (data) cell = mapDots(cell, data, dotsInternal);
      table[character] = cell;
    }
  }

  return 1;
}

static int
writeTable_Binary (const char *path, FILE *file, const TranslationTable table, void *data) {
  {
    int character;
    for (character=0; character<TRANSLATION_TABLE_SIZE; ++character) {
      unsigned char cell = table[character];
      if (data) cell = mapDots(cell, dotsInternal, data);
      if (fputc(cell, file) == EOF) {
        LogPrint(LOG_ERR, "output error: %s: %s", path, strerror(errno));
        return 0;
      }
    }
  }

  return 1;
}

#ifdef HAVE_ICONV_H
static int
readTable_Gnome (const char *path, FILE *file, TranslationTable table, void *data) {
  return tblLoad_Gnome(path, file, table,
                       TBL_UNDEFINED | TBL_DUPLICATE | TBL_UNUSED);
}

static int
writeTable_Gnome (const char *path, FILE *file, const TranslationTable table, void *data) {
  int i;

  /* TODO UNKNOWN-CHAR %wc all */
  if (fprintf(file, "ENCODING UTF-8\n") == EOF) goto error;
  if (fprintf(file, "# generated by %s\n", programName) == EOF) goto error;

  for (i=0; i<=0XFF; i++) {
    char c = i;
    wchar_t wc;
    wchar_t pattern = BRL_UC_ROW | table[i];

    if ((wc = convertCharToWchar(c)) == WEOF) continue;
    if (isprint(i) && !isspace(i)) {
      Utf8Buffer utf8C, utf8Pattern;
      if (!convertWcharToUtf8(wc, utf8C)) continue;
      if (!convertWcharToUtf8(pattern, utf8Pattern)) continue;
      if (fprintf(file, "UCS-CHAR %s %s\n", utf8C, utf8Pattern) == EOF) goto error;
    } else {
      uint32_t u32 = pattern;
      if (fprintf(file, "UNICODE-CHAR U+%04x U+%04"PRIx32"\n", i, u32) == EOF) goto error;
    }
  }
  return 1;

error:
  return 0;
}
#endif /* HAVE_ICONV_H */

typedef int TableReader (const char *path, FILE *file, TranslationTable table, void *data);
typedef int TableWriter (const char *path, FILE *file, const TranslationTable table, void *data);
typedef struct {
  const char *name;
  TableReader *read;
  TableWriter *write;
  void *data;
} FormatEntry;
static const FormatEntry formatEntries[] = {
  {"tbl", readTable_Native, writeTable_Native, NULL},
  {"a2b", readTable_Binary, writeTable_Binary, &dots12345678},
  {"sbl", readTable_Binary, writeTable_Binary, &dots14253678},

#ifdef HAVE_ICONV_H
  {"gnb", readTable_Gnome, writeTable_Gnome, NULL},
#endif /* HAVE_ICONV_H */
  {NULL}
};

static const FormatEntry *
findFormatEntry (const char *name) {
  const FormatEntry *format = formatEntries;
  while (format->name) {
    if (strcmp(name, format->name) == 0) return format;
    ++format;
  }
  return NULL;
}

static const char *
findFileExtension (const char *path) {
  const char *name = strrchr(path, '/');
  return strrchr((name? name: path), '.');
}

static const FormatEntry *
getFormatEntry (const char *name, const char *path, const char *description) {
  if (!(name && *name)) {
    name = findFileExtension(path);
    if (!(name && *++name)) {
      LogPrint(LOG_ERR, "unspecified %s format.", description);
      exit(2);
    }
  }

  {
    const FormatEntry *format = findFormatEntry(name);
    if (format) return format;
  }

  LogPrint(LOG_ERR, "unknown %s format: %s", description, name);
  exit(2);
}

static FILE *
openTable (const char **file, const char *mode, const char *directory, FILE *stdStream, const char *stdName) {
  if (stdStream) {
    if (strcmp(*file, "-") == 0) {
      *file = stdName;
      return stdStream;
    }
  }

  if (directory) {
    const char *path = makePath(directory, *file);
    if (!path) return NULL;
    *file = path;
  }

  {
    FILE *stream = fopen(*file, mode);
    if (!stream) LogPrint(LOG_ERR, "table open error: %s: %s", *file, strerror(errno));
    return stream;
  }
}

int
main (int argc, char *argv[]) {
  int status;
  const char *inputPath;
  const char *outputPath;
  const FormatEntry *inputFormat;
  const FormatEntry *outputFormat;

  {
    static const OptionsDescriptor descriptor = {
      OPTION_TABLE(programOptions),
      .applicationName = "tbltest",
      .argumentsSummary = "input-table [output-table]"
    };
    processOptions(&descriptor, &argc, &argv);
  }

  {
    char **const paths[] = {
      &opt_dataDirectory,
      NULL
    };
    fixInstallPaths(paths);
  }

  if (argc == 0) {
    LogPrint(LOG_ERR, "missing input table.");
    exit(2);
  }
  inputPath = *argv++, argc--;

  if (argc > 0) {
    outputPath = *argv++, argc--;
  } else if (opt_outputFormat && *opt_outputFormat) {
    const char *extension = findFileExtension(inputPath);
    int prefix = extension? (extension - inputPath): strlen(inputPath);
    char buffer[prefix + 1 + strlen(opt_outputFormat) + 1];
    snprintf(buffer, sizeof(buffer), "%.*s.%s", prefix, inputPath, opt_outputFormat);
    outputPath = strdupWrapper(buffer);
  } else {
    outputPath = NULL;
  }

  if (argc > 0) {
    LogPrint(LOG_ERR, "too many parameters.");
    exit(2);
  }

  inputFormat = getFormatEntry(opt_inputFormat, inputPath, "input");
  if (outputPath) {
    outputFormat = getFormatEntry(opt_outputFormat, outputPath, "output");
  } else {
    outputFormat = NULL;
  }

  if (opt_characterSet && !setCharset(opt_characterSet)) {
    LogPrint(LOG_ERR, "can't establish character set: %s", opt_characterSet);
    exit(9);
  }

  if (opt_translate) {
    FILE *inputFile = openTable(&inputPath, "r", opt_dataDirectory, NULL, NULL);

    if (inputFile) {
      TranslationTable inputTable;

      if (inputFormat->read(inputPath, inputFile, inputTable, inputFormat->data)) {
        if (outputPath) {
          FILE *outputFile = openTable(&outputPath, "r", opt_dataDirectory, NULL, NULL);

          if (outputFile) {
            TranslationTable outputTable;

            if (outputFormat->read(outputPath, outputFile, outputTable, outputFormat->data)) {
              TranslationTable table;

              {
                TranslationTable unoutputTable;
                int byte;
                reverseTranslationTable(outputTable, unoutputTable);
                memset(&table, 0, sizeof(table));
                for (byte=TRANSLATION_TABLE_SIZE-1; byte>=0; byte--)
                  table[byte] = unoutputTable[inputTable[byte]];
              }

              status = 0;
              while (1) {
                int character;

                if ((character = fgetc(stdin)) == EOF) {
                  if (ferror(stdin)) {
                    LogPrint(LOG_ERR, "input error: %s", strerror(errno));
                    status = 6;
                  }
                  break;
                }

                if (fputc(table[character], stdout) == EOF) {
                  LogPrint(LOG_ERR, "output error: %s", strerror(errno));
                  status = 7;
                  break;
                }
              }
            } else {
              status = 4;
            }

            if (fclose(outputFile) == EOF) {
              if (!status) {
                LogPrint(LOG_ERR, "output error: %s", strerror(errno));
                status = 7;
              }
            }
          } else {
            status = 3;
          }
        } else {
          LogPrint(LOG_ERR, "output table not specified.");
          status = 2;
        }
      } else {
        status = 4;
      }

      fclose(inputFile);
    } else {
      status = 3;
    }
  } else {
    if (outputFormat != inputFormat) {
      FILE *inputFile = openTable(&inputPath, "r", opt_dataDirectory, stdin, "<standard-input>");

      if (inputFile) {
        TranslationTable table;

        if (inputFormat->read(inputPath, inputFile, table, inputFormat->data)) {
          if (outputPath) {
            FILE *outputFile = openTable(&outputPath, "w", NULL, stdout, "<standard-output>");

            if (outputFile) {
              if (outputFormat->write(outputPath, outputFile, table, outputFormat->data)) {
                status = 0;
              } else {
                status = 6;
              }

              fclose(outputFile);
            } else {
              status = 5;
            }
          } else {
            status = 0;
          }
        } else {
          status = 4;
        }

        fclose(inputFile);
      } else {
        status = 3;
      }
    } else {
      LogPrint(LOG_ERR, "same input and output formats: %s", outputFormat->name);
      status = 2;
    }
  }

  return status;
}
