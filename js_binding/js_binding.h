#ifndef _JS_BINDING_H_
#define _JS_BINDING_H_
#include "../cubicat.h"
#include "3rd_party/mjs/mjs.h"
#include "3rd_party/mjs/src/mjs_core.h"

extern "C" {

typedef void (*RegisterCallback)();

extern struct mjs *mjs; 
extern std::set<std::string> g_errorMsgs;

#define MJS_CALL(funcName, n, ...) \
do{ \
    if (!mjs) \
        return; \
    assert(n <= 6); \
    mjs_val_t func = mjs_get(mjs, mjs_get_global(mjs), funcName, strlen(funcName)); \
    if (!mjs_is_function(func)) { \
        printf("Function: [%s] not found!\n", funcName); \
        return; \
    } \
    mjs_err_t err; \
    if (n == 0) { \
        err = mjs_call(mjs, NULL, func, MJS_UNDEFINED, 0, NULL); \
    } else { \
        err = mjs_call(mjs, NULL, func, MJS_UNDEFINED, n, ##__VA_ARGS__); \
    } \
    if (err != MJS_OK) { \
        LOGE("JSCall failed call function: [%s] error: %s\n", funcName, mjs->error_msg); \
        g_errorMsgs.insert(mjs->error_msg); \
    } \
} while(0);

// partition name where js files are stored
void JSBindingInit(const char* fileDir, RegisterCallback registerCallback = nullptr);
void JSShowErrorMsg();

// [JS_BINDING_BEGIN] 
 SceneManager* getSceneManager();
 ResourceManager* getResourceManager();
 Display* getLCD();
// [JS_BINDING_END] 

}


#endif