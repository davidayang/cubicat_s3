#include "scene_manager.h"
#include <map>
#include "drawable/sprite.h"
#include "drawable/sprite_sheet.h"
#include "drawable/bmfont.h"
#include "drawable/gb2312font_20.h"

SceneManager* SceneManager::m_sInstance = nullptr;
SceneManager::SceneManager()
{
    m_pRoot =  Node::create("root");
}
SceneManager::~SceneManager() {
}
WidgetPtr SceneManager::createUICanvas(uint16_t width, uint16_t height) {
    m_pUICanvas = Widget::create(width, height);
    m_pUICanvas->setName("canvas");
    return m_pUICanvas;
}
NodePtr SceneManager::createNode(const char* name) {
    NodePtr node = Node::create(name);
    node->setParent(m_pRoot);
    m_NodeMap[node->getId()] = node;
    return node;
}
NodePtr SceneManager::createSpriteNode(const ImageData& img,const Vector2& pivot,const char* name) {
    DrawablePtr sprite;
    if (img.col > 1 || img.row > 1)
        sprite = SpriteSheet::create(img.width, img.height, img.data, img.col, img.row, img.hasMask, img.maskColor, img.palette, img.bpp);
    else 
        sprite = Sprite::create(img.width, img.height, img.data, img.hasMask, img.maskColor, img.palette, img.bpp);
    sprite->setPivot(pivot.x, pivot.y);
    auto node = createNode(name);
    node->attachDrawable(sprite);
    return node;
}
NodePtr SceneManager::createPloygon(const ImageData& img,const Vector2& pivot,const char* name) {
    auto texture = Texture::create(img.width, img.height, img.data, img.col, img.row, img.palette, img.bpp, img.hasAlpha);
    auto polygon = Polygon::create(texture->getFrameWidth(), texture->getFrameHeight(), img.hasMask, img.maskColor);
    polygon->setPivot(pivot.x, pivot.y);
    polygon->setTexture(texture);
    auto node = createNode(name);
    node->attachDrawable(polygon);
    return node;
}
NodePtr SceneManager::getObjectById(unsigned int id) {
    auto objItr = m_NodeMap.find(id);
    if (objItr != m_NodeMap.end()) {
        return objItr->second;
    }
    return NodePtr(nullptr);
}
void SceneManager::deleteObject(unsigned int id) {
    auto objItr = m_NodeMap.find(id);
    if (objItr != m_NodeMap.end()) {
        objItr->second->setParent(nullptr);
        m_NodeMap.erase(objItr);
    }
}

void SceneManager::update(float deltaTime) {
    m_drawables.clear();
    // update scene graph
    m_pRoot->update(deltaTime);
    if (m_pUICanvas)
        m_pUICanvas->update(deltaTime);
    // update and collect all drawables
    m_pRoot->findDrawables(m_drawables);
    if (m_pUICanvas) {
        m_pUICanvas->findDrawables(m_drawables);
    }
}

void SceneManager::setCameraPos(const Vector2& pos) {
    m_pRoot->setPosition(-pos);
}