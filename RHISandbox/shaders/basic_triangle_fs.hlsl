Texture2D tex : register(t2, space1);
float4 main(float3 color : COLOR, float4 scrnSpc : SV_Position) : SV_Target
{
    int3 loc = {(scrnSpc.x/1280.f) * 512, (scrnSpc.y/720.f) * 512, 0};
    return float4(color, 1.f);
}