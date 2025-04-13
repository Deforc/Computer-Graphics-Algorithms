Texture2D sceneTex : register(t0);
SamplerState samLinear : register(s0);

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 PSMain(VSOutput input) : SV_Target
{
    float4 color = sceneTex.Sample(samLinear, input.TexCoord);
    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
    float3 sepia = float3(
        gray * 1.2f,
        gray * 0.9f,
        gray * 0.6f
    );
    
    return float4(sepia, color.a);
}