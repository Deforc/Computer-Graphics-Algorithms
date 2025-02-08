#include "framework.h"
#include "computer_graphics_lab_1.h"
#include <d3d11.h>
#include <dxgi.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "dxguid.lib")
#endif

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CleanupDirectX();
HRESULT InitDirectX(HWND hWnd);
void Render();
HRESULT ResizeBuffers(UINT width, UINT height);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COMPUTERGRAPHICSLAB1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    CleanupDirectX();
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = szWindowClass;
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    InitDirectX(hWnd);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

HRESULT InitDirectX(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION, &scd,
        &g_pSwapChain, &g_pDevice, nullptr, &g_pDeviceContext);

    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create device and swap chain.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    ID3D11Texture2D* pBackBuffer;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (FAILED(hr) || pBackBuffer == nullptr) {
        MessageBox(hWnd, L"Failed to get back buffer.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create render target view.", L"Error", MB_OK | MB_ICONERROR);
        pBackBuffer->Release();
        return hr;
    }
    pBackBuffer->Release();

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    return S_OK;
}

void CleanupDirectX()
{
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pDeviceContext) g_pDeviceContext->Release();
    if (g_pDevice) g_pDevice->Release();
}

void Render()
{
    float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, color);
    g_pSwapChain->Present(1, 0);
}

HRESULT ResizeBuffers(UINT width, UINT height)
{
    if (g_pRenderTargetView) g_pRenderTargetView->Release();

    HRESULT hr = g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to resize swap chain buffers.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    ID3D11Texture2D* pBackBuffer;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (FAILED(hr) || pBackBuffer == nullptr) {
        MessageBox(nullptr, L"Failed to get back buffer.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create render target view.", L"Error", MB_OK | MB_ICONERROR);
        pBackBuffer->Release();
        return hr;
    }
    pBackBuffer->Release();

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (g_pSwapChain) ResizeBuffers(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
