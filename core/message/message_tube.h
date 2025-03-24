#ifndef _MESSAGE_TUBE_H_
#define _MESSAGE_TUBE_H_
#include "message.h"
#include <unordered_map>
#include <vector>
#include <set>

namespace cubicat {

class MessageTube;
class MessageReceiver {
public:
    MessageReceiver();
    virtual ~MessageReceiver();
    virtual void onMessage(int id, const void* msg) = 0;
    void registerGlobalMessage(int id);
    void unregisterGlobalMessage(int id);
    void registerLocalMessage(MessageTube* tube, int id);
    void unregisterLocalMessage(MessageTube* tube, int id);
private:
    std::set<MessageTube*>      m_MsgTubes;
};

class MessageDispatcher {
public:
    // [JS_BINDING_BEGIN]
    static void sendGlobalMessage(int id, const void* msg);
    static void sendLocalMessage(MessageTube* tube, int id, const void* msg);
    // [JS_BINDING_END]
};

class MessageTube {
    friend class MessageReceiver;
private:
    std::unordered_map<int,std::set<MessageReceiver*>>   m_Receives;
    void registerMessage(int id, MessageReceiver* receiver);
    void unregisterMessage(int id, MessageReceiver* receiver);
    void removeReceiver(MessageReceiver* receiver);
public:
    void broadcastMessage(int id, const void* msg);
};

}
#endif