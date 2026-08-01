BITS 32
global mm_init
global mm_size

%include "src\kernel\include\common.inc"

TEST_PATTERN1 equ 0x00000000
TEST_PATTERN2 equ 0xFFFFFFFF
TEST_PATTERN3 equ 0x55555555
TEST_PATTERN4 equ 0xAAAAAAAA

section .bss
mm_size dd ?

section .text

mm_init:

    ret
