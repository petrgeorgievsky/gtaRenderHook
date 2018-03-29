RWTexture3D<float4> txVoxelGrid : register(u2);


// Injects 
void InjectColorAndNormal(int3 pos, float4 color, float3 normals)
{
    txVoxelGrid[pos] = color;
}