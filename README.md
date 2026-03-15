# citrus

A (infinite) register based VM.

## Build instruction

To build the project use the following command:

```bash
make
```

This should build the `citrus` binary in the `build` folder.

## Running a program

You can run the program as follows:

```bash
./build/citrus tests/print-char.csm
```

To enter debug mode run the following:

```bash
./build/citrus --debugger tests/print-char.csm
```

To check usage run the following:

```bash
./build/cirtus --help
```

Usage output:

```bash
Usage: citrus [OPTIONS] <filepath>

OPTIONS:
    --debugger  Run the vm in debugger mode
    --help      This screen

```

## Registers

The VM has infinitly many registers, but by default there are 16 registers.

These 2 register are special registers.

```
r0 <- Instruction Pointer
r1 <- Stack Pointer
```

Rest of the registers are general purpose register.

## Instructions

### No operation instruction

Arguments:
- No arguments

Ignored and moved to the next instruction.

Snippet:

```
INST_NOP
```

### Halt instruction

Arguments:
- No arguments

Stop the virtual machine from running.

Snippet:

```
INST_HLT
```

### Load constant instruction

Arguments:
- 1st argument: Register identifier
- 2nd argument: 64-bit value for the constant

Load constant value into a register

Snippet:

```
INST_LOAD_CONST r3 123
```

Here 123 is loaded into the r3 register

### Load instruction

Arguments:
- 1st argument: Destination register indentifier
- 2nd argument: Source register identifier
- 3rd argument: Size that is loaded

Load the content of a register to another register but only the least 
significant bytes of given byte size (<= 8 bytes).

Snippet:

```
INST_LOAD r3 r4 8
```

Load the content of r4 into r3 but take only the 1st 8 least significant byte.

### Load indirect instruction

Arguments:
- 1st argument: Destination register identifier
- 2nd argument: Source register index (where memory address exists);
- 3rd argument: Size that is loaded

Load the content of a register's memory content to another register but only
the least significant bytes.

Snippet:

```
INST_LOAD_INDIRECT r3 r4 8
```

Load the value pointed by register r4 to r3, load the first 8 bytes from the
least significant bytes.

### Push instruction

Arguments:
- 1st argument: Source register identifier

Push register value to the stack (8 bytes is pushed)

Snippet:

```
INST_PUSH r3
```

Push the content of r3 register into the stack.

### Pop instruction

Arguments;
- 1st argument: Destination register identifier

Pop the stack and put the value of the pop element into the register.

Snippet:

```
INST_POP r3
```

Pop the value of stack and put it in the r3 register.

### Store Indirect Instruction

Arguments:
- 1st argument: Destination register identifier (with memory address)
- 2nd argument: Source register identifier
- 3rd argument: Size that is loaded

Store the content of a register to a register's memory address only the 
least significant digits upto a given size.

Snippet:

```
INST_STORE_INDIRECT r3 r4 8
```

Store the content of r4 into the pointer value pointed by r3 and store the
8 least significant bytes.

### Jump Instruction

Arguments:
- 1st argument: Jump to the given offset or label

Jump to a given offset value (or label).

Snippet:

```
JMP 1
# OR
JMP start 
```

Jump to the 1st instruction in the list of instruction, or jump to the
instruction with the given label "start"

### Jump If False Instruction

Arguments:
- 1st argument: Source Register identifier
- 2nd argument: Offset or label

Jump to a given label or offset if the source register value is zero

Snippet:

```
INST_JMP_FALSE r3 start
```

Jump to start label if r3 is zero.

### Jump If True Instruction

Arguments:
- 1st argument: Source Register identifier
- 2nd argument: Offset or label

Jump to a given label or offset if the source register value is non zero

Snippet:

```
INST_JMP_TRUE r3 start
```

Jump to start label if r3 is non zero.

### Add instruction

Arguments:
- 1st argument: Destination Register identifier
- 2nd argument: Left Register identifier
- 3rd argument: Right Register identifier

Add content of 2 register and store it in a destination register

Snippet:

```
INST_ADD r4 r2 r3
```

Here r4 = r2 + r3

### Subtract instruction

Arguments:
- 1st argument: Destination Register identifier
- 2nd argument: Left Register identifier
- 3rd argument: Right Register identifier

subtract content of 2 register and store it in a destination register

Snippet:

```
INST_SUB r4 r2 r3
```

Here r4 = r2 - r3

### Multiply instruction

Arguments:
- 1st argument: Destination Register identifier
- 2nd argument: Left Register identifier
- 3rd argument: Right Register identifier

multiply content of 2 register and store it in a destination register

Snippet:

```
INST_MUL r4 r2 r3
```

Here r4 = r2 * r3

### Divide instruction

Arguments:
- 1st argument: Destination Register identifier
- 2nd argument: Left Register identifier
- 3rd argument: Right Register identifier

divide content of 2 register and store it in a destination register

Snippet:

```
INST_DIV r4 r2 r3
```

Here r4 = r2 / r3

### Modulus instruction

Arguments:
- 1st argument: Destination Register identifier
- 2nd argument: Left Register identifier
- 3rd argument: Right Register identifier

modulus content of 2 register and store it in a destination register

Snippet:

```
INST_MOD r4 r2 r3
```

Here r4 = r2 % r3

### Function Call instruction

Arguments:
- 1st argument: Jump to the given offset or label

Call a function

Snippet:

```
main:
    INST_CALL 0
```

### Function Return instruction

Arguments:
- No arguments

Return from the function

Snippet:

```
main:
    INST_RET
```


### Syscall instruction

By default, the following way is to make a syscall. Most of the syscall is based
on linux's syscall calling convention.

```
SYSCALL <SYSCALL_NUMBER>
```

Registers used for arguments and return value for the syscall

```
r3 <- Return value if any for the syscall
r4 <- arg0
r5 <- arg1
r6 <- arg2
r7 <- arg3
r8 <- arg4
r9 <- arg5
```

Most of the calling convention are based on this [table](https://www.chromium.org/chromium-os/developer-library/reference/linux-constants/syscalls/#x86_64-64-bit)

#### `read` syscall

Read the content of a file descriptor to a buffer

```
SYSCALL_NUMBER <- 0
r3 <- Return value (how much character read)
r4 <- unsigned int fd
r5 <- char *buf
r6 <- size_t count
```

[Example](./tests/test-syscall-read.csm)

#### `write` syscall

Write the content of a string to a file descriptor

```
SYSCALL_NUMNER <- 1
r3 <- Return value (how much character wrote)
r4 <- unsigned int fd
r5 <- const char *buf
r6 <- size_t count
```

[Example](./tests/test-syscall-write.csm)

#### `exit` syscall

Exit a given process with the given exit code

```
SYSCALL_NUMBER <- 60
r3 <- Return value (no idea)
r4 <- int error_code
```

[Example](./tests/test-syscall-exit.csm)

#### `clock_gettime` syscall

Get the seconds and nanoseconds

```
SYSCALL_NUMBER <- 228
r3 <- Return value (no idea)
r4 <- clockid_t which_clock
r5 <- struct __kernel_timespec *tp
```

[Example](./tests/test-syscall-clock_gettime.csm)

