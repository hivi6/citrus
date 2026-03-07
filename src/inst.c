#include "inst.h"

#include <stdio.h>
#include <stdlib.h>

// ========================================
// inst.h - definition
// ========================================

void inst_print(inst_t *inst, uint64_t inst_size) {
	for (uint64_t i = 0; i < inst_size; i++) {
		if (inst[i].label) {
			printf("%s:\n", inst[i].label);
		}

		const char *inst_str = NULL;
		int arg = 0;

		switch (inst[i].type) {
		case INST_NOP:
			inst_str = "INST_NOP";
			break;
		case INST_HLT:
			inst_str = "INST_HLT";
			break;
		case INST_LOAD_CONST:
			inst_str = "INST_LOAD_CONST";
			arg = 2;
			break;
		case INST_LOAD:
			inst_str = "INST_LOAD";
			arg = 3;
			break;
		case INST_LOAD_INDIRECT:
			inst_str = "INST_LOAD_INDIRECT";
			arg = 3;
			break;
		case INST_PUSH:
			inst_str = "INST_PUSH";
			arg = 1;
			break;
		case INST_POP:
			inst_str = "INST_POP";
			arg = 1;
			break;
		case INST_STORE_INDIRECT:
			inst_str = "INST_STORE_INDIRECT";
			arg = 3;
			break;
		case INST_JMP:
			inst_str = "INST_JMP";
			arg = 1;
			break;
		case INST_JMP_FALSE:
			inst_str = "INST_JMP_FALSE";
			arg = 2;
			break;
		case INST_ADD:
			inst_str = "INST_ADD";
			arg = 3;
			break;
		case INST_SUB:
			inst_str = "INST_SUB";
			arg = 3;
			break;
		}

		printf("    %-20s ", inst_str);
		if (arg >= 1) printf("%20llu ", inst[i].arg1);
		if (arg >= 2) printf("%20llu ", inst[i].arg2);
		if (arg >= 3) printf("%20llu ", inst[i].arg3);
		printf("\n");
	}
}

