#ifndef _DEVICES_H_
#define _DEVICES_H_
#include "display.h"
#ifdef CONFIG_USE_AUDIO_CODEC
#include "audio_codec.h"
#else
#include "microphone.h"
#include "speaker.h"
#endif
#include "wifi.h"
#include "unified_storage.h"
#include "ble_client.h"
#endif