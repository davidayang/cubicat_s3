#include "js_binding.h"
#include "3rd_party/mjs/mjs.h"
#include "3rd_party/mjs/src/mjs_core.h"
#include "cubicat.h"

#define STRINGIFY(x) #x
#define StringHash(s) std::hash<std::string>{}(s)
struct mjs *mjs = NULL;
std::unordered_map<uint32_t, void*>* g_APIMap;
std::set<std::string> g_errorMsgs;

extern void Register_CUBICAT_API();

char *cs_read_file(const char *path, size_t *size)
{
    FILE *fp = CUBICAT.storage.openFileFlash(path);
    char *data = NULL;
    if (fp == NULL)
    {
        
    }
    else if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
    }
    else
    {
        *size = ftell(fp);
        data = (char *)psram_prefered_malloc(*size + 1);
        if (data != NULL)
        {
            fseek(fp, 0, SEEK_SET); /* Some platforms might not have rewind(), Oo */
            if (fread(data, 1, *size, fp) != *size)
            {
                free(data);
                return NULL;
            }
            data[*size] = '\0';
        }
        fclose(fp);
    }
    return data;
}

SceneManager* getSceneManager() {
    return CUBICAT.engine.getSceneManager();
}
ResourceManager* getResourceManager() {
    return CUBICAT.engine.getResourceManager();
}
Display* getLCD() {
    return &CUBICAT.lcd;
}

void *cubicat_dlsym(void *handle, const char *name)
{
    auto itr = g_APIMap->find(StringHash(name));
    if (itr != g_APIMap->end())
        return itr->second;
    std::string err = "API not found: " + std::string(name);
    LOGE("%s\n", err.c_str());
    g_errorMsgs.insert(err);
    return NULL;
}

void JSBindingInit(const char *fileDir, RegisterCallback registerCallback)
{
    if (mjs)
        return;
    g_APIMap = new std::unordered_map<uint32_t, void*>;
    Register_CUBICAT_API();
    if (registerCallback) {
        registerCallback();
    }
    auto oldPartition = CUBICAT.storage.getActivePartition();
    CUBICAT.storage.partitionSelect(fileDir);
    mjs = mjs_create();
    mjs_val_t res = MJS_UNDEFINED;
    mjs_set_ffi_resolver(mjs, cubicat_dlsym);
    auto err = mjs_exec_file(mjs, "main.js", &res);
    if (err != MJS_OK)
    {
        LOGE("error: %s\n", mjs->error_msg);
    }
    mjs_gc(mjs, 1);
    CUBICAT.storage.partitionSelect(oldPartition.c_str());
}

void JSCall(const char *func, int nargs, ...) {
    if (!mjs)
        return;
    assert(nargs <= 6);
    mjs_val_t res;
    va_list ap;
    mjs_val_t args[6];
    va_start(ap, nargs);
    for (int i = 0; i < nargs; i++) {
      args[i] = va_arg(ap, mjs_val_t);
    }
    va_end(ap);
    if (mjs_call_func(mjs, &res, func, nargs, args) != MJS_OK)
    {
        LOGE("JSCall failed call function: [%s] msg: %s\n", func, mjs->error_msg);
        g_errorMsgs.insert(mjs->error_msg);
    }
}

void JSShowErrorMsg() {
    int yOffset = 0;
    for (auto msg : g_errorMsgs) {
        uint16_t width, height;
        calculateUTFStringSize(msg.c_str(), &width, &height);
        int totalHeight = (width / CUBICAT.lcd.width() + 1) * height;
        CUBICAT.lcd.fillRect(0, yOffset, CUBICAT.lcd.width(), totalHeight, BLACK);
        auto region = CUBICAT.lcd.drawText(0, yOffset, msg.c_str(), WHITE, 2);
        yOffset += region.y2 - region.y1 + 2;
    }
}