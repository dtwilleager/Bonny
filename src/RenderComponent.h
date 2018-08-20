#pragma once
#include "Component.h"
#include "Mesh.h"
#include "Graphics.h"
#include "UniformBuffer.h"
#include "View.h"
#include "LightComponent.h"

using std::enable_shared_from_this;

namespace Bonny 
{
  class Graphics;
  class Mesh;

  class RenderComponent :
    public Component, public enable_shared_from_this<RenderComponent>
  {
  public:
    RenderComponent(string name);
    ~RenderComponent();

    void                addMesh(shared_ptr<Mesh> mesh);
    shared_ptr<Mesh>    getMesh(size_t index);
    size_t              numMeshes();
    void                setVisible(bool visible);
    bool                IsVisible();

  private:
    vector<shared_ptr<Mesh>>  m_meshes;
    bool                      m_visible;
  };
}

