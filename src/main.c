#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"
#include "vm.h"
#include "asm.h"

// ========================================
// helper declaration
// ========================================

void show_inst(inst_t *inst, uint64_t inst_size);
void show_vm_state(vm_t vm);
void debugger(vm_t *vm);
void debugger_usage();

// ========================================
// main definition
// ========================================

int main(int argc, char **argv) {
	int arg_cur = 1;
	int debugger_flag = 0;
	while (arg_cur < argc) {
		if (strcmp("--debugger", argv[arg_cur]) == 0) {
			debugger_flag = 1;
			arg_cur++;
		}
		else break;
	}

	if (argc <= arg_cur) {
		fprintf(stderr, "No file provided\n");
		exit(1);
	}

	inst_t *inst;
	uint64_t inst_size;

	load_assembly(argv[arg_cur], &inst, &inst_size);

	vm_t vm;
	vm_init(&vm, 1024 * 1024);
	vm_load(&vm, inst, inst_size);

	if (debugger_flag) {
		debugger(&vm);
	}
	else {
		while (!vm_next(&vm));
	}

	vm_free(&vm);

	return 0;
}

// ========================================
// helper definition
// ========================================

void show_inst(inst_t *inst, uint64_t inst_size) {
	printf("========================================\n");
	printf("INSTRUCTIONS\n");
	printf("========================================\n");
	printf("\n");

	for (uint64_t i = 0; i < inst_size; i++) {
		inst_print(inst, i);
	}

	printf("\n");
}

void show_vm_state(vm_t vm) {
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

void debugger(vm_t *vm) {
	printf("DEBUGGER MODE\n");
	debugger_usage();

	int vm_state = 0;
	char last_cmd[1024];
	while (!vm_state) {
		char cmd[1024];
		printf("> ");
		fgets(cmd, sizeof(cmd), stdin);
		cmd[strcspn(cmd, "\n")] = '\0';

		if (strlen(cmd) == 0) {
			strcpy(cmd, last_cmd);
		}

		if (strcmp(cmd, "next") == 0) {
			vm_state = vm_next(vm);
		}
		else if (strcmp(cmd, "cur") == 0) {
			inst_print((inst_t *) vm->r[0], 0);
		}
		else if (strcmp(cmd, "state") == 0) {
			show_vm_state(*vm);
		}
		else if (strcmp(cmd, "showall") == 0) {
			show_inst(vm->inst, vm->inst_size);
		}
		else if (strcmp(cmd, "exit") == 0) {
			vm_state = 1;
		}
		else if (strcmp(cmd, "help") == 0) {
			debugger_usage();
		}
		else {
			printf("Unknown command\n");
			continue;
		}

		strcpy(last_cmd, cmd);
	}
}

void debugger_usage() {
	printf("Usage:\n");
	printf("    cur     Show the current instruction\n");
	printf("    next    Go to the next instruction\n");
	printf("    state   Show the current vm state\n");
	printf("    showall Show all instructions\n");
	printf("    exit    Exist debugger mode\n");
	printf("    help    This screen\n");
	printf("\n");

}

