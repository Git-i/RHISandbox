
RWStructuredBuffer<float4> tex : register(u0, space0);
StructuredBuffer<float4> tax : register(t1, space0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    tex[DTid.x] = float4((float) DTid.x / 256.f, (float) DTid.y / 256.f, 1.f,tax[DTid.y].z);
}