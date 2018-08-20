#pragma once

#include "View.h"
#include "RenderBuffer.h"

#include <memory>
#include <vector>
#include <string>

namespace Bonny
{
  class RenderScreenView: public View
  {
  public:
    RenderScreenView(string name);
    ~RenderScreenView();

    void                      setRenderBuffer(shared_ptr<RenderBuffer> renderBuffer);
    shared_ptr<RenderBuffer>  getRenderBuffer();

  private:
    shared_ptr<RenderBuffer> m_renderBuffer;
  };
}

