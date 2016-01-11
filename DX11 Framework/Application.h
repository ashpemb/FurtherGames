#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "Camera.h"
#include "LookToCamera.h"
#include "Structures.h"
#include "OBJLoader.h"
#include "GameObject.h"

using namespace DirectX;

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11RasterizerState*  _wireframe;
	ID3D11RasterizerState*  _solid;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pPyramidVertexBuffer;
	ID3D11Buffer*           _pGridVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pPyramidIndexBuffer;
	ID3D11Buffer*           _pGridIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;
	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11ShaderResourceView * _pTexturePlane = nullptr;
	ID3D11ShaderResourceView * _pTextureTerrain = nullptr;
	ID3D11SamplerState * _pSamplerLinear = nullptr;
	MeshData				objMeshData;
	MeshData				planeMesh;
	MeshData				terrainMesh;
	MeshData				starMesh;
	GameObject*				_sphere;
	GameObject*				_terrain;
	GameObject*				_plane;
	GameObject*             _star;
	XMFLOAT4X4              _world;
	XMFLOAT4X4				_world2;
	XMFLOAT4X4				_world3;
	XMFLOAT4X4				_world4;
	XMFLOAT4X4				_world5;
	XMFLOAT4X4				_worldGrid;
	XMFLOAT3				lightDirection;
	XMFLOAT4				diffuseMaterial;
	XMFLOAT4				diffuseLight;
	XMFLOAT4				ambientLight;
	XMFLOAT4				ambientMaterial;
	XMFLOAT4				specMaterial;
	XMFLOAT4				specLight;
	float					specPower;
	XMFLOAT3				eyePos;
	float					gTime;
	Camera*					camera1;
	LookToCamera*			camera2;
	LookToCamera*			camera3;
	LookToCamera*			camera4;
	LookToCamera*			camera5;
	XMFLOAT4X4				_View;
	XMFLOAT4X4				_Projection;
	XMFLOAT4				lookToMove;
	XMFLOAT4				lookToMove2;
	XMFLOAT4				lookToMoveUpX;
	XMFLOAT4				lookToMoveDownX;
	XMFLOAT4				lookToMoveUpY;
	XMFLOAT4				lookToMoveDownY;
	int						activeCamera;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	HRESULT InitPyramidVertexBuffer();
	HRESULT InitPyramidIndexBuffer();
	HRESULT InitGridVertexBuffer();
	HRESULT InitGridIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();


};

