#include <iostream>
#include <cstdio>
#include <cstdint>
#include "globals.h"
#include "riscv.h"

int32_t mem[MEM_SIZE]; //4096 palavras (de 32-bits), 16384 bytes (de 8-bits)

int32_t lw(uint32_t address, int32_t kte);
int32_t lb(uint32_t address, int32_t kte);
int32_t lbu(uint32_t address, int32_t kte);
void sw(uint32_t address, int32_t kte, int32_t dado);
void sb(uint32_t address, int32_t kte, int8_t dado);

void init() {

};

void fetch(instruction_context_st& ic) {
    ri = lw(pc, 0); //carrega instrução endereçada pelo pc
    pc = pc + 4;                //aponta para a próxima instrução
}

void decode (instruction_context_st& ic) {
    int32_t tmp;
    opcode	= ri & 0x7F;
    rs2		= (ri >> 20) & 0x1F;
    rs1		= (ri >> 15) & 0x1F;
    rd		= (ri >> 7)  & 0x1F;
    shamt	= (ri >> 20) & 0x1F;
    funct3	= (ri >> 12) & 0x7;
    funct7  = (ri >> 25);
    imm12_i = ((int32_t)ri) >> 20;
    tmp     = get_field(ri, 7, 0x1f);
    imm12_s = set_field(imm12_i, 0, 0x1f, tmp);
    imm13   = imm12_s;
    imm13 = set_bit(imm13, 11, imm12_s&1);
    imm13 = imm13 & ~1;
    imm20_u = ri & (~0xFFF);
    // mais aborrecido...
    imm21 = (int32_t)ri >> 11;			// estende sinal
    tmp = get_field(ri, 12, 0xFF);		// le campo 19:12
    imm21 = set_field(imm21, 12, 0xFF, tmp);	// escreve campo em imm21
    tmp = get_bit(ri, 20);				// le o bit 11 em ri(20)
    imm21 = set_bit(imm21, 11, tmp);			// posiciona bit 11
    tmp = get_field(ri, 21, 0x3FF);
    imm21 = set_field(imm21, 1, 0x3FF, tmp);
    imm21 = imm21 & ~1;					// zero bit 0
    ic.ins_code = get_instr_code(opcode, funct3, funct7);
    ic.ins_format = get_i_format(opcode, funct3, funct7);
    ic.rs1 = (REGISTERS)rs1;
    ic.rs2 = (REGISTERS)rs2;
    ic.rd = (REGISTERS)rd;
    ic.shamt = shamt;
    ic.imm12_i = imm12_i;
    ic.imm12_s = imm12_s;
    ic.imm13 = imm13;
    ic.imm21 = imm21;
    ic.imm20_u = imm20_u;
}

int32_t lw(uint32_t address, int32_t kte) {
    int32_t endereco = address + kte;
    int32_t mem_value;

    if (endereco % 4 != 0) {
        printf("Erro: endereco nao e multiplo de 4.\n");
        return 0;
    } else {
        int mem_index = endereco >> 2;
        mem_value = mem[mem_index];

        return mem_value;
    }
};

int32_t lb(uint32_t address, int32_t kte) {
    uint32_t mask  = 0x000000ff;
    uint32_t sub_address;
    int32_t mem_value;
    int8_t mem_byte;

    address = address + kte;
    sub_address = address % 4;

    int mem_index = address >> 2;
    mem_value = mem[mem_index];

    mask = mask << sub_address * 8;
    mem_byte = mask & mem_value;
    mem_byte = mem_byte >> sub_address * 8;

    int32_t extended_byte = mem_byte;
    return extended_byte;

};

int32_t lbu(uint32_t address, int32_t kte) {
    uint32_t mask = 0x000000ff;
    uint32_t sub_address;
    uint32_t mem_value;
    uint8_t mem_byte;

    address = address + kte;
    sub_address = address % 4;

    int mem_index = address >> 2;
    mem_value = mem[mem_index];

    mask = mask << sub_address * 8;
    mem_byte = mask & mem_value;
    mem_byte = mem_byte >> sub_address * 8;

    mask = mask >> sub_address * 8;
    mem_byte = mask & mem_byte;

    return mem_byte;

};

void sw(uint32_t address, int32_t kte, int32_t dado) {
    int32_t endereco = address + kte;

    if (endereco % 4 != 0) {
        printf("Erro: endereco nao e multiplo de 4.\n");
    } else {
        int mem_index = endereco >> 2;
        mem[mem_index] = dado;
    }
};

void sb(uint32_t address, int32_t kte, int8_t dado) {
    uint32_t dado_mask   = 0x000000ff;
    uint32_t erase_mask  = 0x000000ff;
    uint32_t sub_address;
    int32_t mem_value;

    address = address + kte;
    sub_address = address % 4;

    int mem_index = address >> 2;
    mem_value = mem[mem_index];

    dado_mask = dado_mask & dado;
    dado_mask = dado_mask << sub_address * 8;

    erase_mask = erase_mask << sub_address * 8;
    erase_mask = ~erase_mask;
    int32_t erased_value = mem_value & erase_mask;
    int32_t written_value = erased_value ^ dado_mask;

    mem[mem_index] = written_value;
};
