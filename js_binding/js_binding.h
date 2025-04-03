#ifndef _JS_BINDING_H_
#define _JS_BINDING_H_
#include "../cubicat.h"

extern "C" {

typedef void (*RegisterCallback)();

// partition name where js files are stored
void JSBindingInit(const char* fileDir, RegisterCallback registerCallback = nullptr);
void JSBindingDeinit();
void JSCall(const char* funcName, int nargs, ...);
void JSShowErrorMsg();

// [JS_BINDING_BEGIN] 
 SceneManager* getSceneManager();
 ResourceManager* getResourceManager();
 Display* getLCD();
// [JS_BINDING_END] 

}


#endif