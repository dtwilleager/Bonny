// Bonny.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Bonny.h"

#include "WorldManager.h"
#include "RenderComponent.h"
#include "LightComponent.h"
#include "Entity.h"
#include "FirstPersonProcessor.h"
#include "RotationProcessor.h"
#include "TranslationProcessor.h"
#include "RenderScreenView.h"

#include <memory>
#include <array>

using std::shared_ptr;
using std::array;
using std::make_shared;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

Bonny::WorldManager*  g_worldManager;
unsigned int              g_windowX = 0;
unsigned int              g_windowY = 0;
unsigned int              g_windowWidth = 1200;
unsigned int              g_windowHeight = 800;
bool g_appDone;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BONNY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    g_appDone = false;

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BONNY));

    MSG msg;

    // Main message loop:
    while (!g_appDone)
    {
      while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        switch (msg.message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
          if (msg.wParam == VK_ESCAPE)
          {
            g_appDone = true;
          }
          g_worldManager->handleKeyboard(&msg);
          break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
          g_worldManager->handleMouse(&msg);
          break;

        }
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      g_worldManager->executeFrame();
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BONNY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BONNY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_worldManager = new Bonny::WorldManager("WorldManager", hInst, hWnd);

   // Load the Sponza World
   shared_ptr<Bonny::Entity> rootEntity = make_shared<Bonny::Entity>("Root Entity");

   //shared_ptr<Bonny::Entity> gltfModel = g_worldManager->loadGLTFModel("models/gltf/star_wars/scene.gltf");
   //mat4 gltfcale = glm::scale(mat4(), vec3(0.3f, 0.3f, 0.3f));
   //gltfModel->setTransform(gltfcale);
   //rootEntity->addChild(gltfModel);

   shared_ptr<Bonny::Entity> sponzaModel = g_worldManager->loadAssimpModel("models/sponzaPBR/sponza.obj");
   //shared_ptr<Bonny::Entity> sponzaModel = g_worldManager->loadAssimpModel("models/bistro/Bistro_Research_Interior.fbx");
   mat4 sponzaScale = glm::scale(mat4(), vec3(0.1f, 0.1f, 0.1f));
   sponzaModel->setTransform(sponzaScale);
   rootEntity->addChild(sponzaModel);

   shared_ptr<Bonny::Entity> teapotModel = g_worldManager->loadAssimpModel("models/teapot.obj");
   mat4 teapotScale = glm::scale(mat4(), vec3(0.3f, 0.3f, 0.3f));
   teapotModel->setTransform(teapotScale);
   teapotModel->setCastShadow(false);

   // Create the screen view and its processor
   shared_ptr<Bonny::RenderBuffer> renderBuffer = make_shared<Bonny::RenderBuffer>(1200, 800);
   renderBuffer->createDepthAttatchment(Bonny::RenderBuffer::RB_FLOAT_32);
   renderBuffer->addColorAttatchment(Bonny::RenderBuffer::RB_UNORM_BGRA);

   shared_ptr<Bonny::RenderScreenView> screenView = make_shared<Bonny::RenderScreenView>("Screen View");
   screenView->setRenderBuffer(renderBuffer);
   screenView->setViewportSize(vec2(1200, 800));
   //mat4 transform = glm::lookAt(vec3(0.0f, -15.0f, 0.0f), vec3(0.0f, -15.0f, 0.0f) + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));
   mat4 transform = glm::lookAt(vec3(0.0f, 15.0f, 0.0f), vec3(0.0f, 15.0f, 0.0f) + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
   screenView->setViewTransform(transform);
   g_worldManager->addView(screenView);

   shared_ptr<Bonny::Entity> screenViewEntity = make_shared<Bonny::Entity>("Screen View Entity");
   shared_ptr<Bonny::FirstPersonProcessor> screenViewProcessor = make_shared<Bonny::FirstPersonProcessor>("Screen View Processor", screenView);
   screenViewEntity->addComponent(screenViewProcessor);
   rootEntity->addChild(screenViewEntity);

   // Add 2 lights
   //vec3 yAxis(0.0f, 1.0f, 0.0f);
   //shared_ptr<Bonny::Entity> light1E = make_shared<Bonny::Entity>("Light 1");
   //shared_ptr<Bonny::LightComponent> light1C = make_shared<Bonny::LightComponent>("Light1", Bonny::LightComponent::POINT, true);
   //light1C->setDiffuse(vec3(0.0f, 0.0f, 1.0f));
   //light1C->setPosition(vec3(111.0f, -18.0f, 40.0f));
   //light1C->setCastShadow(false);
   //light1E->addComponent(light1C);
   //shared_ptr<Bonny::RotationProcessor> light1Processor = make_shared<Bonny::RotationProcessor>("Light 1 Rotation Processor", light1E, yAxis, 0.1f);
   //light1E->addComponent(light1Processor);
   //light1E->addChild(teapotTranslation);
   //rootEntity->addChild(light1E);

   //shared_ptr<Bonny::Entity> light2E = make_shared<Bonny::Entity>("Light 2");
   //shared_ptr<Bonny::LightComponent> light2C = make_shared<Bonny::LightComponent>("Light2", Bonny::LightComponent::POINT, true);
   //light2C->setDiffuse(vec3(1.0f, 0.0f, 0.0f));
   //light2C->setPosition(vec3(-120.0f, -18.0f, 40.0f));
   //light2C->setCastShadow(false);
   //light2E->addComponent(light2C);
   //rootEntity->addChild(light2E);

   //shared_ptr<Bonny::Entity> light3E = make_shared<Bonny::Entity>("Light 3");
   //shared_ptr<Bonny::LightComponent> light3C = make_shared<Bonny::LightComponent>("Light3", Bonny::LightComponent::POINT, true);
   //light3C->setDiffuse(vec3(0.0f, 1.0f, 0.0f));
   //light3C->setPosition(vec3(-120.0f, -18.0f, -46.0f));
   //light3C->setCastShadow(false);
   //light3E->addComponent(light3C);
   //rootEntity->addChild(light3E);

   shared_ptr<Bonny::Entity> light4E = make_shared<Bonny::Entity>("Light 4");
   shared_ptr<Bonny::LightComponent> light4C = make_shared<Bonny::LightComponent>("Light4", Bonny::LightComponent::POINT, true);
   light4C->setDiffuse(vec3(1.0f, 1.0f, 1.0f));
   //light4C->setPosition(vec3(-110.0f, -7.0f, 0.0f));
   light4C->setPosition(vec3(0.0f, -1.0f, 0.0f));
   light4C->setCastShadow(false);
   light4E->addComponent(light4C);
   shared_ptr<Bonny::TranslationProcessor> light4Processor = make_shared<Bonny::TranslationProcessor>("Light 4 Translation Processor", -5.5f, 5.5f, 0.05f, vec3(0.0f, 0.0f, 1.0f), light4E);
   light4E->addComponent(light4Processor);
   rootEntity->addChild(light4E);

   array<vec3, 7> colors;
   colors[0] = vec3(0.0f, 0.0f, 1.0f);
   colors[1] = vec3(0.0f, 1.0f, 0.0f);
   colors[2] = vec3(0.0f, 1.0f, 1.0f);
   colors[3] = vec3(1.0f, 0.0f, 0.0f);
   colors[4] = vec3(1.0f, 0.0f, 1.0f);
   colors[5] = vec3(1.0f, 1.0f, 0.0f);
   colors[6] = vec3(1.0f, 1.0f, 1.0f);

   //for (size_t i = 0; i < 500; i++)
   //{
   //  shared_ptr<Bonny::Entity> lightE = make_shared<Bonny::Entity>("Light " + i);
   //  shared_ptr<Bonny::LightComponent> lightC = make_shared<Bonny::LightComponent>("Light " + i, Bonny::LightComponent::POINT, false);
   //  vec3 position;

   //  uint32_t lightIndex = rand() % 6;

   //  position.x = rand() % 200 - 100.0f;
   //  position.y = (float)(-rand() % 50);
   //  position.z = rand() % 100 - 50.0f;

   //  lightC->setDiffuse(colors[lightIndex]);
   //  lightC->setPosition(position);
   //  lightE->addComponent(lightC);
   //  rootEntity->addChild(lightE);
   //}


   g_worldManager->addEntity(rootEntity);

   g_worldManager->buildFrame();
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                g_appDone = true;
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SHOWWINDOW:
    case WM_SIZE:
    {
      RECT rect;
      if (GetClientRect(hWnd, &rect))
      {
        g_windowWidth = rect.right - rect.left;
        g_windowHeight = rect.bottom - rect.top;
      }
      if (g_worldManager != nullptr)
      {
        g_worldManager->updateWindow(g_windowX, g_windowY, g_windowWidth, g_windowHeight);
      }
    }
    break;
    case WM_MOVE:
    {
      RECT rect;
      if (GetClientRect(hWnd, &rect))
      {
        g_windowWidth = rect.right - rect.left;
        g_windowHeight = rect.bottom - rect.top;
      }
      g_windowX = (int)(short)LOWORD(lParam);
      g_windowY = (int)(short)HIWORD(lParam);
      if (g_worldManager != nullptr)
      {
        g_worldManager->updateWindow(g_windowX, g_windowY, g_windowWidth, g_windowHeight);
      }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        g_appDone = true;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
