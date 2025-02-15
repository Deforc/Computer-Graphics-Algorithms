#include "framework.h"
#include "computer_graphics_lab_1.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

Vertex Vertices[] = {
    { DirectX::XMFLOAT3(0.0f,  0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },  // Red
    { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },  // Green
    { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }   // Blue
};

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CleanupDirectX();
HRESULT InitDirectX(HWND hWnd);
void Render();
HRESULT ResizeBuffers(UINT width, UINT height, HWND hWnd);
HRESULT CompileShader(const wchar_t* filename, const char* entryPoint, const char* target, ID3DBlob** ppBlob);
HRESULT CreateShaders();
HRESULT CreateBuffers();


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
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
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
    pBackBuffer->Release();
    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create render target view.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    D3D11_VIEWPORT viewport;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    g_pDeviceContext->RSSetViewports(1, &viewport);


    if (FAILED(CreateShaders()))
    {
        MessageBox(hWnd, L"Failed to create shaders.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateBuffers()))
    {
        MessageBox(hWnd, L"Failed to create buffers.", L"Error", MB_OK);
        return hr;
    }
    return S_OK;
}



void CleanupDirectX()
{
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pInputLayout) g_pInputLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pDeviceContext) g_pDeviceContext->Release();
    if (g_pDevice) g_pDevice->Release();
}

HRESULT CompileShader(const wchar_t* filename, const char* entryPoint, const char* target, ID3DBlob** ppBlob)
{
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, dwShaderFlags, 0, ppBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Shader Compile Error", MB_OK | MB_ICONERROR);
            pErrorBlob->Release();
        }
        return hr;
    }
    return S_OK;
}

HRESULT CreateShaders()
{
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShader(L"VertexShader.hlsl", "VSMain", "vs_5_0", &pVSBlob);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = g_pDevice->CreateInputLayout(layout,
        ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pInputLayout);
    pVSBlob->Release();
    if (FAILED(hr)) return hr;

    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShader(L"PixelShader.hlsl", "PSMain", "ps_5_0", &pPSBlob);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr)) return hr;

    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateBuffers()
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = Vertices;

    HRESULT hr = g_pDevice->CreateBuffer(&bufferDesc, &initData, &g_pVertexBuffer);
    if (FAILED(hr)) return hr;
}

void Render()
{
    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MidnightBlue);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pDeviceContext->IASetInputLayout(g_pInputLayout);;


    g_pDeviceContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pDeviceContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pDeviceContext->Draw(3, 0);

    g_pSwapChain->Present(0, 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (g_pSwapChain && g_pDevice) ResizeBuffers(LOWORD(lParam), HIWORD(lParam), hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HRESULT ResizeBuffers(UINT width, UINT height, HWND hWnd)
{
    HRESULT hr = S_OK;
    if (g_pRenderTargetView) {
        g_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        g_pRenderTargetView->Release();
        g_pRenderTargetView = nullptr;
    }
    g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr) || pBackBuffer == nullptr) {
        OutputDebugStringA("Failed to get back buffer from swap chain.\n");
        return hr;
    }

    hr = g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create render target view.\n");
        return hr;
    }

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    RECT rc;
    GetClientRect(hWnd, &rc);
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(rc.right - rc.left);
    vp.Height = static_cast<FLOAT>(rc.bottom - rc.top);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    g_pDeviceContext->RSSetViewports(1, &vp);

    return S_OK;
}