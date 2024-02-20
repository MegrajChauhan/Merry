#
# Our build system: Sorry! I don't know Cmake or make or Ninja or anything like that
#

# This is my own version of a build system
# I do not know how to use Cmake or make or Ninja or anything like that and hence i had to use this
# If you know how to write a build file in anything, you can do that. The order in which the source files should be compiled is in "compile.txt"
import os
import sys
import platform
# from cpuinfo import cpuinfo

# def has_fpu():
#     """
#     Check if the CPU has a Floating Point Unit (FPU).
#     Should work according to the internet
#     """
#     cpu_info = cpuinfo.get_cpu_info()
#     features = cpu_info.get('flags', [])
#     ## since different platforms could have different names or FPUs, we need to add checks for them here as well 
#     return 'fpu' in features or 'vfpu' in features

# def uses_ieee754():
#     ## should tell if the host uses IEEE754 or not according to the internet 
#     if sys.float_info.__getattribute__('max') == float('inf'):
#         return True
#     else:
#         return False

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

    if platform.system() == 'Windows':
      with open(".\compilewindows.txt", "r") as read:
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
    compile_command = f"gcc -O3  {sys.argv[3] if avai == True else ' '} {final_file} -o {destination}"
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
    
    # print("Compiling assembler (not finished!)")
    # result = os.system("c++ -ggdb -fsanitize=address -O2 -std=c++23 -o ./build/asm ./MerryAssembler/src/*.cc -Wall -Wextra -Wswitch-enum")
    # if result == 0:
    #     print(f"Compilation successful with return value of {result}")
    # else:
    #     print(f"Compilation failed with a return value of {result}")
    #     quit(result)
    # print("Done building all!")

if __name__ == "__main__":
    main()
            
