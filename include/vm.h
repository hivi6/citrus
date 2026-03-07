#ifndef VM_H
#define VM_H

#include "inst.h"

enum {
	VM_CONTINUE = 0,
	VM_HALT,
};

struct vm_t {
	// Instruction space
	inst_t *inst;
	uint64_t inst_size;

	// registers (infinite)
	// r[0] = instruction pointer
	// r[1] = stack pointer
	// r[2..] = general pointer (but using r[2] for frame pointer)
	uint64_t *r;
	uint64_t r_size;

	// stack
	uint8_t *stack;
	uint64_t stack_size;
};
typedef struct vm_t vm_t;

/**
 * Initialize the vm
 *
 * Params:
 * 	vm          machine that needs initializing
 * 	stack_size  size of the stack in the machine
 */
void vm_init(vm_t *vm, uint64_t stack_size);

/**
 * Load instruction into the vm
 *
 * Params:
 * 	vm         machine where instruction is loaded
 * 	inst       list of instruction
 * 	inst_size  size of the list of instruction
 */
void vm_load(vm_t *vm, inst_t *inst, uint64_t inst_size);

/**
 * Run the next instruction
 *
 * Params:
 * 	vm  machine where instruction is ran
 */
int vm_next(vm_t *vm);

/**
 * Free the vm
 *
 * Params:
 * 	vm  machine that needs freeing
 */
void vm_free(vm_t *vm);

#endif // VM_H

