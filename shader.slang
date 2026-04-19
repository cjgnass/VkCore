struct UBO {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

struct VertexInput {
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float3 color;
};

struct VertexOutput {
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    float4 worldPos = mul(ubo.model, float4(input.position, 1.0));
    float4 viewPos  = mul(ubo.view, worldPos);
    output.sv_position = mul(ubo.proj, viewPos);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;
    return float4(color, 1.0);
}
