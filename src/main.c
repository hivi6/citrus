#include <stdio.h>

#include "inst.h"

int main() {
	inst_t inst[] = {
		{"main", INST_NOP},
		{NULL, INST_HLT},
		{NULL, INST_LOAD_CONST, 3, 1024},
		{NULL, INST_LOAD, 4, 3, 8},
		{NULL, INST_PUSH, 4},
		{NULL, INST_POP, 5},
		{NULL, INST_LOAD_INDIRECT, 4, 3, 8},
		{NULL, INST_STORE_INDIRECT, 4, 3, 8},
		{NULL, INST_JMP, 0},
		{NULL, INST_JMP_FALSE, 3, 0},
		{NULL, INST_ADD, 3, 3, 4},
		{NULL, INST_SUB, 3, 3, 4},
	};
	uint64_t inst_size = 12;

	inst_print(inst, inst_size);

	return 0;
}

