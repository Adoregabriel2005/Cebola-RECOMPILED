#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include "glue.h"
#include "replay.h"
#include "theme.h"
#include "preferences.h"
#define MENU_COUNT 7

enum { OPEN_ROM=100, PLAY, PAUSE, SAVE, LOAD, SETTINGS, EXIT_APP, MUTE, FULLSCREEN, OPTIONS, ABOUT, SLOT1=120 };
static HWND window, controls[MENU_COUNT];
static int hovered=-1,rom_ready;
static Preferences cfg;
static wchar_t asset_dir[32768];
static wchar_t ui_test_path[32768];
static wchar_t navigation_test_path[32768];
static wchar_t audio_test_path[32768];
#define keys cfg.keys
#define volume cfg.volume
#define resolution cfg.resolution
#define muted cfg.muted
static WNDPROC original_button_proc;
static void navigation(void);
static void toggle_fullscreen(void);
static HFONT font, title_font;
static HBRUSH background;
static wchar_t rom_path[32768], root[32768], ini[32768], pending_load[32768];
static Replay movie;
static int running, paused, start_requested, quitting, restart_requested, slot=1, full_screen;
static int session_ended;
static int console_pause_pulse;
static int console_pause_hover;


static uint32_t framebuffer[256*192];
static LARGE_INTEGER frequency, deadline;
static uint64_t last_frame;
static WINDOWPLACEMENT placement={sizeof(WINDOWPLACEMENT)};


static HWAVEOUT audio_device;
static HANDLE audio_event;
static WAVEHDR audio_headers[8];
static int16_t audio_buffers[8][4096];
static int audio_index;
static unsigned audio_fill,audio_chunk,audio_prefill;
static int audio_priming,audio_fault;
static uint64_t audio_output_frames,audio_submitted_frames,audio_waits;
static HDC present_dc;
static HBITMAP present_bitmap,present_old;
static int present_w,present_h;
static wchar_t notification[600];
static ULONGLONG notification_until;
static uint64_t audio_phase;
static int64_t audio_sum[2];
static unsigned audio_samples;

static void message(const wchar_t *text){wcsncpy(notification,text,599);notification[599]=0;notification_until=GetTickCount64()+9000;InvalidateRect(window,NULL,FALSE);}
static void draw_notification(HDC dc,int w,int h){
    if(!notification[0] || GetTickCount64()>=notification_until)return;
    RECT r={(!running||paused)?w*45/100:32,h-146,w-32,h-42};HBRUSH brush=CreateSolidBrush(RGB(12,31,26));HPEN pen=CreatePen(PS_SOLID,1,RGB(255,205,91));HGDIOBJ ob=SelectObject(dc,brush),op=SelectObject(dc,pen);
    RoundRect(dc,r.left,r.top,r.right,r.bottom,14,14);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(brush);DeleteObject(pen);
    HFONT old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,242,208));InflateRect(&r,-18,-12);DrawTextW(dc,notification,-1,&r,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);SelectObject(dc,old);
}
static void save_settings(void){
    WritePrivateProfileStringW(L"app",L"rom",rom_path,ini);
    cfg.fullscreen=full_screen;preferences_save(&cfg,ini);
}
static void slot_path(wchar_t *out){ swprintf(out,32768,L"%ls\\slot-%d.msave",root,slot); }
static void audio_reset(void){
    if(audio_device){waveOutReset(audio_device);waveOutPause(audio_device);}
    audio_index=0;audio_fill=0;audio_prefill=0;audio_priming=1;
    audio_phase=0; audio_samples=0; audio_sum[0]=audio_sum[1]=0;
}
static void audio_close(void){
    if(!audio_device) return;
    waveOutReset(audio_device);
    for(int i=0;i<8;i++) waveOutUnprepareHeader(audio_device,&audio_headers[i],sizeof(WAVEHDR));
    waveOutClose(audio_device); audio_device=NULL;
    if(audio_event){CloseHandle(audio_event);audio_event=NULL;}
    fprintf(stderr,"AUDIO output=%llu submitted=%llu waits=%llu fault=%d\n",audio_output_frames,audio_submitted_frames,audio_waits,audio_fault);
}
static void audio_open(void){
    WAVEFORMATEX fmt={WAVE_FORMAT_PCM,2,(DWORD)cfg.sample_rate,(DWORD)cfg.sample_rate*4,4,16,0};UINT selected=WAVE_MAPPER;
    for(UINT i=0;i<waveOutGetNumDevs();i++){WAVEOUTCAPSW caps;if(waveOutGetDevCapsW(i,&caps,sizeof caps)==MMSYSERR_NOERROR && !wcscmp(caps.szPname,cfg.audio_device)){selected=i;break;}}
    audio_event=CreateEventW(NULL,FALSE,FALSE,NULL);
    if(!audio_event || waveOutOpen(&audio_device,selected,&fmt,(DWORD_PTR)audio_event,0,CALLBACK_EVENT)!=MMSYSERR_NOERROR){audio_device=NULL;if(audio_event)CloseHandle(audio_event);audio_event=NULL;message(L"A saída de áudio não aceitou a configuração. Escolha outro dispositivo ou frequência em Configurações > Áudio.");return;}
    audio_chunk=(unsigned)cfg.sample_rate/100*2;audio_fault=0;audio_output_frames=audio_submitted_frames=audio_waits=0;
    memset(audio_headers,0,sizeof audio_headers);
    for(int i=0;i<8;i++){
        audio_headers[i].lpData=(LPSTR)audio_buffers[i];audio_headers[i].dwBufferLength=sizeof audio_buffers[i];
        if(waveOutPrepareHeader(audio_device,&audio_headers[i],sizeof(WAVEHDR))!=MMSYSERR_NOERROR){audio_fault=1;audio_close();message(L"Não foi possível preparar a saída de áudio.");return;}
    }
    audio_reset();
}
static int audio_buffer_ready(void){
    DWORD start=GetTickCount();
    while(audio_headers[audio_index].dwFlags&WHDR_INQUEUE){
        audio_waits++;WaitForSingleObject(audio_event,10);
        if(GetTickCount()-start>1000){audio_fault=1;return 0;}
    }return 1;
}
static int audio_submit(void){
    WAVEHDR *h=&audio_headers[audio_index];h->dwBufferLength=audio_fill*sizeof(int16_t);
    if(waveOutWrite(audio_device,h,sizeof *h)!=MMSYSERR_NOERROR){audio_fault=1;return 0;}
    audio_submitted_frames+=audio_fill/2;audio_fill=0;audio_index=(audio_index+1)%8;
    if(audio_priming && ++audio_prefill>=(unsigned)(cfg.latency==0?2:cfg.latency==1?4:6)){
        audio_priming=0;if(!paused)waveOutRestart(audio_device);
    }return 1;
}
static void audio_sink(const int16_t *samples,size_t count){
    /* Always drain synthesis during replay. Only device presentation is skipped. */
    if(!audio_device || audio_fault || muted || (movie.target && glue_frame_count()<movie.target)) return;
    unsigned rate=glue_audio_sample_rate();
    for(size_t i=0;i<count;i++){
        audio_sum[0]+=samples[2*i]; audio_sum[1]+=samples[2*i+1]; audio_samples++;audio_phase+=cfg.sample_rate;
        if(audio_phase>=rate){
            audio_phase-=rate;
            if(!audio_fill && !audio_buffer_ready())return;
            int l=(int)(audio_sum[0]/(int)audio_samples),r=(int)(audio_sum[1]/(int)audio_samples);if(cfg.mono)l=r=(l+r)/2;
            audio_buffers[audio_index][audio_fill++]=(int16_t)(l*volume/100);audio_buffers[audio_index][audio_fill++]=(int16_t)(r*volume/100);audio_output_frames++;
            audio_sum[0]=audio_sum[1]=0;audio_samples=0;
            if(audio_fill==audio_chunk && !audio_submit())return;
        }
    }
}
static void set_paused(int value){
    if(paused==value)return;
    paused=value;if(audio_device){if(paused)waveOutPause(audio_device);else if(!audio_priming)waveOutRestart(audio_device);}
    QueryPerformanceCounter(&deadline);InvalidateRect(window,NULL,FALSE);
    navigation();
    if(!paused)SetFocus(window);
}
static int key_down(int key){return GetForegroundWindow()==window && (GetAsyncKeyState(key)&0x8000);}
static uint8_t input_mask(void){
    uint8_t mask=0;for(int i=0;i<7;i++)if(key_down(keys[i]))mask|=1<<i;
    if(GetForegroundWindow()==window){InputState input=input_read();mask|=input.pad;if(input.menu&INPUT_START)mask|=SMS_PAD_START;}
    if(console_pause_pulse){mask|=SMS_PAD_START;console_pause_pulse=0;}return mask;
}
static void paint_content(HDC dc,RECT client){
    int w=client.right,h=client.bottom;
    if(!running || paused){theme_render(dc,w,h,paused,framebuffer,rom_ready,slot);draw_notification(dc,w,h);return;}
    FillRect(dc,&client,background);
    RECT source,dest=video_destination(w,h,cfg.aspect,cfg.scale,&source);
    if(dest.right<=dest.left||dest.bottom<=dest.top)return;
    static uint32_t display[256*192];
    for(int y=0;y<192;y++)for(int x=0;x<256;x++){
        uint32_t p=framebuffer[y*256+x];int gain=cfg.brightness;
        if((y&1)&&cfg.scanlines)gain=gain*(cfg.scanlines==1?80:55)/100;
        int r=((p>>16)&255)*gain/100,g=((p>>8)&255)*gain/100,b=(p&255)*gain/100;
        if(r>255)r=255;if(g>255)g=255;if(b>255)b=255;display[y*256+x]=0xFF000000u|(r<<16)|(g<<8)|b;
    }
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=256;info.bmiHeader.biHeight=-192;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    SetStretchBltMode(dc,cfg.filter?HALFTONE:COLORONCOLOR);
    StretchDIBits(dc,dest.left,dest.top,dest.right-dest.left,dest.bottom-dest.top,source.left,source.top,source.right-source.left,source.bottom-source.top,display,&info,DIB_RGB_COLORS,SRCCOPY);
    RECT button={w-48,14,w-14,48};HBRUSH b=CreateSolidBrush(console_pause_hover?RGB(146,34,43):RGB(12,31,26));HPEN p=CreatePen(PS_SOLID,1,RGB(211,184,112));HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,p);
    RoundRect(dc,button.left,button.top,button.right,button.bottom,10,10);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);
    b=CreateSolidBrush(RGB(255,225,159));RECT bar={w-37,24,w-33,39};FillRect(dc,&bar,b);OffsetRect(&bar,8,0);FillRect(dc,&bar,b);DeleteObject(b);
    draw_notification(dc,w,h);
}
static void paint(HDC dc,RECT client){
    int w=client.right,h=client.bottom;if(w<1||h<1)return;
    if(!present_dc || present_w!=w || present_h!=h){
        if(present_dc){SelectObject(present_dc,present_old);DeleteObject(present_bitmap);DeleteDC(present_dc);}
        present_dc=CreateCompatibleDC(dc);present_bitmap=CreateCompatibleBitmap(dc,w,h);present_old=SelectObject(present_dc,present_bitmap);present_w=w;present_h=h;
    }
    paint_content(present_dc,client);BitBlt(dc,0,0,w,h,present_dc,0,0,SRCCOPY);
}
static int validate_rom(const wchar_t *path){
    FILE *f=_wfopen(path,L"rb");if(!f)return 0;uint8_t *data=malloc(MONICA_ROM_SIZE);if(!data){fclose(f);return 0;}
    size_t n=fread(data,1,MONICA_ROM_SIZE,f);int extra=fgetc(f);
    int ok=n==MONICA_ROM_SIZE && extra==EOF && monica_crc32(data,n)==MONICA_ROM_CRC;free(data);fclose(f);return ok;
}
static int choose_rom(void){
    wchar_t path[32768]=L"";OPENFILENAMEW of={sizeof of};of.hwndOwner=window;of.lpstrFilter=L"ROM do Master System (*.sms)\0*.sms\0\0";
    of.lpstrFile=path;of.nMaxFile=32768;of.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&of))return 0;
    if(!validate_rom(path)){message(L"ROM incompatível. Esta versão requer Mônica no Castelo do Dragão (Brazil), 256 KiB, CRC32 01D67C0B.");return 0;}
    wcscpy(rom_path,path);rom_ready=1;save_settings();navigation();InvalidateRect(window,NULL,TRUE);return 1;
}
static void save_game(void){
    if(!running || movie.target){message(L"Inicie o jogo e aguarde a reconstrução terminar antes de salvar.");return;}
    wchar_t path[32768],note[160];slot_path(path);uint64_t hash=glue_state_hash();
    if(!hash || !replay_save(path,&movie,last_frame,hash)){message(L"Não foi possível salvar. O slot anterior foi preservado.");return;}
    swprintf(note,160,L"Slot %d salvo no quadro %llu.\nO carregamento reconstrói a sessão desde o início.",slot,last_frame);message(note);
}
static void load_game(void){
    wchar_t path[32768];slot_path(path);Replay test={0};
    if(!replay_load(path,&test)){message(L"Slot vazio, corrompido ou incompatível com esta versão.");return;}replay_free(&test);
    if(!rom_ready || !validate_rom(rom_path))return;
    wcscpy(pending_load,path);if(running)restart_requested=1;else start_requested=1;
}
static void settings(int tab){
    if(running)set_paused(1);
    preferences_open(window,&cfg,tab,asset_dir);
    navigation();
}
static void toggle_fullscreen(void){
    if(!full_screen){
        GetWindowPlacement(window,&placement);MONITORINFO mi={sizeof mi};GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTOPRIMARY),&mi);
        SetWindowLongW(window,GWL_STYLE,WS_POPUP|WS_VISIBLE|WS_CLIPCHILDREN);SetWindowPos(window,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_FRAMECHANGED);
    }else{SetWindowLongW(window,GWL_STYLE,WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN);SetWindowPlacement(window,&placement);SetWindowPos(window,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);}
    full_screen=!full_screen;cfg.fullscreen=full_screen;save_settings();
}
static void navigation(void){
    if(!window)return;
    const wchar_t *home[]={L"Jogar",L"Continuar",L"Controles",L"Configurações",L"Selecionar ROM",L"Sobre o jogo",L"Sair"};
    const wchar_t *pause_labels[]={L"Continuar aventura",L"Salvar progresso",L"Carregar progresso",L"Controles",L"Configurações",L"Sobre o jogo",L"Sair"};
    const int home_ids[]={PLAY,LOAD,SETTINGS,OPTIONS,OPEN_ROM,ABOUT,EXIT_APP};
    const int pause_ids[]={PAUSE,SAVE,LOAD,SETTINGS,OPTIONS,ABOUT,EXIT_APP};
    for(int i=0;i<MENU_COUNT;i++)if(controls[i]){
        SetWindowTextW(controls[i],running?pause_labels[i]:home[i]);
        SetWindowLongPtrW(controls[i],GWLP_ID,running?pause_ids[i]:home_ids[i]);
        EnableWindow(controls[i],running || i>1 || rom_ready);
        ShowWindow(controls[i],preferences_active() || (running&&!paused)?SW_HIDE:SW_SHOW);
    }
    theme_layout(window,controls,MENU_COUNT);
    HMENU menu=GetMenu(window);EnableMenuItem(menu,PLAY,MF_BYCOMMAND|(!running&&rom_ready?MF_ENABLED:MF_GRAYED));EnableMenuItem(menu,LOAD,MF_BYCOMMAND|(rom_ready?MF_ENABLED:MF_GRAYED));
}
static LRESULT CALLBACK menu_button_proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    int index=-1;for(int i=0;i<MENU_COUNT;i++)if(controls[i]==hwnd)index=i;
    if(msg==WM_MOUSEMOVE){
        if(hovered!=index){hovered=index;for(int i=0;i<MENU_COUNT;i++)InvalidateRect(controls[i],NULL,FALSE);}
        TRACKMOUSEEVENT track={sizeof track,TME_LEAVE,hwnd,0};TrackMouseEvent(&track);
    }
    if(msg==WM_MOUSELEAVE){hovered=-1;for(int i=0;i<MENU_COUNT;i++)InvalidateRect(controls[i],NULL,FALSE);}
    if(msg==WM_SETFOCUS || msg==WM_KILLFOCUS)for(int i=0;i<MENU_COUNT;i++)if(controls[i])InvalidateRect(controls[i],NULL,FALSE);
    if(msg==WM_KEYDOWN){
        if(wp==VK_UP || wp==VK_DOWN){int delta=wp==VK_UP?MENU_COUNT-1:1;for(int n=0;n<MENU_COUNT;n++){index=(index+delta)%MENU_COUNT;if(IsWindowEnabled(controls[index]))break;}SetFocus(controls[index]);return 0;}
        if(wp==VK_RETURN){SendMessageW(window,WM_COMMAND,GetDlgCtrlID(hwnd),0);return 0;}
        if(wp==VK_ESCAPE || wp==VK_F2 || wp==VK_F5 || wp==VK_F9 || wp==VK_F11){SendMessageW(window,msg,wp,lp);return 0;}
    }
    return CallWindowProcW(original_button_proc,hwnd,msg,wp,lp);
}
static void apply_preferences(void){
    input_select(cfg.device);input_configure(cfg.buttons);input_set_deadzone(cfg.deadzone);input_update();
    if(cfg.fullscreen!=full_screen)toggle_fullscreen();
    if(!full_screen){int widths[]={960,1280,1600,1920,2560,3840};int an=cfg.aspect==1?5:cfg.aspect==2?16:4,ad=cfg.aspect==1?4:cfg.aspect==2?9:3;
        RECT r={0,0,widths[resolution],MulDiv(widths[resolution],ad,an)};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,FALSE);SetWindowPos(window,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER);}
    audio_close();audio_phase=0;audio_samples=0;audio_sum[0]=audio_sum[1]=0;if(running)audio_open();
    if(paused&&audio_device)waveOutPause(audio_device);save_settings();InvalidateRect(window,NULL,FALSE);
}
static LRESULT CALLBACK window_proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
    case WM_DRAWITEM:{
        DRAWITEMSTRUCT *d=(DRAWITEMSTRUCT*)lp;
        for(int i=0;i<MENU_COUNT;i++)if(d->hwndItem==controls[i]){if(preferences_active())return TRUE;int selected=hovered==i || (hovered<0 && (GetFocus()==controls[i] || (GetFocus()==window && i==0)));theme_button(d,selected,i);return TRUE;}
        return FALSE;
    }
    case WM_SIZE:if(controls[0])theme_layout(hwnd,controls,MENU_COUNT);preferences_resize();InvalidateRect(hwnd,NULL,FALSE);return 0;
    case WM_TIMER:if((!running || paused)&&!preferences_active()){InvalidateRect(hwnd,NULL,FALSE);for(int i=0;i<MENU_COUNT;i++)InvalidateRect(controls[i],NULL,FALSE);}return 0;
    case WM_MOUSEMOVE:{RECT r;GetClientRect(hwnd,&r);console_pause_hover=LOWORD(lp)>r.right-48&&LOWORD(lp)<r.right-14&&HIWORD(lp)>=14&&HIWORD(lp)<48;return 0;}
    case WM_LBUTTONUP:{RECT r;GetClientRect(hwnd,&r);if(running&&!paused&&!preferences_active()&&LOWORD(lp)>r.right-48&&LOWORD(lp)<r.right-14&&HIWORD(lp)>=14&&HIWORD(lp)<48)console_pause_pulse=1;return 0;}
    case PREFS_APPLIED:apply_preferences();return 0;
    case PREFS_CLOSED:navigation();SetFocus(controls[running?0:rom_ready?0:4]);return 0;
    case WM_COMMAND:
        switch(LOWORD(wp)){
        case OPEN_ROM:if(!running)choose_rom();break;
        case PLAY:if(!running && rom_ready){rom_ready=validate_rom(rom_path);if(rom_ready){pending_load[0]=0;start_requested=1;}else navigation();}break;
        case PAUSE:if(running)set_paused(!paused);break;
        case SAVE:if(running){set_paused(1);save_game();}break;
        case LOAD:if(running)set_paused(1);load_game();break;
        case SETTINGS:settings(0);break;
        case OPTIONS:settings(1);break; case ABOUT:settings(3);break;
        case EXIT_APP:quitting=1;break;
        case MUTE:muted=!muted;audio_reset();save_settings();CheckMenuItem(GetMenu(hwnd),MUTE,MF_BYCOMMAND|(muted?MF_CHECKED:MF_UNCHECKED));break;
        case FULLSCREEN:toggle_fullscreen();break;
        default:if(LOWORD(wp)>=SLOT1 && LOWORD(wp)<SLOT1+3){slot=LOWORD(wp)-SLOT1+1;CheckMenuRadioItem(GetMenu(hwnd),SLOT1,SLOT1+2,LOWORD(wp),MF_BYCOMMAND);}break;
        }return 0;
    case WM_KEYDOWN:
        if(lp & (1L<<30))return 0;
        if(running&&!paused&&wp==(WPARAM)keys[6]){console_pause_pulse=1;return 0;}
        if((!running || paused) && (wp==VK_UP || wp==VK_DOWN)){SetFocus(controls[wp==VK_UP?MENU_COUNT-1:rom_ready?0:4]);return 0;}
        if((!running || paused) && wp==VK_RETURN){SendMessageW(hwnd,WM_COMMAND,GetDlgCtrlID(controls[rom_ready?0:4]),0);return 0;}
        if(wp==VK_ESCAPE)SendMessageW(hwnd,WM_COMMAND,PAUSE,0);if(wp==VK_F5)SendMessageW(hwnd,WM_COMMAND,SAVE,0);
        if(wp==VK_F9)SendMessageW(hwnd,WM_COMMAND,LOAD,0);if(wp==VK_F2)SendMessageW(hwnd,WM_COMMAND,SETTINGS,0);if(wp==VK_F11)SendMessageW(hwnd,WM_COMMAND,FULLSCREEN,0);return 0;
    case WM_KILLFOCUS:if(running && !preferences_active() && !IsChild(hwnd,(HWND)wp))set_paused(1);return 0;
    case WM_GETMINMAXINFO:{MINMAXINFO *size=(MINMAXINFO*)lp;size->ptMinTrackSize.x=1024;size->ptMinTrackSize.y=640;return 0;}
    case WM_CLOSE:quitting=1;return 0;
    case WM_ERASEBKGND:return 1;
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);RECT r;GetClientRect(hwnd,&r);paint(dc,r);EndPaint(hwnd,&ps);return 0;}
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
static void pump(void){
    MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){
        if(msg.message==WM_QUIT){quitting=1;return;}if(preferences_route(&msg))continue;TranslateMessage(&msg);DispatchMessageW(&msg);
    }
    input_update();static uint16_t prior;uint16_t now=GetForegroundWindow()==window?input_read().menu:0;
    uint16_t pressed=now&~prior;
    if(!preferences_active() && !running && (pressed&INPUT_START) && rom_ready)SendMessageW(window,WM_COMMAND,PLAY,0);
    if(!preferences_active() && running && (pressed&INPUT_BACK)){set_paused(!paused);prior=now;return;}
    if((!running || paused) && !preferences_active()){
        int index=rom_ready?0:4;for(int i=0;i<MENU_COUNT;i++)if(GetFocus()==controls[i])index=i;
        if(pressed&(INPUT_UP|INPUT_DOWN)){int delta=pressed&INPUT_UP?MENU_COUNT-1:1;for(int tries=0;tries<MENU_COUNT;tries++){index=(index+delta)%MENU_COUNT;if(IsWindowEnabled(controls[index]))break;}SetFocus(controls[index]);}
        if((pressed&INPUT_A)&&IsWindowEnabled(controls[index]))SendMessageW(window,WM_COMMAND,GetDlgCtrlID(controls[index]),0);
        if((pressed&(INPUT_B|INPUT_BACK))&&running)set_paused(0);
    }prior=now;
}
static int frame(const uint32_t *fb,int w,int h){
    (void)w;(void)h;last_frame=glue_frame_count();
    if(movie.target && last_frame<movie.target){
        if(last_frame%120==0){pump();wchar_t title[160];swprintf(title,160,L"Reconstruindo save: %llu / %llu quadros",last_frame,movie.target);SetWindowTextW(window,title);}
        if(quitting || restart_requested)return 1;glue_set_pad1(movie.inputs[last_frame-1]);return 0;
    }
    memcpy(framebuffer,fb,sizeof framebuffer);
    if(movie.target){
        if(glue_state_hash()!=movie.state_hash){message(L"A reconstrução divergiu do save. O carregamento foi interrompido; o arquivo foi preservado.");quitting=1;return 1;}
        movie.target=0;audio_reset();set_paused(1);SetWindowTextW(window,L"Mônica no Castelo do Dragão");
    }
    InvalidateRect(window,NULL,FALSE);UpdateWindow(window);pump();
    while(paused && !quitting && !restart_requested){pump();Sleep(10);}
    if(quitting || restart_requested)return 1;
    if(!replay_append(&movie,input_mask())){message(L"Limite de gravação da sessão atingido. A execução será encerrada.");return 1;}
    glue_set_pad1(movie.inputs[movie.count-1]);
    LARGE_INTEGER now;QueryPerformanceCounter(&now);deadline.QuadPart+=(LONGLONG)(frequency.QuadPart/glue_frame_rate());
    if(now.QuadPart-deadline.QuadPart>frequency.QuadPart/4)deadline=now;
    while(now.QuadPart<deadline.QuadPart){Sleep(1);QueryPerformanceCounter(&now);}return 0;
}
static int restart_process(void){
    wchar_t exe[32768];GetModuleFileNameW(NULL,exe,32768);size_t capacity=wcslen(exe)+wcslen(rom_path)+wcslen(pending_load)+wcslen(root)+100;
    wchar_t *cmd=calloc(capacity,sizeof(wchar_t));if(!cmd)return 0;swprintf(cmd,capacity,L"\"%ls\" --rom \"%ls\" --load \"%ls\" --data-dir \"%ls\"",exe,rom_path,pending_load,root);
    STARTUPINFOW si={sizeof si};PROCESS_INFORMATION pi;int ok=CreateProcessW(exe,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
    free(cmd);if(ok){CloseHandle(pi.hProcess);CloseHandle(pi.hThread);}return ok;
}
static void test_navigation(void){
    int isolated=1,restored=1,embedded=1,pages=0;
    for(int cycle=0;cycle<4;cycle++){
        running=cycle&1;paused=running;settings(cycle%4);
        HWND page=FindWindowExW(window,NULL,L"MonicaPreferences",NULL);
        if(!page){isolated=embedded=0;break;}
        embedded &= !!(GetWindowLongW(page,GWL_STYLE)&WS_CHILD);
        embedded &= !(GetWindowLongW(page,GWL_STYLE)&WS_CAPTION);
        for(int tab=0;tab<6;tab++){
            SendMessageW(page,WM_COMMAND,600+tab,0);pages++;
            SendMessageW(window,WM_KILLFOCUS,(WPARAM)page,0);
            navigation();SendMessageW(window,WM_SIZE,0,0);
            for(int i=0;i<MENU_COUNT;i++)isolated &= !(GetWindowLongW(controls[i],GWL_STYLE)&WS_VISIBLE);
        }
        SendMessageW(page,WM_COMMAND,2,0);
        DWORD until=GetTickCount()+1000;
        while(preferences_active() && (LONG)(until-GetTickCount())>0){pump();Sleep(2);}
        restored &= !preferences_active();
        for(int i=0;i<MENU_COUNT;i++)restored &= !!(GetWindowLongW(controls[i],GWL_STYLE)&WS_VISIBLE);
    }
    FILE *out=_wfopen(navigation_test_path,L"w");if(out){fprintf(out,"{\"menu_isolated\":%d,\"menu_restored\":%d,\"embedded_pages\":%d,\"page_switches\":%d,\"no_menu_bar\":%d}\n",isolated,restored,embedded,pages,GetMenu(window)==NULL);fclose(out);}
    running=paused=0;quitting=1;
}
static void test_audio(void){
    FILE *out=_wfopen(audio_test_path,L"w");int rates[]={44100,48000,96000};int16_t silence[4096]={0};
    muted=0;paused=0;cfg.latency=1;
    for(int test=0;test<3;test++){
        cfg.sample_rate=rates[test];audio_open();int opened=audio_device!=NULL;
        if(opened){for(int f=0;f<100 && !audio_fault;f++){audio_sink(silence,2048);audio_sink(silence,1685);}if(audio_fill&&!audio_fault)audio_submit();}
        uint64_t expected=(uint64_t)3733*100*rates[test]/glue_audio_sample_rate();
        int pass=opened&&!audio_fault&&audio_output_frames==expected&&audio_submitted_frames==expected;
        if(out)fprintf(out,"rate=%d output=%llu submitted=%llu expected=%llu waits=%llu pass=%d\n",rates[test],audio_output_frames,audio_submitted_frames,expected,audio_waits,pass);
        if(audio_device){waveOutRestart(audio_device);DWORD start=GetTickCount();for(;;){int queued=0;for(int i=0;i<8;i++)queued|=!!(audio_headers[i].dwFlags&WHDR_INQUEUE);if(!queued||GetTickCount()-start>3000)break;WaitForSingleObject(audio_event,10);}}
        audio_close();
    }
    if(out)fclose(out);quitting=1;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show){
    (void)previous;(void)command;wchar_t data[32768];if(!GetEnvironmentVariableW(L"LOCALAPPDATA",data,32768))GetTempPathW(32768,data);
    swprintf(root,32768,L"%ls\\MonicaRecompPrototype",data);
    int argc;wchar_t **argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    for(int i=1;i<argc;i++)if(!wcscmp(argv[i],L"--data-dir")&&i+1<argc)wcsncpy(root,argv[++i],32767);
    CreateDirectoryW(root,NULL);swprintf(ini,32768,L"%ls\\settings.ini",root);
    GetPrivateProfileStringW(L"app",L"rom",L"",rom_path,32768,ini);
    preferences_load(&cfg,ini);
    for(int i=1;i<argc;i++){
        if(!wcscmp(argv[i],L"--rom")&&i+1<argc)wcsncpy(rom_path,argv[++i],32767);
        else if(!wcscmp(argv[i],L"--load")&&i+1<argc){wcsncpy(pending_load,argv[++i],32767);start_requested=1;}
        else if(!wcscmp(argv[i],L"--selftest-ui")&&i+1<argc)wcsncpy(ui_test_path,argv[++i],32767);
        else if(!wcscmp(argv[i],L"--selftest-navigation")&&i+1<argc)wcsncpy(navigation_test_path,argv[++i],32767);
        else if(!wcscmp(argv[i],L"--selftest-audio")&&i+1<argc)wcsncpy(audio_test_path,argv[++i],32767);
    }LocalFree(argv);rom_ready=validate_rom(rom_path);QueryPerformanceFrequency(&frequency);
    background=CreateSolidBrush(RGB(12,31,26));font=CreateFontW(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");title_font=CreateFontW(-36,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    WNDCLASSW wc={0};wc.lpfnWndProc=window_proc;wc.hInstance=instance;wc.lpszClassName=L"MonicaRecomp";wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(101));RegisterClassW(&wc);
    
    HMENU menu=NULL;
    int widths[]={960,1280,1600,1920,2560,3840};int an=cfg.aspect==1?5:cfg.aspect==2?16:4,ad=cfg.aspect==1?4:cfg.aspect==2?9:3;RECT wr={0,0,widths[resolution],MulDiv(widths[resolution],ad,an)};AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,FALSE);
    window=CreateWindowW(L"MonicaRecomp",L"Mônica no Castelo do Dragão",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,wr.right-wr.left,wr.bottom-wr.top,0,menu,instance,0);
    const wchar_t *names[]={L"Jogar",L"Continuar",L"Controles",L"Configurações",L"Selecionar ROM",L"Sobre o jogo",L"Sair"};int ids[]={PLAY,LOAD,SETTINGS,OPTIONS,OPEN_ROM,ABOUT,EXIT_APP};
    for(int i=0;i<MENU_COUNT;i++){
        controls[i]=CreateWindowW(L"BUTTON",names[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,0,0,10,10,window,(HMENU)(INT_PTR)ids[i],instance,0);
        WNDPROC old=(WNDPROC)SetWindowLongPtrW(controls[i],GWLP_WNDPROC,(LONG_PTR)menu_button_proc);if(!original_button_proc)original_button_proc=old;
    }
    wchar_t art_path[32768];GetModuleFileNameW(NULL,art_path,32768);wchar_t *last=wcsrchr(art_path,L'\\');if(last)last[1]=0;
    wcscpy(asset_dir,art_path);wcscat(asset_dir,L"assets");wcscat(art_path,L"assets\\menu-forest.png");theme_init(art_path);navigation();SetTimer(window,1,16,NULL);
    input_init();input_select(cfg.device);input_configure(cfg.buttons);input_set_deadzone(cfg.deadzone);input_update();
    SetCurrentDirectoryW(root);wchar_t log[32768];swprintf(log,32768,L"%ls\\runtime.log",root);_wfreopen(log,L"w",stderr);
    if(audio_test_path[0]){test_audio();}
    else if(navigation_test_path[0]){test_navigation();}
    else if(ui_test_path[0]){
        FILE *test=_wfopen(ui_test_path,L"w");if(test){fprintf(test,"{\"rom_valid\":%d,\"play_enabled\":%d,\"continue_enabled\":%d,\"select_enabled\":%d,\"about_enabled\":%d}\n",rom_ready,!!IsWindowEnabled(controls[0]),!!IsWindowEnabled(controls[1]),!!IsWindowEnabled(controls[4]),!!IsWindowEnabled(controls[5]));fclose(test);}quitting=1;
    }else{ShowWindow(window,show);UpdateWindow(window);if(cfg.fullscreen)toggle_fullscreen();SetFocus(controls[rom_ready?0:4]);}
    while(!quitting){
        pump();if(start_requested){
            if(session_ended){start_requested=0;if(restart_process())quitting=1;else message(L"Não foi possível iniciar uma nova sessão.");continue;}
            start_requested=0;if(!validate_rom(rom_path)){message(L"Escolha a ROM correta para iniciar.");continue;}
            if(pending_load[0]&&!replay_load(pending_load,&movie)){message(L"Save inválido ou incompatível.");pending_load[0]=0;continue;}
            if(!glue_load_rom_wide(rom_path)){message(L"Falha ao ler a ROM.");continue;}
            for(int i=0;i<MENU_COUNT;i++)ShowWindow(controls[i],SW_HIDE);EnableMenuItem(menu,OPEN_ROM,MF_BYCOMMAND|MF_GRAYED);EnableMenuItem(menu,PLAY,MF_BYCOMMAND|MF_GRAYED);
            SetFocus(window);running=1;paused=0;glue_init(false,0);glue_require_native(true);glue_set_audio_sink(audio_sink);glue_set_frame_callback(frame);audio_open();QueryPerformanceCounter(&deadline);glue_run_step();audio_close();running=0;
            if(restart_requested){if(restart_process())quitting=1;else{message(L"Não foi possível reiniciar para carregar o save.");restart_requested=0;}}
            else if(!quitting){session_ended=1;paused=0;navigation();InvalidateRect(window,NULL,FALSE);message(glue_dispatch_miss_count()?L"Este trecho ainda não foi traduzido. A partida foi interrompida e os detalhes foram registrados em runtime.log. Seus saves existentes foram preservados.":L"A partida terminou. Você pode iniciar uma nova sessão pelo menu.");}
        }Sleep(10);
    }
    audio_close();replay_free(&movie);KillTimer(window,1);DestroyWindow(window);theme_close();if(present_dc){SelectObject(present_dc,present_old);DeleteObject(present_bitmap);DeleteDC(present_dc);}DeleteObject(font);DeleteObject(title_font);DeleteObject(background);input_close();return 0;
}

