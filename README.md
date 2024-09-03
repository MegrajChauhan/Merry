# MERRY

Merry is  a 64-bit Virtual Machine that emulates a non-existsent CPU. Merry has it's own custom ISA that is small but versatile and prone to changes. Merry aspires to be a very easy-to-use and simple tool for problem solving. It has 180 different instructions that can be employed to solve all sorts of problems.

> **Note:**
>_If you wish to learn about the instructions that Merry supports or how they are encoded, please look in_ <span style="background-color: #000000; color: white; padding: 0.2em;">docs/opcodes.txt</span> _where I have tried my best to explain it._

# Building the VM:
In order to build the VM, you need MAKE.
```bash
git clone https://www.github.com/MegrajChauhan/Merry
cd Merry
mkdir build
make all dirs=build/ # use this instead of the build.sh
```
"gcc" is the compiler needed for the compilation of the VM while "g++" is needed for the assembler.

**NOTE:** Please do use the assembler and tell me how it works.

# Running Programs:
In order to make merry run any program, first go into the directory where the compiled executable resides and type the following command:
```bash
./mvm -f <input file path>
```
This will make merry read the input file and start executing it.
To run programs, you need to first write them. Take a look at **_asm/tests/masm/test.masm_** for more info. It doesn't cover everything but that much is enough to get started. Using the following command will assemble your programs.
```bash
./masm <inp_file_path> -o <output>
```

# Things to know:
Merry is still in development and hence it is appreciated for feedback on test failures. Many features are yet to be implemented. 

**_NOTE_**: _test.t and output.bin are one of the tests of the newer file format. You can compile the VM and then run output.bin to see what happens._\
**_NOTE_**: _The new format is still fully untested. Manual testing with handwritten binary programs will be bothersome but I have no choice and testing will take time._

# Refer to:
For any queries, please refer to **info/**. Though there might not be much information, the one available should be enough for now.
I cannot write comprehensive documents as I am not very good at explaining things to others but I will try.
