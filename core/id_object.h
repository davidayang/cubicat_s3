#ifndef _ID_OBJECT_H_
#define _ID_OBJECT_H_

class IDObject
{
public:
    unsigned int getId() { return m_nId; }
protected:
    IDObject();
private:
    unsigned int        m_nId;
    static unsigned int m_nIdCounter;
};

#endif