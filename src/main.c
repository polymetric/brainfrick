#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <stdbool.h>

const int BF_MEM_SIZE = 65536;

enum {
	BF_ERROR_NONE = 0,
	BF_ERROR_PTR_OOB,
};

int bf_error = BF_ERROR_NONE;

typedef enum Direction {
	DIR_INC,
	DIR_DEC,
} Direction;

typedef int bf_ptr_t;
typedef char bf_byte_t;

typedef struct ProgramState {
	const char* program;
	bf_byte_t *mem;
	bf_ptr_t data_ptr;
	bf_ptr_t ins_ptr;
} ProgramState;

ProgramState *bf_init_program_state(const int mem_size, const char* program) {
	ProgramState *state = (ProgramState*) malloc(sizeof(ProgramState));
	bf_ptr_t mem_host_bytes_ct = sizeof(bf_byte_t) * mem_size;
	state->program = program;
	state->mem = (bf_byte_t*) malloc(mem_host_bytes_ct);
	memset(state->mem, 0, mem_host_bytes_ct);
	state->data_ptr = 0;
	state->ins_ptr = 0;
	return state;
}

void bf_check_ptr_bounds(const bf_ptr_t data_ptr) {
	if (data_ptr < 0 || data_ptr >= BF_MEM_SIZE) {
		bf_error = BF_ERROR_PTR_OOB;
		return;
	}
}

bf_byte_t bf_deref_ptr(const ProgramState *state) {
	return state->mem[state->data_ptr];
}

char bf_current_sym(const ProgramState *state) {
	return state->program[state->ins_ptr];
}

void bf_mv_ptr(ProgramState *state, const Direction dir) {
	bf_ptr_t temp_ptr = state->data_ptr;

	switch (dir) {
		case DIR_INC:
			temp_ptr += 1;
			break;
		case DIR_DEC:
			temp_ptr -= 1;
			break;
		default:
			break;
	}

	bf_check_ptr_bounds(temp_ptr);

	if (bf_error != BF_ERROR_PTR_OOB) {
		state->data_ptr = temp_ptr;
	}
}

void bf_change_val(ProgramState *state, const Direction dir) {
	bf_byte_t *mem = state->mem;
	bf_ptr_t data_ptr = state->data_ptr;

	switch (dir) {
		case DIR_INC:
			mem[data_ptr] += 1;
			break;
		case DIR_DEC:
			mem[data_ptr] -= 1;
			break;
		default:
			break;
	}
}

void bf_print_val(const ProgramState *state) {
	putchar((char) bf_deref_ptr(state));
}

void bf_put_val(ProgramState *state, const char x) {
	state->mem[state->data_ptr] = x;
}

int bf_exec_step(ProgramState *state) {
	switch (bf_current_sym(state)) {
		case '\0':
			return 1;
			break;
		case '>': // increment pointer
			bf_mv_ptr(state, DIR_INC);
			state->ins_ptr += 1;
			break;
		case '<': // decrement pointer
			bf_mv_ptr(state, DIR_DEC);
			state->ins_ptr += 1;
			break;
		case '+': // increment value
			bf_change_val(state, DIR_INC);
			state->ins_ptr += 1;
			break;
		case '-': // decrement value
			bf_change_val(state, DIR_DEC);
			state->ins_ptr += 1;
			break;
		case '.':
			bf_print_val(state);
			state->ins_ptr += 1;
			break;
		case ',':
			bf_put_val(state, getchar());
			state->ins_ptr += 1;
			break;
		case '[':
			if (bf_deref_ptr(state) == 0) {
				int starts = 1;
				while (starts > 0) {
					state->ins_ptr += 1;
					if (bf_current_sym(state) == '[') {
						starts += 1;
					} else if (bf_current_sym(state) == ']') {
						starts -= 1;
					}
				}
			} else {
				state->ins_ptr += 1;
			}
			break;
		case ']':
			if (bf_deref_ptr(state) != 0) {
				int starts = 1;
				while (starts > 0) {
					state->ins_ptr -= 1;
					if (bf_current_sym(state) == ']') {
						starts += 1;
					} else if (bf_current_sym(state) == '[') {
						starts -= 1;
					}
				}
			} else {
				state->ins_ptr += 1;
			}
			break;
		default:
			state->ins_ptr += 1;
			break;
	}

	return 0;
}

void bf_exec(ProgramState *state) {
	while (1) {
		if (bf_exec_step(state)) {
			return;
		}
	}
}

int main(const int argc, const char **argv) {
	if (argc < 2) {
		printf("not enough arguments\n");
		exit(-1);
	}

	// read file to string
	FILE *file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("filerror");
		exit(-1);
	}
	// get file size
	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *program = malloc(filesize + 1);
	fread(program, 1, filesize, file);
	fclose(file);

	ProgramState *state = bf_init_program_state(BF_MEM_SIZE, program);

	bf_exec(state);
}
