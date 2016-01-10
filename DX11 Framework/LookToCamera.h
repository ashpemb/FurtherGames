#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

class LookToCamera
{
public:
	LookToCamera(XMFLOAT4 _eye, XMFLOAT4 _to, XMFLOAT4 _up, UINT WindowWidth, UINT WindowHeight);
	~LookToCamera();

	XMFLOAT4X4 CreateView();
	XMFLOAT4X4 CreateProjection();
	void MoveEye(XMFLOAT4 _move, float gtime);
	XMFLOAT4 GetVector();

private:
	XMFLOAT4 Eye;
	XMFLOAT4 To;
	XMFLOAT4 Up;
	UINT _WindowWidth;
	UINT _WindowHeight;
};

