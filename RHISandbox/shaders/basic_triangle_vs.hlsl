cbuffer buf :register(b0)
{
    float4x4 model;
};
cbuffer buf2 :register(b1)
{
    float4x4 viewa;
}
cbuffer buafzasd3 :register(b0, space1)
{
    float4x4 projection;
};
cbuffer buf4 :register(b2, space1)
{
    float4x4 padasda;
}

struct VS_OUT
{
    float3 color : COLOR;
    float4 position : SV_Position;
};
VS_OUT main(float3 pos : SEM0, float3 color : SEM1)
{
    float4x4 lmao =
    {
        model._21, model._12, model._13, model._14,
        model._11, model._22, model._23, model._24,
        model._31, model._32, model._33, model._34,
        model._41, model._42, model._43, model._44
    };
    VS_OUT vso;
    vso.position = float4(pos, 1.0);
    vso.position = mul(vso.position, model);
    vso.position = mul(vso.position, viewa);
    vso.position = mul(vso.position, projection);
    float4 lol = mul(vso.position, padasda);
    vso.color = lol.xyz;
    vso.color = color;
    return vso;
}