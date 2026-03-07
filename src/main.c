#include <stdio.h>

#include "inst.h"
#include "vm.h"

int main() {
	inst_t inst[] = {
		{"main", INST_NOP},
		{NULL, INST_LOAD_CONST, 3, 1023},
		{NULL, INST_LOAD, 4, 3, 8},
		{NULL, INST_PUSH, 4},
		{NULL, INST_POP, 5},
		{NULL, INST_LOAD_INDIRECT, 4, 3, 8},
		{NULL, INST_STORE_INDIRECT, 4, 3, 8},
		{NULL, INST_JMP, 0},
		{NULL, INST_JMP_FALSE, 3, 0},
		{NULL, INST_ADD, 3, 3, 4},
		{NULL, INST_SUB, 3, 3, 4},
		{NULL, INST_HLT},
	};
	uint64_t inst_size = 12;

	printf("========================================\n");
	printf("INSTRUCTIONS\n");
	printf("========================================\n");
	printf("\n");

	inst_print(inst, inst_size);

	printf("\n");

	vm_t vm;
	vm_init(&vm, 1024 * 1024);
	vm_load(&vm, inst, inst_size);

	while (!vm_next(&vm));

	printf("========================================\n");
	printf("VM STATE\n");
	printf("========================================\n");
	printf("\n");

	printf("NUMBER OF REGISTERS: %llu\n", vm.r_size);
	for (uint64_t i = 0; i < vm.r_size; i++) {
		printf("r%llu: ", i);

		if (i <= 1) {
			printf("%p", (void *)vm.r[i]);
		}
		else printf("%llu", vm.r[i]);

		printf("\n");
	}

	printf("\n");

	vm_free(&vm);

	return 0;
}

