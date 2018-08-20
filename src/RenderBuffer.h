#pragma once

#include <memory>
#include <vector>

namespace Bonny
{
  class RenderBuffer
  {
  public:
    RenderBuffer(uint32_t width, uint32_t height);
    ~RenderBuffer();

    enum RBFormat {
      RB_FLOAT_32,
      RB_FLOAT_R16G16B16A16,
      RB_UNORM_R8G8B8A8,
      RB_UNORM_RGBA,
      RB_UNORM_BGRA
    };

    struct RBAttatchment {
      RBFormat  m_format;
      void*     m_graphicsData;
    };

    uint32_t        getWidth();
    uint32_t        getHeight();
    void            addColorAttatchment(RBFormat format);
    uint32_t        getNumColorAttatchment();
    RBAttatchment*  getColorAttatchment(uint32_t index);
    void            createDepthAttatchment(RBFormat format);
    RBAttatchment*  getDepthAttatchment(RBFormat format);

    void            resize(uint32_t width, uint32_t height);
    void            setGraphicsData(void * graphicsData);
    void*           getGraphicsData();

  private:
    uint32_t                    m_width;
    uint32_t                    m_height;
    std::vector<RBAttatchment*> m_colorAttatchments;
    RBAttatchment*              m_depthAttatchment;
    void*                       m_graphicsData;
  };
}

