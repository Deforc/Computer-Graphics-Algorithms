#include "framework.h"
#include "computer_graphics_lab_1.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXTex.h>
#include <DDSTextureLoader.h>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define MAX_LOADSTRING 100

const int NUM_INSTANCES = 10;
const int NUM_TEX = 2;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pConstantBufferM = nullptr;
ID3D11Buffer* g_pConstantBufferM2 = nullptr;
ID3D11Buffer* g_pConstantBufferM3 = nullptr;
ID3D11Buffer* g_pConstantBufferVP = nullptr;
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11ShaderResourceView* g_pTextureView = nullptr;
ID3D11ShaderResourceView* g_pTextureView_naggets = nullptr;
ID3D11SamplerState* g_pSamplerState = nullptr;

ID3D11InputLayout* g_pSkyboxInputLayout = nullptr;
ID3D11Buffer* g_pConstantBufferMSkyBox = nullptr;
ID3D11Buffer* g_pConstantBufferVPSkyBox = nullptr;
ID3D11ShaderResourceView* g_pCubemapView = nullptr;
ID3D11VertexShader* g_pSkyboxVertexShader = nullptr;
ID3D11PixelShader* g_pSkyboxPixelShader = nullptr;
ID3D11Buffer* g_pSkyboxVertexBuffer = nullptr;
ID3D11Buffer* g_pSkyboxIndexBuffer = nullptr;

ID3D11InputLayout* g_pPlaneInputLayout = nullptr;
ID3D11PixelShader* g_pPlanePixelShader = nullptr;
ID3D11VertexShader* g_pPlaneVertexShader = nullptr;
ID3D11Buffer* g_pPlaneVertexBuffer = nullptr;
ID3D11Buffer* g_pPlaneIndexBuffer = nullptr;
ID3D11Buffer* g_pConstantBufferMPlane = nullptr;
ID3D11Buffer* g_pConstantBufferVPPlane = nullptr;
ID3D11Buffer* g_pConstantColorBuffer = nullptr;

ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11DepthStencilState* g_pDepthStencilState = nullptr;
ID3D11DepthStencilState* g_pDepthStencilStateForTransparent = nullptr;
ID3D11BlendState* g_pBlendState = nullptr;
ID3D11RasterizerState* g_pRasterizerState = nullptr;

ID3D11Buffer* g_pLightBuffer = nullptr;
ID3D11Buffer* g_pLightColorBuffer = nullptr;
ID3D11PixelShader* g_pLightPixelShader = nullptr;
ID3D11ShaderResourceView* g_pNormalMapSRV = nullptr;
ID3D11Buffer* g_pLightVertexBuffer = nullptr;
ID3D11Buffer* g_pLightIndexBuffer = nullptr;

bool g_EnablePostProcessing = false;
ID3D11Texture2D* g_pPostProcessingTexture = nullptr;
ID3D11RenderTargetView* g_pPostProcessingRTV = nullptr;
ID3D11ShaderResourceView* g_pPostProcessingSRV = nullptr;
ID3D11VertexShader* g_pPostProcessingVS = nullptr;
ID3D11PixelShader* g_pPostProcessingPS = nullptr;
ID3D11Buffer* g_pPostprocessingVertexBuffer = nullptr;
ID3D11InputLayout* g_pPostProcessingLayout = nullptr;

DirectX::XMFLOAT3 g_CameraPosition = { 0.0f, 0.0f, -5.0f };
float g_RotationAngle = 0.0f;
float g_RotationAngleX = 0.0f;
float g_RotationAngleY = 0.0f;
float g_CameraMoveSpeed = 15.0f;
float g_CameraDistance = 5.0f;
ULONGLONG g_timeStart = 0;
float g_DeltaTime = 0.0f;

DirectX::XMFLOAT3 g_lightPosition = { 2.5f, 0.6f, 0.0f };
DirectX::XMFLOAT3 lightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); 
DirectX::XMFLOAT3 ambient = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f); 

POINT g_MousePos;
bool g_MousePressed = false;

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
};

struct AABB {
    DirectX::XMFLOAT3 min;
    DirectX::XMFLOAT3 max;
};

struct ConstantBufferData
{
    DirectX::XMMATRIX matrix;
};

struct CNBufferData
{
    DirectX::XMMATRIX models[NUM_INSTANCES];
    DirectX::XMMATRIX normals[NUM_INSTANCES];
    DirectX::XMFLOAT4 isNormalMapActive[NUM_INSTANCES];
};

struct ColorBuffer {
    DirectX::XMFLOAT4 color;
};

struct ConstantBufferSkyBox {
    DirectX::XMMATRIX matrix;
    DirectX::XMFLOAT4 size_or_cameraPos;
};

struct VertexSkybox {
    float x, y, z;
};

struct VertexPlane {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};

struct FrustumPlane {
    float a, b, c, d;
};

struct FullScreenVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texCoord;
};

struct alignas(16) LightBuffer {
    DirectX::XMFLOAT3 lightPos;
    float padding0; 
    DirectX::XMFLOAT3 lightColor;
    float padding1; 
    DirectX::XMFLOAT3 ambient;
    float padding2; 
    DirectX::XMFLOAT3 cameraPosition;
    float padding3; 
};

Vertex Vertices[] = {

    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Top face
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Front face
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Back face
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Left face
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Right face
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) }
};

VertexSkybox skyboxVertices[] = {
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    {-1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f}
};

VertexPlane planeVertices[] = {
{ DirectX::XMFLOAT3(0.0f, -0.5f, 0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, 0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, .0f) }
};


UINT Indices[] = {
    0, 2, 1, 0, 3, 2,
    4, 6, 5, 4, 7, 6,
    8, 10, 9, 8, 11, 10,
    12, 14, 13, 12, 15, 14,
    16, 18, 17, 16, 19, 18,
    20, 22, 21, 20, 23, 22
};

UINT planeIndices[] = {
    0, 1, 2,
    0, 2, 3
};

FullScreenVertex FullScreenVertices[] = {
        { {-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-1.0f,  1.0f, 0.0f}, {3.0f, 1.0f} },
        { { 1.0f, -1.0f, 0.0f}, {-1.0f, -3.0f} }
};

DirectX::XMFLOAT4 g_ColorBuffers[2] = { DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f), 
                                  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.5f) }; 

DirectX::XMMATRIX g_modelMatrices[2] = { DirectX::XMMatrixTranslation(1.0f, 0.0f, 0.0f),
                                       DirectX::XMMatrixTranslation(1.5f, 0.0f, 0.0f) };

CNBufferData g_mBuffers;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CleanupDirectX();
HRESULT InitDirectX(HWND hWnd);
void Render();
HRESULT ResizeBuffers(UINT width, UINT height, HWND hWnd);
HRESULT CompileShader(const wchar_t* filename, const char* entryPoint, const char* target, ID3DBlob** ppBlob);
HRESULT CreateShaders();
HRESULT CreateShadersSkyBox();
HRESULT CreateShadersPlane();
HRESULT CreateBuffers();
HRESULT CreateBuffersSkyBox();
HRESULT CreateBuffersPlane();
HRESULT CreateConstantBuffers();
HRESULT CreateConstantBuffersSkybox();
HRESULT CreateConstantBuffersPlane();
HRESULT CreateConstantColorBuffer();
void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const DirectX::XMMATRIX& data);
HRESULT LoadTexture(const wchar_t* filename);
HRESULT LoadTexture_2(const wchar_t* filename);
HRESULT LoadNormalMap(const wchar_t* filename);
HRESULT CreateSampler();
HRESULT LoadSkybox(const wchar_t* filename);
HRESULT CreateDepthStencilBuffer();
HRESULT CreateDepthStencilState();
HRESULT CreateBlendState();
HRESULT CreateRasterizerState();
HRESULT CreateLightConstantBuffers();
HRESULT CreateShadersLight();
HRESULT CreateShadersPostProcess();
HRESULT CreatePostProcessingBuffers();
HRESULT CreatePostProcessingResources(UINT screenWidth, UINT screenHeight);

std::array<FrustumPlane, 6> ExtractFrustumPlanes(const DirectX::XMMATRIX& viewProjMatrix) {
    std::array<FrustumPlane, 6> planes;

    DirectX::XMFLOAT4X4 matrix;
    DirectX::XMStoreFloat4x4(&matrix, viewProjMatrix);

    // Near plane
    planes[0] = { matrix._13, matrix._23, matrix._33, matrix._43 };
    // Far plane
    planes[1] = { matrix._14 - matrix._13, matrix._24 - matrix._23, matrix._34 - matrix._33, matrix._44 - matrix._43 };
    // Left plane
    planes[2] = { matrix._14 + matrix._11, matrix._24 + matrix._21, matrix._34 + matrix._31, matrix._44 + matrix._41 };
    // Right plane
    planes[3] = { matrix._14 - matrix._11, matrix._24 - matrix._21, matrix._34 - matrix._31, matrix._44 - matrix._41 };
    // Top plane
    planes[4] = { matrix._14 - matrix._12, matrix._24 - matrix._22, matrix._34 - matrix._32, matrix._44 - matrix._42 };
    // Bottom plane
    planes[5] = { matrix._14 + matrix._12, matrix._24 + matrix._22, matrix._34 + matrix._32, matrix._44 + matrix._42 };

    for (auto& plane : planes) {
        float length = sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
        plane.a /= length;
        plane.b /= length;
        plane.c /= length;
        plane.d /= length;
    }

    return planes;
}

std::vector<DirectX::XMFLOAT3> ExtractCubePositions(const CNBufferData& mBuffers) {
    std::vector<DirectX::XMFLOAT3> positions;
    for (int i = 0; i < NUM_INSTANCES; ++i) {
        DirectX::XMFLOAT4X4 modelMatrix;
        DirectX::XMStoreFloat4x4(&modelMatrix, mBuffers.models[i]);
        DirectX::XMFLOAT3 position = { modelMatrix._41, modelMatrix._42, modelMatrix._43 };
        positions.push_back(position);
    }
    return positions;
}

std::vector<AABB> CreateAABBs(const std::vector<DirectX::XMFLOAT3>& positions, float cubeSize) {
    std::vector<AABB> aabbs;

    float halfDiagonal = (cubeSize * std::sqrt(3.0f)) / 2.0f;

    for (const auto& pos : positions) {
        AABB aabb;
        aabb.min = { pos.x - halfDiagonal, pos.y - halfDiagonal, pos.z - halfDiagonal };
        aabb.max = { pos.x + halfDiagonal, pos.y + halfDiagonal, pos.z + halfDiagonal };
        aabbs.push_back(aabb);
    }

    return aabbs;
}

bool IsBoxInside(const std::array<FrustumPlane, 6>& frustum, const DirectX::XMFLOAT3& bbMin, const DirectX::XMFLOAT3& bbMax) {
    for (const auto& plane : frustum) {
        DirectX::XMFLOAT3 norm = { plane.a, plane.b, plane.c };
        DirectX::XMFLOAT3 p(
            signbit(norm.x) ? bbMin.x : bbMax.x,
            signbit(norm.y) ? bbMin.y : bbMax.y,
            signbit(norm.z) ? bbMin.z : bbMax.z
        );

        float distance = plane.a * p.x + plane.b * p.y + plane.c * p.z + plane.d;
        if (distance < 0.0f) {
            return false;
        }
    }
    return true;
}

std::vector<int> PerformFrustumCulling(const std::array<FrustumPlane, 6>& frustumPlanes,
    const std::vector<AABB>& aabbs) {
    std::vector<int> visibleIndices;
    for (size_t i = 0; i < aabbs.size(); ++i) {
        if (IsBoxInside(frustumPlanes, aabbs[i].min, aabbs[i].max)) {
            visibleIndices.push_back(static_cast<int>(i));
        }
    }
    return visibleIndices;
}

void SetCubesMatrices() {
    for (int i = 0; i < NUM_INSTANCES; i++) {
        float randomX = static_cast<float>(std::rand() % 50) / 10.0f - 5.0f;
        float randomY = static_cast<float>(std::rand() % 50) / 10.0f - 5.0f;
        float randomZ = static_cast<float>(std::rand() % 50) / 10.0f - 5.0f;

        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(randomX, randomY, randomZ);

        g_mBuffers.isNormalMapActive[i] = DirectX::XMFLOAT4(i % 2, i % 2, i % 2, i % 2);
        g_mBuffers.models[i] = translationMatrix;
        g_mBuffers.normals[i] = DirectX::XMMatrixInverse(nullptr, g_mBuffers.models[i]);
        g_mBuffers.normals[i] = DirectX::XMMatrixTranspose(g_mBuffers.normals[i]);
    }
}

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
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
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

    if (FAILED(CreateDepthStencilBuffer())) 
    {
        MessageBox(hWnd, L"Failed to create DepthStencilBuffer.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateDepthStencilState())) 
    {
        MessageBox(hWnd, L"Failed to create DepthStencilState.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateBlendState()))
    {
        MessageBox(hWnd, L"Failed to create BlendState.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateRasterizerState())) {
        MessageBox(hWnd, L"Failed to create RasterizerState.", L"Error", MB_OK);
        return hr;
    }

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    D3D11_VIEWPORT viewport;
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    g_pDeviceContext->RSSetViewports(1, &viewport);

    if (FAILED(CreatePostProcessingResources(800, 600)))
    {
        MessageBox(hWnd, L"Failed to create post processing resources.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreatePostProcessingBuffers()))
    {
        MessageBox(hWnd, L"Failed to create post processing vertex buffer.", L"Error", MB_OK);
        return hr;
    }

    //Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext);

    if (FAILED(CreateShadersPostProcess()))
    {
        MessageBox(hWnd, L"Failed to create Post Processing shaders.", L"Error", MB_OK);
        return hr;
    }

    if(FAILED(CreateShadersLight()))
    {
        MessageBox(hWnd, L"Failed to create light shaders.", L"Error", MB_OK);
        return hr;
    }

    hr = CreateLightConstantBuffers();
    if (FAILED(hr))
    {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create light constant buffers. HRESULT: 0x%08X", hr);
        MessageBox(hWnd, errorMsg, L"Error", MB_OK);

        if (hr == E_OUTOFMEMORY)
            MessageBox(hWnd, L"Out of memory!", L"Error", MB_OK);
        else if (hr == E_INVALIDARG)
            MessageBox(hWnd, L"Invalid argument!", L"Error", MB_OK);
        else if (hr == DXGI_ERROR_DEVICE_REMOVED)
            MessageBox(hWnd, L"Device removed!", L"Error", MB_OK);
        else if (hr == DXGI_ERROR_DEVICE_RESET)
            MessageBox(hWnd, L"Device reset!", L"Error", MB_OK);
    }
    if (FAILED(CreateShaders()))
    {
        MessageBox(hWnd, L"Failed to create shaders.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateConstantBuffers())) {
        MessageBox(hWnd, L"Failed to create constant buffers.", L"Error", MB_OK);
        return hr;
    }
    if (FAILED(CreateBuffers()))
    {
        if (FAILED(hr)) {
            MessageBox(hWnd, L"Failed to create buffers.", L"Error", MB_OK);
            wchar_t error_message[256];
            swprintf(error_message, 256, L"Error code: 0x%08X", hr);
            MessageBox(hWnd, error_message, L"Error Code", MB_OK);
            return hr;
        }
    }

    SetCubesMatrices();

    if (FAILED(CreateShadersSkyBox()))
    {
        MessageBox(hWnd, L"Failed to create shaders for skybox.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateConstantBuffersSkybox())) {
        MessageBox(hWnd, L"Failed to create constant buffers for skybox.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateBuffersSkyBox())) {
        MessageBox(hWnd, L"Failed to create skybox buffers.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateShadersPlane())) {
        MessageBox(hWnd, L"Failed to create plane shaders.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateConstantBuffersPlane())) {
        MessageBox(hWnd, L"Failed to create constant buffers for planes.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateConstantColorBuffer())) {
        MessageBox(hWnd, L"Failed to create constant color buffers.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateBuffersPlane())) {
        MessageBox(hWnd, L"Failed to create plane buffers.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(LoadTexture_2(L"naggets2.dds"))) {
        MessageBox(hWnd, L"Failed to load texture.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(LoadTexture(L"material.dds"))) {
        MessageBox(hWnd, L"Failed to load texture.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(LoadNormalMap(L"materialNM.dds"))) {
        MessageBox(hWnd, L"Failed to load normal map texture.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(LoadSkybox(L"cubemap.dds"))) {
        MessageBox(hWnd, L"Failed to load cubemap.", L"Error", MB_OK);
        return hr;
    }

    if (FAILED(CreateSampler())) {
        MessageBox(hWnd, L"Failed to create sampler.", L"Error", MB_OK);
        return hr;
    }
    
    return S_OK;
}

HRESULT LoadTexture(const wchar_t* filename) {
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DirectX::CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &g_pTextureView);
    return hr;
}

HRESULT LoadTexture_2(const wchar_t* filename) {
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DirectX::CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &g_pTextureView_naggets);
    return hr;
}

HRESULT LoadNormalMap(const wchar_t* filename) {
    DirectX::ScratchImage image;
    HRESULT hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, nullptr, image);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DirectX::CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &g_pNormalMapSRV);
    return hr;
}

HRESULT LoadSkybox(const wchar_t* filename)
{
    HRESULT hr = S_OK;

    // Загрузка кубической текстуры (Skybox)
    hr = DirectX::CreateDDSTextureFromFile(
        g_pDevice,
        filename,    // Путь к DDS-файлу с текстурой
        nullptr,             // Параметры загрузки
        &g_pCubemapView      // Указатель на Shader Resource View для кубической текстуры
    );

    if (FAILED(hr))
    {
        return hr; // Возврат ошибки, если текстура не загрузилась
    }

    return hr; // Возвращаем статус успеха
}

HRESULT CreatePostProcessingResources(UINT screenWidth, UINT screenHeight)
{
    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    hr = g_pDevice->CreateTexture2D(&textureDesc, nullptr, &g_pPostProcessingTexture);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create post-processing texture.", L"Error", MB_OK | MB_ICONERROR);
        return hr;
    }
    hr = g_pDevice->CreateRenderTargetView(g_pPostProcessingTexture, nullptr, &g_pPostProcessingRTV);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create render target view for post-processing.", L"Error", MB_OK | MB_ICONERROR);
        if (g_pPostProcessingTexture) g_pPostProcessingTexture->Release();
        return hr;
    }

    hr = g_pDevice->CreateShaderResourceView(g_pPostProcessingTexture, nullptr, &g_pPostProcessingSRV);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create shader resource view for post-processing.", L"Error", MB_OK | MB_ICONERROR);
        if (g_pPostProcessingTexture) g_pPostProcessingTexture->Release();
        if (g_pPostProcessingRTV) g_pPostProcessingRTV->Release();
        return hr;
    }

    return S_OK;
}

HRESULT CreateSampler() {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.MaxLOD = FLT_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    return g_pDevice->CreateSamplerState(&samplerDesc, &g_pSamplerState);
}

void CleanupDirectX()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pConstantBufferM) g_pConstantBufferM->Release();
    if (g_pConstantBufferM2) g_pConstantBufferM2->Release();
    if (g_pConstantBufferM3) g_pConstantBufferM3->Release();
    if (g_pConstantBufferVP) g_pConstantBufferVP->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pInputLayout) g_pInputLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pSkyboxInputLayout) g_pSkyboxInputLayout->Release();
    if (g_pConstantBufferMSkyBox) g_pConstantBufferMSkyBox->Release();
    if (g_pConstantBufferVPSkyBox) g_pConstantBufferVPSkyBox->Release();
    if (g_pSkyboxVertexShader) g_pSkyboxVertexShader->Release();
    if (g_pSkyboxPixelShader) g_pSkyboxPixelShader->Release();
    if (g_pSkyboxVertexBuffer) g_pSkyboxVertexBuffer->Release();
    if (g_pSkyboxIndexBuffer) g_pSkyboxIndexBuffer->Release();
    if (g_pPlaneInputLayout) g_pPlaneInputLayout->Release();
    if (g_pPlanePixelShader) g_pPlanePixelShader->Release();
    if (g_pPlaneVertexShader) g_pPlaneVertexShader->Release();
    if (g_pPlaneVertexBuffer) g_pPlaneVertexBuffer->Release();
    if (g_pPlaneIndexBuffer) g_pPlaneIndexBuffer->Release();
    if (g_pConstantBufferMPlane) g_pConstantBufferMPlane->Release();
    if (g_pConstantBufferVPPlane) g_pConstantBufferVPPlane->Release();
    if (g_pConstantColorBuffer) g_pConstantColorBuffer->Release();
    if (g_pCubemapView) g_pCubemapView->Release();
    if (g_pTextureView) g_pTextureView->Release(); 
    if (g_pTextureView_naggets) g_pTextureView_naggets->Release();
    if (g_pNormalMapSRV) g_pNormalMapSRV->Release();
    if (g_pSamplerState) g_pSamplerState->Release();
    if (g_pDepthStencilBuffer) g_pDepthStencilBuffer->Release();
    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pDepthStencilState) g_pDepthStencilState->Release();
    if (g_pDepthStencilStateForTransparent) g_pDepthStencilStateForTransparent->Release();
    if (g_pBlendState) g_pBlendState->Release();
    if (g_pRasterizerState) g_pRasterizerState->Release();

    if (g_pLightBuffer) g_pLightBuffer->Release();
    if (g_pLightColorBuffer) g_pLightColorBuffer->Release();
    if (g_pLightPixelShader) g_pLightPixelShader->Release();

    if (g_pPostProcessingLayout) g_pPostProcessingLayout->Release();
    if (g_pPostprocessingVertexBuffer) g_pPostprocessingVertexBuffer->Release();
    if (g_pPostProcessingPS) g_pPostProcessingPS->Release();
    if (g_pPostProcessingVS) g_pPostProcessingVS->Release();
    if (g_pPostProcessingTexture) g_pPostProcessingTexture->Release();
    if (g_pPostProcessingSRV) g_pPostProcessingSRV->Release();
    if (g_pPostProcessingRTV) g_pPostProcessingRTV->Release();
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
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

HRESULT CreateShadersLight()
{
    HRESULT hr = S_OK;

    ID3DBlob* pPSLightBlob = nullptr;
    hr = CompileShader(L"PointLightPixelShader.hlsl", "PSMain", "ps_5_0", &pPSLightBlob);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreatePixelShader(pPSLightBlob->GetBufferPointer(), pPSLightBlob->GetBufferSize(), nullptr, &g_pLightPixelShader);
    pPSLightBlob->Release();
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateShadersSkyBox() {
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlobSkyBox = nullptr;
    hr = CompileShader(L"SkyBoxVertexShader.hlsl", "VSMain", "vs_5_0", &pVSBlobSkyBox);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreateVertexShader(pVSBlobSkyBox->GetBufferPointer(), pVSBlobSkyBox->GetBufferSize(), nullptr, &g_pSkyboxVertexShader);
    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = g_pDevice->CreateInputLayout(skyboxLayout,
        ARRAYSIZE(skyboxLayout), pVSBlobSkyBox->GetBufferPointer(), pVSBlobSkyBox->GetBufferSize(), &g_pSkyboxInputLayout);
    pVSBlobSkyBox->Release();
    if (FAILED(hr)) return hr;

    ID3DBlob* pPSBlobSkyBox = nullptr;
    hr = CompileShader(L"SkyBoxPixelShader.hlsl", "PSMain", "ps_5_0", &pPSBlobSkyBox);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreatePixelShader(pPSBlobSkyBox->GetBufferPointer(), pPSBlobSkyBox->GetBufferSize(), nullptr, &g_pSkyboxPixelShader);
    pPSBlobSkyBox->Release();

    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateShadersPlane() {
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlobPlane = nullptr;
    hr = CompileShader(L"PlaneVertexShader.hlsl", "VSMain", "vs_5_0", &pVSBlobPlane);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreateVertexShader(pVSBlobPlane->GetBufferPointer(), pVSBlobPlane->GetBufferSize(), nullptr, &g_pPlaneVertexShader);
    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC PlaneLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = g_pDevice->CreateInputLayout(PlaneLayout,
        ARRAYSIZE(PlaneLayout), pVSBlobPlane->GetBufferPointer(), pVSBlobPlane->GetBufferSize(), &g_pPlaneInputLayout);
    pVSBlobPlane->Release();
    if (FAILED(hr)) return hr;

    ID3DBlob* pPSBlobPlane = nullptr;
    hr = CompileShader(L"PlanePixelShader.hlsl", "PSMain", "ps_5_0", &pPSBlobPlane);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreatePixelShader(pPSBlobPlane->GetBufferPointer(), pPSBlobPlane->GetBufferSize(), nullptr, &g_pPlanePixelShader);
    pPSBlobPlane->Release();

    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateShadersPostProcess()
{
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlobPP = nullptr;
    hr = CompileShader(L"VertexShaderPostProcess.hlsl", "VSMain", "vs_5_0", &pVSBlobPP);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreateVertexShader(pVSBlobPP->GetBufferPointer(), pVSBlobPP->GetBufferSize(), nullptr, &g_pPostProcessingVS);
    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC PostProcessingLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = g_pDevice->CreateInputLayout(PostProcessingLayout,
        ARRAYSIZE(PostProcessingLayout), pVSBlobPP->GetBufferPointer(), pVSBlobPP->GetBufferSize(), &g_pPostProcessingLayout);
    pVSBlobPP->Release();
    if (FAILED(hr)) return hr;

    ID3DBlob* pPSBlobPP = nullptr;
    hr = CompileShader(L"PixelShaderPostProcess.hlsl", "PSMain", "ps_5_0", &pPSBlobPP);
    if (FAILED(hr)) return hr;

    hr = g_pDevice->CreatePixelShader(pPSBlobPP->GetBufferPointer(), pPSBlobPP->GetBufferSize(), nullptr, &g_pPostProcessingPS);
    pPSBlobPP->Release();

    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateConstantBuffers()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC cbdM = {};
    cbdM.Usage = D3D11_USAGE_DEFAULT;
    cbdM.ByteWidth = sizeof(CNBufferData);
    cbdM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdM.CPUAccessFlags = 0;

    hr = g_pDevice->CreateBuffer(&cbdM, nullptr, &g_pConstantBufferM);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = g_pDevice->CreateBuffer(&cbdM, nullptr, &g_pConstantBufferM2);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = g_pDevice->CreateBuffer(&cbdM, nullptr, &g_pConstantBufferM3);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_BUFFER_DESC cbdVP = {};
    cbdVP.Usage = D3D11_USAGE_DYNAMIC;
    cbdVP.ByteWidth = sizeof(ConstantBufferData);
    cbdVP.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdVP.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pDevice->CreateBuffer(&cbdVP, nullptr, &g_pConstantBufferVP);
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT CreateConstantBuffersSkybox() {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC cbdMSkyBox= {};
    cbdMSkyBox.Usage = D3D11_USAGE_DEFAULT;
    cbdMSkyBox.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdMSkyBox.CPUAccessFlags = 0;
    cbdMSkyBox.ByteWidth = sizeof(ConstantBufferSkyBox);

    hr = g_pDevice->CreateBuffer(&cbdMSkyBox, nullptr, &g_pConstantBufferMSkyBox);
    if (FAILED(hr)) return hr;

    D3D11_BUFFER_DESC cbdVPSkyBox = {};
    cbdVPSkyBox.Usage = D3D11_USAGE_DYNAMIC;
    cbdVPSkyBox.ByteWidth = sizeof(ConstantBufferSkyBox);
    cbdVPSkyBox.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdVPSkyBox.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pDevice->CreateBuffer(&cbdVPSkyBox, nullptr, &g_pConstantBufferVPSkyBox);
    if (FAILED(hr)) return hr;

    return hr;
}

HRESULT CreateConstantBuffersPlane()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC cbdM = {};
    cbdM.Usage = D3D11_USAGE_DEFAULT;
    cbdM.ByteWidth = sizeof(CNBufferData);
    cbdM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdM.CPUAccessFlags = 0;

    hr = g_pDevice->CreateBuffer(&cbdM, nullptr, &g_pConstantBufferMPlane);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_BUFFER_DESC cbdVP = {};
    cbdVP.Usage = D3D11_USAGE_DYNAMIC;
    cbdVP.ByteWidth = sizeof(ConstantBufferData);
    cbdVP.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdVP.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pDevice->CreateBuffer(&cbdVP, nullptr, &g_pConstantBufferVPPlane);
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT CreateConstantColorBuffer() {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC cbdColor = {};
    cbdColor.Usage = D3D11_USAGE_DEFAULT;
    cbdColor.ByteWidth = sizeof(ColorBuffer);
    cbdColor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdColor.CPUAccessFlags = 0;

    hr = g_pDevice->CreateBuffer(&cbdColor, nullptr, &g_pConstantColorBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT CreateLightConstantBuffers() 
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC cbdLight = {};
    cbdLight.Usage = D3D11_USAGE_DEFAULT;
    cbdLight.ByteWidth = sizeof(DirectX::XMFLOAT4);
    cbdLight.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbdLight.CPUAccessFlags = 0;
    hr = g_pDevice->CreateBuffer(&cbdLight, nullptr, &g_pLightColorBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufferDesc.ByteWidth = sizeof(LightBuffer);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = 0;

    hr = g_pDevice->CreateBuffer(&lightBufferDesc, nullptr, &g_pLightBuffer);
    if (FAILED(hr))
        return hr;
}


HRESULT CreateBuffers()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = Vertices;

    hr = g_pDevice->CreateBuffer(&bufferDesc, &initData, &g_pVertexBuffer);
    if (FAILED(hr)) return hr;

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Indices);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    initData.pSysMem = Indices;

    hr = g_pDevice->CreateBuffer(&bufferDesc, &initData, &g_pIndexBuffer);
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateBuffersSkyBox() {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bufferDeskSkyBox = {};
    bufferDeskSkyBox.Usage = D3D11_USAGE_DEFAULT;
    bufferDeskSkyBox.ByteWidth = sizeof(skyboxVertices);
    bufferDeskSkyBox.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDeskSkyBox.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = skyboxVertices;

    hr = g_pDevice->CreateBuffer(&bufferDeskSkyBox, &initData, &g_pSkyboxVertexBuffer);
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateBuffersPlane() {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bufferDescPlane = {};
    bufferDescPlane.Usage = D3D11_USAGE_DEFAULT;
    bufferDescPlane.ByteWidth = sizeof(planeVertices);
    bufferDescPlane.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDescPlane.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = planeVertices;

    hr = g_pDevice->CreateBuffer(&bufferDescPlane, &initData, &g_pPlaneVertexBuffer);
    if (FAILED(hr)) return hr;

    bufferDescPlane.Usage = D3D11_USAGE_DEFAULT;
    bufferDescPlane.ByteWidth = sizeof(planeIndices);
    bufferDescPlane.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDescPlane.CPUAccessFlags = 0;
    initData.pSysMem = planeIndices;

    hr = g_pDevice->CreateBuffer(&bufferDescPlane, &initData, &g_pPlaneIndexBuffer);
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreatePostProcessingBuffers()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bufferDescPP = {};
    bufferDescPP.Usage = D3D11_USAGE_DEFAULT;
    bufferDescPP.ByteWidth = sizeof(FullScreenVertices);
    bufferDescPP.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDescPP.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = FullScreenVertices;

    hr = g_pDevice->CreateBuffer(&bufferDescPP, &initData, &g_pPostprocessingVertexBuffer);
    if (FAILED(hr)) return hr;

    return S_OK;
}

HRESULT CreateDepthStencilBuffer() {
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 800;  
    depthDesc.Height = 600;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    HRESULT hr = g_pDevice->CreateTexture2D(&depthDesc, nullptr, &g_pDepthStencilBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &dsvDesc, &g_pDepthStencilView);
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT CreateDepthStencilState() {
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = FALSE;

    HRESULT hr = g_pDevice->CreateDepthStencilState(&dsDesc, &g_pDepthStencilState);
    if (FAILED(hr))
    {
        return hr;
    }

    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    hr = g_pDevice->CreateDepthStencilState(&dsDesc, &g_pDepthStencilStateForTransparent);
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT CreateBlendState() {
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = g_pDevice->CreateBlendState(&blendDesc, &g_pBlendState);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT CreateRasterizerState() {
    HRESULT hr = S_OK;

    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.AntialiasedLineEnable = TRUE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = FALSE;
    rastDesc.DepthBias = 0;
    rastDesc.SlopeScaledDepthBias = 0.0f;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = TRUE;
    rastDesc.ScissorEnable = FALSE;
    rastDesc.MultisampleEnable = FALSE;

    hr = g_pDevice->CreateRasterizerState(&rastDesc, &g_pRasterizerState);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const DirectX::XMMATRIX& data)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &data, sizeof(DirectX::XMMATRIX));
        context->Unmap(buffer, 0);
    }
}

void RenderSkyBox() 
{
    UINT stride = sizeof(VertexSkybox);
    UINT offset = 0;

    g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pSkyboxVertexBuffer, &stride, &offset);
    g_pDeviceContext->IASetIndexBuffer(g_pSkyboxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    g_pDeviceContext->IASetInputLayout(g_pSkyboxInputLayout);

    g_pDeviceContext->VSSetShader(g_pSkyboxVertexShader, nullptr, 0);
    g_pDeviceContext->PSSetShader(g_pSkyboxPixelShader, nullptr, 0);

    ConstantBufferSkyBox mBufferSkyBox;
    mBufferSkyBox.matrix = DirectX::XMMatrixIdentity();
    mBufferSkyBox.size_or_cameraPos = DirectX::XMFLOAT4(50.0f, 50.0f, 50.0f, 1.0f);

    g_pDeviceContext->UpdateSubresource(g_pConstantBufferMSkyBox, 0, nullptr, &mBufferSkyBox, 0, 0);
    g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferMSkyBox);

    ConstantBufferSkyBox vpBufferSkyBox;
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(
        g_CameraDistance * sin(g_RotationAngleY),
        g_CameraDistance * sin(g_RotationAngleX),
        g_CameraDistance * cos(g_RotationAngleY),
        1.0f
    );

    DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
    vpBufferSkyBox.matrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixLookAtLH(eye, at, up),
        DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f)
    );
    DirectX::XMStoreFloat3(&g_CameraPosition, eye);
    vpBufferSkyBox.size_or_cameraPos = DirectX::XMFLOAT4(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 1.0f);

    D3D11_MAPPED_SUBRESOURCE mappedResourceSkyBox;
    HRESULT hr = g_pDeviceContext->Map(g_pConstantBufferVPSkyBox, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceSkyBox);
    if (SUCCEEDED(hr)) {
        memcpy(mappedResourceSkyBox.pData, &vpBufferSkyBox, sizeof(vpBufferSkyBox));
        g_pDeviceContext->Unmap(g_pConstantBufferVPSkyBox, 0);
    }
    else {
        OutputDebugStringA("Failed to map VPBufferSkyBox.\n");
    }
    g_pDeviceContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferVPSkyBox);

    g_pDeviceContext->PSSetShaderResources(0, 1, &g_pCubemapView);
    g_pDeviceContext->PSSetSamplers(0, 1, &g_pSamplerState);

    g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pDeviceContext->Draw(36, 0);
}

void RenderPlanes() {
    g_pDeviceContext->OMSetBlendState(g_pBlendState, nullptr, 0xffffffff);
    g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateForTransparent, 1);
    g_pDeviceContext->RSSetState(g_pRasterizerState);
    UINT stride = sizeof(VertexPlane);
    UINT offset = 0;
    g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pPlaneVertexBuffer, &stride, &offset);
    g_pDeviceContext->IASetIndexBuffer(g_pPlaneIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    g_pDeviceContext->IASetInputLayout(g_pPlaneInputLayout);
    g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pDeviceContext->VSSetShader(g_pPlaneVertexShader, nullptr, 0);
    g_pDeviceContext->PSSetShader(g_pPlanePixelShader, nullptr, 0);

    struct TransparentObject
    {
        DirectX::XMMATRIX modelMatrix;
        DirectX::XMFLOAT4 color;
        float distanceToCamera;
    };

    std::vector<TransparentObject> transparentObjects;

    DirectX::XMVECTOR cameraPosition = DirectX::XMVectorSet(
        g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 1.0f
    );

    for (int i = 0; i < 2; ++i)
    {
        DirectX::XMVECTOR objPos = DirectX::XMVector3Transform(
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            g_modelMatrices[i]
        );

        DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(objPos, cameraPosition);
        float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

        transparentObjects.push_back({ g_modelMatrices[i], g_ColorBuffers[i], distance});
    }

    std::sort(transparentObjects.begin(), transparentObjects.end(),
        [](const TransparentObject& a, const TransparentObject& b)
        {
            return a.distanceToCamera > b.distanceToCamera;
        });

    for (const auto& obj : transparentObjects)
    {
        CNBufferData mBufferPlane;
        ConstantBufferData vpBufferPlane;
        ColorBuffer colbuf;
        colbuf.color = obj.color;
        DirectX::XMVECTOR eye = DirectX::XMVectorSet(
            g_CameraDistance * sin(g_RotationAngleY),
            g_CameraDistance * sin(g_RotationAngleX),
            g_CameraDistance * cos(g_RotationAngleY),
            1.0f
        );

        DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
        vpBufferPlane.matrix = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixLookAtLH(eye, at, up),
            DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f)
        );

        mBufferPlane.models[0] = obj.modelMatrix;
        DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixInverse(nullptr, obj.modelMatrix);
        normalMatrix = DirectX::XMMatrixTranspose(normalMatrix);
        mBufferPlane.normals[0] = normalMatrix;
        g_pDeviceContext->UpdateSubresource(g_pConstantBufferMPlane, 0, nullptr, &mBufferPlane, 0, 0);
        g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferMPlane);

        UpdateConstantBuffer(g_pDeviceContext, g_pConstantBufferVPPlane, vpBufferPlane.matrix);
        g_pDeviceContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferVPPlane);

        g_pDeviceContext->UpdateSubresource(g_pConstantColorBuffer, 0, nullptr, &colbuf, 0, 0);
        g_pDeviceContext->VSSetConstantBuffers(2, 1, &g_pConstantColorBuffer);
        g_pDeviceContext->DrawIndexed(6, 0, 0);
    }

    g_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    g_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
}

void RenderLightCube(DirectX::XMFLOAT3 lightPosition)
{
    LightBuffer lightData;
    lightData.lightPos = lightPosition;
    lightData.lightColor = lightColor;
    lightData.ambient = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    lightData.cameraPosition = g_CameraPosition;
    g_pDeviceContext->UpdateSubresource(g_pLightBuffer, 0, nullptr, &lightData, 0, 0);
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f);
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
    CNBufferData mBuffer3;
    mBuffer3.models[0] = scale * translation;
    mBuffer3.normals[0] = DirectX::XMMatrixInverse(nullptr, mBuffer3.models[0]);
    mBuffer3.normals[0] = DirectX::XMMatrixTranspose(mBuffer3.normals[0]);
    g_pDeviceContext->UpdateSubresource(g_pConstantBufferM3, 0, nullptr, &mBuffer3, 0, 0);

    g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    g_pDeviceContext->IASetInputLayout(g_pInputLayout);

    g_pDeviceContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pDeviceContext->PSSetShader(g_pLightPixelShader, nullptr, 0);

    g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferM3);
    g_pDeviceContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferVP);

    g_pDeviceContext->DrawIndexedInstanced(36,1, 0, 0, 0);
}

void Render()
{
    ULONGLONG timeCur = GetTickCount64();
    if (g_timeStart == 0)
        g_timeStart = timeCur;

    g_DeltaTime = (timeCur - g_timeStart) / 1000.0f;
    g_timeStart = timeCur;

    if (g_DeltaTime > 0.1f) g_DeltaTime = 0.1f;

    g_RotationAngle += 1.0f * g_DeltaTime;
    if (g_RotationAngle > DirectX::XM_2PI) {
        g_RotationAngle -= DirectX::XM_2PI;
    }

    if (g_EnablePostProcessing)
    {
        g_pDeviceContext->OMSetRenderTargets(1, &g_pPostProcessingRTV, g_pDepthStencilView);
    }
    else
    {
        g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
    }

    ID3D11RenderTargetView* currentRTV = g_EnablePostProcessing ? g_pPostProcessingRTV : g_pRenderTargetView;
    g_pDeviceContext->ClearRenderTargetView(currentRTV, DirectX::Colors::MidnightBlue);
    g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(300, 200));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Light Controls");

    ImGui::SliderFloat("Light X", &g_lightPosition.x, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Y", &g_lightPosition.y, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Z", &g_lightPosition.z, -10.0f, 10.0f);

    ImGui::ColorEdit3("Light Color", reinterpret_cast<float*>(&lightColor));

    ImGui::Checkbox("Enable Post Processing", &g_EnablePostProcessing);
    ImGui::End();

    RenderSkyBox();

    g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilState, 1);
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    g_pDeviceContext->IASetInputLayout(g_pInputLayout);;


    g_pDeviceContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pDeviceContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pDeviceContext->PSSetConstantBuffers(0, 1, &g_pLightBuffer);

    CNBufferData mBuffers;
    ConstantBufferData vpBuffer;

    for (int i = 0; i < NUM_INSTANCES; i++) {
        if(i % 2 != 0)
        g_mBuffers.models[i] = DirectX::XMMatrixRotationY(g_RotationAngle / 1000.0f) * g_mBuffers.models[i];
        g_mBuffers.normals[i] = DirectX::XMMatrixInverse(nullptr, g_mBuffers.models[i]);
        g_mBuffers.normals[i] = DirectX::XMMatrixTranspose(g_mBuffers.normals[i]);
    }

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(
        g_CameraDistance * sin(g_RotationAngleY),
        g_CameraDistance * sin(g_RotationAngleX),
        g_CameraDistance * cos(g_RotationAngleY),
        1.0f
    );

    DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f);
    DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
    vpBuffer.matrix = viewProjMatrix;

    std::array<FrustumPlane, 6> frustumPlanes = ExtractFrustumPlanes(viewProjMatrix);

    std::vector<DirectX::XMFLOAT3> cubePositions = ExtractCubePositions(g_mBuffers);
    float cubeSize = 1.0f;
    std::vector<AABB> cubeAABBs = CreateAABBs(cubePositions, cubeSize);

    std::vector<int> visibleCubeIndices = PerformFrustumCulling(frustumPlanes, cubeAABBs);

    CNBufferData visibleBuffers = {};
    for (size_t i = 0; i < visibleCubeIndices.size(); ++i) {
        int index = visibleCubeIndices[i];
        visibleBuffers.models[i] = g_mBuffers.models[index];
        visibleBuffers.normals[i] = g_mBuffers.normals[index];
        visibleBuffers.isNormalMapActive[i] = g_mBuffers.isNormalMapActive[index];
    }

    g_pDeviceContext->UpdateSubresource(g_pConstantBufferM, 0, nullptr, &visibleBuffers, 0, 0);
    g_pDeviceContext->VSSetConstantBuffers(0, 1, &g_pConstantBufferM);

    UpdateConstantBuffer(g_pDeviceContext, g_pConstantBufferVP, vpBuffer.matrix);
    g_pDeviceContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferVP);

    g_pDeviceContext->PSSetShaderResources(0, 1, &g_pTextureView);
    g_pDeviceContext->PSSetShaderResources(2, 1, &g_pTextureView_naggets);
    g_pDeviceContext->PSSetShaderResources(1, 1, &g_pNormalMapSRV);
    g_pDeviceContext->PSSetSamplers(0, 1, &g_pSamplerState);

    g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pDeviceContext->DrawIndexedInstanced(36, static_cast<UINT>(visibleCubeIndices.size()), 0, 0, 0);

    ImGui::Begin("Statistics");
    ImGui::Text("Visible Cubes: %d", static_cast<int>(visibleCubeIndices.size()));
    ImGui::End();
    RenderLightCube(g_lightPosition);
    RenderPlanes();

    if (g_EnablePostProcessing && g_pPostProcessingRTV && g_pPostProcessingSRV) {
        g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
        g_pDeviceContext->IASetInputLayout(g_pPostProcessingLayout);
        g_pDeviceContext->VSSetShader(g_pPostProcessingVS, nullptr, 0);
        g_pDeviceContext->PSSetShader(g_pPostProcessingPS, nullptr, 0);

        UINT postProcessStride = sizeof(FullScreenVertex);
        UINT postProcessOffset = 0;
        g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        g_pDeviceContext->PSSetShaderResources(0, 1, &g_pPostProcessingSRV);
        g_pDeviceContext->Draw(3, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        g_pDeviceContext->PSSetShaderResources(0, 1, &nullSRV);
    }
    else {
        g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        g_MousePressed = true;
        SetCapture(hWnd);
        GetCursorPos(&g_MousePos);
        break;

    case WM_LBUTTONUP:
        g_MousePressed = false;
        ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if (!ImGui::GetIO().WantCaptureMouse && g_MousePressed)
        {
            POINT newMousePos;
            GetCursorPos(&newMousePos);
            float deltaX = static_cast<float>(newMousePos.x - g_MousePos.x);
            float deltaY = static_cast<float>(newMousePos.y - g_MousePos.y);

            g_RotationAngleY += deltaX * 0.01f; 
            g_RotationAngleX += deltaY * 0.01f; 

            g_MousePos = newMousePos;
        }
        break;
    case WM_MOUSEWHEEL:
    {
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_CameraDistance += wheelDelta > 0 ? -1.0f : 1.0f;

        float minDistance = 1.0f;
        float maxDistance = 40.0f;

        g_CameraDistance = max(g_CameraDistance, minDistance); 
        g_CameraDistance = min(g_CameraDistance, maxDistance); 

        break;
    }
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

    ID3D11DeviceContext* currentContext = g_pDeviceContext;
    currentContext->OMSetRenderTargets(0, nullptr, nullptr);
    g_pRenderTargetView->Release();
    g_pDepthStencilView->Release();
    g_pDepthStencilBuffer->Release();
    g_pPostProcessingRTV->Release();
    g_pPostProcessingSRV->Release();
    g_pPostProcessingTexture->Release();

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

    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    hr = g_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &g_pDepthStencilBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create depth stencil buffer.\n");
        return hr;
    }

    hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, nullptr, &g_pDepthStencilView);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create depth stencil view.\n");
        return hr;
    }

    g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    if (FAILED(CreatePostProcessingResources(width, height)))
    {
        MessageBox(hWnd, L"Failed to create post processing resources.", L"Error", MB_OK);
        return hr;
    }
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