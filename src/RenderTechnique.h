#pragma once

#include "RenderComponent.h"
#include "LightComponent.h"
#include "Entity.h"
#include "View.h"
#include "Graphics.h"
#include "UniformBuffer.h"
#include "RenderTechnique.h"
#include "WorldManager.h"

#include <string>
#include <memory>
#include <vector>
#include <atlstr.h>
#define _USE_MATH_DEFINES
#include <math.h>

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::vector;
using glm::ivec4;

namespace Bonny
{
  class Graphics;
  class RenderComponent;
  class LightComponent;
  class Mesh;
  class WorldManager;

  class RenderTechnique
  {
  public:
    RenderTechnique(string name, WorldManager* worldManager, HINSTANCE hinstance, HWND window, shared_ptr<Graphics> graphics);
    ~RenderTechnique();

    void addRenderComponent(shared_ptr<RenderComponent> renderComponent, shared_ptr<Entity> entity);
    void removeRenderComponent(shared_ptr<RenderComponent> renderComponent, shared_ptr<Entity> entity);
    void addLightComponent(shared_ptr<LightComponent> lightComponent, shared_ptr<Entity> entity);
    void removeLightComponent(shared_ptr<LightComponent> lightComponent, shared_ptr<Entity> entity);
    size_t getNumLightComponents();
    shared_ptr<LightComponent> getLightComponent(uint32_t index);
    void addView(shared_ptr<View> view);
    void removeView(shared_ptr<View> view);
    void updateWindow(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    virtual void build();
    virtual void render();

  protected:
    string                                m_name;
    HINSTANCE                             m_hinstance;
    HWND                                  m_window;
    shared_ptr<Graphics>                  m_graphics;

    WorldManager*                         m_worldManager;
    vector<shared_ptr<RenderComponent>>   m_renderComponents;
    vector<shared_ptr<Entity>>            m_renderEntities;
    vector<shared_ptr<LightComponent>>    m_lightComponents;
    vector<shared_ptr<Entity>>            m_lightEntities;
    vector<shared_ptr<View>>              m_views;
    shared_ptr<View>                      m_onscreenView;

  private:
    void setClusterEntityFreeze(bool freeze);
    void updateFrameData(uint32_t frameIndex);
    void updateClusterData(shared_ptr<View> view, uint32_t frameIndex);
    void updateCurrentLight(uint32_t frameIndex, int lightIndex);
    void renderMeshes(shared_ptr<View> view, uint32_t frameIndex);
    void updateMeshData(shared_ptr<View> view, uint32_t frameIndex);
    void updateMeshData(shared_ptr<View> view, shared_ptr<Mesh> mesh, shared_ptr<Entity> entity, uint32_t frameIndex, uint32_t meshIndex);
    void createCompositeMeshes();
    void buildFrustumLines(shared_ptr<View> view);
    vec4 planeEquation(vec3 p1, vec3 p2, vec3 p3);
    float updatePlaneD(vec4 plane, vec3 p);
    bool intersectsCluster(uint32_t clusterIndex, vec3 lightViewPosition, float radius);


    struct Light
    {
      mat4 light_view_projections[6];
      vec4 light_position;
      vec4 light_color;
    };

    struct FrameShaderParamBlock {
      ivec4 lightInfo;
      vec4 viewPosition;
      Light lights[6];
    };

    struct ObjectShaderParamBlock {
      mat4 model;
      mat4 view_projection;
      vec4 albedoColor;
      vec4 emmisiveColor;
      vec4 metallicRoughness;
      vec4 flags;
    };

    struct Cluster {
      uint32_t                            m_verts[8];
	    vec4	                              m_planes[6];
      vector<shared_ptr<LightComponent>>* m_lights;
    };

    struct ClusterData {
      uint32_t  m_numClusterVerts;
      vec3*     m_clusterVerts;
      float*    m_localVerts;
	    uint32_t  m_numXSegments;
	    uint32_t  m_numYSegments;
	    uint32_t  m_numZSegments;
      Cluster*  m_clusters;
    };


    vector<shared_ptr<UniformBuffer>>     m_frameDataUniformBuffers;
    vector<shared_ptr<UniformBuffer>>     m_objectDataUniformBuffers;
    size_t                                m_frameDataAlignedSize;
    size_t                                m_objectDataAlignedSize;
    size_t                                m_numMeshes;
    uint32_t*                             m_meshOffsets;
    int                                   m_currentLight;
    bool                                  m_depthPrepass;
    ClusterData*                          m_clusterData;
    shared_ptr<Entity>                    m_clusterEntity;
    bool                                  m_freezeClusterEntity;

    uint32_t                              m_frameIndex;
  };
}
