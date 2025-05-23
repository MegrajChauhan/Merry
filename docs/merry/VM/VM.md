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

---
