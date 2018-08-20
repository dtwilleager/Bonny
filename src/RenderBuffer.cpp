#include "RenderBuffer.h"


namespace Bonny {
  RenderBuffer::RenderBuffer(uint32_t width, uint32_t height) :
    m_width(0),
    m_height(0),
    m_graphicsData(nullptr),
    m_depthAttatchment(nullptr)
  {
  }


  RenderBuffer::~RenderBuffer()
  {
  }

  uint32_t RenderBuffer::getWidth()
  {
    return m_width;
  }

  uint32_t RenderBuffer::getHeight()
  {
    return m_height;
  }

  void RenderBuffer::addColorAttatchment(RBFormat format)
  {
    RenderBuffer::RBAttatchment* colorAttatchment = new RBAttatchment();
    colorAttatchment->m_format = format;
    colorAttatchment->m_graphicsData = nullptr;
    m_colorAttatchments.push_back(colorAttatchment);
  }

  uint32_t RenderBuffer::getNumColorAttatchment()
  {
    return m_colorAttatchments.size();
  }

  RenderBuffer::RBAttatchment* RenderBuffer::getColorAttatchment(uint32_t index)
  {
    return m_colorAttatchments[index];
  }

  void RenderBuffer::createDepthAttatchment(RBFormat format)
  {
    m_depthAttatchment = new RBAttatchment();
    m_depthAttatchment->m_format = format;
    m_depthAttatchment->m_graphicsData = nullptr;
  }

  RenderBuffer::RBAttatchment* RenderBuffer::getDepthAttatchment(RBFormat format)
  {
    return m_depthAttatchment;
  }


  void RenderBuffer::resize(uint32_t width, uint32_t height)
  {
    m_width = width;
    m_height = height;
  }

  void RenderBuffer::setGraphicsData(void * graphicsData)
  {
    m_graphicsData = graphicsData;
  }

  void* RenderBuffer::getGraphicsData()
  {
    return m_graphicsData;
  }

}