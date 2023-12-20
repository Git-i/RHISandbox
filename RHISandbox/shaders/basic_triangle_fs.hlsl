Texture2D tex : register(t2, space1);
float4 main(float3 color : COLOR, float4 scrnSpc : SV_Position) : SV_Target
{
    SamplerState ss;
    float2 loc = {(scrnSpc.x/1280.f), (scrnSpc.y/720.f)};
    return tex.Sample(ss, loc);
}