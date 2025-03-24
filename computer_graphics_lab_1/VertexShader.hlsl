struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 tex : TEXCOORD2;
};

cbuffer mBuffer : register(b0)
{
    float4x4 model;
    float4x4 normal;
};

cbuffer vpBuffer : register(b1)
{
    float4x4 vp;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 WorldPos = mul(model, float4(input.pos, 1.0));
    output.pos = mul(vp, WorldPos);
    output.worldPos = WorldPos.xyz;
    output.normal = mul((float3x3) normal, input.normal);
    output.tex = input.tex;
    return output;
}