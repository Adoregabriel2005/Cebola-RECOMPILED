#include "theme.h"
#include <stdlib.h>
int wmain(int argc,wchar_t **argv){
    if((argc!=3&&argc!=4) || !theme_init(argv[1]))return 2;
    int n=WideCharToMultiByte(CP_UTF8,0,argv[2],-1,NULL,0,NULL,NULL);char *path=malloc(n);
    if(!path){theme_close();return 2;}WideCharToMultiByte(CP_UTF8,0,argv[2],-1,path,n,NULL,NULL);
    int ok=theme_preview_state(path,argc==3);free(path);theme_close();return ok?0:1;
}
