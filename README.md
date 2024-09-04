# MERRY

Merry is a 64-bit hybrid Virtual Machine. Merry utilizes stack and registers to aid in program execution. Merry provides low level control to its internals as in real hardware. The machine can be used to learn low level programming though it is not yet suggested due to the unfinished and untested product but any feedback through testings are highly welcomed.
Merry implements a very simple ISA comprising of 180 different instructions which includes variants of the same instruction as well. The simple ISA is a trade-off for quick execution as complex encodings in a VM slows execution.
There are many such trade-offs made throughout the VM.   

> **Note:**
>_If you wish to learn about the instructions that Merry supports or how they are encoded, please look in_ <span style="background-color: #000000; color: white; padding: 0.2em;">docs/opcodes.txt</span> _where I have tried my best to explain it._

# Building the VM:
In order to build the VM, you need MAKE.
```bash
git clone https://www.github.com/MegrajChauhan/Merry
cd Merry
mkdir build
make all dirs=build/ # use this instead of the build.sh
# This doesn't install the binaries into the system right now
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

# Refer to:
For any queries, please refer to **info/**. Though there might not be much information, the one available should be enough for now.
I cannot write comprehensive documents as I am not very good at explaining things to others but I will try.

# What Merry provides:
Merry previously had a completely separate module that abstracted the underlying hardware and OS for the VM and the running programs but that has been removed entirely and replaced by the "syscall" instruction which makes low-level system calls to the Operating System.
Merry also provides multi-processing and multi-threading capabilities along with 
debugging options. Educational Operating Systems could be written if Merry had support for virtual memory and CPU-level protection. Interrupts would be child's play to implement.
Merry also provides atomic instructions for synchronization constructs, a versatile input file format for debuggers etc.

# What is Lacking:
Merry lacks in almost everything- no programs, tests and new features. The reason being a sole developer working on it but soon enough it will be usable.
The assembler is pretty useless as far as I can tell. I can't even tell if it will work unless I perform serious tests first.

# Future Goal
The current goal is routine bug fixes while focusing mostly on the stdlib of the assembler.
