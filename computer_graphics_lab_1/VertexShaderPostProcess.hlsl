struct VSInput
{
    uint vertexId : SV_VertexID;
};
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput result;
    float4 pos = float4(0, 0, 0, 0);
    switch (input.vertexId)
    {
        case 0:
            pos = float4(-1, -3, 0, 1);
            break;
        case 1:
            pos = float4(-1, 1, 0, 1);
            break;
        case 2:
            pos = float4(3, 1, 0, 1);
            break;
    }
    result.Pos = pos;
    result.TexCoord = float2(pos.x * 0.5 + 0.5, 0.5 - pos.y * 0.5);
    return result;
}