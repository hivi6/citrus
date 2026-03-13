#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// ========================================
// helper declaration
// ========================================

void register_set(vm_t *vm, uint64_t reg, uint64_t value);
uint64_t register_get(vm_t *vm, uint64_t reg);
uint64_t mask(uint64_t value, uint64_t mask_value);
uint64_t addr_get(uint64_t addr, uint64_t size);
void addr_set(uint64_t addr, uint64_t value, uint64_t size);
void vm_syscall(vm_t *vm, uint64_t number);

// ========================================
// vm.h - definition
// ========================================

void vm_init(vm_t *vm, uint64_t stack_size) {
	vm->inst = NULL;
	vm->inst_size = 0;

	vm->r_size = 16;
	vm->r = malloc(sizeof(uint64_t) * vm->r_size);

	vm->stack_size = stack_size;
	vm->stack = malloc(sizeof(uint8_t) * stack_size);

	uint8_t *sp = vm->stack_size + vm->stack;
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
	uint8_t *prev_sp = sp;

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
	case INST_LOAD_INDIRECT: {
		uint64_t addr = register_get(vm, arg2);
		uint64_t size = arg3;
		register_set(vm, arg1, addr_get(addr, size));
		ip++;
		break;
	}
	case INST_PUSH: {
		uint64_t value = register_get(vm, arg1);
		sp -= 8;
		addr_set((uint64_t) sp, value, 8);
		ip++;
		break;
	}
	case INST_POP: {
		uint64_t value = addr_get((uint64_t) sp, 8);
		sp += 8;
		register_set(vm, arg1, value);
		ip++;
		break;
	}
	case INST_STORE_INDIRECT: {
		uint64_t addr = register_get(vm, arg1);
		uint64_t value = register_get(vm, arg2);
		uint64_t size = arg3;
		addr_set(addr, value, size);
		ip++;
		break;
	}
	case INST_JMP: {
		uint64_t offset = arg1;
		ip = vm->inst + offset;
		break;
	}
	case INST_JMP_FALSE: {
		uint64_t value = register_get(vm, arg1);
		uint64_t offset = arg2;
		if (!value) ip = &vm->inst[offset];
		else ip++;
		break;
	}
	case INST_JMP_TRUE: {
		uint64_t value = register_get(vm, arg1);
		uint64_t offset = arg2;
		if (value) ip = &vm->inst[offset];
		else ip++;
		break;
	}
	case INST_ADD: {
		uint64_t left = register_get(vm, arg2);
		uint64_t right = register_get(vm, arg3);
		register_set(vm, arg1, left + right);
		ip++;
		break;
	}
	case INST_SUB: {
		uint64_t left = register_get(vm, arg2);
		uint64_t right = register_get(vm, arg3);
		register_set(vm, arg1, left - right);
		ip++;
		break;
	}
	case INST_MUL: {
		uint64_t left = register_get(vm, arg2);
		uint64_t right = register_get(vm, arg3);
		register_set(vm, arg1, left * right);
		ip++;
		break;
	}
	case INST_DIV: {
		uint64_t left = register_get(vm, arg2);
		uint64_t right = register_get(vm, arg3);
		register_set(vm, arg1, left / right);
		ip++;
		break;
	}
	case INST_MOD: {
		uint64_t left = register_get(vm, arg2);
		uint64_t right = register_get(vm, arg3);
		register_set(vm, arg1, left % right);
		ip++;
		break;
	}
	case INST_CALL: {
		inst_t *new_ip = vm->inst + arg1;
		// store the instruction next to call
		uint64_t value = (uint64_t) (ip + 1); 
		sp -= 8;
		addr_set((uint64_t) sp, value, 8);
		ip = new_ip;
		break;
	}
	case INST_RET: {
		uint64_t ret_ip = addr_get((uint64_t) sp, 8);
		sp += 8;
		ip = (inst_t *) ret_ip;
		break;
	}
	case INST_SYSCALL: {
		vm_syscall(vm, arg1);
		ip++;
		break;
	}
	default:
		fprintf(stderr, "What is this instruction?\n");
		return VM_HALT;
	}
	vm->r[0] = (uint64_t) ip;
	if (prev_sp != sp) vm->r[1] = (uint64_t) sp;

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

uint64_t addr_get(uint64_t addr, uint64_t size) {
	assert(0 <= size && size <= 8);
	uint64_t res = 0;
	uint8_t *res_ptr = (uint8_t *) &res;
	uint8_t *addr_ptr = (uint8_t *) addr;
	for (uint64_t i = 0; i < size; i++) {
		res_ptr[i] = addr_ptr[i];
	}
	return res;
}

void addr_set(uint64_t addr, uint64_t value, uint64_t size) {
	assert(0 <= size && size <= 8);
	uint8_t *addr_ptr = (uint8_t *) addr;
	uint8_t *value_ptr = (uint8_t *) &value;
	for (uint64_t i = 0; i < size; i++) {
		addr_ptr[i] = value_ptr[i];
	}
}

void vm_syscall(vm_t *vm, uint64_t number) {
	uint64_t returnValue = 0;
	uint64_t arg0 = vm->r[4];
	uint64_t arg1 = vm->r[5];
	uint64_t arg2 = vm->r[6];
	uint64_t arg3 = vm->r[7];
	uint64_t arg4 = vm->r[8];
	uint64_t arg5 = vm->r[9];

	switch (number) {
	case 0: // READ
		returnValue = read(arg0, (void *) arg1, arg2);
		break;
	case 1: // WRITE
		returnValue = write(arg0, (void *) arg1, arg2);
		break;
	case 60: // EXIT
		exit(arg0);
		break;
	default:
		fprintf(stderr, "No such syscall number: %llu", number);
		exit(1);
	}

	vm->r[3] = returnValue;
}

