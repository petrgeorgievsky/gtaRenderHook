#include "stdafx.h"
#include "RwMethods.h"

void
CameraRotate(RwCamera *camera, const RwV3d *pos, RwReal angle)
{
	RwV3d invCamPos;
	RwFrame *cameraFrame;
	RwMatrix *cameraMatrix;
	RwV3d camPos;

	cameraFrame = RwCameraGetFrame(camera);
	cameraMatrix = RwFrameGetMatrix(cameraFrame);

	camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

	RwV3dScale(&invCamPos, &camPos, -1.0f);

	/*
	* Translate the camera back to the rotation origin...
	*/
	RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

	/*
	* Get the camera's AT vector and use this as the axis of rotation...
	*/
	RwMatrixRotate(cameraMatrix, RwMatrixGetRight(cameraMatrix),
		angle, rwCOMBINEPOSTCONCAT);

	/*
	* Translate the camera back to its original position...
	*/
	RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);

	return;
}
void
CameraRotateAroundAxis(RwCamera *camera, const RwV3d *axis, const RwV3d *pos, RwReal angle)
{
	RwV3d invCamPos;
	RwFrame *cameraFrame;
	RwMatrix *cameraMatrix;
	RwV3d camPos;

	cameraFrame = RwCameraGetFrame(camera);
	cameraMatrix = RwFrameGetMatrix(cameraFrame);

	camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

	RwV3dScale(&invCamPos, &camPos, -1.0f);

	RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

	RwMatrixRotate(cameraMatrix, axis, angle, rwCOMBINEPOSTCONCAT);

	RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);

	return;
}
void
CameraRotate2(RwCamera *camera, const RwV3d *pos, RwReal angle)
{
	RwV3d invCamPos;
	RwFrame *cameraFrame;
	RwMatrix *cameraMatrix;
	RwV3d camPos;

	cameraFrame = RwCameraGetFrame(camera);
	cameraMatrix = RwFrameGetMatrix(cameraFrame);

	camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

	RwV3dScale(&invCamPos, &camPos, -1.0f);

	/*
	* Translate the camera back to the rotation origin...
	*/
	RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

	/*
	* Get the camera's AT vector and use this as the axis of rotation...
	*/
	RwMatrixRotate(cameraMatrix, RwMatrixGetUp(cameraMatrix),
		angle, rwCOMBINEPOSTCONCAT);

	/*
	* Translate the camera back to its original position...
	*/
	RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);

	return;
}

float rwV3D_Dist(const RwV3d &a, const RwV3d &b) {
	float
		dX = b.x - a.x,
		dY = b.y - a.y,
		dZ = b.z - a.z;

	return sqrt(dX*dX + dY*dY + dZ*dZ);
}
RwV3d rwV3D_Normalize(const RwV3d & a)
{
	float length = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	return { a.x / length ,a.y / length,a.z / length };
}
RwV3d rwV3D_Cross(const RwV3d & a, const RwV3d & b)
{
	return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z,     a.x*b.y - a.y*b.x };
}
float rwV3D_Dot(const RwV3d & a, const RwV3d & b)
{
	return { a.x*b.x + a.y*b.y + a.z*b.z };
}

RwV3d rwV3D_Max(const RwV3d & a, const RwV3d & b)
{
	return (a.x>b.x&&a.y>b.y&&a.z>b.z)?a:b;
}
RwV3d rwV3D_Min(const RwV3d & a, const RwV3d & b)
{
	return (a.x>b.x&&a.y>b.y&&a.z>b.z) ? b : a;
}
