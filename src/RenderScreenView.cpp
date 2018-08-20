#include "RenderScreenView.h"


namespace Bonny {
  RenderScreenView::RenderScreenView(string name): View(name),
    m_renderBuffer(nullptr)
  {
  }


  RenderScreenView::~RenderScreenView()
  {
  }

  void RenderScreenView::setRenderBuffer(shared_ptr<RenderBuffer> renderBuffer)
  {
    m_renderBuffer = renderBuffer;
  }

  shared_ptr<RenderBuffer> RenderScreenView::getRenderBuffer()
  {
    return m_renderBuffer;
  }
}
