use std::fs::File; // for file IO
use std::collections::String; // strings
use std::collections::Vector; // vectors for all arguments

use merrylang::utils::configs;

// we expect the ending extension for the input file to be ".me"

enum Args_t
{
  _FILE_NAME, // the file provided[must follow ./merrylang <path to file> format] 
  _HELP, // the help option[-h, --h, -help or --help]
 _VERSION, // the version  option[-v, -version]
 // the use of HELP and VERSION shadows other options, making them useless.
 _OUTPUT_NAME, // specify the name of the output file[-o]
}

// represents the command line options
struct CommandLine
{
  _args_: Vec<String>,
  // now for each option we expect
  _file_pos: usize , // the position of the file name in the command must always be 1, can't be 0
  _help_opt_provided_: bool, // is the help option provided?
  _version_opt_provided_: bool, // is the version option provided?
  _output_filename_: usize, // 0 indicates that there was none provided
}
