#include "LookToCamera.h"


LookToCamera::LookToCamera(XMFLOAT4 _eye, XMFLOAT4 _to, XMFLOAT4 _up, UINT WindowWidth, UINT WindowHeight)
{
	Eye = _eye;
	To = _to;
	Up = _up;

	_WindowWidth = WindowWidth;
	_WindowHeight = WindowHeight;
}

XMFLOAT4X4 LookToCamera::CreateView()
{
	XMFLOAT4X4 view;
	XMVECTOR eye = XMLoadFloat4(&Eye);
	XMVECTOR at = XMLoadFloat4(&To);
	XMVECTOR up = XMLoadFloat4(&Up);
	XMStoreFloat4x4(&view, XMMatrixLookToLH(eye, at, up));
	return view;
}


XMFLOAT4X4 LookToCamera::CreateProjection()
{
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT)_WindowHeight, 0.01f, 100.0f));
	return projection;
}

void LookToCamera::MoveEye(XMFLOAT4 _move, float gTime)
{
	XMFLOAT4 move = _move;
	gTime = gTime / 10000000.0f;
	Eye.w = Eye.w + (move.w * gTime);
	Eye.x = Eye.x + (move.x * gTime);
	Eye.y = Eye.y + (move.y * gTime);
	Eye.z = Eye.z + (move.z * gTime);
	CreateView();
}

XMFLOAT4 LookToCamera::GetVector()
{
	return Eye;
}

LookToCamera::~LookToCamera()
{
}
