#ifndef _ConfigManager_H_
#define _ConfigManager_H_
#include <map>
#include <vector>
#include <string>
#include <stdio.h>

class KeyPairs
{
public:
    const char* GetValue(const char* key) const;
    void SetValue(const char* key,const char* value);
private:
    std::map<int,std::string>  m_KPMap;
};

class KeyPairsList
{
public:
    KeyPairsList();
    ~KeyPairsList();
    KeyPairsList& operator= (const KeyPairsList& ref);
    const KeyPairs& Get(int i) const {return m_pKPList[i];}
    void SetValue(int idx,const char* key,const char* value);
    int Count() const;
    void Push(const KeyPairs& kp);
private:
    int             m_iCount;
    KeyPairs*       m_pKPList;
};

class ItemTable
{
public:
    ItemTable& operator= (const ItemTable& ref);
    const KeyPairs* GetKeyPairsByLevel(const char* name,int lvl) const;
    std::vector<std::string> GetNames() const;
    int GetTableCount();
    int GetItemLevelCount(const char* name) const;
    //internal use only ,do not call directly
    void InsertKeyPair(const char* name,int idx,const char* key,const char* value);
private:
    //key : template name
    std::map<std::string, KeyPairsList > m_Table;
};

extern void LoadConfigFromBuffer(const char* filename,const char* buffer,unsigned int buffLen);
extern const ItemTable* GetItemTable(const char* configName);
extern const KeyPairs* GetKeyPairs(const char* configName,const char* categoryName,int lvl);
extern const char* GetValue(const char* configName,const char* categoryName,int lvl,const char* key,bool bUseLevel0AsDefault = true);
extern int GetItemLevelCount(const char* configName,const char* categoryName);
extern std::vector<std::string> GetItemNames(const char* configName);
extern int GetGlobalValue(const char* name);

#endif