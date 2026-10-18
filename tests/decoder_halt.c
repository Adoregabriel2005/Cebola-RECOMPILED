/* Regression for ignored index prefixes on HALT. Compare the build decoder
   with the independent instruction executor on a tiny, original program. */
#include <assert.h>
#include <stdio.h>
#include "z80_decoder.h"
#include "z80.h"
static unsigned char memory[65536];
static uint8_t read_byte(void *unused,uint16_t address){(void)unused;return memory[address];}
static void write_byte(void *unused,uint16_t address,uint8_t value){(void)unused;memory[address]=value;}
int main(void){
    unsigned char programs[][2]={{0x76,0},{0xdd,0x76},{0xfd,0x76}};
    for(int i=0;i<3;i++){
        unsigned length=i?2:1;Z80Insn in;assert(z80_decode(programs[i],length,0,&in)==(int)length);
        assert(in.is_halt&&!in.uses_disp&&!in.illegal&&in.opcode==0x76);
        memory[0]=programs[i][0];memory[1]=programs[i][1];z80 reference;z80_init(&reference);
        reference.read_byte=read_byte;reference.write_byte=write_byte;z80_step(&reference);
        assert(reference.halted&&reference.pc==length&&reference.cyc==length*4);
    }
    puts("PASS decoder HALT, DD HALT and FD HALT: halt flag, length, no indexed displacement; reference PC/cycles agree");return 0;
}
