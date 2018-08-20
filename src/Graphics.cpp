#include "stdafx.h"
#include "Graphics.h"

namespace Bonny
{
  Graphics::Graphics(string name, HINSTANCE hinstance, HWND window) :
    m_name(name),
    m_hinstance(hinstance),
    m_window(window)
  {
  }


  Graphics::~Graphics()
  {
  }

  void Graphics::createDevice(uint32_t numFrames)
  {
  }

  void Graphics::createView(shared_ptr<View> view)
  {
  }

  void Graphics::resize(uint32_t width, uint32_t height)
  {
  }

  void Graphics::buildBuffers(vector<shared_ptr<RenderComponent>>& renderComponents)
  {
  }

  void Graphics::createPipeline(shared_ptr<Pipeline>, shared_ptr<Mesh>, shared_ptr<Material>)
  {
  }

  void Graphics::beginCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
  }

  void Graphics::bindPipeline(shared_ptr<View> view, shared_ptr<Pipeline>, uint32_t frameIndex)
  {
  }

  void Graphics::draw(shared_ptr<View> view, shared_ptr<Mesh>, shared_ptr<Material>, uint32_t frameIndex)
  {
  }

  void Graphics::compute(shared_ptr<View> view)
  {
  }

  void Graphics::trace(shared_ptr<View> view)
  {
  }

  void Graphics::endCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
  }

  void Graphics::executeCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
  }

  void Graphics::present(shared_ptr<View> view, uint32_t frameIndex)
  {
  }
}