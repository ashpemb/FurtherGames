#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;
	float gTime;
	XMFLOAT4 AmbientMaterial;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;
};

struct MeshData
{
	ID3D11Buffer * VertexBuffer;
	ID3D11Buffer * IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};