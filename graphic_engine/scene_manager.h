/*
* @author       Isaac
* @date         2024-08-22
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
* @description  scene manager class for graphic engine, including 2d/3d objects create/destroy and update.
*/
#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
#include <vector>
#include <unordered_map>
#include "renderer/renderer.h"
#include "drawable/image_data.h"
#include "widgets/widget.h"
#include "node2d.h"
#include "resource_manager.h"

using namespace cubicat;

enum Layer2D {
    BACKGROUND,
    FOREGROUND,
    // UI,
    // OVERLAY
};

class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    WidgetPtr createUICanvas(uint16_t width, uint16_t height);
    // 2D nodes
    Node2DPtr createSpriteNode(const ImageData& imgData,const Vector2f& pivot = Vector2f(0.5, 0.5), Layer2D layer = FOREGROUND);
    Node2DPtr createQuad(const ImageData& imgData,const Vector2f& pivot = Vector2f(0.5, 0.5), Layer2D layer = FOREGROUND);
    // [JS_BINDING_BEGIN]
    Node2DPtr createSpriteNode(TexturePtr texture,const Vector2f& pivot = Vector2f(0.5, 0.5), Layer2D layer = FOREGROUND);
    Node2DPtr createQuad(TexturePtr texture, const Vector2f& pivot = Vector2f(0.5, 0.5), Layer2D layer = FOREGROUND);
    Node2DPtr createNode2D(Layer2D layer = FOREGROUND);
    NodePtr getObjectById(uint32_t id);
    NodePtr getObjectByName(const string& name);
    void deleteObject(uint32_t id);
    void addNode(NodePtr node);
    // [JS_BINDING_END]
    WidgetPtr getUICanvas() {return m_pUICanvas;}
    void update(float deltaTime);
    const std::vector<DrawablePtr>& getDrawables() {return m_drawables;}
private:
    Node2DPtr getRoot2D() {return m_pRoot2D;}
    WidgetPtr                                       m_pUICanvas;
    Node2DPtr                                       m_pBackgroud2D;
    Node2DPtr                                       m_pRoot2D;
    static SceneManager*                            m_sInstance;
    std::unordered_map<uint32_t, NodePtr>           m_nodeMap;
    std::unordered_map<std::string, uint32_t>       m_nameIdMap;
    std::vector<DrawablePtr>                        m_drawables;
};
#endif