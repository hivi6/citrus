#ifndef INST_H
#define INST_H

#include <stdint.h>

enum {
	// No operations.
	// No arguments;
	INST_NOP,

	// Halt processor.
	// No arguments;
	INST_HLT,

	// Load const into register.
	// arg1 = destination register index;
	// arg2 = const value (64-bit value);
	INST_LOAD_CONST,

	// load the content of a register to another register but 
	// only the least significant bytes.
	// arg1 = destination register index;
	// arg2 = source register index;
	// arg3 = size = [0, 8];
	INST_LOAD,

	// Load the content of a register's memory to another register
	// only the least significant bytes.
	// arg1 = destination register index;
	// arg2 = source register index (where memory address exists);
	// arg3 = size = [0, 8];
	INST_LOAD_INDIRECT,

	// Push register value to the stack.
	// arg1 = source register index;
	INST_PUSH,

	// Pop register value from the stack.
	// arg1 = destination register index;
	INST_POP,

	// Store the content of a register to a register's memory address
	// only the least significant digits.
	// arg1 = destination register index (where memory address exists);
	// arg2 = source register index;
	// arg3 = size = [0, 8];
	INST_STORE_INDIRECT,

	// Jump to the given offset in the instruction
	// arg1 = offset in the instruction;
	INST_JMP,

	// Jump to the given offset in the instruction if register is zero
	// arg1 = register index;
	// arg2 = offset in the instruction;
	INST_JMP_FALSE,

	// Jump to the given offset in the instruction if register is non zero
	// arg1 = register index;
	// arg2 = offset in the instruction;
	INST_JMP_TRUE,

	// Add content of 2 register and store it to a destination resgier
	// arg1 = destination register index;
	// arg2 = left operand;
	// arg3 = right operand;
	INST_ADD,
	
	// Subtract content of 2 register and store it to a destination resgier
	// arg1 = destination register index;
	// arg2 = left operand;
	// arg3 = right operand;
	INST_SUB,
	
	// Multiply content of 2 register and store it to a destination resgier
	// arg1 = destination register index;
	// arg2 = left operand;
	// arg3 = right operand;
	INST_MUL,
	
	// Divide content of 2 register and store it to a destination resgier
	// arg1 = destination register index;
	// arg2 = left operand;
	// arg3 = right operand;
	INST_DIV,
	
	// Modulus content of 2 register and store it to a destination resgier
	// arg1 = destination register index;
	// arg2 = left operand;
	// arg3 = right operand;
	INST_MOD,

	// Function call to a given label
	// arg1 = offset
	INST_CALL,

	// Function return
	// No arguments
	INST_RET,

	// Make a syscall
	// arg1 = syscall number;
	INST_SYSCALL,
};

struct inst_t {
	const char *label;

	uint64_t type;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
};
typedef struct inst_t inst_t;

/**
 * Print all the instruction
 *
 * Params:
 * 	inst       list of instructions
 * 	inst_size  size of the instruction list
 */
void inst_print(inst_t *inst, uint64_t inst_size);

#endif // INST_H

