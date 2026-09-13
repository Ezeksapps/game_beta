#pragma once

/* Every 'scene' is both the glTF with the visual scene and a JSON controlling the Entities and other metadata contained in a full scene
 * This class wraps all of this data and handles collisions. The Renderer can only hold one instance of a loaded scene
 */

#include "../entity/entity.hpp"
#include <cstdint>

struct Box2D { // 2D bound box used for quadtree and spatial hash
    vec2 min;
    vec2 max;

    Box2D includeVertex(const vec2& vertex) {
        min.x = std::min(min.x, vertex.x);
        min.y = std::min(min.y, vertex.y);
        max.x = std::max(max.x, vertex.x);
        max.y = std::max(max.y, vertex.y);
        return *this;
    }
};

struct AABB { // matching Diligent::BBox
    vec3 min;
    vec3 max;
};

// use the highest possible value of a uint16_t to represent null values
constexpr uint16_t nullNode = uint16_t(-1);

// each node of a quadtree may be split into four more nodes, default to having no child nodes
struct Node {
    uint16_t children[2][2]{
        {nullNode, nullNode},
        {nullNode, nullNode}
    };
};

struct Quadtree {
    AABB     bbox;            // region enclosed by this quadtree (should be dimensions of whole scene)
    uint16_t root;            // root node index
    std::vector<Node> nodes;  // all nodes
};

class Scene {

friend class Renderer; // TODO remove
//friend class Engine; // TODO remove

public:

    Scene(const std::string& sceneDir);
    ~Scene();

    // checks collisions between scene entities & 3D scene meshes
    void resolveCollisions();

    std::vector<std::shared_ptr<Entity>>& getEntities();

    std::vector<std::shared_ptr<Entity>> m_pEntities; // TEMP

    std::vector<AABB>                m_bboxes;

private:

    void loadSceneJson(const std::string& sceneFilepath);
    void genSceneQuadtree();

    std::string m_glbFilepath;

    Quadtree m_sceneQuadtree;
    // bounding boxes for all meshes within this scene
    std::vector<AABB> m_sceneBboxes;
    // walkable bounds of this scene, defined in JSON. Will be the bounds used for the quadtree
    std::array<vec2, 4> m_sceneBounds;
};
