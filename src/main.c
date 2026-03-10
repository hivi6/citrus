#include <stdio.h>
#include <stdlib.h>

#include "inst.h"
#include "vm.h"
#include "asm.h"

int main(int argc, char **argv) {
	int show_state = 0;

	inst_t *inst;
	uint64_t inst_size;

	if (argc != 2) {
		fprintf(stderr, "No file provided\n");
		exit(1);
	}

	load_assembly(argv[1], &inst, &inst_size);

	vm_t vm;
	vm_init(&vm, 1024 * 1024);
	vm_load(&vm, inst, inst_size);

	while (!vm_next(&vm));

	if (show_state) {
		printf("========================================\n");
		printf("INSTRUCTIONS\n");
		printf("========================================\n");
		printf("\n");

		for (uint64_t i = 0; i < inst_size; i++) {
			inst_print(inst, i);
		}

		printf("\n");


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
	}

	vm_free(&vm);

	return 0;
}

