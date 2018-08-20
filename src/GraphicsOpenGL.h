#pragma once
#pragma once
#include "Graphics.h"
#include "Mesh.h"
#include "Material.h"

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <iostream>
#include <sstream>
#include <set>
#include <array>
#include <stdio.h>
#include <fstream>
#include <cassert>

#include <GL/glew.h>
#include <gl/GL.h>

using std::string;
using std::shared_ptr;
using std::vector;
using std::queue;
using std::set;
using std::array;
using std::ifstream;

namespace Bonny
{
  class GraphicsOpenGL :
    public Graphics
  {
  public:
    GraphicsOpenGL(string name, HINSTANCE hinstance, HWND window);
    ~GraphicsOpenGL();

    void                createDevice(uint32_t numFrames);

    void                createView(shared_ptr<View> view);
    void                resize(uint32_t width, uint32_t height);

    void                buildBuffers(vector<shared_ptr<RenderComponent>>& renderComponents);
    void                createPipeline(shared_ptr<Pipeline>, shared_ptr<Mesh>, shared_ptr<Material>);

    void                beginCommands(shared_ptr<View> view, uint32_t frameIndex);
    void                bindPipeline(shared_ptr<View> view, shared_ptr<Pipeline> pipeline, uint32_t frameIndex);
    void                draw(shared_ptr<View> view, shared_ptr<Mesh>, shared_ptr<Material>, uint32_t frameIndex);
    void                compute(shared_ptr<View> view);
    void                trace(shared_ptr<View> view);
    void                endCommands(shared_ptr<View> view, uint32_t frameIndex);
    void                executeCommands(shared_ptr<View> view, uint32_t frameIndex);
    void                present(shared_ptr<View> view, uint32_t frameIndex);

  private:

    void                update(shared_ptr<View> view, shared_ptr<Material>);
    void                createMesh(shared_ptr<Mesh>);
    void                createMaterial(shared_ptr<Material>);

    // Per mesh graphics data
    struct glMeshData
    {
      GLuint  m_vertexArrayID;
      GLuint* m_vertexArrayBufferIDs;
    };

    // Per texture graphics data
    struct glTextureData
    {
      GLuint m_diffuseTextureID;
      GLuint m_diffuseTextureLocation;
    };

    // Per material graphics data
    struct glMaterialData
    {
      vector<char>  m_vertexShaderCode;
      GLint         m_vertexShaderCodeSize;
      vector<char>  m_fragmentShaderCode;
      GLint         m_fragmentShaderCodeSize;

      GLuint m_vertexShader;
      GLuint m_fragmentShader;
      GLuint m_shaderProgram;

      GLuint m_modelViewUniform;
      GLuint m_projectionUniform;
      GLuint m_normalMatrixUniform;

      // Material Info
      GLuint m_diffuseColorUniform;
      GLuint m_specularColorUniform;
      GLuint m_ambientColorUniform;
      GLuint m_emissiveColorUniform;
      GLuint m_transparentColorUniform;
      GLuint m_shininessUniform;

      struct Light
      {
        GLuint position;
        GLuint direction;
        GLuint ambient;
        GLuint diffuse;
        GLuint specular;
        GLuint attenuation;
      };

      Light m_lightsUniform[8];
      GLuint m_numLightsUniform;
    };

    // Per buffer graphics data
    struct glUniformBufferData
    {
      uint8_t*                      m_data;
    };

    // Data needed for back buffers
    struct glBackBuffer {
    };

    // Per View graphics data
    struct glViewData
    {
      uint32_t m_currentWidth;
      uint32_t m_currentHeight;
      uint32_t m_frameIndex;
    };

    struct glPipelineCacheInfo
    {
    };

    void          loadTexture(shared_ptr<Texture> texture);
    vector<char>  readFile(const string& filename);
    void          render(shared_ptr<Material> material);

    uint32_t  m_frameIndex;
    HINSTANCE m_hinstance;
    HWND      m_window;

    HGLRC    m_renderContext;
    HDC      m_deviceContext;
    GLuint   m_frameBufferTextureId;
    GLuint   m_frameBufferObjectId;
    GLuint   m_frameBufferDepthTextureId;
  };
}


