#define set1 space1
Texture2D tex[2] : register(t2, set1);
StructuredBuffer<float4> buf : register(t5, space1);
#undef set1
float4 main(float3 color : COLOR, float4 scrnSpc : SV_Position) : SV_Target
{
    int3 loc = {(scrnSpc.x/1280.f) * 512, (scrnSpc.y/720.f) * 512, 0};
    float4 lol = tex[0].Load(loc);
    lol *= buf[0];
    int set1;
    int space1;
    return lol;
}