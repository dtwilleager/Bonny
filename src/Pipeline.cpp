#include "Pipeline.h"


namespace Bonny
{
  Pipeline::Pipeline(shared_ptr<Mesh> mesh, shared_ptr<Material> material):
    m_mesh(mesh),
    m_material(material),
    m_graphicsData(nullptr),
    m_dirty(true)
  {
  }

  Pipeline::~Pipeline()
  {
  }

  shared_ptr<Mesh> Pipeline::getMesh()
  {
    return m_mesh;
  }

  shared_ptr<Material>  Pipeline::getMaterial()
  {
    return m_material;
  }

  void Pipeline::setDirty(bool dirty)
  {
    m_dirty = dirty;
  }

  bool Pipeline::isDirty()
  {
    return m_dirty;
  }

  void Pipeline::setGraphicsData(void * graphicsData)
  {
    m_graphicsData = graphicsData;
  }

  void* Pipeline::getGraphicsData()
  {
    return m_graphicsData;
  }

}
