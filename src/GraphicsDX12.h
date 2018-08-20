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
#include <map>

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12SDKLayers.h>

using std::string;
using std::shared_ptr;
using std::vector;
using std::queue;
using std::set;
using std::array;
using std::ifstream;
using std::map;
using Microsoft::WRL::ComPtr;

namespace Bonny
{
  class GraphicsDX12 :
    public Graphics
  {
  public:
    GraphicsDX12(string name, HINSTANCE hinstance, HWND window);
    ~GraphicsDX12();

    void                createDevice(uint32_t numFrames);

    void                createView(shared_ptr<View> view);
    void                resize(uint32_t width, uint32_t height);

    void                buildBuffers(vector<shared_ptr<RenderComponent>>& renderComponents);
    void                buildMaterial(shared_ptr<Material>);
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
    HRESULT             createAdapter();
    void                enableDebugLayer();
    void                enableDeveloperMode();
    HRESULT             createCommandQueue();
    void                flushCommandQueue();
    HRESULT             createSwapchain(uint32_t numFrames);

    ComPtr<ID3D12Resource>  createAndUploadBuffer(uint32_t size, void* data, D3D12_HEAP_TYPE heapFlags, ComPtr<ID3D12Resource>& uploadBuffer);
    ComPtr<ID3DBlob>    loadShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,
      const std::string& target);
    
    void                printLog(string s);
    void                update(shared_ptr<View> view, shared_ptr<Material>);

    // Vertex Structures
    struct Vertex4
    {
      DirectX::XMFLOAT3 position;
      DirectX::XMFLOAT3 normal;
      DirectX::XMFLOAT2 texCoord;
      DirectX::XMFLOAT3 tangent;
    };

    struct Vertex3
    {
      DirectX::XMFLOAT3 position;
      DirectX::XMFLOAT3 normal;
      DirectX::XMFLOAT2 texCoord;
    };

    struct Vertex2
    {
      DirectX::XMFLOAT3 position;
      DirectX::XMFLOAT3 normal;
    };

    // Per mesh graphics data
    struct Dx12MeshData
    {
      uint32_t m_vertexStart;
      uint32_t m_numVertices;
      uint32_t m_indexStart;
      uint32_t m_numIndeces;
    };

    // Per texture graphics data
    struct Dx12TextureData
    {

    };

    // Per material graphics data
    struct Dx12MaterialData
    {
      ComPtr<ID3DBlob> m_vsByteCode = nullptr;
      ComPtr<ID3DBlob> m_psByteCode = nullptr;
    };

    // Per buffer graphics data
    struct Dx12ConstantBufferData
    {
    };

    // Per View graphics data
    struct Dx12ViewData
    {
      uint32_t m_currentWidth;
      uint32_t m_currentHeight;
      uint32_t m_frameIndex;
    };

    struct Dx12SwapchainBufferData
    {
      D3D12_RESOURCE_STATES   m_usageState;
      uint32_t                m_width;
      uint32_t                m_height;
      uint32_t                m_arraySize;
      DXGI_FORMAT             m_format;
      ComPtr<ID3D12Resource>  m_resource;
      D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
    };

    struct FrameData
    {
      ComPtr<ID3D12CommandAllocator>      m_graphicsAllocator;
      uint64_t                            m_currentFence;
    };

    uint32_t  m_frameIndex;
    HINSTANCE m_hinstance;
    HWND      m_window;
    uint32_t  m_numFrames;

    DXGI_FORMAT m_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ComPtr<IDXGIFactory4>   m_dxgiFactory;
    ComPtr<IDXGIAdapter1>   m_adapter;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D12Device>    m_device;

    uint32_t                m_rtvDescriptorSize = 0;
    uint32_t                m_dsvDescriptorSize = 0;
    uint32_t                m_cbvSrvUavDescriptorSize;

    vector<Dx12SwapchainBufferData*>    m_swapchainBufferResources;
    vector<FrameData*>                  m_frameData;
    Dx12SwapchainBufferData*            m_depthStencilBufferResources;
    ComPtr<ID3D12DescriptorHeap>        m_swapchainBufferRTVHeap;
    ComPtr<ID3D12DescriptorHeap>        m_swapchainBufferDSVHeap;

    ComPtr<ID3D12CommandAllocator>      m_graphicsCommandAllocator;
    ComPtr<ID3D12CommandQueue>          m_graphicsCommandQueue;
    ComPtr<ID3D12GraphicsCommandList>   m_graphicsCommandList;
    ComPtr<ID3D12Fence>                 m_graphicsFence;
    uint64_t                            m_currentGraphicsFence;
    HANDLE                              m_graphicsFenceEventHandle;

    D3D12_VIEWPORT                      m_screenViewport;
    D3D12_RECT                          m_scissorRect;

    uint32_t                            m_vertexComps[3];

    ComPtr<ID3D12CommandAllocator>      m_resourceCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList>   m_resourceCommandList;

    ComPtr<ID3D12Resource>              m_vertexBuffer;
    ComPtr<ID3D12Resource>              m_indexBuffer;

    map<std::wstring, ComPtr<ID3DBlob>>  m_vsMap;
    map<std::wstring, ComPtr<ID3DBlob>>  m_psMap;
  };
}


