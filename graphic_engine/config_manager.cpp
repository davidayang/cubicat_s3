#include "config_manager.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
typedef std::map<unsigned int,ItemTable>  ConfigMap;
ConfigMap   g_ConfigMaps;
unsigned int _StringHash(const char* str)
{
    unsigned int seed = 131;
    unsigned int hash = 0;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}
const char* KeyPairs::GetValue(const char* key) const
{
    int k = _StringHash(key);
    std::map<int,std::string>::const_iterator itr = m_KPMap.find(k);
    if (itr != m_KPMap.end())
        return itr->second.c_str();
    return NULL;
}

void KeyPairs::SetValue(const char* key,const char* value)
{
    int k = _StringHash(key);
    m_KPMap[k] = value;
}
const KeyPairs* ItemTable::GetKeyPairsByLevel(const char* name,int lvl) const
{
    assert(lvl > 0);
    if (lvl > 0)
    {
        lvl -= 1;
        std::map<std::string,KeyPairsList >::const_iterator itr = m_Table.find(name);
        if (itr != m_Table.end())
        {
            const KeyPairsList& kpList = itr->second;
            if (lvl < kpList.Count())
                return &kpList.Get(lvl);
        }
    }
    return NULL;
}
KeyPairsList::KeyPairsList()
:m_iCount(0),m_pKPList(NULL)
{
}
KeyPairsList::~KeyPairsList()
{
    if (m_pKPList)
    {
        delete[] m_pKPList;
        m_pKPList = NULL;
    }
}
void KeyPairsList::Push(const KeyPairs &kp)
{
    KeyPairs* temp = new KeyPairs[m_iCount + 1];
    for (int i=0;i<m_iCount;++i)
        temp[i] = m_pKPList[i];
    temp[m_iCount] = kp;
    if (m_pKPList)
        delete [] m_pKPList;
    m_pKPList = temp;
    m_iCount++;
}
KeyPairsList& KeyPairsList::operator=(const KeyPairsList &ref)
{
    int count = ref.Count();
    if (count)
    {
        if (m_pKPList)
            delete [] m_pKPList;
        m_pKPList = new KeyPairs[count];
        for (int i=0;i<count;++i)
            m_pKPList[i] = ref.Get(i);
        m_iCount = count;
    }
    return *this;
}
int KeyPairsList::Count() const
{
    return m_iCount;
}
void KeyPairsList::SetValue(int idx,const char* key,const char* value)
{
    assert(idx < m_iCount);
    m_pKPList[idx].SetValue(key, value);
}

std::vector<std::string> ItemTable::GetNames() const
{
    std::vector<std::string> ret;
    std::map<std::string,KeyPairsList >::const_iterator itr = m_Table.begin();
    for (;itr != m_Table.end();++itr)
        if (itr->first != "string" && itr->first != "int")
            ret.push_back(itr->first);
    return ret;
}
ItemTable& ItemTable::operator= (const ItemTable& ref)
{
    std::map<std::string,KeyPairsList>::const_iterator itr = ref.m_Table.begin();
    for (;itr != ref.m_Table.end();++itr)
        m_Table[itr->first] = itr->second;
    return *this;
}
int ItemTable::GetTableCount()
{
    return (int)m_Table.size();
}
int ItemTable::GetItemLevelCount(const char* name) const
{
    std::map<std::string,KeyPairsList >::const_iterator itr = m_Table.find(name);
    if (itr != m_Table.end())
        return itr->second.Count();
    return 0;
}
void ItemTable::InsertKeyPair(const char* name,int idx,const char* key,const char* value)
{
    KeyPairsList& kplist = m_Table[std::string(name)];
    if (idx >= kplist.Count())
    {
        KeyPairs kp = KeyPairs();
        kp.SetValue(key,value);
        kplist.Push(kp);
    }
    else 
        kplist.SetValue(idx,key,value);
}

void LoadConfigFromBuffer(const char* cfgName,const char* buffer,unsigned int buffLen)
{
    unsigned int cfgNameHash = _StringHash(cfgName);
    ConfigMap::iterator cfg_itr = g_ConfigMaps.find(cfgNameHash);
    if (cfg_itr != g_ConfigMaps.end())
        return;
    ItemTable  attMap;
    std::vector<std::string>  attNameVec;
    int lvl = 0;
    int lineCount = 0;
    int columIndex = 0;
    int startPos = 0;
    int endPos = 0;
    std::string category;
    bool bStartRecord = false;
    for (int i=0;i<buffLen;++i)
    {
        char letter = buffer[i];
        bool lineEnd = false;
        bool changeColum = false;
        bool fileEnd = i == (buffLen-1);
        if (letter == '\n' || letter == '\r')
        {
            lineEnd = true;
            if (bStartRecord)
            {
                bStartRecord = false;
                endPos = i;
            }
        }
        else if (letter == ',')
        {
            if (bStartRecord)
            {
                bStartRecord = false;
                endPos = i;
            }
            changeColum = true;
        }
        else // 普通字符
        {
            if (!bStartRecord)
            {
                bStartRecord = true;
                startPos = i;
            }
            if (fileEnd) {
                endPos = i+1;
            }
        }
        if (endPos > startPos)
        {
            int len = endPos-startPos;
            char* val = (char*)malloc(len+1);
            val[len] = 0;
            memcpy(val, &buffer[startPos], len);
            if (columIndex == 0)
            {
                lvl = 0;
                category = val;
            }
            if (lineCount == 0)
                attNameVec.push_back(val);
            else if (columIndex > 0)
                attMap.InsertKeyPair(category.c_str(), lvl, attNameVec[columIndex].c_str(),val);
            free(val);
            startPos = endPos;
        } else if (columIndex == 0) {
            lvl++;
        }
        if (lineEnd)
        {
            lineCount++;
            columIndex = 0;
        }
        if (changeColum)
        {
            columIndex++;
        }
    }
    g_ConfigMaps[cfgNameHash] = attMap;
}
const ItemTable* GetItemTable(const char* configName)
{
    ConfigMap::const_iterator itr = g_ConfigMaps.find(_StringHash(configName));
    if (itr != g_ConfigMaps.end())
        return &itr->second;
    else
        return NULL;
}
const KeyPairs* GetKeyPairs(const char* configName,const char* itemId, int lvl)
{
    const ItemTable* pTable = GetItemTable(configName);
    if (pTable)
        return pTable->GetKeyPairsByLevel(itemId, lvl);
    else
        return NULL;
}
const char* GetValue(const char* configName,const char* itemId,int lvl,const char* attributeName,bool bUseLevel0AsDefault)
{
    const KeyPairs* kp = GetKeyPairs(configName, itemId, lvl);
    if (kp)
    {
        const char* val = kp->GetValue(attributeName);
        if (val)
            return val;
        if (bUseLevel0AsDefault)
        {
            kp = GetKeyPairs(configName, itemId, 1);
            if (kp)
                return kp->GetValue(attributeName);
        }
    }
    return NULL;
}
int GetItemLevelCount(const char* configName,const char* itemId)
{
    const ItemTable* pTable = GetItemTable(configName);
    if (pTable)
        return pTable->GetItemLevelCount(itemId);
    return 0;
}

std::vector<std::string> GetItemNames(const char* configName)
{
    const ItemTable* pTable = GetItemTable(configName);
    if (pTable)
        return pTable->GetNames();
    std::vector<std::string> empty;
    return empty;
}
int GetGlobalValue(const char*name)
{
    int val = atoi(GetValue("Globals.csv", name, 1, "Value"));
    return val;
}