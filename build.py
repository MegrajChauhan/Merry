#
# Our build system: Sorry! I don't know Cmake or make or Ninja or anything like that
#

# This is my own version of a build system
# I do not know how to use Cmake or make or Ninja or anything like that and hence i had to use this
# If you know how to write a build file in anything, you can do that. The order in which the source files should be compiled is in "compile.txt"
import os
import sys
import platform
from time import sleep

def print_usage():
    print("python build.py <Destination Directory>")
    print("The path to the files to be compiled should be in compile.txt in the order in which it needs to be compiled")

def compile_merry():
    destination_directory = sys.argv[1].rstrip("/")
    output_file_name = "merry"
    if not os.path.exists(destination_directory):
        os.makedirs(destination_directory)

    if platform.system() == 'Windows':
      with open(".\\compilewindows.txt", "r") as read:
         files_to_compile = read.readlines()
    else:
      with open("./compile.txt", "r") as read:
         files_to_compile = read.readlines()
    
    final_file = " "

    for file_path in files_to_compile:
        file_path = file_path.strip()  # Remove leading/trailing whitespaces and newlines
        if os.path.exists(file_path):
            print(f"Note: Adding file {file_path}...")
            final_file += file_path + " " ## add the path
        else:
            print(f"Warning: File not found - {file_path}")
    destination = os.path.join(destination_directory, output_file_name)
    compile_command = f"gcc -O3 {final_file} -o {destination}"
    print("Compiling the source files...")
    print(f"Command run: '{compile_command}'")
    print("\nCOMPILER MESSAGES IF ANY:")
    # -ggdb -Wall -Wextra -ggdb -Wall -Wextra
    result = os.system(compile_command)
    if result == 0:
        print(f"Compilation successful with return value of {result}")
    else:
        print(f"Compilation failed with a return value of {result}")
        quit(result)
    print("Finished compiling Merry")

# def compile_asm():
#     print("Compiling the assembler:")
#     print("Compiler messages if any: ")
#     result = os.system("g++ -O3 -std=c++23 -o build/masm Assembler/src/*.cpp Assembler/main.cpp")
#     if result == 0:
#         print(f"Compilation successful with return value of {result}")
#     else:
#         print(f"Compilation failed with a return value of {result}")
#         quit(result)
    

def main():
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)
    build_lim_avai = False
    if len(sys.argv) >= 3:
        build_lim_avai = True
    if build_lim_avai:
      if sys.argv[2] == "all":
          compile_merry()
          compile_asm()
      elif sys.argv[2] == 'vm':
          compile_merry()
 #     else:
  #        compile_asm()
    else:
        compile_merry()
   #     compile_asm()
    
if __name__ == "__main__":
    main()
            
