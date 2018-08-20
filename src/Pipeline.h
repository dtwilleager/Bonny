#pragma once

#include "Mesh.h"
#include "Material.h"

#include <string>
#include <memory>

using std::string;
using std::shared_ptr;

namespace Bonny
{
  class Pipeline
  {
  public:
    Pipeline(shared_ptr<Mesh> mesh, shared_ptr<Material> material);
    ~Pipeline();

    shared_ptr<Mesh>      getMesh();
    shared_ptr<Material>  getMaterial();
    void                  setDirty(bool dirty);
    bool                  isDirty();
    void                  setGraphicsData(void * graphicsData);
    void*                 getGraphicsData();

  private:
    shared_ptr<Mesh>      m_mesh;
    shared_ptr<Material>  m_material;
    bool                  m_dirty;
    void*                 m_graphicsData;
  };
}

