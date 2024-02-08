#
# Our build system: Sorry! I don't know Cmake or make or Ninja or anything like that
#

# This is my own version of a build system
# I do not know how to use Cmake or make or Ninja or anything like that and hence i had to use this
# If you know how to write a build file in anything, you can do that. The order in which the source files should be compiled is in "compile.txt"
import os
import sys
import glob

def print_usage():
    print("python build.py <Destination Directory> <Output Name> <Extra flags>")
    print("The path to the files to be compiled should be in compile.txt in the order in which it needs to be compiled")

def main():
    if len(sys.argv) < 3:
        print_usage()
        sys.exit(1)
    avai = False
    if len(sys.argv) == 4:
        avai = True
    destination_directory = sys.argv[1].rstrip("/")
    output_file_name = sys.argv[2]
    if not os.path.exists(destination_directory):
        os.makedirs(destination_directory)

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
    compile_command = f"gcc -O3 -D_MERRY_LOGGER_ENABLED_ -ggdb -Wall -Wextra {sys.argv[3] if avai == True else " "} {final_file} -o {destination}"
    print("Compiling the source files...")
    print(f"Command run: '{compile_command}'")
    print("\nCOMPILER MESSAGES IF ANY:")
    result = os.system(compile_command)
    if result == 0:
        print(f"Compilation successful with return value of {result}")
    else:
        print(f"Compilation failed with a return value of {result}")
        quit(result)
    
    print("Compiling assembler (not finished!)")
    os.system("mkdir -p ./build")
    os.system("mkdir -p ./build/asmOutFiles/")
    os.chdir("./MerryAssembler/src/")
    for file in glob.glob("*.cc"):
        print(f"Compiling CXX object: {file}")
        result = os.system(f"c++ -c -ggdb -O3 -std=c++23 -o ../.././build/asmOutFiles/{file}.o {file} -Wall -Wextra -Wswitch-enum")
    os.chdir("../../")
    result = os.system("c++ -ggdb -O3 -std=c++23 ./build/asmOutFiles/*.o -o ./build/asm -Wall -Wextra -Wswitch-enum")
    if result == 0:
        print(f"Compilation successful with return value of {result}")
    else:
        print(f"Compilation failed with a return value of {result}")
        quit(result)
    print("Done building all!")

if __name__ == "__main__":
    main()
            
