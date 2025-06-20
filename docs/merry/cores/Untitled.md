# GPC

**GPC**, or **General Purpose Core**, internally referred to as `core64` inside the Merry VM, is a 64-bit general-purpose virtual core designed for standard computation. Within virtual machine terminology, it can be considered a **hybrid execution model** that uses both **registers** and a **stack**.

The **Instruction Set Architecture (ISA)** of GPC is intentionally simple. This simplicity is a legacy design choice, inherited and slightly adapted from earlier iterations of the Merry VM. While this comes at the cost of flexibility, it enables **faster execution** due to reduced decoding complexity — a deliberate trade-off compared to more complex but slower ISAs.

Another vcore type (to be introduced) will support general computation with a **more sophisticated ISA**, offering greater flexibility for advanced workloads.

## Key Components of GPC

| Component            | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| Registers            | 16 general-purpose 64-bit registers                                          |
| Stack                | 2 MB static stack, byte-addressable                                          |
| Flags Register       | Holds status flags for integer operations                                    |
| Float Flags Register | Holds flags related to floating-point operations                             |
| Wild Handler Address | (To be documented later)                                                     |
| Call Stack           | (To be documented later)                                                     |
| Traceback Stack      | (To be documented later)                                                     |

## Register Layout

| Register | Address | Purpose               |
|----------|---------|------------------------|
| `Acc`    | `0x00`  | Accumulator            |
| `SP`     | `0x01`  | Stack Pointer          |
| `BP`     | `0x02`  | Base Pointer           |
| `R0`     | `0x03`  | General-purpose        |
| `R1`     | `0x04`  | General-purpose        |
| `R2`     | `0x05`  | General-purpose        |
| `R3`     | `0x06`  | General-purpose        |
| `R4`     | `0x07`  | General-purpose        |
| `R5`     | `0x08`  | General-purpose        |
| `R6`     | `0x09`  | General-purpose        |
| `R7`     | `0x0A`  | General-purpose        |
| `R8`     | `0x0B`  | General-purpose        |
| `R9`     | `0x0C`  | General-purpose        |
| `R10`    | `0x0D`  | General-purpose        |
| `R11`    | `0x0E`  | General-purpose        |
| `R12`    | `0x0F`  | General-purpose        |
| `PC`     | (N/A)   | Program Counter *(inaccessible)* |
| `IR`     | (N/A)   | Instruction Register *(inaccessible)* |

## Memory Access & Layout

- **Instruction Memory** is private to each vcore type.
- **Data Memory** is **shared** among all vcores. This shared memory is initialized by the reader after parsing the input file.
- **Stack Memory** is **fully byte-addressable**.
- **Instruction Memory** for GPC is **quadword-accessible only** — each instruction is 8 bytes (64 bits), read in a single fetch.

Although every byte in instruction memory is addressable, the execution unit fetches only full **quadwords** (8 bytes) at a time. While user programs cannot directly access instruction memory, toolchain or backend developers must understand this model to build correct tooling.

## Example: Instruction Memory View

When instruction memory is populated, it might appear as:

| Address | Content (64-bit)         |
|---------|---------------------------|
| `0x0000` | `0x0100000000000000`     |
| `0x0008` | `0x0000123400569971`     |

Here:
- The first instruction is located from `0x0000` to `0x0007`.
- The second instruction starts at `0x0008`.

This structure and memory access behavior is **specific to GPC** and may not apply to other vcore types.

## Flags and Float Flags

The **flags register** is affected by integer operations, while the **float flags register** is affected by floating-point operations. The flags register is directly mapped to the host system, whereas the float flags register is defined by Merry.

Neither of these registers is accessible to user programs directly.

Since the flags register depends on the host system, we focus only on the most relevant flags:

### 1) Flags Register

| Flag           | Description                                                              |
|----------------|---------------------------------------------------------------------------|
| Sign Flag      | Set if the result of an operation is negative.                            |
| Zero Flag      | Set if the result of an operation is zero.                                |
| Overflow Flag  | Set if a signed operation results in an integer overflow.                 |
| Carry Flag     | Set if an unsigned operation results in an integer overflow (carry-out).  |

### 2) Float Flags

The float flags register is defined by the VM and is affected by floating-point operations.

| Flag             | Description                                                                 |
|------------------|-----------------------------------------------------------------------------|
| Zero Flag        | Set if the result of an operation is zero.                                  |
| Sign Flag        | Set if the result of an operation is negative.                              |
| Invalid Flag     | Set if an operation produces an invalid floating-point result (e.g., NaN).  |
| Overflow Flag    | Set if the result exceeds the maximum representable floating-point value.   |
| Underflow Flag   | Set if the result is too small to be represented normally.                  |
| Unordered Flag   | Set if the operands are unordered (typically due to NaN involvement).       |

Although user programs cannot directly access these flag registers, they can observe their effects through certain instructions. These include **comparison** and **conditional** instructions, which internally rely on the values of the flags.

## Call Stack

During function calls, a vcore must remember where it last was. Thus, to facilitate function calls, the vcore utilities the call stack. In practice, the program stack is used to store the return addresses. This is very risky as the entire stack is accessible to user programs which could lead to intentional and unintentional changes to that return address. To remove the entire security risk, the call stack is used. This stack is not accessible to user programs, thus, one cannot modify these addresses. It could be useful to change the return addresses in a controlled way in some cases but it is unsafe nonetheless.

## Trace Back Stack

This is a utility component for debugging. Each time a function is called, the starting address of the function is stored on the Trace Back Stack. This all allows the any debugger to know what the state of the program is at any point during execution.

# The ISA

The ISA of GPC is extremely simple. Here are all of the instructions with their opcodes and examples along with a description:

Note: op = Opcode, dc= don't care, reg = register and everything is in hex, nxdc = 'n' don't care bytes, nximm = n-byte immediate, x:y = upper 4-bits for x and lower 4-bits for y, a< = in x:y, a is referring to x, a> = in x:y, a is referring to y, [n] = n-byte operation only, mem = 8-byte memory address, *mem = dereferencing the address.
