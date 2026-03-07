#include <stdio.h>

#include "inst.h"
#include "vm.h"

int main() {
	inst_t inst[] = {
		{"main", INST_NOP},
		{NULL, INST_LOAD_CONST, 2, 8},
		{NULL, INST_SUB, 2, 1, 2},
		{NULL, INST_LOAD_CONST, 3, (1 << 3) - 1},
		{NULL, INST_LOAD_CONST, 4, 8},
		{NULL, INST_SUB, 4, 1, 4},
		{NULL, INST_LOAD_INDIRECT, 4, 4, 8},
		{NULL, INST_PUSH, 3},
		{NULL, INST_LOAD_CONST, 5, 0xf12345},
		{NULL, INST_PUSH, 5},
		{NULL, INST_POP, 6},
		{NULL, INST_POP, 7},
		{NULL, INST_STORE_INDIRECT, 2, 5, 1},
		{NULL, INST_JMP_FALSE, 10, 0},
		{NULL, INST_HLT},
	};
	uint64_t inst_size = 15;

	printf("========================================\n");
	printf("INSTRUCTIONS\n");
	printf("========================================\n");
	printf("\n");

	for (uint64_t i = 0; i < inst_size; i++) {
		inst_print(inst, i);
	}

	printf("\n");

	vm_t vm;
	vm_init(&vm, 1024 * 1024);
	vm_load(&vm, inst, inst_size);

	vm.stack[vm.stack_size-1] = 12;
	vm.stack[vm.stack_size-2] = 13;

	while (!vm_next(&vm));

	printf("========================================\n");
	printf("VM STATE\n");
	printf("========================================\n");
	printf("\n");

	printf("STACK: %llu MB\n", vm.stack_size / 1024 / 1024);
	uint64_t row = 0;
	for (uint8_t *sp = (uint8_t *)(vm.stack_size + vm.stack) - 1; sp >= vm.stack; row++) {
		for (uint64_t i = 0; i < 8 && sp >= vm.stack; i++, sp--) {
			printf("%p - 0x%02x ", sp, *sp);
		}
		printf("\n");
		if (row >= 8) break;
	}
	printf("[...]\n");
	printf("\n");

	printf("NUMBER OF REGISTERS: %llu\n", vm.r_size);
	for (uint64_t i = 0; i < vm.r_size; i++) {
		printf("r%-2llu -> ", i);

		printf("0x%016llx (%llu)", vm.r[i], vm.r[i]);

		printf("\n");
	}

	printf("\n");

	vm_free(&vm);

	return 0;
}

