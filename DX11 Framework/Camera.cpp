#include "Camera.h"


Camera::Camera(XMFLOAT4 _eye, XMFLOAT4 _at, XMFLOAT4 _up, UINT WindowWidth, UINT WindowHeight)
{
	Eye = _eye;
	At = _at;
	Up = _up;

	_WindowWidth = WindowWidth;
	_WindowHeight = WindowHeight;
}

XMFLOAT4X4 Camera::CreateView()
{
	XMFLOAT4X4 view;
	XMVECTOR eye = XMLoadFloat4(&Eye);
	XMVECTOR at = XMLoadFloat4(&At);
	XMVECTOR up = XMLoadFloat4(&Up);
	XMStoreFloat4x4(&view, XMMatrixLookAtLH(eye, at, up));
	return view;
}


XMFLOAT4X4 Camera::CreateProjection()
{
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT)_WindowHeight, 0.01f, 100.0f));
	return projection;
}

Camera::~Camera()
{
}
