# Merry

**Merry** is a 64-bit virtual machine written in C, designed as educational material for new programmers to dive deep into low-level concepts while building a strong understanding of high-level systems from scratch. Merry features a custom and simple ISA, which is highly expandable and currently supports over 200 instructions, including types with their variants. Programmers may learn how their programs are executed after being compiled or how the Python interpreter is working behind the scenes.

## Getting Started

To begin, clone the repository:

```bash
git clone https://www.github.com/MegrajChauhan/Merry

```
Then, you need to make sure that you have **g++, gcc, make** installed on your systems.

#### Linux
```bash
cd Merry
make all dirs=A_BUILD_DIRECTORY_WITH_THIS_SEPARATOR/ args=ANY_EXTRA_COMPILE_ARGUMENTS
```
Currently, the binaries cannot be installed into the system as the standard library is under development. However, you can run programs using the following commands:
```bash
./BUILD_DIR/masm INPUT_FILE.asm
./BUILD_DIR/mvm -f INPUT_FILE.mbin
```

**NOTE:** __You may look at the available commands using the help command__
```bash
./BUILD_DIR/masm -h
./BUILD_DIR/mvm -h
```

# What Merry boasts?
- **Expandable ISA:** Merry offers a simple and expandable Instruction Set Architecture (ISA).
- **Flexible Input File Format:** Merry uses a robust input file format, allowing for ease of programming, even at the binary level. This structure also supports debuggers.
- **Multi-threading and Multi-processing Support:** Merry provides built-in support for both multi-threading and multi-processing, mapped directly to the host operating system's concepts (no Global Interpreter Lock). Multi-processing operates similarly to how it functions on Windows.
- **Dynamic Library Loading:** Merry allows programs to dynamically load compiled libraries, with the ability to call functions from those libraries freely.
- **Interrupt-driven Services:** Merry employs the concept of interrupts, coupled with a Manager thread, to provide essential services without race conditions.
- **Efficient Memory Management:** Merry uses a paging system for memory management, making it simple for programs to request more memory directly from the host operating system.
- **Separate Memory Regions for Code and Data:** This architecture ensures that programs cannot modify their own instructions.
- **System Calls:** Programs can directly make system calls to the host operating system, managed by the Manager thread to ensure execution safety.
- **Basic Debugger Support:** Merry currently provides simple and untested debugger support, which is under development.
- **Evolving Standard Library:** Merry features a simple standard library that is continually evolving.
- **Custom Assembly Language:** Merry includes a custom assembly language, which is also in active development.

# Problems:
- **No 32-bit Support:** Merry does not support 32-bit systems and has only been tested on Linux (Fedora 40).
- **Limited Platform Support:** Full support for other platforms is not yet available.
- **Standard Library:** Merry lacks a fully functional standard library, a robust linkable format, and a versatile assembler, though these features are actively being worked on. 
- **Limited Complex Instructions:** Due to the simplicity of the ISA, the VM requires multiple instructions to perform more complex operations. 
- **Host OS Features:** Merry currently underutilizes features provided by the host operating system and the C standard library.
- **Difficult Integration:** Merry cannot easily serve as a backend for other projects, such as a script executor in text editors.
- **Complex Debugger Setup:** The file format's flexibility allows individuals to define unique sections, complicating the process of writing debuggers.
- **Error Handling:** Both the VM's internal error handling and the standard library's error handling need significant improvements.

# Some examples

1. **A hello world program**
   ```asm
   proc main ;; declare a procedure
   
   entry main    ;; Tell the assembler that this procedure is the entry
   ;; entry main ;; Doing this line multiple times means "Create a new thread that starts from the same address."
   
   main                        ;; we don't have ':' here and for a good reason
       mov Ma, [PTR msg_len]   ;; Expressions are written within '[]' and PTR is an operator which gives you the pointer to some variable
       mov Mc, msg_len
       sout Ma                 ;; 'sout' takes two arguments- one address to the string and one length of the string in 'Mc'
       halt                    ;; necessary
   
   ds msg "Hello World!".10.0 ;; The '.' is an operator that concatanates a number to a string
                              ;; 'ds' means 'define string'
   db msg_len 13      ;; Message length excluding the terminating byte
                      ;; 'db' means 'define byte'
   ;; Note that ';;' is a valid comment while ';' isn't
   ;; Any ',' are ignored so do whatever with commas
   ```
   Save it and assemble. Run to see 'Hello World!' being printed.

2. **Printing the number of CMD options**
   This will require the use of the standard library
   ```asm
   depends stdinit.asm   ;; we need this for this
   proc main  ;; the name doesn't have to be 'main'
   entry main

   main
        call std::init  ;; This is why ':' is reserved
        
        ;; prepare the arguments
        mov Ma, [PTR arg_count]
        mov Mb, [PTR arg_val]
        call std::cmd_options
        loadq Ma, Ma
        uoutq Ma   ;; print as unsigned qword
        halt

   rq arg_count 1 ;; 'rq' means 'reserve qword or 8 bytes' and since '1' was given, it will reserve exactly 1 qword
   rq arg_val 1
   ```
   If you assemble this and run the program as instructed by the help manual(try using the -h to get help), you will see the number of arguments you passed
   being printed.

3. **Opening a file and writing to it**
    See _stdtest/filetest.asm_ for the actual code.
    ```asm
        depends stdfio.asm

    entry main
    proc main

    main
        mov Ma, [PTR FILENAME]
        movl Mb, _M_CREATE_
        or Mb, _M_READ_WRITE_
        movl Mc, _M_CREATE_RWX
        call std::fio::fopen
        cmp Ma, ERR
        je _exit
        ;; Ma already has the fd
        ;; but store it
        storeq Ma, fd
        mov Mb, [PTR _output_text_]
        mov Mc, _output_text_len_
        call std::fio::fwrite

        cmp Ma, ERR
        je _exit

        loadq Ma, fd
        call std::fio::fclose
        movl Ma, 0

    _exit
        halt

    ds FILENAME "Test.txt".0
    dc ERR 0xFFFFFFFFFFFFFFFF

    ds _output_text_ "Hello World!".10.0
    dc _output_text_len_ 12

    dq fd 1
    ```
    Assembling and running the above code will create a file named 'Test.txt' and write 'Hello World!' to it.

# LICENSE
This project is released under the [MIT License](https://www.github.com/MegrajChauhan/Merry/master/LICENSE).

# Contributing
Currently there are no guidelines however, you may:
1. Fork the repo and clone it.
2. Make a new branch with the name of the issue or problem being solved.
3. Make the changes.
4. Create a pull request with a detailed explanation of what you did.

You may also contribute via [Patreon](https://www.patreon.com/AryanChauhan)

# Contacting
You may contact me via the links provided on the [profile](https://github.com/MegrajChauhan).