#pragma once
#include "D3D1XShader.h"
#include "D3D1XConstantBuffer.h"
struct CBVoxel
{
    RwMatrix View[6];
    float  voxelGridScale;
    float  _padding[3];
};
class CVoxelOctreeRenderer
{
private:

    static RwTexture* voxelOctreeBricks;
    static RwTexture* voxelOctreeStruct;
    static RwRaster* voxelRadiance[3];

    static RwRaster* voxelClipMap[3];

    //static RwRaster* voxelNormals[4];
    static CD3D1XComputeShader* m_voxelCS;
    //static CD3D1XComputeShader* m_blurCS;
    static CD3D1XComputeShader* m_radianceInjectCS;
    const static UINT32 voxelTreeSize = 32;
    static CD3D1XConstantBuffer<CBVoxel>* m_pVoxelCB;
public:
    static RwCamera* m_pVoxelCamera;
    static void Init();
    static void Shutdown();
    static void CleanVoxelOctree();
    static void InjectRadiance( RwRaster* shadow, void( *emmissiveObjRender )( ), int voxelTextureID = 0 );
    static void FilterVoxelOctree();
    static void RenderToVoxelOctree( void( *render )( ), const int &lod );
    static void SetVoxelTex( RwRGBA &color );
    static void SetVoxelLODSize( const int &lod );
};
/*class VoxelOctree {
private:
    CComPtr<ID3D11Buffer>						m_VoxelDataBuffer;
    CComPtr<ID3D11Buffer>						m_VoxelLookupUAV;
    CComPtr<ID3D11UnorderedAccessView>			m_VoxelDataUAV;
    CComPtr<ID3D11UnorderedAccessView>			m_VoxelLookupUAV;
    CComPtr<ID3D11ShaderResourceView>			m_VoxelDataSRV;
    CComPtr<ID3D11ShaderResourceView>			m_VoxelLookupSRV;
public:
    VoxelOctree(ID3D11Device * pDev) {
        D3D11_BUFFER_DESC desc = {};
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

        pDev->CreateBuffer(&desc,nullptr,&m_VoxelDataBuffer);
    }
};*/