# Merry Virtual Machine

## Terms

- **Virtual Core (vcore)**: A virtual core refers to the implementation of a computation model within the virtual machine.  
- **GPC**: General Purpose Computation.  
- **Virtual Thread (vthread)**: An OS-level thread abstracted within the virtual layer.

## Overview

**Merry Virtual Machine (MVM)** is a **broadly hybrid** virtual machine that integrates multiple computation models into a unified system.

The term **hybrid** in MVM does *not* refer to a combination of register- and stack-based computing alone. Instead, it refers to MVM’s ability to operate under *any* computation model, as long as the required virtual core (vcore) is implemented. For example, MVM can behave like a stack-based VM, a register-based VM, or a combination of both. To clarify this unique flexibility, we use the term **broadly hybrid**.

MVM is designed with extensibility in mind: any computation model can be supported through a corresponding vcore. Currently, MVM supports the 64-bit GPC model—commonly known as the Hybrid Model—which utilizes both a stack and registers. However, the architecture allows for potentially hundreds of other vcore implementations.

To illustrate MVM’s flexibility, consider these examples:

- The GPC model is Turing complete and supports standard CPU-like instructions (arithmetic, logic, branching, etc.).
- A vcore could be designed for high-performance vector computations using GPU acceleration—ideal for tasks like rendering.
- Another vcore could specialize in parsing and processing common file formats.

The only limits are your **hardware**, **operating system capabilities**, and **imagination**. You could even write a vcore that simply draws a flower on the screen—and that’s valid. If you can dream it and your OS can support it, MVM can run it. This is the essence of being **broadly hybrid**.

### Extensibility for All

Will I personally write a new vcore for every idea I have? What about your ideas? How will I know about them?  
(**P.S. You can always share them with me on [Patreon]!**)

That’s where MVM’s **Subsystem Architecture** comes in.

This architecture allows *anyone* to create and plug in their own computation models (subsystems), provided they follow a few simple conventions. While this subsystem system isn't implemented yet, the design is complete. Once available, documentation and examples will guide users through creating their own subsystems in *any* programming language they prefer.

I will focus on developing powerful, foundational vcores built into the VM. You—yes, you—can use them or build your own subsystems to extend the system even further.

---

**Side Note**: The name *Merry* comes from the *Going Merry* in *One Piece*. I chose it because I happened to be watching Luffy give her a farewell on YouTube Shorts at the time. 

## The Architecture

MVM’s architecture is elegantly simple and consists of the following components:

- **Graves**: The managing director of the entire VM.
- **VCores**: The workers that execute code based on specific computation models.
- **Memory**: No explanation needed.
- **Request Queue**: How vcores communicate with Graves.
- **Reader**: Handles reading and parsing the input file.

The input file format—**BEB (Broadly Emitted Binary)**—will be detailed in a separate document. For now, it's more important to understand that the **Reader** serves a deeper role than just reading the file.

The architecture is very simple and the way each component works and communicates is also extremely simple. Let's go through each of the components and also see how they work together.

**Graves** is the manager for the entire virtual machine. It is responsible for:
- managing resources
- providing OS interface for the programs
- and providing various features and services to the programs

Graves also manages its own request queue using which the vcores communicate with it. VCores post their requests onto the queue and Graves fulfills them one by one. In a sense, Graves is an event-driven entity.
To simplify, except for executing instructions, reading input file, storing/retrieving data/instructions, **everything else is the work of Graves**.

VCores do not require any explanation, I believe. 

The memory is interesting though. Each vcore has its own dedicated instruction memory but one data memory is shared among all. Yes! Instructions and data are separated
to increase security. VCores never write to the instruction memory and one cannot access the instructions of another. 

It is important to note that some tradeoffs have been made here and there to either be more flexible or get more speed. I will get into those later on. 

**Reader** won't get a separate section but the file format will so let's discuss the reader right here. 

Once the command line options are parsed and the input file is identified, the reader not only reads the input file but prepares the instruction/data memory for Graves which is then provided to the corresponding vcore.
As reader reads the input, the respective instruction memory or data memory is populated on a page basis(I will get to pages a bit later). 
The reader
also provides the Read On Demand feature. It is better demonstrated with an example. Imagine you wish to play RDR2. It is a very heavy game! The rate of reading the source binary into the RAM is limited and so is the RAM!
The RAM has not enough space so even if RDR2 took 100% of the available space for itself, it still wouldn't fit so then how? The solution to this problem lies in paging and Read On Demand features provided by the host
Operating system in conjunction with the hardware. How this works is a bit complicated but let me summarize. So the OS stores the say 90GB game in chunks called Pages. One page is 4KB or more in size depending on the OS.
That makes for a lot of pages. Now say the details required to render Arthur is in page 100 but the details required to render his gun is in page 1000. What will the OS do? The OS will read just those two pages into the
RAM and the rest are left on the hard drive waiting to be retrived on need, hence, Read On Demand. 

The reader does something similar with some mentionable differences. For eg: The OS will swap pages to accomodate other programs but the reader will just read on demand and there is no swapping and such involved.
The memory of MVM is also paged. One page is 1MB in size. Say there is a 20MB program. Reading time increases with larger file size so to solve the problem, the reader reads just the first 1MB and if other parts are needed
then they are read as requested on demand.

---

## Trade-offs

As MVM was written, some choices were made. Each choice either added to what Merry could do or it made Merry faster. The choices were entirely based on what would I want as a user. What situation brought what changes shall
be discussed as the corresponding topic is discussed. A few choices that were made are:

- Allowing unalligned memory accesses.
- Separating data and instructions.
- Separating instructions for every vcore type.

## How Graves Handles multiple vcore types?

In all honesty, Graves has no knowledge of the vcores. VCores follow some conventions that Graves does understand. Hence, anything that follows those conventions is a vcore to Graves. Upon parsing the input file, Graves 
runs the VM and the first thing it does is spawn a core of the type that was first on the ITIT(Instruction Type Identification Table) as an entry. From the creation of a vcore to its termination, Graves uses the same
interface for communicating with the vcore.
