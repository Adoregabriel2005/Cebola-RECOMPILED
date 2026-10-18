#pragma once
#include <stdint.h>
#include <wchar.h>
enum { INPUT_UP=1,INPUT_DOWN=2,INPUT_LEFT=4,INPUT_RIGHT=8,INPUT_A=16,INPUT_B=32,INPUT_START=64,INPUT_BACK=128 };
typedef struct { uint32_t id;wchar_t name[180];char guid[33],key[512];int mapped; } InputDevice;
typedef struct { uint8_t pad;uint16_t menu;int connected; } InputState;
int input_init(void);
void input_close(void);
void input_update(void);
int input_devices(InputDevice *devices,int capacity);
void input_select(const char *guid);
const char *input_preferred(void);
void input_set_buttons(int one,int two,int start,int back);
void input_get_buttons(int *one,int *two,int *start,int *back);
void input_configure(const int bindings[8]);
void input_set_deadzone(int percent);
int input_deadzone(void);
InputState input_read(void);
const wchar_t *input_name(void);
int input_button_count(void);
int input_first_pressed(void);
void input_default_label(int action,wchar_t *out,int capacity);
