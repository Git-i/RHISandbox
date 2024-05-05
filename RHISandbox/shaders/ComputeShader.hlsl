
struct ClusterAABB
{
    float4 minExtent;
    float4 maxExtent;
};

cbuffer buf0 : register(b0, space0)
{
    float2 resolution;
    uint3 dimensions;
}
RWStructuredBuffer<ClusterAABB> outbuf : register(u1, space0);

[numthreads(1, 1, 1)] //num thread groups
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 eyePos = float3(0, 0, 0);
    //one thread per cluster
    ClusterAABB aabb;
    float2 clusterSize = resolution / dimensions.xy;
    aabb.minExtent= float4(
        clusterSize * DTid.xy,
        0.1f * pow(1000.f / 0.1f, (float)DTid.z / float(dimensions.z)),
        0.0);
    aabb.maxExtent = float4(
        clusterSize * (DTid.xy + uint2(1, 1)),
        0.1f * pow(1000.f / 0.1f, (float) (1 + DTid.z) / float(dimensions.z)), 0.0);
    outbuf[DTid.x  + DTid.y * dimensions.x + DTid.z * dimensions.x * dimensions.y] = aabb;
                                

}