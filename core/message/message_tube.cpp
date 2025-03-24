#include "message_tube.h"
#include <algorithm>

using namespace cubicat;

MessageTube* g_MsgTube = new MessageTube();
// ====dispatcher====
void MessageDispatcher::sendGlobalMessage(int id, const void* msg) {
    g_MsgTube->broadcastMessage(id, msg);
}
void MessageDispatcher::sendLocalMessage(MessageTube* tube, int id, const void* msg) {
    tube->broadcastMessage(id, msg);
}
// ====receiver======
MessageReceiver::MessageReceiver() {
    m_MsgTubes.insert(g_MsgTube);
}
MessageReceiver::~MessageReceiver() {
    for (auto tube : m_MsgTubes) {
        tube->removeReceiver(this);
    }
}
void MessageReceiver::registerGlobalMessage(int id) {
    g_MsgTube->registerMessage(id, this);
}
void MessageReceiver::unregisterGlobalMessage(int id) {
    for (auto tube : m_MsgTubes) {
        tube->unregisterMessage(id, this);
    }
}
void MessageReceiver::registerLocalMessage(MessageTube* tube, int id) {
    tube->registerMessage(id, this);
}
void MessageReceiver::unregisterLocalMessage(MessageTube* tube, int id) {
    tube->unregisterMessage(id, this);
}
// ====message tube==
void MessageTube::registerMessage(int id, MessageReceiver* receiver) {
    auto itr = m_Receives.find(id);
    if (itr != m_Receives.end()) {
        itr->second.insert(receiver);
    } else {
        std::set<MessageReceiver*> receiverSet;
        receiverSet.insert(receiver);
        m_Receives.emplace(id, receiverSet);
    }
}
void MessageTube::unregisterMessage(int id, MessageReceiver* receiver) {
    auto itr = m_Receives.find(id);
    if (itr != m_Receives.end()) {
        auto it = std::find(itr->second.begin(), itr->second.end(), receiver);
        if (it != itr->second.end()) {
            itr->second.erase(it);
        }
    }
}
void MessageTube::removeReceiver(MessageReceiver* receiver) {
    for (auto pair : m_Receives) {
        unregisterMessage(pair.first, receiver);
    }
}
void MessageTube::broadcastMessage(int id, const void* msg) {
    auto itr = m_Receives.find(id);
    if (itr != m_Receives.end()) {
        for (auto r : itr->second) {
            r->onMessage(id, msg);
        }
    }
}