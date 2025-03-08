struct VSInput
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};

cbuffer mBuffer : register(b0)
{
    float4x4 model;
};

cbuffer vpBuffer : register(b1)
{
    float4x4 vp;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 WorldPos = float4(input.pos, 1.0f);
    output.pos = mul(WorldPos, model);
    output.pos = mul(vp, output.pos);
    output.tex = input.tex;
    return output;
}