#include "asm.h"
#include "inst.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================================
// helper declaration
// ========================================

enum {
	TT_COLON,

	TT_IDENTIFIER,
	TT_INT_LITERAL,

	// Special type of identifier
	TT_REGISTER,
	TT_LABEL,
};

typedef struct token_t {
	int type;
	int start;
	int end;

	struct token_t *next;
} token_t;

static const char *g_filepath;
static char *g_source;
static int g_source_len;
static int g_current;
static token_t *g_head, *g_tail, *g_cur_token;
static char **g_labels;
static uint64_t *g_labels_pos;
static int g_labels_size;
static inst_t *g_inst;
static int g_inst_size;
static int *g_jmp_labels_index;
static const char **g_jmp_labels_name;
static int g_jmp_labels_size;

void init(const char *filepath);
void load_file();
void parse();
char eof();
void parse_line();
void tokenize();
void token_free();
char current_char();
void ignore_whitespace();
void next_char();
void token_append(int start, int end, int type);
int current_pos();
token_t *peek();
void next_token();
void check(int inst, int size, ...);
void errormsg(int start, int end, const char *msg);
char token_cmp(token_t *token, const char *name);
void store_label(token_t *token);
void inst_append(const char *label, uint64_t type, uint64_t arg1, uint64_t arg2, uint64_t arg3);
uint64_t get_register(token_t *token);
uint64_t get_label_pointer(const char *token);
uint64_t get_int_literal(token_t *token);
char *get_label();
char *token_lexical(token_t *token);
void apply_labels();


// ========================================
// asm.h - definition
// ========================================

void load_assembly(const char *filepath, inst_t **inst, uint64_t *inst_size) {
	init(filepath);
	load_file();
	parse();
	apply_labels();

	*inst = g_inst;
	*inst_size = g_inst_size;
}

// ========================================
// helper definition
// ========================================

void init(const char *filepath) {
	g_filepath = filepath;
	g_current = 0;
	g_head = g_tail = NULL;
	g_labels = NULL;
	g_labels_pos = NULL;
	g_labels_size = 0;
	g_inst = NULL;
	g_inst_size = 0;
	g_jmp_labels_index = NULL;
	g_jmp_labels_name = NULL;
	g_jmp_labels_size = 0;
}

void load_file() {
	FILE *fd = stdin;
	int isFile = (strcmp(g_filepath, "-") != 0);
	if (isFile) fd = fopen(g_filepath, "r");
	if (fd == NULL) {
		char buffer[1024];
		snprintf(buffer, 1024, "Error opening '%s'", g_filepath);
		perror(buffer);
		exit(1);
	}

	int cap = 1024, len = 0;
	char *buffer = (char *) malloc(cap * sizeof(char));
	if (buffer == NULL) {
		perror("Error in load_file while malloc");
		exit(1);
	}

	while (1) {
		int size = fread(buffer, 1, cap - len, fd);
		if (size == 0) break;

		len += size;
		if (len == cap) {
			cap *= 2;
			buffer = realloc(buffer, cap * sizeof(char));
			if (buffer == NULL) {
				perror("Error in load_file while malloc");
				exit(1);
			}
		}
	}

	buffer = realloc(buffer, len + 1);
	if (buffer == NULL) {
		perror("Error in read_file with realloc - 2");
		exit(1);
	}
	buffer[len] = '\0';

	if (isFile) fclose(fd);

	g_source = buffer;
	g_source_len = len;
}

void parse() {
	while (!eof()) {
		parse_line();
	}
}

char eof() {
	return g_current >= g_source_len;
}

void parse_line() {
	tokenize();
	if (!peek()) return;

	if (peek()->type != TT_IDENTIFIER) {
		errormsg(peek()->start, peek()->end,
			"What is this token type?");
		exit(1);
	}

	token_t *identifier = peek();
	next_token();

	if (peek() && peek()->type == TT_COLON) {
		next_token();

		if (peek()) {
			errormsg(peek()->start, peek()->end, 
				"After label nothing should exists");
			exit(1);
		}

		store_label(identifier);
		return;
	}

	if (token_cmp(identifier, "INST_NOP")) 
		check(INST_NOP, 0);
	else if (token_cmp(identifier, "INST_HLT")) 
		check(INST_HLT, 0);
	else if (token_cmp(identifier, "INST_LOAD_CONST")) 
		check(INST_LOAD_CONST, 2, TT_REGISTER, TT_INT_LITERAL);
	else if (token_cmp(identifier, "INST_LOAD")) 
		check(INST_LOAD, 3, TT_REGISTER, TT_REGISTER, TT_INT_LITERAL);
	else if (token_cmp(identifier, "INST_LOAD_INDIRECT"))
		check(INST_LOAD_INDIRECT, 3, TT_REGISTER, TT_REGISTER, TT_INT_LITERAL);
	else if (token_cmp(identifier, "INST_PUSH"))
		check(INST_PUSH, 1, TT_REGISTER);
	else if (token_cmp(identifier, "INST_POP"))
		check(INST_POP, 1, TT_REGISTER);
	else if (token_cmp(identifier, "INST_STORE_INDIRECT"))
		check(INST_STORE_INDIRECT, 3, TT_REGISTER, TT_REGISTER, TT_INT_LITERAL);
	else if (token_cmp(identifier, "INST_JMP"))
		check(INST_JMP, 1, TT_LABEL);
	else if (token_cmp(identifier, "INST_JMP_FALSE"))
		check(INST_JMP_FALSE, 2, TT_REGISTER, TT_LABEL);
	else if (token_cmp(identifier, "INST_ADD"))
		check(INST_ADD, 3, TT_REGISTER, TT_REGISTER, TT_REGISTER);
	else if (token_cmp(identifier, "INST_SUB"))
		check(INST_SUB, 3, TT_REGISTER, TT_REGISTER, TT_REGISTER);
	else if (token_cmp(identifier, "INST_CALL"))
		check(INST_CALL, 1, TT_LABEL);
	else if (token_cmp(identifier, "INST_RET"))
		check(INST_RET, 0);
	else if (token_cmp(identifier, "INST_SYSCALL"))
		check(INST_SYSCALL, 1, TT_INT_LITERAL);
	else {
		errormsg(identifier->start, identifier->end, "What is this opcode?");
		exit(1);
	}
}

void tokenize() {
	token_free();

	while (!eof() && current_char() != '\n') {
		ignore_whitespace();

		if (eof() || current_char() == '\n') break;

		int start = current_pos();
		char ch = current_char();
		int type = TT_COLON;
		if (ch == ':') {
			next_char();
			type = TT_COLON;
		}
		else if (ch == '#') { // comments
			while (!eof() && current_char() != '\n') {
				next_char();
				ignore_whitespace();
			}
			break;
		}
		else if (isalpha(ch) || ch == '_') {
			while (!eof() && (ch = current_char()) && 
				(isalnum(ch) || ch == '_')) {
				next_char();
			}

			type = TT_IDENTIFIER;
		}
		else if (isdigit(ch)) {
			while (!eof() && isdigit(current_char())) {
				next_char();
			}
			type = TT_INT_LITERAL;
		}
		else {
			errormsg(start, start + 1, "What is this character?");
			exit(1);
		}

		token_append(start, current_pos(), type);
	}
	next_char();
}

void token_free() {
	while (g_head) {
		token_t *prev = g_head;
		g_head = g_head->next;
		free(prev);
	}
	g_head = g_tail = NULL;
}

char current_char() {
	if (eof()) return 0;
	return g_source[g_current];
}

void ignore_whitespace() {
	while (current_char() == ' ' || current_char() == '\t') next_char();
}

void next_char() {
	g_current++;
}

void token_append(int start, int end, int type) {
	token_t *cur = malloc(sizeof(token_t));
	cur->start = start;
	cur->end = end;
	cur->type = type;
	cur->next = NULL;

	if (g_head == NULL) g_head = g_cur_token = g_tail = cur;
	else {
		g_tail->next = cur;
		g_tail = cur;
	}
}

int current_pos() {
	return g_current;
}

token_t *peek() {
	return g_cur_token;
}

void next_token() {
	g_cur_token = g_cur_token->next;
}

void check(int inst, int size, ...) {
	assert(0 <= size && size <= 3);

	va_list ap;
	va_start(ap, size);

	uint64_t arg1, arg2, arg3;
	for (int i = 0; i < size; i++) {
		if (!peek()) {
			fprintf(stderr, "Expected some token in check()\n");
			exit(1);
		}

		int type = va_arg(ap, int);
		uint64_t value = 0;
		if (type == TT_REGISTER) {
			value = get_register(peek());
		}
		else if (type == TT_LABEL) {
			int index = g_jmp_labels_size;
			g_jmp_labels_size++;
			g_jmp_labels_index = realloc(g_jmp_labels_index, 
				sizeof(int) * g_jmp_labels_size);
			g_jmp_labels_name = realloc(g_jmp_labels_name, 
				sizeof(char *) * g_jmp_labels_size);
			g_jmp_labels_index[index] = g_inst_size;
			g_jmp_labels_name[index] = token_lexical(peek());
			value = -1; // For now just a placeholder
		}
		else if (type == TT_INT_LITERAL) {
			value = get_int_literal(peek());
		}

		if (i == 0) arg1 = value;
		else if (i == 1) arg2 = value;
		else if (i == 2) arg3 = value;

		next_token();
	}

	if (peek()) {
		errormsg(peek()->start, peek()->end, 
			"Not expected this token");
		exit(1);
	}

	inst_append(get_label(), inst, arg1, arg2, arg3);

	va_end(ap);
}

void errormsg(int start, int end, const char *msg) {
	while (start - 1 >= 0 && g_source[start - 1] != '\n') 
		start--;
	while (end < g_source_len - 1 && g_source[end] != '\n')
		end++;

	printf("ERROR: %s\n", msg);
	printf("\n");
	for (int i = start; i < end; i++) printf("%c", g_source[i]);
	printf("\n\n");
}

char token_cmp(token_t *token, const char *name) {
	if (strlen(name) != token->end - token->start) return 0;
	return strncmp(g_source + token->start, name, strlen(name)) == 0;
}

void store_label(token_t *token) {
	char *token_str = token_lexical(token);
	for (int i = 0; i < g_labels_size; i++) {
		char *label_str = g_labels[i];
		if (strcmp(token_str, label_str) == 0) {
			errormsg(token->start, token->end, 
				"Label already defined");
			exit(0);
		}
	}

	g_labels_size++;
	g_labels = realloc(g_labels, g_labels_size * sizeof(token_t *));
	g_labels_pos = realloc(g_labels_pos, g_labels_size * sizeof(uint64_t));
	g_labels[g_labels_size-1] = token_str;
	g_labels_pos[g_labels_size-1] = g_inst_size;
}

void inst_append(const char *label, uint64_t type, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	inst_t inst = {label, type, arg1, arg2, arg3};

	g_inst_size++;
	g_inst = realloc(g_inst, g_inst_size * sizeof(inst_t));
	g_inst[g_inst_size-1] = inst;
}

uint64_t get_register(token_t *token) {
	char *lexical = token_lexical(token);
	int size = strlen(lexical);
	if (size <= 1 || lexical[0] != 'r') {
		errormsg(token->start, token->end, "Not a valid register");
		exit(1);
	}

	uint64_t reg = 0;
	for (int i = 1; i < size; i++) {
		if (!isdigit(lexical[i])) {
			errormsg(token->start, token->end, 
				"Not a valid register number");
			exit(1);
		}
		reg = reg * 10 + (lexical[i] - '0');
	}
	free(lexical);
	return reg;
}

uint64_t get_label_pointer(const char *token_str) {
	int i = 0;
	int res = -1;
	while (i < g_labels_size) {
		char *label_str = g_labels[i];
		if (strcmp(label_str, token_str) == 0) {
			res = g_labels_pos[i];
			break;
		}
		i++;
	}

	if (res == -1) {
		fprintf(stderr, "No label exists");
		exit(1);
	}

	return res;
}

uint64_t get_int_literal(token_t *token) {
	uint64_t res = 0;
	char *token_str = token_lexical(token);

	for (int i = 0; token_str[i]; i++) {
		if (!isdigit(token_str[i])) {
			errormsg(token->start, token->end,
				"No a valid digit");
			exit(1);
		}
		res = res * 10 + (token_str[i] - '0');
	}

	free(token_str);
	return res;
}

char *get_label() {
	for (int i = 0; i < g_labels_size; i++) {
		if (g_labels_pos[i] == g_inst_size) {
			int size = strlen(g_labels[i]) + 5;
			char *str = malloc(size * sizeof(char));
			sprintf(str, "%s", g_labels[i]);
			return str;
		}
	}
	return NULL;
}

char *token_lexical(token_t *token) {
	int size = token->end - token->start;
	char *str = malloc((size + 1) * sizeof(char));
	for (int i = token->start; i < token->end; i++) {
		str[i-token->start] = g_source[i];
	}
	return str;
}

void apply_labels() {
	for (int i = 0; i < g_jmp_labels_size; i++) {
		int index = g_jmp_labels_index[i];
		const char *name = g_jmp_labels_name[i];
		int jmp_pointer = get_label_pointer(name);

		if (g_inst[index].type == INST_JMP) {
			g_inst[index].arg1 = jmp_pointer;
		}
		else if (g_inst[index].type == INST_JMP_FALSE) {
			g_inst[index].arg2 = jmp_pointer;
		}
		else if (g_inst[index].type == INST_CALL) {
			g_inst[index].arg1 = jmp_pointer;
		}
		else {
			fprintf(stderr, "What is this instruction???\n");
			exit(1);
		}
	}
}

