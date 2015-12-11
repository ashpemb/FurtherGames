#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

class Camera
{
public:
	Camera(XMFLOAT4 _eye, XMFLOAT4 _at, XMFLOAT4 _up, UINT WindowWidth, UINT WindowHeight);
	~Camera();
	XMFLOAT4 Eye;
	XMFLOAT4 At;
	XMFLOAT4 Up;
	//XMFLOAT4X4              _view;
	//XMFLOAT4X4              _projection;
	UINT _WindowWidth;
	UINT _WindowHeight;



	XMFLOAT4X4 CreateView();
	XMFLOAT4X4 CreateProjection();
};

