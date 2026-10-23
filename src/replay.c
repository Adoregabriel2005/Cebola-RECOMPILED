#include "replay.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t cebola_hash(const void *data, size_t n, uint64_t h){
    const uint8_t *p=data;
    while(n--) { h ^= *p++; h *= UINT64_C(1099511628211); }
    return h;
}
uint32_t cebola_crc32(const void *data,size_t n){
    const uint8_t *p=data; uint32_t c=~0u;
    while(n--){ c ^= *p++; for(int i=0;i<8;i++) c=(c>>1)^(0xEDB88320u & (0u-(c&1))); }
    return ~c;
}
void replay_free(Replay *r){ free(r->inputs); memset(r,0,sizeof *r); }
int replay_append(Replay *r,uint8_t input){
    if(r->count>=REPLAY_MAX_FRAMES || (input & ~0x7f)) return 0;
    if(r->count==r->capacity){
        uint32_t cap=r->capacity ? r->capacity*2 : 4096;
        if(cap>REPLAY_MAX_FRAMES) cap=REPLAY_MAX_FRAMES;
        uint8_t *p=realloc(r->inputs,cap); if(!p) return 0;
        r->inputs=p; r->capacity=cap;
    }
    r->inputs[r->count++]=input; return 1;
}
static void put32(uint8_t *p,uint32_t v){ for(int i=0;i<4;i++) p[i]=(uint8_t)(v>>(i*8)); }
static uint32_t get32(const uint8_t *p){ return p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static void put64(uint8_t *p,uint64_t v){ put32(p,(uint32_t)v); put32(p+4,(uint32_t)(v>>32)); }
static uint64_t get64(const uint8_t *p){ return get32(p)|((uint64_t)get32(p+4)<<32); }
int replay_save(const wchar_t *path,const Replay *r,uint64_t frame,uint64_t hash){
    if(!frame || frame!=((uint64_t)r->count+1) || r->count>REPLAY_MAX_FRAMES) return 0;
    uint8_t h[64]={0}; memcpy(h,"MNCRPLY1",8);
    put32(h+8,REPLAY_BUILD); put32(h+12,CEBOLA_ROM_CRC); put32(h+16,CEBOLA_ROM_SIZE);
    put32(h+20,r->count); put64(h+24,frame); put64(h+32,hash);
    put32(h+40,cebola_crc32(r->inputs,r->count)); put32(h+44,cebola_crc32(h,44));
    wchar_t temp[32768]; if(wcslen(path)>32000) return 0;
    swprintf(temp,32768,L"%ls.tmp",path);
    FILE *f=_wfopen(temp,L"wb"); if(!f) return 0;
    int ok=fwrite(h,1,sizeof h,f)==sizeof h;
    if(r->count && fwrite(r->inputs,1,r->count,f)!=r->count) ok=0;
    if(fflush(f)) ok=0;
    if(fclose(f)) ok=0;
    if(ok) ok=MoveFileExW(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok) DeleteFileW(temp);
    return ok;
}
int replay_load(const wchar_t *path,Replay *out){
    FILE *f=_wfopen(path,L"rb"); if(!f) return 0;
    uint8_t h[64]; Replay r={0}; int ok=0;
    if(fread(h,1,64,f)!=64) goto done;
    if(memcmp(h,"MNCRPLY1",8) || get32(h+8)!=REPLAY_BUILD || get32(h+12)!=CEBOLA_ROM_CRC ||
       get32(h+16)!=CEBOLA_ROM_SIZE || get32(h+44)!=cebola_crc32(h,44)) goto done;
    for(int i=48;i<64;i++) if(h[i]) goto done;
    r.count=r.capacity=get32(h+20); r.target=get64(h+24); r.state_hash=get64(h+32);
    if(r.count>REPLAY_MAX_FRAMES || r.target!=((uint64_t)r.count+1)) goto done;
    r.inputs=malloc(r.count ? r.count : 1); if(!r.inputs) goto done;
    if(fread(r.inputs,1,r.count,f)!=r.count || fgetc(f)!=EOF) goto done;
    if(cebola_crc32(r.inputs,r.count)!=get32(h+40)) goto done;
    for(uint32_t i=0;i<r.count;i++) if(r.inputs[i] & ~0x7f) goto done;
    ok=1;
done:
    fclose(f);
    if(ok){ replay_free(out); *out=r; } else replay_free(&r);
    return ok;
}
