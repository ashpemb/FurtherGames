#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pPyramidIndexBuffer = nullptr;
	_pPyramidVertexBuffer = nullptr;
	_pGridIndexBuffer = nullptr;
	_pGridVertexBuffer = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	objMeshData = OBJLoader::Load("sphere.obj", _pd3dDevice);

	XMFLOAT4 Eye = { 0.0f, 5.0f, -10.0f, 0.0f };
	XMFLOAT4 At = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT4 Up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMFLOAT4 Eye2 = { 0.0f, 0.0f, -10.0f, 0.0f };
	XMFLOAT4 To = { 0.0f, 0.0f, 1.0f, 0.0f };
	XMFLOAT4 Up2 = { 0.0f, 1.0f, 0.0f, 0.0f };

	camera1 = new Camera(Eye, At, Up, _WindowWidth, _WindowHeight);
	camera2 = new LookToCamera(Eye2, To, Up2, _WindowWidth, _WindowHeight);

	lookToMove = { 0.0f, 0.0f, 0.2f, 0.0f };
	lookToMove2 = { 0.0f, 0.0f, -0.2f, 0.0f };
	lookToMoveUpX = { 0.2f, 0.0f, 0.0f, 0.0f };
	lookToMoveDownX = { -0.2f, 0.0f, 0.0f, 0.0f };
	lookToMoveUpY = { 0.0f, 0.2f, 0.0f, 0.0f };
	lookToMoveDownY = { 0.0f, -0.2f, 0.0f, 0.0f };

	activeCamera = 1;

	_View = camera1->CreateView();
	_Projection = camera1->CreateProjection();

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

	XMStoreFloat4x4(&_world2, XMMatrixIdentity());

	XMStoreFloat4x4(&_world3, XMMatrixIdentity());
	XMStoreFloat4x4(&_world4, XMMatrixIdentity());
	XMStoreFloat4x4(&_world5, XMMatrixIdentity());
	XMStoreFloat4x4(&_worldGrid, XMMatrixIdentity());
	XMStoreFloat4x4(&_worldSphere, XMMatrixIdentity());


	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);



	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }



	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);

	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }
	};

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
        0, 1, 2,
		2, 1, 3,

		5, 4, 7,
		7, 4, 6,

		8,9,10,
		10,9,11,

		13,12,15,
		15,12,14,

		16,17,18,
		18,17,19,

		22,23,20,
		20,23,21
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitPyramidVertexBuffer()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex pyramidVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 5;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pyramidVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPyramidVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitPyramidIndexBuffer()
{
	HRESULT hr;

	// Create index buffer
	WORD pyramidIndices[] =
	{
		4,1,0,
		4,3,1,
		4,2,3,
		4,0,2,
		0,1,2,
		2,1,3,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 18;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pyramidIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPyramidIndexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitGridVertexBuffer()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex gridVertices[] =
	{
		{ XMFLOAT3(0.0f, 0.0f, 2.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },		
		{ XMFLOAT3(0.5f, 0.0f, 2.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 2.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.5f, 0.0f, 2.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 0.0f, 2.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 1.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.0f, 1.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 1.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.5f, 0.0f, 1.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 0.0f, 1.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.5f, 0.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.0f, 0.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.5f, 0.0f, 0.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 0.0f, 0.5f), XMFLOAT3(1.0f, -1.0f, 0.0f) },

		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(0.5f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.5f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(2.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, -1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex)* 25;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = gridVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pGridVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitGridIndexBuffer()
{
	HRESULT hr;

	// Create index buffer
	WORD gridIndices[] =
	{
		0, 1, 6,
		0, 6, 5,
		1, 2, 7,
		1, 7, 6,
		2, 3, 8,
		2, 8, 7,
		3, 4, 9,
		3, 9, 8,

		5, 6, 11,
		5, 11, 10,
		6, 7, 12,
		6, 12, 11,
		7, 8, 13,
		7, 13, 12,
		8, 9, 14,
		8, 14, 13,

		10, 11, 16,
		10, 16, 15,
		11, 12, 17,
		11, 17, 16,
		12, 13, 18,
		12, 18, 17,
		13, 14, 19,
		13, 19, 18,

		15, 16, 21,
		15, 21, 20,
		16, 17, 22,
		16, 22, 21,
		17, 18, 23,
		17, 23, 22,
		18, 19, 24,
		18, 24, 23,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD)* 96;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = gridIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pGridIndexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 900, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireframe);

	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_BACK;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_solid);

	if (FAILED(hr))
		return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitPyramidVertexBuffer();
	InitGridVertexBuffer();

    // Set vertex buffer
    
    

	InitIndexBuffer();
	InitPyramidIndexBuffer();
	InitGridIndexBuffer();
    // Set index buffer
    

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;
	// Light direction from surface (XYZ)
	lightDirection = XMFLOAT3(0.0f, 0.0f, -1.0f);
	// Diffuse material properties (RGBA)
	diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
	// Diffuse light colour (RGBA)
	diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	specMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	specLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	specPower = float(10.0f);
	eyePos = XMFLOAT3(0.0f, 5.0f, -10.0f);
	

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pPyramidIndexBuffer) _pPyramidIndexBuffer->Release();
	if (_pPyramidVertexBuffer) _pPyramidVertexBuffer->Release();
	if (_pGridVertexBuffer) _pGridVertexBuffer->Release();
	if (_pGridIndexBuffer) _pGridIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireframe) _wireframe->Release();
	if (_solid) _solid->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

	gTime = t;



	if (GetAsyncKeyState('V'))
	{
		_pImmediateContext->RSSetState(_solid);
	}
	if (GetAsyncKeyState('B'))
	{
		_pImmediateContext->RSSetState(_wireframe);
	}

	if (GetAsyncKeyState('Z'))
	{
		_View = camera2->CreateView();
		_Projection = camera2->CreateProjection();
		activeCamera = 2;
	}

	if (GetAsyncKeyState('X'))
	{
		_View = camera1->CreateView();
		_Projection = camera1->CreateProjection();
		activeCamera = 1;
	}
	if (activeCamera == 1)
	{
		if (GetAsyncKeyState('W'))
		{
			float speed = 0.1f;
			camera1->ZoomEye(gTime, speed);
			_View = camera1->CreateView();
		}

		if (GetAsyncKeyState('S'))
		{
			float speed = -0.1f;
			camera1->ZoomEye(gTime, speed);
			_View = camera1->CreateView();
		}
	}
	else if (activeCamera == 2)
	{

		if (GetAsyncKeyState('W'))
		{
			camera2->MoveEye(lookToMove, gTime);
			_View = camera2->CreateView();
		}

		if (GetAsyncKeyState('S'))
		{
			camera2->MoveEye(lookToMove2, gTime);
			_View = camera2->CreateView();
		}

		if (GetAsyncKeyState('A'))
		{
			camera2->MoveEye(lookToMoveDownX, gTime);
			_View = camera2->CreateView();
		}

		if (GetAsyncKeyState('D'))
		{
			camera2->MoveEye(lookToMoveUpX, gTime);
			_View = camera2->CreateView();
		}

		if (GetAsyncKeyState('R'))
		{
			camera2->MoveEye(lookToMoveUpY, gTime);
			_View = camera2->CreateView();
		}

		if (GetAsyncKeyState('F'))
		{
			camera2->MoveEye(lookToMoveDownY, gTime);
			_View = camera2->CreateView();
		}
	}

    //
    // Animate the cube
    //
	XMStoreFloat4x4(&_world, XMMatrixRotationY(t));
	XMStoreFloat4x4(&_world2, XMMatrixTranslation(0.0f, 0.0f, 20.0f));
	XMStoreFloat4x4(&_world3, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(7.0f, 0.0f, 0.0f) * XMMatrixRotationY(t) * XMMatrixTranslation(15.0f, 0.0f, 0.0f) * XMMatrixRotationY(t));
	XMStoreFloat4x4(&_world4, XMMatrixRotationY(2*t) * XMMatrixTranslation(-25.0f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	XMStoreFloat4x4(&_world5, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(7.0f, 0.0f, 0.0f) * XMMatrixRotationY(-2*t) * XMMatrixTranslation(-25.0f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	XMStoreFloat4x4(&_worldGrid, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, -5.0f));
	XMStoreFloat4x4(&_worldSphere, XMMatrixTranslation(0.0f, 10.0f, 0.0f) * XMMatrixRotationY(t));
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //



    float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);

	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_View);
	XMMATRIX projection = XMLoadFloat4x4(&_Projection);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;


    //
    // Update variables
    //

    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	cb.DiffuseLight = diffuseLight;
	cb.DiffuseMtrl = diffuseMaterial;
	cb.LightVecW = lightDirection;
	cb.AmbientLight = ambientLight;
	cb.AmbientMaterial = ambientMaterial;
	cb.gTime = gTime;
	cb.SpecularMaterial = specMaterial;
	cb.SpecularLight = specLight;
	cb.SpecularPower = specPower;
	cb.EyePosW = eyePos;

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);



	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //
    // Renders a triangle
    //
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DrawIndexed(36, 0, 0);   

	_pImmediateContext->IASetVertexBuffers(0, 1, &_pPyramidVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_pPyramidIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	world = XMLoadFloat4x4(&_world2);
	// 2nd cube
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(18, 0, 0);

	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	world = XMLoadFloat4x4(&_world3);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);

	world = XMLoadFloat4x4(&_world4);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);

	world = XMLoadFloat4x4(&_world5);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(36, 0, 0);

	_pImmediateContext->IASetVertexBuffers(0, 1, &_pGridVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_pGridIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	world = XMLoadFloat4x4(&_worldGrid);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(96, 0, 0);

	_pImmediateContext->IASetVertexBuffers(0, 1, &objMeshData.VertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(objMeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	world = XMLoadFloat4x4(&_worldSphere);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}