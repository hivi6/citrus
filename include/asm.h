#ifndef ASM_H
#define ASM_H

#include "inst.h"

/**
 * Load assembly file and convert to instructions
 *
 * Params:
 * 	filepath   Path to the file that is loaded
 * 	inst       List of instruction pointer
 * 	inst_size  Instruction size
 */
void load_assembly(const char *filepath, inst_t **inst, uint64_t *inst_size);

#endif // ASM_H

