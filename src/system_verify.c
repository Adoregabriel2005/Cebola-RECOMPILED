#include "preferences.h"
#include "choice.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(TEST) do{if(!(TEST)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#TEST);return 1;}}while(0)
static void select_virtual(SDL_JoystickID id){InputDevice d[32];int n=input_devices(d,32);for(int i=0;i<n;i++)if(d[i].id==id){input_select(d[i].key);input_update();return;}}
int wmain(int argc,wchar_t **argv){
    CHECK(argc==2);CHECK(input_init());
    choice_register(GetModuleHandleW(NULL));
    HWND host=CreateWindowW(L"STATIC",L"test",WS_OVERLAPPED,0,0,500,200,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND choice=CreateWindowW(L"MonicaChoice",L"",WS_CHILD,0,0,300,32,host,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(host&&choice);CHECK((GetWindowLongW(choice,GWL_STYLE)&WS_POPUP)==0);
    SendMessageW(choice,CB_ADDSTRING,0,(LPARAM)L"Janela");SendMessageW(choice,CB_ADDSTRING,0,(LPARAM)L"Tela cheia");
    SendMessageW(choice,CB_SETCURSEL,0,0);SendMessageW(choice,WM_KEYDOWN,VK_RIGHT,0);CHECK(SendMessageW(choice,CB_GETCURSEL,0,0)==1);
    SendMessageW(choice,WM_KEYDOWN,VK_RIGHT,0);CHECK(SendMessageW(choice,CB_GETCURSEL,0,0)==0);
    SendMessageW(choice,WM_KEYDOWN,VK_LEFT,0);wchar_t selected[64];SendMessageW(choice,CB_GETLBTEXT,1,(LPARAM)selected);CHECK(!wcscmp(selected,L"Tela cheia"));
    SendMessageW(choice,CB_RESETCONTENT,0,0);CHECK(SendMessageW(choice,CB_GETCOUNT,0,0)==0&&SendMessageW(choice,CB_GETCURSEL,0,0)==-1);
    DestroyWindow(host);puts("PASS inline selector: keyboard changes, wrap, Unicode values, reset, no popup style");
    SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_UNKNOWN;desc.naxes=2;desc.nbuttons=12;desc.nhats=1;desc.vendor_id=0xCAFE;desc.product_id=1;desc.name="Monica test A";
    SDL_JoystickID a=SDL_AttachVirtualJoystick(&desc);CHECK(a!=0);desc.product_id=2;desc.name="Monica test B";SDL_JoystickID b=SDL_AttachVirtualJoystick(&desc);CHECK(b!=0);
    SDL_Joystick *ja=SDL_OpenJoystick(a),*jb=SDL_OpenJoystick(b);CHECK(ja&&jb);select_virtual(b);
    CHECK(input_read().connected);CHECK(wcsstr(input_name(),L"test B")!=NULL);
    CHECK(SDL_SetJoystickVirtualButton(ja,0,true));input_update();CHECK(!(input_read().pad&INPUT_A));
    CHECK(SDL_SetJoystickVirtualButton(jb,1,true));input_update();CHECK(input_read().pad&INPUT_B);
    CHECK(SDL_SetJoystickVirtualButton(jb,1,false));input_update();
    input_set_deadzone(25);CHECK(SDL_SetJoystickVirtualAxis(jb,0,4000));input_update();CHECK(!(input_read().pad&INPUT_RIGHT));
    CHECK(SDL_SetJoystickVirtualAxis(jb,0,16000));input_update();CHECK(input_read().pad&INPUT_RIGHT);
    CHECK(SDL_SetJoystickVirtualAxis(jb,0,0));CHECK(SDL_SetJoystickVirtualHat(jb,0,SDL_HAT_UP));input_update();CHECK(input_read().pad&INPUT_UP);
    CHECK(SDL_SetJoystickVirtualHat(jb,0,SDL_HAT_CENTERED));int bindings[8]={5,4,7,6,-1,-1,8,9};input_configure(bindings);
    CHECK(SDL_SetJoystickVirtualButton(jb,5,true));CHECK(SDL_SetJoystickVirtualButton(jb,8,true));input_update();CHECK((input_read().pad&(INPUT_A|INPUT_LEFT))==(INPUT_A|INPUT_LEFT));
    CHECK(SDL_DetachVirtualJoystick(b));input_update();CHECK(!input_read().connected);
    SDL_CloseJoystick(jb);SDL_CloseJoystick(ja);CHECK(SDL_DetachVirtualJoystick(a));input_close();
    RECT src,dst=video_destination(1920,1080,2,0,&src);CHECK(dst.left==240&&dst.top==0&&dst.right==1680&&dst.bottom==1080);CHECK(src.left==0&&src.top==0&&src.right==256&&src.bottom==192);
    dst=video_destination(1920,1080,2,2,&src);CHECK(dst.left==0&&dst.right==1920&&src.top==24&&src.bottom==168);
    dst=video_destination(1920,1080,2,1,&src);CHECK(dst.right-dst.left==1280&&dst.bottom-dst.top==960);
    for(int aspect=0;aspect<3;aspect++){dst=video_destination(1280,1024,aspect,0,&src);CHECK(abs((dst.right-dst.left)*3-(dst.bottom-dst.top)*4)<=4);CHECK(dst.left>=0&&dst.top>=0&&dst.right<=1280&&dst.bottom<=1024);}
    Preferences p,q;preferences_defaults(&p);p.aspect=2;p.volume=35;p.sample_rate=96000;p.buttons[2]=8;p.keys[4]='J';p.scale=1;p.deadzone=40;
    preferences_save(&p,argv[1]);preferences_load(&q,argv[1]);CHECK(q.aspect==2&&q.volume==35&&q.sample_rate==96000&&q.buttons[2]==8&&q.keys[4]=='J'&&q.scale==1&&q.deadzone==40);
    WritePrivateProfileStringW(L"video",L"aspect",L"99",argv[1]);WritePrivateProfileStringW(L"audio",L"volume",L"-500",argv[1]);preferences_load(&q,argv[1]);CHECK(q.aspect==2&&q.volume==0);
    puts("PASS device selection, raw remapping, axes/deadzone, hat, disconnect, non-stretched geometry, crop, integer scale, settings roundtrip/clamping");return 0;
}
