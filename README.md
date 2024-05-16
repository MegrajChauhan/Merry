# MERRY

Merry is  a 64-bit Virtual Machine that emulates a non-existsent CPU. Merry has it's own custom ISA that is still small but is growing steadily. Merry aspires to be a very easy-to-use and simple tool for problem solving. It has 180 different instructions that can be employed to solve all sorts of problems and that is still only a fraction of what it can do in the future.

> **Note:**
>_If you wish to learn about the instructions that Merry supports or how they are encoded, please look in_ <span style="background-color: #000000; color: white; padding: 0.2em;">docs/opcodes.txt</span> _where I have tried my best to explain it._

# Features:
1. Custom ISA.
2. Very fast execution.
3. Custom input format.
4. IO capabilities.
5. Multi-threading(Untested)
6. Arithmetic instructions(signed and unsigned).
7. Floating point instructions(Untested).
8. Assembler(Basic one done).
9. Programming Language(In development).  
10. Dynamic Library Loading(tested for linux only).
11. File IO(Not fully tested)
and comming soon....

# Building the VM:
Due to my lack of knowledge in build systems, I have had to use a script written in Python for building. For building the project, here is the command to run:
```bash
python build.py <Destination Folder>
```
Note that the above command must be ran in the same directory as the script. If the destination directory doesn't exist already, create one first.
"gcc" is the compiler needed for the compilation of the VM while "g++" is needed for the assembler.

# Running Programs:
In order to make merry run any program, first go into the directory where the compiled executable resides and type the following command:
```bash
./merry -f <input file path>
```
This will make merry read the input file and start executing it. 
But before running a program, you would need one and for that you can use the assembler to write the program and then assemble it. The old assembler, Masm, has been replaced with Zeta.
Zeta is a much better, more efficient and faster assembler. Perform the following command to get Zeta:
```bash
git clone --depth=1 https://www.github.com/MegrajChauhan/Zeta # remove --depth=1 to clone everything
cd Zeta
g++ -std=c++20 \[More options here\] src/* main.cpp -o zeta # add options there such as -O3 for even faster assembling or enable debugging symbols, your choice
./zeta input_file_path_ending_in_.zasm -o output_file_name_.mbin_is temporary_to_add # no additional options are supported yet
```


# Things to know:
Merry is still in development and hence it is appreciated for feedback on test failures. Many features are yet to be implemented. 

# Run Tests:
Goto **_example/run_** to look at example programs involving the assembly language. 
The syntax is inspired by x86_64 Nasm assembly which should be well knowm.
