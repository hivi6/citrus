#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// ========================================
// helper declaration
// ========================================

void register_set(vm_t *vm, uint64_t reg, uint64_t value);
uint64_t register_get(vm_t *vm, uint64_t reg);
uint64_t mask(uint64_t value, uint64_t mask_value);

// ========================================
// vm.h - definition
// ========================================

void vm_init(vm_t *vm, uint64_t stack_size) {
	vm->inst = NULL;
	vm->inst_size = 0;

	vm->r_size = 8;
	vm->r = malloc(sizeof(uint64_t) * vm->r_size);

	vm->stack_size = stack_size;
	vm->stack = malloc(sizeof(uint8_t) * stack_size);

	uint8_t *sp = vm->stack_size - 1 + vm->stack;
	vm->r[1] = (uint64_t) sp;
}

void vm_load(vm_t *vm, inst_t *inst, uint64_t inst_size) {
	vm->inst = inst;
	vm->inst_size = inst_size;

	vm->r[0] = (uint64_t) inst;
}

int vm_next(vm_t *vm) {
	inst_t *ip = (inst_t *) vm->r[0];
	uint8_t *sp = (uint8_t *) vm->r[1];

	assert(vm->inst <= ip && ip < (vm->inst + vm->inst_size));

	uint64_t arg1 = ip->arg1;
	uint64_t arg2 = ip->arg2;
	uint64_t arg3 = ip->arg3;

	switch (ip->type) {
	case INST_NOP:
		ip++;
		break;
	case INST_HLT:
		return VM_HALT;
	case INST_LOAD_CONST:
		register_set(vm, arg1, arg2);
		ip++;
		break;
	case INST_LOAD:
		register_set(vm, arg1, mask(register_get(vm, arg2), arg3));
		ip++;
		break;
	default:
		fprintf(stderr, "What is this instruction?\n");
		return VM_HALT;
	}
	vm->r[0] = (uint64_t) ip;
	vm->r[1] = (uint64_t) sp;

	return VM_CONTINUE;
}

void vm_free(vm_t *vm) {
	free(vm->r);
	free(vm->stack);
}

// ========================================
// helper definition
// ========================================

void register_set(vm_t *vm, uint64_t reg, uint64_t value) {
	if (vm->r_size <= reg) {
		vm->r_size = reg + 1;
		vm->r = realloc(vm->r, sizeof(uint64_t) * vm->r_size);
	}
	vm->r[reg] = value;
}

uint64_t register_get(vm_t *vm, uint64_t reg) {
	assert(0 <= reg && reg < vm->r_size);
	return vm->r[reg];
}

uint64_t mask(uint64_t value, uint64_t bytes) {
	assert(0 <= bytes && bytes <= 8);
	uint64_t masker = 0;
	for (uint64_t i = 0; i < bytes; i++) {
		masker <<= 8;
		masker |= 0xff;
	}
	return value & masker;
}

