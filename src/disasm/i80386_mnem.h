/* i80386_mnem.h
 * Thomas J. Armytage 2025-2026 ( https://github.com/tommojphillips/ )
 * Intel 80386 Mnemonics/Disassembler
 */

#ifndef I80386_MNEM_H
#define I80386_MNEM_H

#include <stdint.h>

#define I80386_MNEM_MAX_TOKENS 32
typedef enum {
	MNEM_TOKEN_MNEMONIC,
	MNEM_TOKEN_GENERAL_REGISTER,
	MNEM_TOKEN_CONTROL_REGISTER,
	MNEM_TOKEN_DEBUG_REGISTER,
	MNEM_TOKEN_TEST_REGISTER,
	MNEM_TOKEN_IMMEDIATE,
	MNEM_TOKEN_NUMBER,
	MNEM_TOKEN_MEMORY,
	MNEM_TOKEN_SEGMENT,
	MNEM_TOKEN_PREFIX,
	MNEM_TOKEN_OPERATOR,
	MNEM_TOKEN_RELATIVE_ADDRESS,
	MNEM_TOKEN_ABSOLUTE_ADDRESS,
} MNEM_TOKEN_TYPE;

typedef struct {
	const char* text;
	MNEM_TOKEN_TYPE type;
	int operand_size;
	int len;
	uint32_t number;
} MNEM_TOKEN;

typedef struct {
	MNEM_TOKEN tokens[I80386_MNEM_MAX_TOKENS];
	int token_count;
} MNEM_RENDER_LINE;

/* Get mnemonic tokens */
int i80386_mnem_get_tokens(uint32_t address, MNEM_RENDER_LINE* line);

/* Disassemble Opcode at address */
int i80386_mnem_get_str(uint32_t address, char* buffer, size_t size);

#endif
