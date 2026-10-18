#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include "preferences.h"
#include "png_write.h"
/* Renders this project's own interface, with no desktop or other-app capture. */
int wmain(int argc,wchar_t **argv){
    if(argc<4)return 2;CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);input_init();
    int w=argc>4?_wtoi(argv[4]):1280,h=argc>5?_wtoi(argv[5]):720;
    HWND owner=CreateWindowW(L"STATIC",L"Interface preview",WS_OVERLAPPEDWINDOW,0,0,w,h,NULL,NULL,GetModuleHandleW(NULL),NULL);
    RECT r={0,0,w,h};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);SetWindowPos(owner,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);
    Preferences p;preferences_defaults(&p);preferences_open(owner,&p,_wtoi(argv[2]),argv[1]);Sleep(260);preferences_resize();
    HWND page=FindWindowExW(owner,NULL,L"MonicaPreferences",NULL);if(!page)return 3;
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    uint32_t *pixels;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,(void**)&pixels,NULL,0);HGDIOBJ old=SelectObject(dc,bitmap);
    SendMessageW(page,WM_PRINT,(WPARAM)dc,PRF_CLIENT|PRF_CHILDREN|PRF_ERASEBKGND);
    /* Hidden comboboxes omit their selection in WM_PRINT on Windows. Ask the
       same owner-draw routine to render that selection into this test image. */
    for(HWND c=GetWindow(page,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT)){
        wchar_t type[32];GetClassNameW(c,type,32);if(wcscmp(type,L"ComboBox"))continue;
        RECT cr;GetWindowRect(c,&cr);MapWindowPoints(HWND_DESKTOP,page,(POINT*)&cr,2);
        int saved=SaveDC(dc);SetViewportOrgEx(dc,cr.left+2,cr.top+2,NULL);
        DRAWITEMSTRUCT d={0};d.CtlType=ODT_COMBOBOX;d.CtlID=GetDlgCtrlID(c);d.itemID=(UINT)SendMessageW(c,CB_GETCURSEL,0,0);d.hwndItem=c;d.hDC=dc;d.rcItem=(RECT){0,0,cr.right-cr.left-24,cr.bottom-cr.top-4};
        SendMessageW(page,WM_DRAWITEM,d.CtlID,(LPARAM)&d);RestoreDC(dc,saved);
    }
    GdiFlush();for(int i=0;i<w*h;i++)pixels[i]|=0xff000000;
    char output[32768];WideCharToMultiByte(CP_UTF8,0,argv[3],-1,output,sizeof output,NULL,NULL);int ok=png_write_argb(output,pixels,w,h,w)==0;
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);DestroyWindow(owner);input_close();CoUninitialize();return ok?0:4;
}
