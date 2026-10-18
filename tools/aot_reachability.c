/* Static branch closure over observed bank/PC seeds. This is a build tool,
   not a runtime interpreter. Cross-slot edges use observed bank bindings,
   optionally expanded to every page-2 bank;
   indirect destinations and unobserved mappings still require playtesting. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include "z80_decoder.h"
static unsigned char seen[16][0xc000];
static uint32_t queue[16*0xc000];
static unsigned slot_banks[3];
static size_t tail,head;
static void add(unsigned bank,unsigned pc){
    if(pc>=0xc000||bank>=16)return;if(pc<0x400)bank=0;
    if(!seen[bank][pc]){seen[bank][pc]=1;queue[tail++]=(bank<<16)|pc;}
}
static void edge(unsigned bank,unsigned pc,unsigned target){
    if(target>=0xc000)return;
    if(target<0x400){add(0,target);return;}
    if((pc>>14)==(target>>14)){add(bank,target);return;}
    unsigned candidates=slot_banks[target>>14];
    for(unsigned b=0;b<16;b++)if(candidates&(1u<<b))add(b,target);
}
int wmain(int argc,wchar_t **argv){
    if(argc<4||argc>6){fprintf(stderr,"aot-reachability ROM SEEDS OUTPUT [--all-page2-banks] [--all-observed-entrypoints]\n");return 2;}
    int broad=0,complete=0;
    for(int i=4;i<argc;i++){if(!wcscmp(argv[i],L"--all-page2-banks"))broad=1;else if(!wcscmp(argv[i],L"--all-observed-entrypoints"))complete=1;else return 2;}
    FILE *f=_wfopen(argv[1],L"rb");if(!f)return 2;
    fseek(f,0,SEEK_END);long length=ftell(f);rewind(f);
    if(length<=0||length>16*0x4000){fclose(f);return 2;}
    size_t size=(size_t)length;uint8_t *rom=malloc(size);
    if(!rom||fread(rom,1,size,f)!=size){free(rom);fclose(f);return 2;}fclose(f);
    f=_wfopen(argv[2],L"r");if(!f){free(rom);return 2;}
    unsigned b,pc;int parsed;
    while((parsed=fscanf(f,"%x %x",&b,&pc))==2){
        if(b>=16||pc>=0xc000){fclose(f);free(rom);return 2;}
        slot_banks[pc>>14]|=1u<<b;add(b,pc);
    }fclose(f);if(parsed!=EOF||!tail){free(rom);return 2;}
    size_t seeds=tail;
    /* Z80 indirect jumps can enter any instruction boundary. Conservatively
       compile every byte offset of the bank/slot bindings actually observed,
       without claiming every offset is executable code. */
    if(complete)for(unsigned slot=0;slot<3;slot++)for(b=0;b<16;b++)if(slot_banks[slot]&(1u<<b))
        for(pc=slot*0x4000;pc<(slot+1)*0x4000;pc++)add(b,pc);
    /* Sega mapper page 2 can select every ROM bank. Include all direct edges
       into that page, even if the relevant level was never played. */
    if(broad)slot_banks[2]=(1u<<((unsigned)size/0x4000))-1;
    while(head<tail){
        uint32_t e=queue[head++];b=e>>16;pc=e&65535;
        size_t off=(size_t)b*0x4000+(pc&0x3fff);if(off>=size)continue;
        Z80Insn in;if(z80_decode(rom+off,size-off,(uint16_t)pc,&in)<=0||in.illegal)continue;
        if(z80_cf_falls_through(in.cf))edge(b,pc,pc+in.length);
        if(in.has_target)edge(b,pc,in.target);
    }
    f=_wfopen(argv[3],L"w");if(!f){free(rom);return 2;}
    for(b=0;b<16;b++)for(pc=0;pc<0xc000;pc++)if(seen[b][pc])fprintf(f,"%02X %04X\n",b,pc);
    int failed=fclose(f);free(rom);
    printf("AOT static closure: seeds=%zu candidates=%zu; indirect/unobserved mappings are not proven covered\n",seeds,tail);
    return failed?2:0;
}
