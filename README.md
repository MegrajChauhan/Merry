# MERRY

Merry is  a 64-bit Virtual Machine that emulates a non-existsent CPU. Merry has it's own custom ISA that is still small but is growing steadily. Merry aspires to be a very easy-to-use and simple tool for problem solving. It has 140 different instructions that can be employed to solve all sorts of problems and that is still only a fraction of what it can do in the future.

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
8. Assembler(In development).
9. Programming Language(In development).  
10. Dynamic Library Loading(For linux only).
11. File IO(Not fully tested)
and comming soon....

# Some Important Notes:
Merry is not fully tested right now and so it may not work as expected. Hence any help in finding and solving the issues is appreciated. For starters, you may run the **FirstProgram.c** program which should work. Compile it and build the VM first and then run it accordingly. The building process will be explained soon. The VM hasn't been tested in Windows, Mac or any other operating systems. Also the use of **gcc** as the compiler would be recommended.

# Building the VM:
Due to my lack of knowledge in build systems, I have had to use a script written in Python for building. For building the project, here is the command to run:
```
python build.py <Destination Folder> <Final Name of the executable; For now it can be anything>
```
Note that the above command must be ran in the root directory of the project. If the directory doesn't exist already, create one first. The name of the executable can be anything for now but using **merry** is recommended.

# Running Programs:
In order to make merry run any program, first go into the directory where the compiled executable resides and type the following command:
```
./<exe name> -f <input file name>
```

This will make merry read the input file and start executing it. 

# Things to know:
Merry is still in development and hence it is appreciated for feedback on test failures. Many features are yet to be implemented. 

# Helper:
The person who has helped in the project:
- @luccie-cmd