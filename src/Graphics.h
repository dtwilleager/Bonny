#pragma once
#include "stdafx.h"

#include "Mesh.h"
#include "UniformBuffer.h"
#include "View.h"
#include "Pipeline.h"

#include <string>
#include <memory>
#include <vector>

using std::string;
using std::shared_ptr;
using std::vector;

namespace Bonny
{
  class RenderTechnique;
  class Pipeline;
  class RenderComponent;

  class Graphics
  {
  public:
    Graphics(string name, HINSTANCE hinstance, HWND window);
    ~Graphics();

    virtual void                createDevice(uint32_t numFrames);

    virtual void                createView(shared_ptr<View> view);
    virtual void                resize(uint32_t width, uint32_t height);

    virtual void                buildBuffers(vector<shared_ptr<RenderComponent>>& renderComponents);
    virtual void                createPipeline(shared_ptr<Pipeline>, shared_ptr<Mesh>, shared_ptr<Material>);

    virtual void                beginCommands(shared_ptr<View> view, uint32_t frameIndex);
    virtual void                bindPipeline(shared_ptr<View> view, shared_ptr<Pipeline> pipeline, uint32_t frameIndex);
    virtual void                draw(shared_ptr<View> view, shared_ptr<Mesh>, shared_ptr<Material>, uint32_t frameIndex);
    virtual void                compute(shared_ptr<View> view);
    virtual void                trace(shared_ptr<View> view);
    virtual void                endCommands(shared_ptr<View> view, uint32_t frameIndex);
    virtual void                executeCommands(shared_ptr<View> view, uint32_t frameIndex);
    virtual void                present(shared_ptr<View> view, uint32_t frameIndex);


  protected:
    HINSTANCE                     m_hinstance;
    HWND                          m_window;
    uint32_t                      m_width;
    uint32_t                      m_height;

  private:
    string                        m_name;
  };
}
