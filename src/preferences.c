#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include "preferences.h"
#include "choice.h"
#include <commctrl.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <wincodec.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uxtheme.h>

static HWND nav[6];static DWORD page_started,screen_started;static int closing;
static HWND dialog,owner,tabs,items[96],key_buttons[6],pad_combos[8],device_combo,deadzone_combo;
static HWND video_controls[8],audio_controls[6];
static Preferences pending,original,*target;
static InputDevice devices[32];static int device_count,item_count,current_tab,capture_key=-1,capture_pad=-1,release_pad;
static HBRUSH paper;static HFONT normal,heading,caption,widget_font;static wchar_t asset_directory[32768];
typedef struct{uint32_t *pixels;UINT w,h;BITMAPINFO info;} Picture;
static Picture cover,advert,forest;
static wchar_t feedback[200];
static const wchar_t *actions[]={L"Cima",L"Baixo",L"Esquerda",L"Direita",L"Botão 1",L"Botão 2"};
static const wchar_t *pad_actions[]={L"Botão 1",L"Botão 2",L"Pausa",L"Voltar no menu",L"Cima",L"Baixo",L"Esquerda",L"Direita"};
static int clamp(int v,int low,int high){return v<low?low:v>high?high:v;}
static void read_int(const wchar_t *section,const wchar_t *name,int *v,int low,int high,const wchar_t *path){*v=clamp(GetPrivateProfileIntW(section,name,*v,path),low,high);}
static void write_int(const wchar_t *s,const wchar_t *k,int v,const wchar_t *path){wchar_t value[32];swprintf(value,32,L"%d",v);WritePrivateProfileStringW(s,k,value,path);}
void preferences_defaults(Preferences *p){
    memset(p,0,sizeof *p);int initial[]={VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,'Z','X'};memcpy(p->keys,initial,sizeof initial);
    for(int i=0;i<8;i++)p->buttons[i]=-1;p->deadzone=25;p->resolution=1;p->volume=100;p->sample_rate=48000;p->brightness=100;p->latency=1;
}
void preferences_load(Preferences *p,const wchar_t *path){
    preferences_defaults(p);wchar_t key[32],device[512];
    for(int i=0;i<6;i++){swprintf(key,32,L"key%d",i);read_int(L"input",key,&p->keys[i],1,254,path);}
    for(int i=0;i<8;i++){swprintf(key,32,L"button%d",i);read_int(L"input",key,&p->buttons[i],-1,255,path);}
    GetPrivateProfileStringW(L"input",L"device",L"",device,512,path);WideCharToMultiByte(CP_UTF8,0,device,-1,p->device,512,NULL,NULL);
    read_int(L"input",L"deadzone",&p->deadzone,5,80,path);
    read_int(L"video",L"resolution",&p->resolution,0,5,path);read_int(L"video",L"aspect",&p->aspect,0,2,path);
    read_int(L"video",L"scale",&p->scale,0,2,path);read_int(L"video",L"filter",&p->filter,0,1,path);
    read_int(L"video",L"scanlines",&p->scanlines,0,2,path);read_int(L"video",L"brightness",&p->brightness,50,150,path);
    read_int(L"video",L"fullscreen",&p->fullscreen,0,1,path);
    read_int(L"audio",L"volume",&p->volume,0,100,path);read_int(L"audio",L"muted",&p->muted,0,1,path);
    read_int(L"audio",L"sample_rate",&p->sample_rate,44100,96000,path);if(p->sample_rate!=44100&&p->sample_rate!=48000&&p->sample_rate!=96000)p->sample_rate=48000;
    read_int(L"audio",L"mono",&p->mono,0,1,path);read_int(L"audio",L"latency",&p->latency,0,2,path);
    GetPrivateProfileStringW(L"audio",L"device",L"",p->audio_device,128,path);
}
void preferences_save(const Preferences *p,const wchar_t *path){
    wchar_t k[32],device[512];
    for(int i=0;i<6;i++){swprintf(k,32,L"key%d",i);write_int(L"input",k,p->keys[i],path);}
    for(int i=0;i<8;i++){swprintf(k,32,L"button%d",i);write_int(L"input",k,p->buttons[i],path);}
    MultiByteToWideChar(CP_UTF8,0,p->device,-1,device,512);WritePrivateProfileStringW(L"input",L"device",device,path);
    write_int(L"input",L"deadzone",p->deadzone,path);
    #define WIDEN_INNER(X) L##X
    #define WIDEN(X) WIDEN_INNER(X)
    #define STORE(S,F) write_int(WIDEN(S),WIDEN(#F),p->F,path)
    STORE("video",resolution);STORE("video",aspect);STORE("video",scale);STORE("video",filter);STORE("video",scanlines);STORE("video",brightness);STORE("video",fullscreen);
    STORE("audio",volume);STORE("audio",muted);STORE("audio",sample_rate);STORE("audio",mono);STORE("audio",latency);
    #undef STORE
    #undef WIDEN
    #undef WIDEN_INNER
    WritePrivateProfileStringW(L"audio",L"device",p->audio_device,path);
}
RECT video_destination(int w,int h,int aspect,int scale,RECT *source){
    RECT dst={0,0,0,0};*source=(RECT){0,0,256,192};if(w<=0||h<=0)return dst;
    int an=aspect==1?5:aspect==2?16:4,ad=aspect==1?4:aspect==2?9:3;
    int cw=w,ch=MulDiv(w,ad,an);if(ch>h){ch=h;cw=MulDiv(h,an,ad);}int ox=(w-cw)/2,oy=(h-ch)/2;
    int dw=cw,dh=MulDiv(cw,3,4);
    if(scale==2){
        if((int64_t)cw*192>(int64_t)ch*256){int crop=MulDiv(256,ch,cw);source->top=(192-crop)/2;source->bottom=source->top+crop;}
        else{int crop=MulDiv(192,cw,ch);source->left=(256-crop)/2;source->right=source->left+crop;}dh=ch;
    }else if(scale==1 && cw>=256 && ch>=192){int k=cw/256;if(ch/192<k)k=ch/192;dw=256*k;dh=192*k;}
    else if(dh>ch){dh=ch;dw=MulDiv(ch,4,3);}
    dst=(RECT){ox+(cw-dw)/2,oy+(ch-dh)/2,ox+(cw+dw)/2,oy+(ch+dh)/2};return dst;
}
typedef struct {int x,y,w,h,page;} Layout;
static double ui_scale;static int ui_x,ui_y;
static LRESULT CALLBACK hover_button(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data){
    (void)id;(void)data;
    if(msg==WM_MOUSEMOVE){if(!GetPropW(hwnd,L"hover")){SetPropW(hwnd,L"hover",(HANDLE)1);InvalidateRect(hwnd,NULL,FALSE);}TRACKMOUSEEVENT t={sizeof t,TME_LEAVE,hwnd,0};TrackMouseEvent(&t);}
    if(msg==WM_MOUSELEAVE){RemovePropW(hwnd,L"hover");InvalidateRect(hwnd,NULL,FALSE);}
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_NCDESTROY){RemovePropW(hwnd,L"hover");RemoveWindowSubclass(hwnd,hover_button,1);}
    return DefSubclassProc(hwnd,msg,wp,lp);
}
static void position_child(HWND item){Layout *l=(Layout*)GetPropW(item,L"layout");if(!l)return;
    double t=(GetTickCount()-page_started)/220.0;if(t>1)t=1;int offset=l->page?(int)(22*(1-t)*(1-t)):0;
    SetWindowPos(item,NULL,ui_x+(int)((l->x+offset)*ui_scale),ui_y+(int)(l->y*ui_scale),(int)(l->w*ui_scale),(int)(l->h*ui_scale),SWP_NOZORDER|SWP_NOACTIVATE);
}
static HWND child(const wchar_t *type,const wchar_t *text,DWORD style,int x,int y,int w,int h,int id,int page){
    if(!wcscmp(type,L"BUTTON"))style=(style&~15)|BS_OWNERDRAW;
    if(!wcscmp(type,L"COMBOBOX")){type=L"MonicaChoice";style=WS_TABSTOP;}
    HWND item=CreateWindowW(type,text,WS_CHILD|WS_VISIBLE|style,x,y,w,h,dialog,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
    if(!wcscmp(type,L"BUTTON"))SetWindowSubclass(item,hover_button,1,0);
    Layout *l=malloc(sizeof *l);*l=(Layout){x,y,w,h,page};SetPropW(item,L"layout",l);SendMessageW(item,WM_SETFONT,(WPARAM)widget_font,FALSE);SetWindowTheme(item,L"",L"");position_child(item);return item;
}
static void discard(HWND item){free((void*)RemovePropW(item,L"layout"));DestroyWindow(item);}
static HWND widget(const wchar_t *type,const wchar_t *text,DWORD style,int x,int y,int w,int h,int id){
    HWND item=child(type,text,style,x,y,w,h,id,1);if(item_count<96)items[item_count++]=item;return item;
}
void preferences_resize(void){if(!dialog)return;RECT r;GetClientRect(owner,&r);SetWindowPos(dialog,HWND_TOP,0,0,r.right,r.bottom,SWP_NOACTIVATE);
    ui_scale=(double)r.right/1060;if((double)r.bottom/700<ui_scale)ui_scale=(double)r.bottom/700;ui_x=(r.right-(int)(1060*ui_scale))/2;ui_y=(r.bottom-(int)(700*ui_scale))/2;
    HFONT previous=widget_font;widget_font=CreateFontW(-(int)(18*ui_scale),0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    for(HWND c=GetWindow(dialog,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT)){position_child(c);SendMessageW(c,WM_SETFONT,(WPARAM)widget_font,FALSE);}DeleteObject(previous);InvalidateRect(dialog,NULL,FALSE);
}
static void label(const wchar_t *s,int x,int y,int w){widget(L"STATIC",s,0,x,y,w,28,0);}
static HWND combo(int x,int y,int w,int id){return widget(L"COMBOBOX",L"",WS_TABSTOP,x,y,w,32,id);}
static void option(HWND c,const wchar_t *s){SendMessageW(c,CB_ADDSTRING,0,(LPARAM)s);}
static int selection(HWND c){return (int)SendMessageW(c,CB_GETCURSEL,0,0);}
static void select_at(HWND c,int i){SendMessageW(c,CB_SETCURSEL,i,0);}
static void key_name(int key,wchar_t out[80]){UINT scan=MapVirtualKeyW(key,MAPVK_VK_TO_VSC);if(key>=VK_PRIOR&&key<=VK_DELETE)scan|=0x100;GetKeyNameTextW((LONG)(scan<<16),out,80);if(!out[0])swprintf(out,80,L"Tecla %d",key);}
static void binding_label(int action,wchar_t out[80]){int b=pending.buttons[action];if(b<0)input_default_label(action,out,80);else swprintf(out,80,L"Botão físico %d",b+1);}
static void rebuild_devices(void){
    device_count=input_devices(devices,32);SendMessageW(device_combo,CB_RESETCONTENT,0,0);option(device_combo,L"Automático — primeiro disponível");int selected=0;
    for(int i=0;i<device_count;i++){option(device_combo,devices[i].name);if(!strcmp(pending.device,devices[i].key))selected=i+1;}select_at(device_combo,selected);
}
static void read_controls(void){
    if(current_tab==0){for(int i=0;i<8;i++){int v=selection(pad_combos[i]);if(v>=0)pending.buttons[i]=v-1;}int d=selection(deadzone_combo);if(d>=0)pending.deadzone=10+d*5;}
    if(current_tab==1){pending.resolution=selection(video_controls[0]);pending.aspect=selection(video_controls[1]);pending.fullscreen=selection(video_controls[2]);pending.scale=selection(video_controls[3]);pending.filter=selection(video_controls[4]);pending.scanlines=selection(video_controls[5]);pending.brightness=50+selection(video_controls[6])*10;}
    if(current_tab==2){int id=selection(audio_controls[0]);if(id<=0)pending.audio_device[0]=0;else SendMessageW(audio_controls[0],CB_GETLBTEXT,id,(LPARAM)pending.audio_device);
        pending.volume=selection(audio_controls[1])*5;/* mute is kept in pending by the custom checkbox */
        int rates[]={44100,48000,96000};int r=selection(audio_controls[3]);if(r>=0)pending.sample_rate=rates[r];pending.mono=selection(audio_controls[4]);pending.latency=selection(audio_controls[5]);}
}
static void build_page(int page){
    capture_key=capture_pad=-1;for(int i=0;i<item_count;i++)discard(items[i]);item_count=0;current_tab=page;page_started=GetTickCount();
    if(page==0){
        label(L"Dispositivo USB / Bluetooth",32,84,450);device_combo=combo(32,114,475,10);widget(L"BUTTON",L"Atualizar",WS_TABSTOP,520,114,115,30,11);rebuild_devices();
        label(L"TECLADO",32,196,300);
        for(int i=0;i<6;i++){wchar_t name[80];key_name(pending.keys[i],name);label(actions[i],32,235+i*43,160);key_buttons[i]=widget(L"BUTTON",name,WS_TABSTOP,195,230+i*43,220,33,100+i);}
        label(L"Zona morta do analógico",32,518,340);deadzone_combo=combo(32,549,190,12);for(int i=0;i<13;i++){wchar_t p[20];swprintf(p,20,L"%d%%",10+i*5);option(deadzone_combo,p);}select_at(deadzone_combo,clamp((pending.deadzone-10)/5,0,12));
        label(L"BOTÕES DO CONTROLE SELECIONADO",600,304,390);
        int n=input_button_count();if(n<32)n=32;if(n>128)n=128;
        for(int i=0;i<8;i++){
            label(pad_actions[i],600,342+i*32,135);pad_combos[i]=combo(738,337+i*32,170,200+i);wchar_t name[80];input_default_label(i,name,80);option(pad_combos[i],name);
            for(int b=0;b<n;b++){wchar_t text[32];swprintf(text,32,L"Botão físico %d",b+1);option(pad_combos[i],text);}select_at(pad_combos[i],clamp(pending.buttons[i]+1,0,n));
            widget(L"BUTTON",L"Detectar",WS_TABSTOP,915,337+i*32,95,27,300+i);
        }
    }else if(page==1){
        const wchar_t *labels[]={L"Resolução da janela",L"Proporção do quadro",L"Modo de tela",L"Enquadramento da imagem",L"Filtro de imagem",L"Linhas de varredura",L"Brilho"};
        for(int i=0;i<7;i++){label(labels[i],32,98+i*59,310);video_controls[i]=combo(360,92+i*59,610,20+i);}
        const wchar_t *res[]={L"960 pixels de largura",L"1280 pixels de largura",L"1600 pixels de largura",L"1920 pixels de largura",L"2560 pixels de largura",L"3840 pixels de largura"};for(int i=0;i<6;i++)option(video_controls[0],res[i]);
        option(video_controls[1],L"4:3 — original");option(video_controls[1],L"5:4");option(video_controls[1],L"16:9");
        option(video_controls[2],L"Janela");option(video_controls[2],L"Tela cheia sem bordas");
        option(video_controls[3],L"Preservar proporções — com barras quando necessário");option(video_controls[3],L"Pixels inteiros — sem deformar");option(video_controls[3],L"Preencher o quadro — corta bordas, sem esticar");
        option(video_controls[4],L"Pixels nítidos");option(video_controls[4],L"Suavizado");
        option(video_controls[5],L"Desativadas");option(video_controls[5],L"Suaves");option(video_controls[5],L"Fortes");
        for(int i=50;i<=150;i+=10){wchar_t b[20];swprintf(b,20,L"%d%%",i);option(video_controls[6],b);}
        int choices[]={pending.resolution,pending.aspect,pending.fullscreen,pending.scale,pending.filter,pending.scanlines,(pending.brightness-50)/10};for(int i=0;i<7;i++)select_at(video_controls[i],choices[i]);
        label(L"16:9 preserva os pixels; ampliar a área real do cenário ainda está em desenvolvimento.",32,535,940);
    }else if(page==2){
        const wchar_t *labels[]={L"Dispositivo de saída",L"Volume do aplicativo",L"",L"Frequência de saída",L"Mixagem",L"Latência"};
        for(int i=0;i<6;i++)if(i!=2){label(labels[i],32,98+i*67,310);audio_controls[i]=combo(360,92+i*67,610,40+i);}
        option(audio_controls[0],L"Padrão do Windows");int device=0;for(UINT i=0;i<waveOutGetNumDevs();i++){WAVEOUTCAPSW caps;if(waveOutGetDevCapsW(i,&caps,sizeof caps)==MMSYSERR_NOERROR){option(audio_controls[0],caps.szPname);if(!wcscmp(caps.szPname,pending.audio_device))device=(int)SendMessageW(audio_controls[0],CB_GETCOUNT,0,0)-1;}}select_at(audio_controls[0],device);
        for(int i=0;i<=100;i+=5){wchar_t v[20];swprintf(v,20,L"%d%%",i);option(audio_controls[1],v);}select_at(audio_controls[1],pending.volume/5);
        audio_controls[2]=widget(L"BUTTON",L"Silenciar áudio",BS_AUTOCHECKBOX|WS_TABSTOP,360,229,500,32,42);SendMessageW(audio_controls[2],BM_SETCHECK,pending.muted?BST_CHECKED:BST_UNCHECKED,0);
        option(audio_controls[3],L"44.100 Hz");option(audio_controls[3],L"48.000 Hz");option(audio_controls[3],L"96.000 Hz");select_at(audio_controls[3],pending.sample_rate==44100?0:pending.sample_rate==96000?2:1);
        option(audio_controls[4],L"Estéreo");option(audio_controls[4],L"Mono");select_at(audio_controls[4],pending.mono);
        option(audio_controls[5],L"Baixa");option(audio_controls[5],L"Equilibrada");option(audio_controls[5],L"Mais estável");select_at(audio_controls[5],pending.latency);
        label(L"O áudio original deste jogo usa o PSG do Master System. A seleção não muda a velocidade do jogo.",32,535,940);
    }else if(page==3){
        widget(L"BUTTON",L"Ver fonte histórica",WS_TABSTOP,32,557,220,36,50);widget(L"BUTTON",L"Acervo SMS Power!",WS_TABSTOP,264,557,220,36,51);
    }
    for(int i=0;i<6;i++)InvalidateRect(nav[i],NULL,FALSE);
    InvalidateRect(dialog,NULL,FALSE);
}
static void draw_text(HDC dc,int x,int y,int w,int h,const wchar_t *s,HFONT f,COLORREF color){RECT r={x,y,x+w,y+h};SelectObject(dc,f);SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);DrawTextW(dc,s,-1,&r,DT_LEFT|DT_WORDBREAK);}
static void rounded(HDC dc,int x,int y,int w,int h,COLORREF fill,COLORREF stroke){HBRUSH b=CreateSolidBrush(fill);HPEN p=CreatePen(PS_SOLID,2,stroke);HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,p);RoundRect(dc,x,y,x+w,y+h,16,16);SelectObject(dc,op);SelectObject(dc,ob);DeleteObject(p);DeleteObject(b);}
static void draw_pad(HDC dc){
    InputState state=input_read();
    rounded(dc,660,112,310,166,RGB(12,13,15),RGB(91,94,97));rounded(dc,668,121,294,148,RGB(30,32,34),RGB(47,50,52));
    draw_text(dc,798,133,140,30,L"CONTROL PAD",caption,RGB(157,167,177));
    rounded(dc,683,147,100,100,RGB(59,62,66),RGB(14,15,17));
    RECT v={718,157,748,237},h={693,187,773,217};HBRUSH b=CreateSolidBrush(RGB(8,9,11));FillRect(dc,&v,b);FillRect(dc,&h,b);DeleteObject(b);
    rounded(dc,811,184,58,54,state.pad&INPUT_A?RGB(160,31,44):RGB(7,8,10),RGB(77,80,84));rounded(dc,886,184,58,54,state.pad&INPUT_B?RGB(160,31,44):RGB(7,8,10),RGB(77,80,84));
    draw_text(dc,830,151,50,26,L"1",caption,RGB(243,243,242));draw_text(dc,905,151,50,26,L"2",caption,RGB(243,243,242));
    wchar_t one[80],two[80],summary[220];binding_label(0,one);binding_label(1,two);swprintf(summary,220,L"1: %ls   ·   2: %ls",one,two);
    draw_text(dc,600,279,420,24,summary,caption,RGB(205,224,207));
    draw_text(dc,32,153,600,36,input_name(),caption,RGB(180,217,159));
}
static void picture_load(Picture *p,const wchar_t *name){
    wchar_t path[32768];swprintf(path,32768,L"%ls\\%ls",asset_directory,name);IWICImagingFactory *factory=NULL;IWICBitmapDecoder *dec=NULL;IWICBitmapFrameDecode *frame=NULL;IWICFormatConverter *convert=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,&IID_IWICImagingFactory,(void**)&factory);
    if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateDecoderFromFilename(factory,path,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&dec);
    if(SUCCEEDED(hr))hr=IWICBitmapDecoder_GetFrame(dec,0,&frame);if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateFormatConverter(factory,&convert);
    if(SUCCEEDED(hr))hr=IWICFormatConverter_Initialize(convert,(IWICBitmapSource*)frame,&GUID_WICPixelFormat32bppBGR,WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom);
    if(SUCCEEDED(hr))hr=IWICFormatConverter_GetSize(convert,&p->w,&p->h);
    if(SUCCEEDED(hr)&&p->w<=4096&&p->h<=4096){p->pixels=malloc((size_t)p->w*p->h*4);if(p->pixels)IWICFormatConverter_CopyPixels(convert,NULL,p->w*4,p->w*p->h*4,(BYTE*)p->pixels);}
    if(convert)IWICFormatConverter_Release(convert);if(frame)IWICBitmapFrameDecode_Release(frame);if(dec)IWICBitmapDecoder_Release(dec);if(factory)IWICImagingFactory_Release(factory);
    p->info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);p->info.bmiHeader.biWidth=p->w;p->info.bmiHeader.biHeight=-(LONG)p->h;p->info.bmiHeader.biPlanes=1;p->info.bmiHeader.biBitCount=32;
}
static void picture_draw(HDC dc,Picture *p,int x,int y,int w,int h){if(!p->pixels)return;double k=(double)w/p->w;if(p->h*k>h)k=(double)h/p->h;SetStretchBltMode(dc,HALFTONE);StretchDIBits(dc,x,y,(int)(p->w*k),(int)(p->h*k),0,0,p->w,p->h,p->pixels,&p->info,DIB_RGB_COLORS,SRCCOPY);}
static LRESULT CALLBACK procedure(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORBTN||msg==WM_CTLCOLORLISTBOX||msg==WM_CTLCOLOREDIT){SetTextColor((HDC)wp,RGB(249,236,200));SetBkColor((HDC)wp,RGB(12,31,26));return (LRESULT)paper;}
    if(msg==WM_MEASUREITEM){((MEASUREITEMSTRUCT*)lp)->itemHeight=28;return TRUE;}
    if(msg==WM_DRAWITEM){DRAWITEMSTRUCT *d=(DRAWITEMSTRUCT*)lp;RECT r={0,0,d->rcItem.right-d->rcItem.left,d->rcItem.bottom-d->rcItem.top};HDC dc=CreateCompatibleDC(d->hDC);HBITMAP bitmap=CreateCompatibleBitmap(d->hDC,r.right,r.bottom);HGDIOBJ old=SelectObject(dc,bitmap);
        int active=(d->itemState&(ODS_SELECTED|ODS_FOCUS))!=0;int hover=GetPropW(d->hwndItem,L"hover")!=NULL;int tab=(int)d->CtlID-600;if(tab>=0&&tab<6)active=tab==current_tab;
        HBRUSH brush=CreateSolidBrush(active?RGB(146,34,43):hover?RGB(37,67,49):RGB(22,51,42));FillRect(dc,&r,brush);DeleteObject(brush);
        HPEN pen=CreatePen(PS_SOLID,1,active?RGB(255,205,91):RGB(72,101,77));HGDIOBJ op=SelectObject(dc,pen),ob=SelectObject(dc,GetStockObject(NULL_BRUSH));RoundRect(dc,0,0,r.right-1,r.bottom-1,8,8);SelectObject(dc,op);SelectObject(dc,ob);DeleteObject(pen);
        wchar_t text[512]=L"";if(d->CtlType==ODT_COMBOBOX){if(d->itemID!=(UINT)-1)SendMessageW(d->hwndItem,CB_GETLBTEXT,d->itemID,(LPARAM)text);}else GetWindowTextW(d->hwndItem,text,512);
        if(d->CtlID==42)swprintf(text,512,pending.muted?L"[✓]  Áudio silenciado":L"[  ]  Silenciar áudio");
        SelectObject(dc,widget_font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,active?RGB(255,221,145):RGB(249,236,207));r.left+=12;r.right-=10;DrawTextW(dc,text,-1,&r,DT_SINGLELINE|DT_VCENTER|(d->CtlType==ODT_BUTTON?DT_CENTER:DT_LEFT));
        BitBlt(d->hDC,d->rcItem.left,d->rcItem.top,d->rcItem.right-d->rcItem.left,d->rcItem.bottom-d->rcItem.top,dc,0,0,SRCCOPY);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);return TRUE;
    }
    if(msg==WM_PAINT||msg==WM_PRINTCLIENT){PAINTSTRUCT ps;HDC screen=msg==WM_PAINT?BeginPaint(hwnd,&ps):(HDC)wp;RECT r;GetClientRect(hwnd,&r);HDC dc=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,r.right,r.bottom);HGDIOBJ old=SelectObject(dc,bitmap);FillRect(dc,&r,paper);
        SetMapMode(dc,MM_ANISOTROPIC);SetWindowExtEx(dc,1060,700,NULL);SetViewportExtEx(dc,(int)(1060*ui_scale),(int)(700*ui_scale),NULL);SetViewportOrgEx(dc,ui_x,ui_y,NULL);
        picture_draw(dc,&forest,0,0,1060,700);
        rounded(dc,14,72,1032,554,RGB(12,31,26),RGB(74,100,72));
        draw_text(dc,32,646,530,36,feedback[0]?feedback:L"← → ajustar   ·   Tab navegar   ·   Esc voltar",caption,RGB(190,211,165));
        double t=(GetTickCount()-page_started)/220.0;if(t>1)t=1;SetViewportOrgEx(dc,ui_x+(int)(22*(1-t)*(1-t)*ui_scale),ui_y,NULL);
        if(current_tab==0)draw_pad(dc);
        if(current_tab==3){
            draw_text(dc,32,90,620,40,L"Mônica no Castelo do Dragão",heading,RGB(255,210,103));
            draw_text(dc,32,142,610,30,L"1991  •  Master System  •  Brasil",caption,RGB(180,217,159));
            draw_text(dc,32,192,610,130,L"Publicado pela Tec Toy no Brasil, o cartucho adapta Wonder Boy in Monster Land, da Westone/Sega, ao universo de Maurício de Sousa. Mônica protagoniza a aventura com textos em português e personagens da Turma da Mônica.",normal,RGB(245,236,212));
            draw_text(dc,32,321,610,120,L"Plataformas, combates, lojas e equipamentos fazem parte da jornada contra os monstros e os planos do Capitão Feio. As artes ao lado são materiais de época fornecidos para contextualizar o lançamento brasileiro.",normal,RGB(245,236,212));
            draw_text(dc,32,437,610,103,L"Este projeto preserva o reconhecimento a quem criou o jogo original. Conheça a implementação em Projeto e os titulares e bibliotecas em Créditos.",normal,RGB(180,217,159));
            picture_draw(dc,&cover,679,88,315,220);picture_draw(dc,&advert,720,324,250,280);
        }
        if(current_tab==4){
            draw_text(dc,40,96,960,42,L"Como este projeto foi feito",heading,RGB(255,210,103));
            draw_text(dc,40,157,950,75,L"Meu objetivo é aprender como fazer um recompilado: estudar o funcionamento do cartucho e construir uma experiência nativa para Windows, com código e documentação desenvolvidos com auxílio do Codex.",normal,RGB(245,236,212));
            draw_text(dc,40,247,950,92,L"BASE TÉCNICA  •  smsggrecomp, de mstan. O backend AOT transforma instruções Z80 observadas em código C por banco, compilado para x64. A pilha e as interrupções do jogo continuam respeitando o comportamento do Master System.",normal,RGB(245,236,212));
            draw_text(dc,40,354,950,80,L"RUNTIME  •  VDP e PSG da base reproduzem vídeo e áudio. SDL3 recebe controles modernos; Win32 desenha a interface e WinMM entrega o áudio. O núcleo Z80 superzazu é usado nas ferramentas de referência e comparação.",normal,RGB(245,236,212));
            draw_text(dc,40,451,950,80,L"SALVAMENTO  •  A sequência de entradas reconstrói a partida e um hash verifica o estado. Não é um snapshot instantâneo. O backend interrompe a execução se encontrar código ainda não coberto; o jogo completo está em validação.",normal,RGB(180,217,159));
            draw_text(dc,40,548,950,58,L"Arte de menu e ícone: ilustrações de fã geradas com ImageGen. Interface e código: projeto educacional assistido por IA. Sem vínculo com os produtores originais.",caption,RGB(180,217,159));
        }
        if(current_tab==5){
            draw_text(dc,40,96,960,42,L"Créditos e direitos autorais",heading,RGB(255,210,103));
            draw_text(dc,40,156,950,82,L"JOGO ORIGINAL EM CARTUCHO  •  Westone: criação de Wonder Boy in Monster Land. Sega: publicação e plataforma Master System. Tec Toy: adaptação e publicação brasileira de Mônica no Castelo do Dragão (1991).",normal,RGB(245,236,212));
            draw_text(dc,40,253,950,68,L"PERSONAGENS E UNIVERSO  •  Maurício de Sousa e titulares da Turma da Mônica. Marcas, personagens, gráficos, músicas, capas e demais elementos do jogo pertencem aos respectivos titulares.",normal,RGB(245,236,212));
            draw_text(dc,40,337,950,95,L"Este é um projeto de fã, independente, não oficial e educacional. Não inclui a ROM. Exigir uma ROM do usuário não concede licença nem garante isenção de direitos autorais. A reprodução de materiais de época não transfere seus direitos ao projeto.",normal,RGB(245,236,212));
            draw_text(dc,40,447,950,85,L"BASES E BIBLIOTECAS  •  smsggrecomp (mstan; sem licença declarada na revisão usada), superzazu/z80 (MIT) e SDL3 (zlib). O repositório fornece patches e scripts; não redistribui o runtime completo da base nem código derivado da ROM.",normal,RGB(180,217,159));
            draw_text(dc,40,549,950,55,L"Consulte THIRD_PARTY.md para origens e licenças. Créditos individuais adicionais só serão incluídos quando confirmados por uma fonte verificável.",caption,RGB(180,217,159));
        }
        SetMapMode(dc,MM_TEXT);SetViewportOrgEx(dc,0,0,NULL);BitBlt(screen,0,0,r.right,r.bottom,dc,0,0,SRCCOPY);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);if(msg==WM_PAINT)EndPaint(hwnd,&ps);return 0;
    }
    if(msg==WM_TIMER){
        if(closing){
            DWORD elapsed=GetTickCount()-screen_started;
            if(elapsed>=230){DestroyWindow(hwnd);return 0;}
            RECT r;GetClientRect(owner,&r);double t=elapsed/230.0;
            SetWindowPos(hwnd,NULL,(int)(r.right*t*t),0,r.right,r.bottom,SWP_NOZORDER|SWP_NOACTIVATE);
            InvalidateRect(owner,NULL,FALSE);return 0;
        }
        if(GetTickCount()-page_started<240){for(int i=0;i<item_count;i++)position_child(items[i]);InvalidateRect(hwnd,NULL,FALSE);}
        input_update();
        static uint16_t previous;uint16_t now=input_read().menu,pressed=now&~previous;previous=now;
        if(capture_pad<0 && GetForegroundWindow()==owner){
            if(pressed&(INPUT_B|INPUT_BACK)){SendMessageW(hwnd,WM_COMMAND,2,0);return 0;}
            HWND focus=GetFocus();int tab=-1;for(int i=0;i<6;i++)if(focus==nav[i])tab=i;
            if(tab>=0 && (pressed&(INPUT_LEFT|INPUT_RIGHT))){int next=(tab+(pressed&INPUT_LEFT?5:1))%6;read_controls();build_page(next);SetFocus(nav[next]);}
            else if(pressed&(INPUT_UP|INPUT_DOWN))SetFocus(GetNextDlgTabItem(hwnd,focus,!!(pressed&INPUT_UP)));
            else if(focus && (pressed&(INPUT_LEFT|INPUT_RIGHT|INPUT_A))){
                wchar_t type[32];GetClassNameW(focus,type,32);
                if(!wcscmp(type,L"MonicaChoice")){int count=(int)SendMessageW(focus,CB_GETCOUNT,0,0);int value=selection(focus);if(count>0){select_at(focus,(value+(pressed&INPUT_LEFT?count-1:1))%count);SendMessageW(hwnd,WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(focus),CBN_SELCHANGE),(LPARAM)focus);InvalidateRect(focus,NULL,FALSE);}}
                else if(pressed&INPUT_A)SendMessageW(focus,BM_CLICK,0,0);
            }
        }
        if(current_tab==0){
            if(capture_pad>=0){int pressed=input_first_pressed();if(pressed<0)release_pad=1;else if(release_pad){pending.buttons[capture_pad]=pressed;select_at(pad_combos[capture_pad],pressed+1);SetWindowTextW(hwnd,L"Mônica • Configurações");capture_pad=-1;}}
            RECT r={20,148,1020,310};InvalidateRect(hwnd,&r,FALSE);
        }return 0;
    }
    if(msg==WM_COMMAND){int id=LOWORD(wp);
        if(closing)return 0;
        if(id==1){read_controls();*target=pending;SendMessageW(owner,PREFS_APPLIED,0,0);closing=1;screen_started=GetTickCount();return 0;}
        if(id>=600&&id<606){read_controls();build_page(id-600);return 0;}
        if(id==42){pending.muted=!pending.muted;InvalidateRect(audio_controls[2],NULL,FALSE);return 0;}
        if(id==2){input_select(original.device);input_configure(original.buttons);input_set_deadzone(original.deadzone);closing=1;screen_started=GetTickCount();return 0;}
        if(id==11){rebuild_devices();return 0;}
        if(id==10 && HIWORD(wp)==CBN_SELCHANGE){int i=selection(device_combo);snprintf(pending.device,sizeof pending.device,"%s",i>0&&i<=device_count?devices[i-1].key:"");input_select(pending.device);input_update();InvalidateRect(hwnd,NULL,FALSE);return 0;}
        if(id>=100&&id<106){capture_key=id-100;SetWindowTextW(key_buttons[capture_key],L"Pressione uma tecla…");return 0;}
        if(id>=300&&id<308){capture_pad=id-300;release_pad=0;SetWindowTextW(hwnd,L"Solte e pressione o botão desejado no controle");return 0;}
        if(id==50)ShellExecuteW(hwnd,L"open",L"https://turmadamonica.fandom.com/pt-br/wiki/M%C3%B4nica_no_Castelo_do_Drag%C3%A3o",NULL,NULL,SW_SHOWNORMAL);
        if(id==51)ShellExecuteW(hwnd,L"open",L"https://www.smspower.org/Games/MonicaNoCasteloDoDragao-SMS",NULL,NULL,SW_SHOWNORMAL);
    }
    if(msg==WM_CLOSE){input_select(original.device);input_configure(original.buttons);input_set_deadzone(original.deadzone);closing=1;screen_started=GetTickCount();return 0;}
    if(msg==WM_DESTROY){for(HWND c=GetWindow(hwnd,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))free((void*)RemovePropW(c,L"layout"));KillTimer(hwnd,1);dialog=NULL;SendMessageW(owner,PREFS_CLOSED,0,0);InvalidateRect(owner,NULL,FALSE);DeleteObject(widget_font);widget_font=NULL;DeleteObject(normal);DeleteObject(heading);DeleteObject(caption);DeleteObject(paper);free(cover.pixels);free(advert.pixels);free(forest.pixels);memset(&forest,0,sizeof forest);memset(&cover,0,sizeof cover);memset(&advert,0,sizeof advert);return 0;}
    return DefWindowProcW(hwnd,msg,wp,lp);
}
void preferences_open(HWND parent,Preferences *p,int tab,const wchar_t *assets){
    if(dialog){SetFocus(nav[current_tab]);return;}choice_register(GetModuleHandleW(NULL));feedback[0]=0;owner=parent;target=p;pending=original=*p;wcsncpy(asset_directory,assets,32767);
    paper=CreateSolidBrush(RGB(12,31,26));normal=CreateFontW(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");heading=CreateFontW(-29,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");caption=CreateFontW(-16,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    WNDCLASSW wc={0};wc.lpfnWndProc=procedure;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"MonicaPreferences";wc.hbrBackground=paper;wc.hCursor=LoadCursor(NULL,IDC_ARROW);UnregisterClassW(wc.lpszClassName,wc.hInstance);RegisterClassW(&wc);
    RECT r;GetClientRect(parent,&r);dialog=CreateWindowExW(WS_EX_CONTROLPARENT,L"MonicaPreferences",L"Configurações da aventura",WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,r.right,r.bottom,parent,NULL,wc.hInstance,NULL);
    ui_scale=(double)r.right/1060;if((double)r.bottom/700<ui_scale)ui_scale=(double)r.bottom/700;ui_x=(r.right-(int)(1060*ui_scale))/2;ui_y=(r.bottom-(int)(700*ui_scale))/2;
    preferences_resize();closing=0;
    wchar_t *names[]={L"Controles",L"Vídeo",L"Áudio",L"O jogo",L"Projeto",L"Créditos"};
    for(int i=0;i<6;i++)nav[i]=child(L"BUTTON",names[i],WS_TABSTOP,32+i*166,22,156,40,600+i,0);
    child(L"BUTTON",L"Concluir",WS_TABSTOP,818,642,202,40,1,0);child(L"BUTTON",L"Descartar",WS_TABSTOP,650,642,154,40,2,0);
    picture_load(&cover,L"period-box.png");picture_load(&advert,L"period-advert.png");picture_load(&forest,L"menu-forest.png");if(forest.pixels)for(size_t i=0;i<(size_t)forest.w*forest.h;i++){uint32_t c=forest.pixels[i];forest.pixels[i]=(((c>>16)&255)*18/100<<16)|(((c>>8)&255)*18/100<<8)|((c&255)*18/100);}screen_started=GetTickCount();build_page(tab);ShowWindow(dialog,SW_SHOW);SetFocus(nav[tab]);SetTimer(dialog,1,16,NULL);

}
int preferences_route(MSG *msg){
    if(!dialog)return 0;
    if(capture_key>=0 && msg->message==WM_KEYDOWN){
        int key=(int)msg->wParam;if(key!=VK_ESCAPE){int used=0;for(int i=0;i<6;i++)if(i!=capture_key&&pending.keys[i]==key)used=1;
            if(used || key==VK_F2||key==VK_F5||key==VK_F9||key==VK_F11){wcscpy(feedback,L"Tecla em uso. Escolha outra para continuar.");InvalidateRect(dialog,NULL,FALSE);return 1;}pending.keys[capture_key]=key;}
        wchar_t name[80];key_name(pending.keys[capture_key],name);SetWindowTextW(key_buttons[capture_key],name);capture_key=-1;feedback[0]=0;InvalidateRect(dialog,NULL,FALSE);return 1;
    }
    if(msg->message==WM_KEYDOWN && msg->wParam==VK_ESCAPE){SendMessageW(dialog,WM_COMMAND,2,0);return 1;}
    if(msg->message==WM_KEYDOWN && (msg->wParam==VK_LEFT||msg->wParam==VK_RIGHT)){
        for(int i=0;i<6;i++)if(GetFocus()==nav[i]){int next=(i+(msg->wParam==VK_LEFT?5:1))%6;SendMessageW(dialog,WM_COMMAND,600+next,0);SetFocus(nav[next]);return 1;}
    }
    return IsDialogMessageW(dialog,msg)!=0;
}
int preferences_active(void){return dialog!=NULL;}
