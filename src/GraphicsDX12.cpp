#include "stdafx.h"
#include "GraphicsDX12.h"
#include "LightComponent.h"
#include <DirectXColors.h>
#include "d3dx12.h"

#include <atlstr.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Bonny
{
  GraphicsDX12::GraphicsDX12(string name, HINSTANCE hinstance, HWND window) : Graphics(name, hinstance, window),
    m_frameIndex(0),
    m_hinstance(hinstance),
    m_window(window),
    m_currentGraphicsFence(0)
  {
    m_vertexComps[0] = 0;
    m_vertexComps[1] = 0;
    m_vertexComps[2] = 0;
  }


  GraphicsDX12::~GraphicsDX12()
  {
  }

  void GraphicsDX12::createDevice(uint32_t numFrames)
  {
    enableDebugLayer();
    if (createAdapter() != S_OK)
    {
      return;
    }
    enableDeveloperMode();
    if (createCommandQueue() != S_OK)
    {
      return;
    }
    if (createSwapchain(numFrames) != S_OK)
    {
      return;
    }
    resize(m_width, m_height);
  }

  HRESULT GraphicsDX12::createAdapter()
  {
    HRESULT hr = E_FAIL;

    // Obtain the DXGI factory
    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory));
    if (!SUCCEEDED(hr))
    {
      printLog("Error: Unable to create DXGI Factory");
      return hr;
    }

    SIZE_T MaxSize = 0;
    Microsoft::WRL::ComPtr<ID3D12Device> device;

    for (uint32_t index = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(index, &m_adapter); ++index)
    {
      DXGI_ADAPTER_DESC1 desc;
      m_adapter->GetDesc1(&desc);
      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      {
        continue;
      }

      if (desc.DedicatedVideoMemory > MaxSize &&
        SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device))))
      {
        m_adapter->GetDesc1(&desc);
        printLog("D3D12 capable hardware found: " + std::string((char*)desc.Description) + "(" + std::to_string(desc.DedicatedVideoMemory) + "MB)");
        MaxSize = desc.DedicatedVideoMemory;
      }

      if (MaxSize > 0)
      {
        m_device = device.Detach();

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        hr = S_OK;
      }
      else
      {
        printLog("No D3D12 capable hardware found");
        hr = E_FAIL;
      }
    }
    return hr;
  }

  void GraphicsDX12::enableDebugLayer()
  {
#if _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
      debugInterface->EnableDebugLayer();
    }
    else
    {
      printLog("WARNING: Unable to enable D3D12 debug validation layer");
    }
#endif
  }

  void GraphicsDX12::enableDeveloperMode()
  {
#ifndef RELEASE
    bool DeveloperModeEnabled = false;

    // Look in the Windows Registry to determine if Developer Mode is enabled
    HKEY hKey;
    LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS)
    {
      DWORD keyValue, keySize = sizeof(DWORD);
      result = RegQueryValueEx(hKey, "AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
      if (result == ERROR_SUCCESS && keyValue == 1)
      {
        DeveloperModeEnabled = true;
      }
      RegCloseKey(hKey);
    }


    // Prevent the GPU from overclocking or underclocking to get consistent timings
    if (DeveloperModeEnabled)
    {
      m_device->SetStablePowerState(TRUE);
    }
#endif
  }

  HRESULT GraphicsDX12::createCommandQueue()
  {
    HRESULT hr = S_OK;
    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    QueueDesc.NodeMask = 1;
    hr = m_device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_graphicsCommandQueue));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create graphics command queue");
      return hr;
    }
    m_graphicsCommandQueue->SetName(L"Direct Graphics Command Queue");

    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_graphicsFence));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create graphics fence");
      return hr;
    }
    m_graphicsFence->SetName(L"Direct Graphics Fence");
    m_graphicsFence->Signal((uint64_t)D3D12_COMMAND_LIST_TYPE_DIRECT << 56);

    m_graphicsFenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    if (m_graphicsFenceEventHandle == INVALID_HANDLE_VALUE)
    {
      printLog("ERROR: Could not create graphics fence");
      return E_FAIL;
    }

    hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_graphicsCommandAllocator));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create graphics allocator");
      return hr;
    }
    m_graphicsCommandAllocator->SetName(L"Direct Graphics Allocator");

    hr = m_device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,
      m_graphicsCommandAllocator.Get(),
      nullptr,
      IID_PPV_ARGS(m_graphicsCommandList.GetAddressOf()));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create graphics command list");
      return hr;
    }

    m_graphicsCommandList->Close();

    // Create a command list for resource uploads
    hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_resourceCommandAllocator));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create resource allocator");
      return hr;
    }
    m_resourceCommandAllocator->SetName(L"Direct Resource Allocator");

    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
      m_resourceCommandAllocator.Get(),
      nullptr,
      IID_PPV_ARGS(m_resourceCommandList.GetAddressOf()));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create resource command list");
      return hr;
    }

    m_resourceCommandList->Close();

    return hr;
  }


  void GraphicsDX12::flushCommandQueue()
  {
    HRESULT hr = S_OK; 
    
    m_currentGraphicsFence++;
    hr = m_graphicsCommandQueue->Signal(m_graphicsFence.Get(), m_currentGraphicsFence);

    if (hr != S_OK)
    {
      printLog("WARNING: graphics command queue signal failed in flushCommandQueue");
      return;
    }

    // Wait until the GPU has completed commands up to this fence point.
    if (m_graphicsFence->GetCompletedValue() < m_currentGraphicsFence)
    {
      HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

      // Fire event when GPU hits current fence.  
      hr = m_graphicsFence->SetEventOnCompletion(m_currentGraphicsFence, eventHandle);
      if (hr != S_OK)
      {
        printLog("WARNING: graphics command queue set event failed in flushCommandQueue");
        CloseHandle(eventHandle);
        return;
      }

      // Wait until the GPU hits current fence event is fired.
      WaitForSingleObject(eventHandle, INFINITE);
      CloseHandle(eventHandle);
    }
  }

  HRESULT GraphicsDX12::createSwapchain(uint32_t numFrames)
  {
    HRESULT hr = S_OK;

    RECT rect;
    m_width = 800;
    m_height = 600;
    if (GetClientRect(m_window, &rect))
    {
      m_width = rect.right - rect.left;
      m_height = rect.bottom - rect.top;
    }

    m_numFrames = numFrames;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = m_swapChainFormat;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_numFrames;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    hr = m_dxgiFactory->CreateSwapChainForHwnd(m_graphicsCommandQueue.Get(), m_window, &swapChainDesc, nullptr, nullptr, &m_swapChain);
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create Swap Chain");
      return hr;
    }

    for (uint32_t i = 0; i < m_numFrames; ++i)
    {
      ComPtr<ID3D12Resource> displayPlane;
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&displayPlane));
      if (!SUCCEEDED(hr))
      {
        printLog("ERROR: Could not get Swap Chain Buffer");
        break;
      }

      Dx12SwapchainBufferData* bufferData = new Dx12SwapchainBufferData();
      D3D12_RESOURCE_DESC resourceDesc = displayPlane->GetDesc();

      bufferData->m_resource.Attach(displayPlane.Get());
      bufferData->m_usageState = D3D12_RESOURCE_STATE_PRESENT;

      bufferData->m_width = (uint32_t)resourceDesc.Width;
      bufferData->m_height = resourceDesc.Height;
      bufferData->m_arraySize = resourceDesc.DepthOrArraySize;
      bufferData->m_format = resourceDesc.Format;
#ifndef RELEASE
      bufferData->m_resource->SetName(L"Primary SwapChain Buffer");
#endif

      displayPlane.Detach();
      m_swapchainBufferResources.push_back(bufferData);

      FrameData* frameData = new FrameData();
      m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(frameData->m_graphicsAllocator.GetAddressOf()));
      frameData->m_currentFence = 0;
      m_frameData.push_back(frameData);
    }

    m_depthStencilBufferResources = new Dx12SwapchainBufferData();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = m_numFrames;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_swapchainBufferRTVHeap));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create swap chain buffer render target view descriptor heap.");
      return hr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_swapchainBufferDSVHeap));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: Could not create swap chain depth stencil descriptor heap.");
      return hr;
    }

    return hr;
  }

  void GraphicsDX12::createPipeline(shared_ptr<Pipeline>, shared_ptr<Mesh>, shared_ptr<Material>)
  {
  }

  void GraphicsDX12::beginCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
    HRESULT hr = S_OK;

    //uint32_t currentFrame = frameIndex % m_numFrames;

    ComPtr<ID3D12CommandAllocator> graphicsAllocator = m_frameData[m_frameIndex]->m_graphicsAllocator;
    Dx12SwapchainBufferData* bufferData = m_swapchainBufferResources[m_frameIndex];

    if (m_frameData[m_frameIndex]->m_currentFence != 0 && m_graphicsFence->GetCompletedValue() < m_frameData[m_frameIndex]->m_currentFence)
    {
      HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
      hr = m_graphicsFence->SetEventOnCompletion(m_frameData[m_frameIndex]->m_currentFence, eventHandle);
      if (hr != S_OK)
      {
        printLog("WARNING: graphics fence set event failed in beginCommands");
        CloseHandle(eventHandle);
        return;
      }
      WaitForSingleObject(eventHandle, INFINITE);
      CloseHandle(eventHandle);
    }

    hr = graphicsAllocator->Reset();
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: command allocator reset failed in beginCommands.");
      return;
    }

    hr = m_graphicsCommandList->Reset(graphicsAllocator.Get(), nullptr);

    m_graphicsCommandList->RSSetViewports(1, &m_screenViewport);
    m_graphicsCommandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate a state transition on the resource usage.
    m_graphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferData->m_resource.Get(),
      D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
      m_swapchainBufferRTVHeap->GetCPUDescriptorHandleForHeapStart(),
      m_frameIndex,
      m_rtvDescriptorSize);
    m_graphicsCommandList->ClearRenderTargetView(rtvHeapHandle, DirectX::Colors::DarkSlateBlue, 0, nullptr);
    m_graphicsCommandList->ClearDepthStencilView(m_swapchainBufferDSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    m_graphicsCommandList->OMSetRenderTargets(1, &rtvHeapHandle, true, &m_swapchainBufferDSVHeap->GetCPUDescriptorHandleForHeapStart());
  }

  void GraphicsDX12::bindPipeline(shared_ptr<View> view, shared_ptr<Pipeline> pipeline, uint32_t frameIndex)
  {
  }

  void GraphicsDX12::draw(shared_ptr<View> view, shared_ptr<Mesh> mesh, shared_ptr<Material> material, uint32_t frameIndex)
  {
  }

  void GraphicsDX12::compute(shared_ptr<View> view)
  {
  }

  void GraphicsDX12::trace(shared_ptr<View> view)
  {
  }

  void GraphicsDX12::endCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
    HRESULT hr = S_OK;
    //uint32_t currentFrame = frameIndex % m_numFrames;
    Dx12SwapchainBufferData* bufferData = m_swapchainBufferResources[m_frameIndex];

    // Indicate a state transition on the resource usage.
    m_graphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferData->m_resource.Get(),
      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    hr = m_graphicsCommandList->Close();
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: command list close failed in endCommands.");
      return;
    }
  }

  void GraphicsDX12::executeCommands(shared_ptr<View> view, uint32_t frameIndex)
  {
    ID3D12CommandList* cmdsLists[] = { m_graphicsCommandList.Get() };
    m_graphicsCommandQueue->ExecuteCommandLists(1, cmdsLists);
  }

  void GraphicsDX12::update(shared_ptr<View> view, shared_ptr<Material> material)
  {
    Dx12MaterialData* materialData = (Dx12MaterialData*)material->getGraphicsData();
  }

  void GraphicsDX12::present(shared_ptr<View> view, uint32_t frameIndex)
  {
    HRESULT hr = S_OK;
    //uint32_t currentFrame = frameIndex % m_numFrames;

    // Swap the back and front buffers
    hr = m_swapChain->Present(0, 0);
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: present failed.");
      return;
    }
    m_frameData[m_frameIndex]->m_currentFence = ++m_currentGraphicsFence;
    m_graphicsCommandQueue->Signal(m_graphicsFence.Get(), m_currentGraphicsFence);

    m_frameIndex++;
    if (m_frameIndex == m_numFrames)
    {
      m_frameIndex = 0;
    }
  }

  void GraphicsDX12::buildBuffers(vector<shared_ptr<RenderComponent>>& renderComponents)
  {
    HRESULT hr = S_OK;
    uint32_t totalIndices = 0;
    uint32_t totalVerts = 0;

    for (size_t i = 0; i < renderComponents.size(); ++i)
    {
      for (size_t j = 0; j < renderComponents[i]->numMeshes(); ++j)
      {
        shared_ptr<Mesh> mesh = renderComponents[i]->getMesh(j);
        totalVerts += mesh->getNumVerts();
        totalIndices += mesh->getIndexBufferSize();
      }
    }

    void* vertexData = malloc(totalVerts * sizeof(Vertex4));
    uint16_t* indexData = (uint16_t*)malloc(totalIndices * sizeof(uint16_t));

    float* meshVertexData[5];
    uint32_t* meshIndexData;

    Vertex4* v4ptr = (Vertex4*)vertexData;
    uint16_t* iptr = indexData;
    uint32_t vertexStart = 0;
    uint32_t indexStart = 0;

    for (size_t i = 0; i < renderComponents.size(); ++i)
    {
      for (size_t j = 0; j < renderComponents[i]->numMeshes(); ++j)
      {
        shared_ptr<Mesh> mesh = renderComponents[i]->getMesh(j);
        Dx12MeshData* meshData = new Dx12MeshData();

        size_t numBuffers = mesh->getNumBuffers();
        for (size_t k = 0; k < numBuffers; ++k)
        {
          meshVertexData[k] = mesh->getVertexBufferData(k);
        }
        meshIndexData = mesh->getIndexBuffer();

        size_t numVerts = mesh->getNumVerts();
        for (size_t k = 0; k < numVerts; ++k)
        {
          v4ptr->position.x = meshVertexData[0][k * 3 + 0];
          v4ptr->position.y = meshVertexData[0][k * 3 + 1];
          v4ptr->position.z = meshVertexData[0][k * 3 + 2];
          v4ptr->normal.x = meshVertexData[1][k * 3 + 0];
          v4ptr->normal.y = meshVertexData[1][k * 3 + 1];
          v4ptr->normal.z = meshVertexData[1][k * 3 + 2];
          v4ptr->texCoord.x = meshVertexData[2][k * 2 + 0];
          v4ptr->texCoord.y = meshVertexData[2][k * 2 + 1];
          v4ptr->tangent.x = meshVertexData[3][k * 3 + 0];
          v4ptr->tangent.y = meshVertexData[3][k * 3 + 1];
          v4ptr->tangent.z = meshVertexData[3][k * 3 + 2];
          v4ptr++;
        }

        size_t numIndeces = mesh->getIndexBufferSize();
        for (size_t k = 0; k < numIndeces; ++k)
        {
          *iptr = (uint16_t)meshIndexData[k];
          iptr++;
        }

        meshData->m_numIndeces = numIndeces;
        meshData->m_numVertices = numVerts;
        meshData->m_vertexStart = vertexStart;
        meshData->m_indexStart = indexStart;
        mesh->setGraphicsData(meshData);
        mesh->setDirty(false);

        vertexStart += numVerts;
        indexStart += numIndeces;

        buildMaterial(mesh->getMaterial());
      }    
    }

    hr = m_resourceCommandList->Reset(m_resourceCommandAllocator.Get(), nullptr);
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: unable to reset resource command list.");
      return;
    }

    ComPtr<ID3D12Resource> vUploadBuffer;
    ComPtr<ID3D12Resource> iUploadBuffer;

    m_vertexBuffer = createAndUploadBuffer(totalVerts * sizeof(Vertex4), vertexData, D3D12_HEAP_TYPE_DEFAULT, vUploadBuffer);
    m_indexBuffer = createAndUploadBuffer(totalIndices * sizeof(uint16_t), indexData, D3D12_HEAP_TYPE_DEFAULT, iUploadBuffer);

    m_resourceCommandList->Close();
    ID3D12CommandList* cmdsLists[] = { m_resourceCommandList.Get() };
    m_graphicsCommandQueue->ExecuteCommandLists(1, cmdsLists);
    flushCommandQueue();
  }

  ComPtr<ID3D12Resource> GraphicsDX12::createAndUploadBuffer(uint32_t size, void* data, D3D12_HEAP_TYPE heapFlags, ComPtr<ID3D12Resource>& uploadBuffer)
  {
    HRESULT hr = S_OK;
    ComPtr<ID3D12Resource> defaultBuffer;

    // Create the actual default buffer resource.
    hr = m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(heapFlags),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer(size),
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: unable to create committed resource.");
      return nullptr;
    }

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    hr = m_device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer(size),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(uploadBuffer.GetAddressOf()));

    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: unable to create committed resource for upload.");
      return nullptr;
    }

    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = data;
    subResourceData.RowPitch = size;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    m_resourceCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

    UpdateSubresources(m_resourceCommandList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

    m_resourceCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    return defaultBuffer;
  }

  void GraphicsDX12::buildMaterial(shared_ptr<Material> material)
  {
    Dx12MaterialData* materialData = (Dx12MaterialData*)material->getGraphicsData();
    if (materialData == nullptr || material->isDirty())
    {
      if (materialData != nullptr)
      {
        // TODO: Free old resources
      }

      materialData = new Dx12MaterialData();

      std::wstring vsName = L"Shaders\\Forward.hlsl";
      materialData->m_vsByteCode = m_vsMap[vsName];
      if (materialData->m_vsByteCode == nullptr)
      {
        materialData->m_vsByteCode = loadShader(vsName, nullptr, "VS", "vs_5_0");
        m_vsMap[vsName] = materialData->m_vsByteCode;
      }

      std::wstring psName = L"Shaders\\Forward.hlsl";
      materialData->m_psByteCode = m_psMap[psName];
      if (materialData->m_psByteCode == nullptr)
      {
        materialData->m_psByteCode = loadShader(psName, nullptr, "PS", "ps_5_0");
        m_psMap[psName] = materialData->m_psByteCode;
      }

      material->setGraphicsData(materialData);
      material->setDirty(false);
    }
  }

  ComPtr<ID3DBlob> GraphicsDX12::loadShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,
    const std::string& target)
  {
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = S_OK;

    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;
    hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
      entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

    if (errors != nullptr)
    {
      OutputDebugStringA((char*)errors->GetBufferPointer());
    }

    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: unable to compile shader.");
      return nullptr;
    }
    return byteCode;
  }

  void GraphicsDX12::createView(shared_ptr<View> view)
  {
    Dx12ViewData* viewData = new Dx12ViewData();
    view->setGraphicsData(viewData);

    vec2 size;
    view->getViewportSize(size);
    viewData->m_currentWidth = (uint32_t)size.x;
    viewData->m_currentHeight = (uint32_t)size.y;

    // Create the surface
    // resize(viewData->m_currentWidth, viewData->m_currentHeight);
    viewData->m_frameIndex = 0;
  }

  void GraphicsDX12::resize(uint32_t width, uint32_t height)
  {
    HRESULT hr = S_OK;

    m_width = width;
    m_height = height;

    flushCommandQueue();

    hr = m_graphicsCommandList->Reset(m_graphicsCommandAllocator.Get(), nullptr);
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: graphics command list rest failed in resize.");
      return;
    }

    // Release the previous resources we will be recreating.
    for (int i = 0; i < m_numFrames; ++i)
    {
      m_swapchainBufferResources[i]->m_resource.Reset();
    }
    m_depthStencilBufferResources->m_resource.Reset();

    hr = m_swapChain->ResizeBuffers(m_numFrames, m_width, m_height, m_swapChainFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: swapchain resize buffers failed.");
      return;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_swapchainBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < m_numFrames; ++i)
    {
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainBufferResources[i]->m_resource));
      if (!SUCCEEDED(hr))
      {
        printLog("ERROR: swapchain resize get buffer failed.");
        return;
      }

      m_device->CreateRenderTargetView(m_swapchainBufferResources[i]->m_resource.Get(), nullptr, rtvHeapHandle);
      rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
    }

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_width;
    depthStencilDesc.Height = m_height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;

    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D32_FLOAT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    hr = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
      D3D12_HEAP_FLAG_NONE,
      &depthStencilDesc,
      D3D12_RESOURCE_STATE_COMMON,
      &optClear,
      IID_PPV_ARGS(m_depthStencilBufferResources->m_resource.GetAddressOf()));
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: create depth buffer resource failed");
      return;
    }

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_depthStencilBufferResources->m_resource.Get(), &dsvDesc, m_swapchainBufferDSVHeap->GetCPUDescriptorHandleForHeapStart());


    // Transition the resource from its initial state to be used as a depth buffer.
    m_graphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBufferResources->m_resource.Get(),
      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // Execute the resize commands.
    hr = m_graphicsCommandList->Close();
    if (!SUCCEEDED(hr))
    {
      printLog("ERROR: close of resize command list failed");
      return;
    }
    ID3D12CommandList* cmdsLists[] = { m_graphicsCommandList.Get() };
    m_graphicsCommandQueue->ExecuteCommandLists(1, cmdsLists);

    // Wait until resize is complete.
    flushCommandQueue();

    // Update the viewport transform to cover the client area.
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = static_cast<float>(m_width);
    m_screenViewport.Height = static_cast<float>(m_height);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_scissorRect = { 0, 0, (LONG)m_width, (LONG)m_height };

    m_frameIndex = 0;
  }

  void GraphicsDX12::printLog(string s)
  {
    string st = s + "\n";
    TCHAR name[256];
    _tcscpy_s(name, CA2T(st.c_str()));
    OutputDebugString(name);
  }
}
