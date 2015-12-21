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


	XMFLOAT4X4 CreateView();
	XMFLOAT4X4 CreateProjection();
	void ZoomEye(float gtime, float speed);
	void LoadVectors();

private:
	XMFLOAT4 Eye;
	XMFLOAT4 At;
	XMFLOAT4 Up;
	XMFLOAT4 Zoom;
	XMVECTOR eye;
	XMVECTOR at;
	XMVECTOR up;
	UINT _WindowWidth;
	UINT _WindowHeight;
};

