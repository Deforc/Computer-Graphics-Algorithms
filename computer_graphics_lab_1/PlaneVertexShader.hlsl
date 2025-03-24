cbuffer MBuffer : register(b0)
{
    float4x4 model;
    float4x4 normal;
};

cbuffer VPBuffer : register(b1)
{
    float4x4 vp;
};

cbuffer ColorBuffer : register(b2)
{
    float4 color;
};

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float4 color : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    output.pos = mul(vp, worldPos);
    output.worldPos = worldPos.xyz;
    output.normal = mul((float3x3) normal, input.normal);
    output.color = color;
    return output;
}

    