#pragma once
#ifndef CAMERAROTATIONS
#define CAMERAROTATIONS
void CameraRotate( RwCamera *camera, const RwV3d *pos, RwReal angle );
void CameraRotateAroundAxis( RwCamera *camera, const RwV3d *axis, const RwV3d *pos, RwReal angle );
void CameraRotate2( RwCamera *camera, const RwV3d *pos, RwReal angle );
float rwV3D_Dist( const RwV3d& a, const RwV3d& b );
RwV3d rwV3D_Normalize( const RwV3d& a );
RwV3d rwV3D_Cross( const RwV3d& a, const RwV3d& b );
float rwV3D_Dot( const RwV3d& a, const RwV3d& b );
RwV3d rwV3D_Min( const RwV3d& a, const RwV3d& b );
RwV3d rwV3D_Max( const RwV3d& a, const RwV3d& b );
#endif