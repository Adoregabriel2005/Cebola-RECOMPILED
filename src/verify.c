#include "glue.h"
#include "replay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static Replay movie;
static uint64_t save_frame, end_frame;
static const wchar_t *save_path;
static int failed;
static uint64_t audio_hash=UINT64_C(14695981039346656037);
static void audio(const int16_t *p,size_t n){ audio_hash=monica_hash(p,n*4,audio_hash); }
static uint8_t script(uint64_t f){
    if(getenv("MONICA_VERIFY_IDLE"))return 0;
    if(f>=200 && f<210) return SMS_PAD_B1;
    if(f>=600 && f<900) return SMS_PAD_RIGHT;
    if(f>=950 && f<970) return SMS_PAD_B2|SMS_PAD_RIGHT;
    return 0;
}
static int frame(const uint32_t *fb,int w,int h){
    (void)w;(void)h;
    uint64_t f=glue_frame_count();
    if(movie.target && f==movie.target){
        uint64_t actual=glue_state_hash();
        if(actual!=movie.state_hash){ fprintf(stderr,"SAVE_STATE_MISMATCH %llx != %llx\n",actual,movie.state_hash); failed=1; return 1; }
        fprintf(stderr,"SAVE_RESTORED frame=%llu hash=%llx\n",f,actual);
    }
    if(save_frame && f==save_frame && !replay_save(save_path,&movie,f,glue_state_hash())){ failed=1; return 1; }
    if(f==end_frame){
        printf("RESULT %llu %016llx %016llx %016llx misses=%d\n",f,glue_state_hash(),
            monica_hash(fb,256*192*4,UINT64_C(14695981039346656037)),audio_hash,glue_dispatch_miss_count());
        return 1;
    }
    uint8_t pad;
    if(f<=movie.count) pad=movie.inputs[f-1];
    else { pad=script(f); if(!replay_append(&movie,pad)){failed=1;return 1;} }
    glue_set_pad1(pad); return 0;
}
int wmain(int argc,wchar_t **argv){
    if(argc<5){ fprintf(stderr,"verify ROM END SAVE_FRAME SAVE_PATH [LOAD_PATH]\n"); return 2; }
    int n=WideCharToMultiByte(CP_UTF8,0,argv[1],-1,0,0,0,0);
    char *rom=malloc(n); WideCharToMultiByte(CP_UTF8,0,argv[1],-1,rom,n,0,0);
    end_frame=wcstoull(argv[2],0,10); save_frame=wcstoull(argv[3],0,10); save_path=argv[4];
    if(argc>5 && !replay_load(argv[5],&movie)){ fprintf(stderr,"INVALID_SAVE\n");return 3; }
    if(!glue_load_rom_wide(argv[1])) return 2;
    free(rom); glue_init(false,0); glue_require_native(true);glue_set_audio_sink(audio); glue_set_frame_callback(frame);
    const char *dump=getenv("MONICA_VERIFY_DUMP");if(dump)glue_set_dump(end_frame,dump);
    if(getenv("MONICA_VERIFY_REFERENCE"))glue_run_interp();else glue_run_step();
    replay_free(&movie); return (failed || glue_frame_count()!=end_frame || glue_dispatch_miss_count()) ? 1 : 0;
}
