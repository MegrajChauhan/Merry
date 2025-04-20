#include "merry_consts.h"

enum __parser_state { HELP, VERSION, INP_FILE, OPTION };

_MERRY_LOCAL_ enum __parser_state state;

_MERRY_INTERNAL_ void __parse_option(char *opt);

mret_t merry_parse_arg(int argc, char **argv, MerryConsts *consts) {
  merry_check_ptr(argv);
  if (surelyF(argc < 2)) {
    merry_err("%s", _MERRY_HELP_MSG_);
    return RET_FAILURE;
  }

  int *args = (int *)malloc(sizeof(int) * argc);
  if (!args) {
    merry_err("FATAL ERROR: Parse argument failed[NO MEMORY]\n", NULL);
    return RET_FAILURE;
  }
  msize_t ind = 0;

  for (msize_t i = 1; i < (msize_t)argc; i++) {
    __parse_option(argv[i]);
    switch (state) {
    case HELP: {
      merry_talk(stdout, "Merry", _MERRY_HELP_MSG_, NULL);
      break;
    }
    case VERSION: {
      merry_talk(stdout, "Version", "%d.%d.%d\n", _MERRY_VERSION_MAJOR_,
                 _MERRY_VERSION_MINOR_, _MERRY_VERSION_CHANGE_);
      break;
    }
    case INP_FILE: {
      if (surelyF((i + 1) >= (msize_t)argv)) {
        merry_talk(stderr, "Error",
                   "Expected INPUT FILE PATH. Got EOF Instead.\n", NULL);
        free(args);
        return RET_FAILURE;
      }
      i++;
      consts->inp_file_index = i;
      break;
    }
    case OPTION: {
      args[ind] = i;
      ind++;
      break;
    }
    }
  }

  consts->program_args = args;
  return RET_SUCCESS;
}

_MERRY_INTERNAL_ void __parse_option(char *opt) {
  // Plain IF-ELSE Ladder
  if ((strcmp(opt, "-f") == 0) || (strcmp(opt, "--file") == 0))
    state = INP_FILE; // An Input File
  else if ((strcmp(opt, "-h") == 0) || (strcmp(opt, "--help") == 0))
    state = HELP;
  else if ((strcmp(opt, "-v") == 0) || (strcmp(opt, "--version") == 0))
    state = VERSION;
  else
    state = OPTION;
}
