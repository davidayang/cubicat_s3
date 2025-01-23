#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
#include <vector>
#include <unordered_map>
#include "renderer.h"
#include "node.h"
#include "drawable/image_data.h"
#include "widgets/widget.h"

class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    WidgetPtr createUICanvas(uint16_t width, uint16_t height);
    NodePtr createSpriteNode(const ImageData& imgData,const Vector2& pivot = Vector2(0.5, 0.5),const char* name="");
    NodePtr createNode(const char* name = "");
    NodePtr createPloygon(const ImageData& imgData,const Vector2& pivot = Vector2(0.5, 0.5), const char* name = "");
    NodePtr getObjectById(unsigned int id);
    WidgetPtr getUICanvas() {return m_pUICanvas;}
    void deleteObject(unsigned int id);
    void update(float deltaTime);
    void setCameraPos(const Vector2& pos);
    const std::vector<DrawablePtr>& getDrawables() {return m_drawables;}
    NodePtr getRoot() {return m_pRoot;}
private:
    WidgetPtr                                       m_pUICanvas;
    NodePtr                                         m_pRoot;
    static SceneManager*                            m_sInstance;
    std::unordered_map<unsigned int, NodePtr>       m_NodeMap;
    std::vector<DrawablePtr>                        m_drawables;
};
#endif