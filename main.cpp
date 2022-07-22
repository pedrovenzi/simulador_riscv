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

}

INSTRUCTIONS get_instr_code(uint32_t opcode, uint32_t func3, uint32_t func7) {
    switch (opcode) {
        case RegType:
            switch (func3) {
                case ADDSUB3:
                    switch (func7) {
                        case ADD7:
                            return I_add;
                        case SUB7:
                            return I_sub;
                    }
                case SLL3:
                    return I_sll;
                case SLT3:
                    return I_slt;
                case SLTU3:
                    return I_sltu;
                case XOR3:
                    return I_xor;
                case SR3:
                    switch (func7) {
                        case SRA7:
                            return I_sra;
                        case SRL7:
                            return I_srl;
                    }
                case OR3:
                    return I_or;
                case AND3:
                    return I_and;
            }
        case ILType:
            switch (func3) {
                case LB3:
                    return I_lb;
                case LH3:
                    return I_lh;
                case LW3:
                    return I_lw;
                case LBU3:
                    return I_lbu;
                case LHU3:
                    return I_lhu;
            }
        case ILAType:
            switch (func3) {
                case ADDI3:
                    return I_addi;
                case SLTI3:
                    return I_slti;
                case SLTIU3:
                    return I_sltiu;
                case XORI3:
                    return I_xori;
                case ORI3:
                    return I_ori;
                case ANDI3:
                    return I_andi;
                case SLLI3:
                    return I_slli;
                case SRI3:
                    switch (func7) {
                        case SRLI7:
                            return I_srli;
                        case SRAI7:
                            return I_srai;
                    }
            }
        case StoreType:
            switch (func3) {
                case SB3:
                    return I_sb;
                case SH3:
                    return I_sh;
                case SW3:
                    return I_sw;
            }
        case BType:
            switch (func3) {
                case BEQ3:
                    return I_beq;
                case BNE3:
                    return I_bne;
                case BLT3:
                    return I_blt;
                case BGE3:
                    return I_bge;
                case BLTU3:
                    return I_bltu;
                case BGEU3:
                    return I_bgeu;
            }
        case LUI:
            return I_lui;
        case AUIPC:
            return I_auipc;
        case JAL:
            return I_jal;
        case JALR:
            return I_jalr;
        case ECALL:
            return I_ecall;
    }


}

FORMATS get_i_format(uint32_t opcode, uint32_t func3, uint32_t func7) {
    switch (opcode) {
        case RegType:
            return RType;
        case ILType:
        case ILAType:
            return IType;
        case StoreType:
            return SType;
        case BType:
            return SBType;
        case LUI:
        case AUIPC:
            return UType;
        case JAL:
        case JALR:
            return UJType;
        case ECALL:
            return NullFormat;
    }

}

void fetch (instruction_context_st& ic) {
    ri = lw(pc, 0); //carrega instrução endereçada pelo pc
    ic.pc = pc;
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

void execute (instruction_context_st& ic) {
    breg[ZERO] = 0;

    switch (ic.ins_code) {
        case I_add:
            breg[rd] = breg[rs1] + breg[rs2];
        case I_addi:
            breg[rd] = breg[rs1] + ic.imm12_i;
        case I_and:
            breg[rd] = breg[rs1] & breg[rs2];
        case I_andi:
            breg[rd] = breg[rs1] & ic.imm12_i;
        case I_auipc:
            breg[rd] = pc + sebregt(ic.imm20_u << 12);
        case I_beq:
            if (rs1 == rs2) pc += sebregt(offset);
        case I_bge:
            if (rs1 >=s rs2) pc += sebregt(offset);
        case I_bgeu:
            if (rs1 >=u rs2) pc += sebregt(offset);
        case I_blt:
            if (rs1 <s rs2) pc += sebregt(offset);
        case I_bltu:
            if (rs1 >u rs2) pc += sebregt(offset);
        case I_bne:
            if (rs1 != rs2) pc += sebregt(offset);
        case I_jal:
            breg[rd] = pc+4; pc += sebregt(offset);
        case I_jalr:
            t =pc+4; pc=(breg[rs1]+sebregt(offset))&∼1; breg[rd]=t;
        case I_lb:
            t =pc+4; pc=(breg[rs1]+sebregt(offset))&∼1; breg[rd]=t;
        case I_lbu:
            breg[rd] = M[breg[rs1] + sebregt(offset)][7:0];
        case I_lw:
            breg[rd] = sebregt(M[breg[rs1] + sebregt(offset)][31:0]);
        case I_lh:
            breg[rd] = sebregt(M[breg[rs1] + sebregt(offset)][15:0]);
        case I_lhu:
            breg[rd] = M[breg[rs1] + sebregt(offset)][15:0];
        case I_lui:
            breg[rd] = sebregt(immediate[31:12] << 12);
        case I_sb:
            M[breg[rs1] + sebregt(offset)] = breg[rs2][7:0];
        case I_sh:
            M[breg[rs1] + sebregt(offset)] = breg[rs2][15:0];
        case I_sw:
            M[breg[rs1] + sebregt(offset)] = breg[rs2][31:0];
        case I_sll:
            breg[rd] = breg[rs1] << breg[rs2];
        case I_slt:
            breg[rd] = breg[rs1] <s breg[rs2];
        case I_slli:
            breg[rd] = breg[rs1] << shamt;
        case I_srl:
            breg[rd] = breg[rs1] >>u breg[rs2];
        case I_sra:
            breg[rd] = breg[rs1] >>s breg[rs2];
        case I_sub:
            breg[rd] = breg[rs1] - breg[rs2];
        case I_slti:
            breg[rd] = breg[rs1] <s sebregt(immediate);
        case I_sltiu:
            breg[rd] = breg[rs1] <u sebregt(immediate);
        case I_xor:
            breg[rd] = breg[rs1] ^ breg[rs2];
        case I_or:
            breg[rd] = breg[rs1] | breg[rs2];
        case I_srli:
            breg[rd] = breg[rs1] >>u shamt;
        case I_srai:
            breg[rd] = breg[rs1] >>s shamt;
        case I_sltu:
            breg[rd] = breg[rs1] <u breg[rs2];
        case I_ori:
            breg[rd] = breg[rs1] | sebregt(immediate);
        case I_ecall:
            RaiseEbregception(EnvironmentCall);
        case I_xori:
            breg[rd] = breg[rs1] ^ sebregt(immediate);
        //case I_nop:
    }
}

void step() {

}

void run() {

}

void dump_mem(int start_byte, int end_byte, char format) {

}

void dump_reg(char format) {

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
