#include "Camera.h"


Camera::Camera(XMFLOAT4 _eye, XMFLOAT4 _at, XMFLOAT4 _up, UINT WindowWidth, UINT WindowHeight)
{
	Eye = _eye;
	At = _at;
	Up = _up;

	_WindowWidth = WindowWidth;
	_WindowHeight = WindowHeight;

	Zoom.x = At.x - Eye.x;
	Zoom.y = At.y - Eye.y;
	Zoom.z = At.z - Eye.z;
}

XMFLOAT4X4 Camera::CreateView()
{
	XMFLOAT4X4 view;
	LoadVectors();
	XMStoreFloat4x4(&view, XMMatrixLookAtLH(eye, at, up));
	return view;
}

XMFLOAT4X4 Camera::CreateProjection()
{
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT)_WindowHeight, 0.01f, 100.0f));
	return projection;
}

void Camera::ZoomEye(float gTime, float Speed)
{
	gTime = gTime / 1000.0f;
	Eye.x += (Zoom.x * Speed) * gTime;
	Eye.y += (Zoom.y * Speed) * gTime;
	Eye.z += (Zoom.z * Speed) * gTime;
	
}

void Camera::LoadVectors()
{
	eye = XMLoadFloat4(&Eye);
	at = XMLoadFloat4(&At);
	up = XMLoadFloat4(&Up);
}

Camera::~Camera()
{
}
