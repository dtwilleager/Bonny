#include "stdafx.h"
#include "RenderComponent.h"

namespace Bonny
{
  RenderComponent::RenderComponent(string name) : Component(name, Component::RENDER)
  {
  }


  RenderComponent::~RenderComponent()
  {
  }

  void RenderComponent::addMesh(shared_ptr<Mesh> mesh)
  {
    m_meshes.push_back(mesh);
    mesh->setRenderComponent(shared_from_this());
  }

  shared_ptr<Mesh> RenderComponent::getMesh(size_t index)
  {
    return m_meshes[index];
  }

  size_t RenderComponent::numMeshes()
  {
    return m_meshes.size();
  }

  void RenderComponent::setVisible(bool visible)
  {
    m_visible = visible;
  }

  bool RenderComponent::IsVisible()
  {
    return m_visible;
  }
}
