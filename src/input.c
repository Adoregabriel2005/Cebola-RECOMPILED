#include "input.h"
#include <SDL3/SDL.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
static SDL_Joystick *joystick;
static SDL_Gamepad *gamepad;
static SDL_JoystickID active;
static char preference[512];
static wchar_t current_name[180]=L"Nenhum controle conectado";
static int bindings[8]={-1,-1,-1,-1,-1,-1,-1,-1},deadzone=24,initialized;
int input_init(void){
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI,"1");
    initialized=SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_GAMEPAD|SDL_INIT_EVENTS);return initialized;
}
static void disconnect(void){if(gamepad)SDL_CloseGamepad(gamepad);if(joystick)SDL_CloseJoystick(joystick);gamepad=NULL;joystick=NULL;active=0;wcscpy(current_name,L"Nenhum controle conectado");}
void input_close(void){disconnect();if(initialized)SDL_Quit();initialized=0;}
static void guid_string(SDL_JoystickID id,char out[33]){SDL_GUIDToString(SDL_GetJoystickGUIDForID(id),out,33);}
int input_devices(InputDevice *out,int capacity){
    if(!initialized)return 0;int count=0,n=0;SDL_JoystickID *ids=SDL_GetJoysticks(&n);if(!ids)return 0;
    for(int i=0;i<n && count<capacity;i++){
        InputDevice *d=&out[count++];d->id=ids[i];d->mapped=SDL_IsGamepad(ids[i]);guid_string(ids[i],d->guid);
        const char *path=SDL_GetJoystickPathForID(ids[i]);snprintf(d->key,sizeof d->key,"%s",path&&*path?path:d->guid);
        const char *name=SDL_GetJoystickNameForID(ids[i]);if(!name)name="Controle";
        MultiByteToWideChar(CP_UTF8,0,name,-1,d->name,180);d->name[179]=0;
    }SDL_free(ids);return count;
}
void input_select(const char *guid){snprintf(preference,sizeof preference,"%s",guid?guid:"");disconnect();}
const char *input_preferred(void){return preference;}
void input_update(void){
    if(!initialized)return;
    SDL_Event event;while(SDL_PollEvent(&event)){}
    if(joystick && !SDL_JoystickConnected(joystick))disconnect();
    if(joystick)return;
    InputDevice devices[32];int n=input_devices(devices,32);
    for(int i=0;i<n;i++){
        if(preference[0] && strcmp(preference,devices[i].key))continue;
        joystick=SDL_OpenJoystick(devices[i].id);if(!joystick)continue;
        active=devices[i].id;if(devices[i].mapped)gamepad=SDL_OpenGamepad(active);
        wcscpy(current_name,devices[i].name);break;
    }
}
void input_set_buttons(int a,int b,int start,int back){bindings[0]=a;bindings[1]=b;bindings[2]=start;bindings[3]=back;}
void input_get_buttons(int *a,int *b,int *start,int *back){*a=bindings[0];*b=bindings[1];*start=bindings[2];*back=bindings[3];}
void input_configure(const int b[8]){memcpy(bindings,b,sizeof bindings);}
void input_set_deadzone(int v){deadzone=v<5?5:v>80?80:v;}
int input_deadzone(void){return deadzone;}
static int button(int action,SDL_GamepadButton standard,int fallback){
    int raw=bindings[action];if(raw>=0)return raw<SDL_GetNumJoystickButtons(joystick) && SDL_GetJoystickButton(joystick,raw);
    return gamepad?SDL_GetGamepadButton(gamepad,standard):fallback<SDL_GetNumJoystickButtons(joystick) && SDL_GetJoystickButton(joystick,fallback);
}
InputState input_read(void){
    InputState s={0};if(!joystick)return s;s.connected=1;int limit=32767*deadzone/100;
    Sint16 x=0,y=0;Uint8 hat=SDL_GetNumJoystickHats(joystick)>0?SDL_GetJoystickHat(joystick,0):0;
    if(gamepad){
        x=SDL_GetGamepadAxis(gamepad,SDL_GAMEPAD_AXIS_LEFTX);y=SDL_GetGamepadAxis(gamepad,SDL_GAMEPAD_AXIS_LEFTY);
        if(SDL_GetGamepadButton(gamepad,SDL_GAMEPAD_BUTTON_DPAD_UP))s.pad|=INPUT_UP;
        if(SDL_GetGamepadButton(gamepad,SDL_GAMEPAD_BUTTON_DPAD_DOWN))s.pad|=INPUT_DOWN;
        if(SDL_GetGamepadButton(gamepad,SDL_GAMEPAD_BUTTON_DPAD_LEFT))s.pad|=INPUT_LEFT;
        if(SDL_GetGamepadButton(gamepad,SDL_GAMEPAD_BUTTON_DPAD_RIGHT))s.pad|=INPUT_RIGHT;
    }else{
        if(SDL_GetNumJoystickAxes(joystick)>0)x=SDL_GetJoystickAxis(joystick,0);
        if(SDL_GetNumJoystickAxes(joystick)>1)y=SDL_GetJoystickAxis(joystick,1);
    }
    if(y < -limit || (hat&SDL_HAT_UP))s.pad|=INPUT_UP;if(y>limit || (hat&SDL_HAT_DOWN))s.pad|=INPUT_DOWN;
    if(x < -limit || (hat&SDL_HAT_LEFT))s.pad|=INPUT_LEFT;if(x>limit || (hat&SDL_HAT_RIGHT))s.pad|=INPUT_RIGHT;
    for(int i=0;i<4;i++)if(bindings[4+i]>=0){s.pad&=~(1<<i);if(bindings[4+i]<SDL_GetNumJoystickButtons(joystick)&&SDL_GetJoystickButton(joystick,bindings[4+i]))s.pad|=1<<i;}
    if(button(0,SDL_GAMEPAD_BUTTON_SOUTH,0))s.pad|=INPUT_A;if(button(1,SDL_GAMEPAD_BUTTON_EAST,1))s.pad|=INPUT_B;
    s.menu=s.pad;
    if(button(2,SDL_GAMEPAD_BUTTON_START,7))s.menu|=INPUT_START;
    if(button(3,SDL_GAMEPAD_BUTTON_BACK,6))s.menu|=INPUT_BACK;
    return s;
}
const wchar_t *input_name(void){return current_name;}
int input_button_count(void){return joystick?SDL_GetNumJoystickButtons(joystick):0;}
int input_first_pressed(void){if(!joystick)return -1;for(int i=0;i<SDL_GetNumJoystickButtons(joystick);i++)if(SDL_GetJoystickButton(joystick,i))return i;return -1;}
void input_default_label(int action,wchar_t *out,int capacity){
    if(action>=4){swprintf(out,capacity,L"Direcional / analógico");return;}
    if(!joystick){swprintf(out,capacity,L"Automático");return;}
    if(!gamepad){int defaults[]={1,2,8,7};swprintf(out,capacity,L"Botão físico %d",defaults[action]);return;}
    if(action>=2){swprintf(out,capacity,action==2?L"Start / Options":L"Back / Share");return;}
    SDL_GamepadButtonLabel label=SDL_GetGamepadButtonLabel(gamepad,action==0?SDL_GAMEPAD_BUTTON_SOUTH:SDL_GAMEPAD_BUTTON_EAST);
    const wchar_t *name=label==SDL_GAMEPAD_BUTTON_LABEL_CROSS?L"Cruz":label==SDL_GAMEPAD_BUTTON_LABEL_CIRCLE?L"Círculo":label==SDL_GAMEPAD_BUTTON_LABEL_A?L"A":label==SDL_GAMEPAD_BUTTON_LABEL_B?L"B":action==0?L"Sul":L"Leste";
    swprintf(out,capacity,L"%ls",name);
}
