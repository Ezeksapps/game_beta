#pragma once

/* Every 'scene' is both the glTF with the visual scene and a JSON controlling the Entities and other metadata contained in a full scene
 * This class wraps all of this data and handles collisions. The Renderer can only hold one instance of a loaded scene
 */

#include "../entity/entity.hpp"
#include "glm/fwd.hpp"
#include <cstdint>


// quadtrees not being worked on currently, sptl hash WIP

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
    AABB     bbox;                         // region enclosed by this quadtree (should be dimensions of whole scene)
    uint16_t root;                         // root node index
    std::vector<Node> nodes;               // all nodes
    std::vector<uint16_t> nodeBBoxesBegin; // start indexes for AABBs corresponding to each node
    std::vector <AABB> bboxes;             // scene bounding boxes (contents of m_bboxes will be transferred to here upon build of quadtree)

    void build(std::vector<AABB>& bboxes);
};

class Scene {

friend class Renderer; // TODO remove

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
    void genSceneSptlHash();

    std::unordered_map<uint32_t, std::vector<std::shared_ptr<Entity>>> m_sptlHash;

    std::string m_glbFilepath;

    Quadtree m_sceneQuadtree;
    // bounding boxes for all meshes within this scene
    std::vector<AABB> m_sceneBboxes;
    // walkable bounds of this scene, defined in JSON. Will be the bounds used for the quadtree
    std::array<vec2, 4> m_sceneBounds;
};
