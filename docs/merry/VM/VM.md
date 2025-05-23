# Merry Virtual Machine

## Terms
**__Virtual Core__**: A virtual core(short for vcore) refers to the implementation of a computation model within the virtual machine 
**__GPC__**:  General Purpose Computation
**__virtual thread__**: OS-level threads at the virtual level. Shortened to vthread.

## Overview

Merry Virtual Machine, or MVM for short, is a hybrid virtual machine that makes use of various different computation models packed into one big system.
The term **hybrid** here doesn't imply that MVM uses both, registers and stack, for its computation. In the case of MVM, the term **hybrid** refers to the capability of MVM to 
make use of multiple different computation models to solve problems. What this implies is that MVM can work like a stack-based VM, or MVM can also work as a registers based VM, or
MVM can also work as a hybrid VM where **hybrid** refers to the use of both stack and registers. This might get a little confusing so the term **broadly hybrid** will be used for MVM.

MVM is designed such that one can make use of any computation model as long as it has a built-in vcore to support that model. As of writing of this document, MVM only supports
the 64-bit GPC model commonly referred to as the Hybrid model which makes use of both a stack and registers. In truth, MVM has the capacity to implement hundreds of different types of vcore.
Just to give you an idea on the flexibiltiy and versatility of MVM's architecture, let's explore some ideas.

The GPC model already provides the ability to perform normal computation that a real CPU would perform such as arithmetic instructions, logical instructions, conditional instructions, data movement instructions,
branch instructions and so on i.e GPC model is turing complete. Let's imagine that a task is being done that requires a lot of data to be processed such as rendering. A vectored computation model could be implemented
as a vcore that is optimized for vectored operations and makes use of the GPU. The program only needs to start a new vthread that runs the vectored model. Another vcore that is optimized and specialized for processing 
and parsing standard file formats could be implemented. 

The only limits here are the hardware capabilities, Operating system features and your imagination. I could simply write a vcore that will just draw a flower on a screen somewhere and be done with it. As long as you
can imagine and the operating system allows for those imaginations to become reality, MVM can encompass it all, hence, **Broadly hybrid**.

So will I keep implementing new vcore for whatever comes to my mind? What will others do? What about the ideas that comes to their mind? How will I know about them? (PS: __You can tell them to me on my patreon ;\)__ )
The simple answer lies in the architecture of Merry. MVM uses an architecture that allows anyone to write their imagination. The only requirement? Follow some extremely simple conventions. As of the moment of writing this
document, the idea is not yet implemented but it is already designed. This new idea is called **Subsystem**.

With this architecture, I can focus on providing extremely useful vcore that are baked into the VM while you, the users, may use what is already available of write subsystems to cover what you are missing. This 
architecture only cares that you follow the conventions and nothing else, thus, you may use any programming language you desire to write these subsystems as long as they follow the conventions. Dedicated documentations
will be provided once the idea is implemented with examples.

**Side Note**: __The name Merry comes from Going Merry in One piece. The name was decided because I was watching Luffy put her to rest on youtube shorts at the time.__

## The Architecture

MVM has a very simple architecture comprised of: 
- **Graves**: The managing director of the entire VM.
- **VCores**: The workers.
- **Memory**: No need to explain.
- **Request Queue**: How else do you think vcores talk to graves?
- **Reader**: Reads the input file.

The input file format **BEB** or **Broadly Emitted Binary** will be discussed separately on a different document, thus, understanding the role of the reader besides reading the input file is more important.

