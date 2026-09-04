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

/* i80386 Mod R/M */
typedef struct I80386_MOD_RM {
	union {
		uint8_t byte;
		struct {
			uint8_t rm  : 3; /* r/m */
			uint8_t reg : 3; /* register */
			uint8_t mod : 2; /* mode */
		};
	};
} I80386_MOD_RM;

/* i80386 SIB */
typedef struct I80386_SIB {
	union {
		uint8_t byte;
		struct {
			uint8_t base  : 3;
			uint8_t index : 3;
			uint8_t scale : 2;
		};
	};
} I80386_SIB;

/* i80386 Effective Address */
typedef struct I80386_EFFECTIVE_ADDRESS {
	uint32_t base;
	uint32_t offset;
	uint8_t segment_index;
	uint8_t stack_address;
	uint8_t valid;
} I80386_EFFECTIVE_ADDRESS;

/* I80386 CPU State */
typedef struct I80386_MNEM {
	uint16_t counter;        /* instruction length */
	uint32_t base;           /* CS.Base */
	uint32_t offset;         /* EIP */
	uint8_t opcode;          /* opcode */
	uint8_t segment_prefix;  /* segement override index */
	uint8_t internal_flags;  /* rep prefix */
	uint8_t operand_size;
	uint8_t addressing_size;
	I80386_MOD_RM modrm;     /* modrm structure */
	I80386_SIB sib;          /* sib structure */

	I80386_EFFECTIVE_ADDRESS effective_address; 
	MNEM_RENDER_LINE* line;
} I80386_MNEM;

/* Get mnemonic tokens */
int i80386_mnem_get_tokens(uint32_t address, MNEM_RENDER_LINE* line);

/* Disassemble Opcode at address */
int i80386_mnem_get_str(uint32_t address, char* buffer, size_t size);

/* Disassemble Opcode at address */
int i80386_mnem_decode(uint32_t address, I80386_MNEM* mnem);

#endif
