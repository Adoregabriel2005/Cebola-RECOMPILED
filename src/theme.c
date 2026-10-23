#include "theme.h"
#include <stdlib.h>
#include <math.h>
#include <wchar.h>
#include "png_write.h"

static DWORD entered;
static HDC canvas;
static HBITMAP canvas_bitmap,previous_bitmap;
static HFONT title,subtitle,label,small,menu;
static int canvas_w,canvas_h;
static const COLORREF cream=RGB(235,247,255),gold=RGB(30,195,238),green=RGB(7,16,33);
static int sx(int x){return MulDiv(x,canvas_w,1280);}
static int sy(int y){return MulDiv(y,canvas_h,720);}
static void fill(HDC dc,RECT r,COLORREF color){HBRUSH b=CreateSolidBrush(color);FillRect(dc,&r,b);DeleteObject(b);}
static void word(HDC dc,int x,int y,int w,int h,const wchar_t *s,HFONT f,COLORREF color,UINT flags){
    RECT r={sx(x),sy(y),sx(x+w),sy(y+h)};HFONT old=SelectObject(dc,f);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);
    DrawTextW(dc,s,-1,&r,flags);SelectObject(dc,old);
}
static HFONT make_font(int size,int weight,const wchar_t *family){
    return CreateFontW(-sy(size),0,0,0,weight,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,family);
}
int theme_init(const wchar_t *path){
    (void)path;
    entered=GetTickCount();return 1;
}
static void free_canvas(void){
    if(canvas){SelectObject(canvas,previous_bitmap);DeleteObject(canvas_bitmap);DeleteDC(canvas);canvas=NULL;}
    DeleteObject(title);DeleteObject(subtitle);DeleteObject(label);DeleteObject(small);DeleteObject(menu);
}
void theme_close(void){free_canvas();}
static void prepare(HDC dc,int w,int h){
    if(w==canvas_w && h==canvas_h && canvas)return;
    free_canvas();canvas_w=w;canvas_h=h;canvas=CreateCompatibleDC(dc);canvas_bitmap=CreateCompatibleBitmap(dc,w,h);previous_bitmap=SelectObject(canvas,canvas_bitmap);
    title=make_font(83,FW_HEAVY,L"Segoe UI Black");subtitle=make_font(25,FW_BOLD,L"Trebuchet MS");
    label=make_font(17,FW_NORMAL,L"Segoe UI");small=make_font(13,FW_SEMIBOLD,L"Segoe UI");menu=make_font(24,FW_SEMIBOLD,L"Trebuchet MS");
}
void theme_render(HDC dc,int w,int h,int paused,const uint32_t *fb,int ready,int slot){
    if(w<=0 || h<=0)return;prepare(dc,w,h);RECT all={0,0,w,h};fill(canvas,all,green);
    word(canvas,75,78,620,110,L"CEBOLA",title,cream,DT_LEFT|DT_SINGLELINE);
    word(canvas,82,184,530,44,L"RECOMPILED",subtitle,gold,DT_LEFT|DT_SINGLELINE);
    word(canvas,82,242,470,28,paused?L"Um respiro antes da próxima aventura.":L"Sua próxima partida começa aqui.",label,RGB(193,214,189),DT_LEFT|DT_SINGLELINE);
    if(paused && fb){
        RECT border={sx(797),sy(333),sx(1211),sy(649)};fill(canvas,border,RGB(239,198,106));
        BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=256;info.bmiHeader.biHeight=-192;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
        SetStretchBltMode(canvas,COLORONCOLOR);StretchDIBits(canvas,sx(803),sy(339),sx(402),sy(304),0,0,256,192,fb,&info,DIB_RGB_COLORS,SRCCOPY);
        word(canvas,798,297,414,28,L"AVENTURA PAUSADA",small,cream,DT_LEFT|DT_SINGLELINE);
    }else{
        double t=GetTickCount64()/1000.0;
        for(int i=0;i<10;i++){
            double p=fmod(t*0.015+i*0.103,1.0);int x=sx(660+(i*67)%570)+(int)(sin(t*0.4+i)*sx(14));int y=sy((int)(670-p*530));
            HBRUSH brush=CreateSolidBrush(i%3?RGB(237,199,102):RGB(255,235,162));HGDIOBJ old=SelectObject(canvas,brush),pen=SelectObject(canvas,GetStockObject(NULL_PEN));
            int size=sy(i%3?2:3);Ellipse(canvas,x,y,x+size+1,y+size+1);SelectObject(canvas,pen);SelectObject(canvas,old);DeleteObject(brush);
        }
    }
    wchar_t status[180];swprintf(status,180,paused?L"Slot %d selecionado  ·  Seu progresso, no seu PC":ready?L"ROM pronta  ·  Master System  ·  Slot %d":L"Selecione sua ROM para começar  ·  Slot %d",slot);
    word(canvas,82,665,760,25,status,small,ready?RGB(175,220,159):cream,DT_LEFT|DT_SINGLELINE);
    word(canvas,82,694,800,20,L"↑ ↓ navegar     Enter selecionar     Esc voltar     F11 tela cheia",small,RGB(155,180,158),DT_LEFT|DT_SINGLELINE);
    BitBlt(dc,0,0,w,h,canvas,0,0,SRCCOPY);
}
void theme_button(const DRAWITEMSTRUCT *item,int selected,int index){
    if(!canvas)return;
    RECT parent_rect={sx(76),sy(280+index*49),0,0};
    if(item->hwndItem){GetWindowRect(item->hwndItem,&parent_rect);MapWindowPoints(HWND_DESKTOP,GetParent(item->hwndItem),(POINT*)&parent_rect,2);}
    HDC target=item->hDC;RECT r=item->rcItem;
    HDC dc=CreateCompatibleDC(target);HBITMAP bitmap=CreateCompatibleBitmap(target,r.right,r.bottom);HGDIOBJ old_bitmap=SelectObject(dc,bitmap);
    BitBlt(dc,0,0,r.right,r.bottom,canvas,parent_rect.left,parent_rect.top,SRCCOPY);
    int disabled=item->itemState&ODS_DISABLED;
    int active=!disabled&&(selected || (item->itemState&ODS_SELECTED));
    static float levels[7];static DWORD stamps[7];DWORD tick=GetTickCount();float dt=(tick-stamps[index])/110.0f;if(dt>1)dt=1;stamps[index]=tick;
    levels[index]+=((active?1.0f:0.0f)-levels[index])*dt;float level=levels[index];
    if(level>0.02f){
        HBRUSH b=CreateSolidBrush(item->itemState&ODS_SELECTED?RGB(8,73,132):RGB(10,(int)(32+52*level),(int)(58+80*level)));HPEN pen=CreatePen(PS_SOLID,sy(1)+1,gold);
        HGDIOBJ oldb=SelectObject(dc,b),oldp=SelectObject(dc,pen);RoundRect(dc,1,1,r.right-1,r.bottom-1,sy(10),sy(10));SelectObject(dc,oldp);SelectObject(dc,oldb);DeleteObject(pen);DeleteObject(b);
    }
    const wchar_t *labels[]={L"Jogar",L"Continuar",L"Controles",L"Configurações",L"Selecionar ROM",L"Sobre o projeto",L"Sair"};
    wchar_t title_text[160];if(item->hwndItem)GetWindowTextW(item->hwndItem,title_text,160);else wcscpy(title_text,labels[index]);
    HFONT old=SelectObject(dc,menu);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,disabled?RGB(102,124,115):active?cream:RGB(231,234,206));
    RECT tr={sx(40),0,r.right-sx(42),r.bottom};DrawTextW(dc,title_text,-1,&tr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    SelectObject(dc,small);SetTextColor(dc,active?gold:RGB(119,159,135));wchar_t n[8];swprintf(n,8,L"%02d",index+1);
    RECT nr={sx(14),0,sx(34),r.bottom};DrawTextW(dc,n,-1,&nr,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    if(active){HPEN p=CreatePen(PS_SOLID,sy(2)+1,gold);HGDIOBJ prev=SelectObject(dc,p);int x=r.right-sx(24),y=r.bottom/2;MoveToEx(dc,x-sx(5),y-sy(5),NULL);LineTo(dc,x,y);LineTo(dc,x-sx(5),y+sy(5));SelectObject(dc,prev);DeleteObject(p);}
    SelectObject(dc,old);BitBlt(target,0,0,r.right,r.bottom,dc,0,0,SRCCOPY);SelectObject(dc,old_bitmap);DeleteObject(bitmap);DeleteDC(dc);
}
void theme_layout(HWND parent,HWND *buttons,int count){
    RECT r;GetClientRect(parent,&r);int w=r.right,h=r.bottom;
    for(int i=0;i<count;i++)SetWindowPos(buttons[i],NULL,MulDiv(76+(GetTickCount()-entered<260?(int)(28*(1.0-(GetTickCount()-entered)/260.0)):0),w,1280),MulDiv(280+i*49,h,720),MulDiv(410,w,1280),MulDiv(44,h,720),SWP_NOZORDER|SWP_NOACTIVATE);
}
int theme_preview_state(const char *path,int ready){
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=1280;info.bmiHeader.biHeight=-720;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    uint32_t *pixels=NULL;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void**)&pixels,NULL,0);if(!bitmap){DeleteDC(dc);return 0;}
    HGDIOBJ old=SelectObject(dc,bitmap);theme_render(dc,1280,720,0,NULL,ready,1);
    HDC row=CreateCompatibleDC(dc);HBITMAP row_bitmap=CreateCompatibleBitmap(dc,410,44);HGDIOBJ row_old=SelectObject(row,row_bitmap);
    for(int i=0;i<7;i++){
        DRAWITEMSTRUCT item={0};item.hDC=row;item.rcItem.right=410;item.rcItem.bottom=44;
        if(!ready&&i<2)item.itemState|=ODS_DISABLED;theme_button(&item,i==(ready?0:4),i);BitBlt(dc,76,280+i*49,410,44,row,0,0,SRCCOPY);
    }
    GdiFlush();for(unsigned i=0;i<1280*720;i++)pixels[i]|=0xFF000000u;
    int ok=png_write_argb(path,pixels,1280,720,1280)==0;
    SelectObject(row,row_old);DeleteObject(row_bitmap);DeleteDC(row);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);return ok;
}
int theme_preview(const char *path){return theme_preview_state(path,1);}
