#include "scene_manager.h"
#include <map>
#include "drawable/bmfont.h"
#include "drawable/gb2312font_20.h"
#include "drawable/quad.h"
#include "component/sheet_animation_component.h"

using namespace cubicat;

SceneManager *SceneManager::m_sInstance = nullptr;
SceneManager::SceneManager()
{
    m_pBackgroud2D = Node2D::create("background2d");
    m_pRoot2D = Node2D::create("root2d");
}
SceneManager::~SceneManager()
{
}
WidgetPtr SceneManager::createUICanvas(uint16_t width, uint16_t height)
{
    m_pUICanvas = Widget::create(width, height);
    m_pUICanvas->setName("canvas");
    return m_pUICanvas;
}
void SceneManager::addNode(NodePtr node)
{
    if (node) {
        if (!node->getParent()) {
            if (node->cast<Node2D>()) {
                node->setParent(m_pRoot2D);
            }
        }
        m_nodeMap[node->getId()] = node;
        if (node->getName().size()) {
            m_nameIdMap[node->getName()] = node->getId();
        }
    }
}
Node2DPtr SceneManager::createNode2D(Layer2D layer)
{
    Node2DPtr node = Node2D::create();
    addNode(node);
    if (layer == BACKGROUND) {
        node->setParent(m_pBackgroud2D);
    }
    return node;
}

Node2DPtr SceneManager::createSpriteNode(const ImageData &img, const Vector2f &pivot, Layer2D layer)
{
    DrawablePtr sprite = Drawable::create();
    auto tex = Texture::create(img.width, img.height, img.data, false, img.col, img.row, img.palette, img.bpp, img.hasAlpha);
    sprite->getMaterial()->setTexture(tex);
    sprite->setPivot(pivot.x, pivot.y);
    auto node = createNode2D(layer);
    node->attachDrawable(sprite);
    return node;
}
Node2DPtr SceneManager::createSpriteNode(TexturePtr texture,const Vector2f& pivot, Layer2D layer) {
    DrawablePtr sprite = Drawable::create();
    sprite->getMaterial()->setTexture(texture);
    sprite->setPivot(pivot.x, pivot.y);
    auto node = createNode2D(layer);
    node->attachDrawable(sprite);
    if (texture && texture->getFrameCount() > 1) {
        auto animSheet = SheetAnimationComponent::create();
        node->addComponent(animSheet);
    }
    return node;
}
Node2DPtr SceneManager::createQuad(const ImageData &img, const Vector2f &pivot, Layer2D layer)
{
    auto texture = Texture::create(img.width, img.height, img.data, false, img.col, img.row, img.palette, img.bpp, img.hasAlpha);
    return createQuad(texture, pivot, layer);
}

Node2DPtr SceneManager::createQuad(TexturePtr texture, const Vector2f &pivot, Layer2D layer)
{
    auto quad = Quad::create(texture->getTextureSize().x, texture->getTextureSize().y);
    quad->setPivot(pivot.x, pivot.y);
    quad->getMaterial()->setTexture(texture);
    auto node = createNode2D(layer);
    node->attachDrawable(quad);
    return node;
}

NodePtr SceneManager::getObjectById(uint32_t id)
{
    auto objItr = m_nodeMap.find(id);
    if (objItr != m_nodeMap.end())
    {
        return objItr->second;
    }
    return NodePtr(nullptr);
}
NodePtr SceneManager::getObjectByName(const string &name)
{
    auto objItr = m_nameIdMap.find(name);
    if (objItr != m_nameIdMap.end())
    {
        return getObjectById(objItr->second);
    }
    return NodePtr(nullptr);
}
void SceneManager::deleteObject(uint32_t id)
{
    auto objItr = m_nodeMap.find(id);
    if (objItr != m_nodeMap.end())
    {
        objItr->second->setParent(nullptr);
        m_nodeMap.erase(objItr);
        m_nameIdMap.erase(objItr->second->getName());
    }
}

void SceneManager::update(float deltaTime)
{
    m_drawables.clear();
    // update scene graph
    m_pBackgroud2D->update(deltaTime, false);
    m_pRoot2D->update(deltaTime, false);
    if (m_pUICanvas)
        m_pUICanvas->update(deltaTime, false);
    // find all 2d drawables
    m_pBackgroud2D->findDrawables(m_drawables);
    m_pRoot2D->findDrawables(m_drawables);
    if (m_pUICanvas)
        m_pUICanvas->findDrawables(m_drawables);
}