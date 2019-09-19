////////////////////////////////////////////////////////////////////////////////
// Filename: shadowmap.vs
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer LightBuffer : register(b1)
{
	matrix g_lightViewMatrix;
	matrix g_lightProjectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 lightViewPosition : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	//Calculate object seen by light
	output.lightViewPosition = mul(input.position, worldMatrix);
	output.lightViewPosition = mul(output.lightViewPosition, g_lightViewMatrix);
	output.lightViewPosition = mul(output.lightViewPosition, g_lightProjectionMatrix);

    return output;
}