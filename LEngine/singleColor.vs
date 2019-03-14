////////////////////////////////////////////////////////////////////////////////
// Filename: skybox.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer LightBuffer
{
	matrix g_lightViewMatrix;
	matrix g_lightProjectionMatrix;
	float3 g_lightPosition;
	float g_lightBufferPadding;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 lightViewPosition : TEXCOORD0;
	float3 lightPos : TEXCOORD1;
	float3 normal : NORMAL;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;

    input.position.w = 1.0f;
	worldPosition = mul(input.position, worldMatrix);
	output.normal = input.normal;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	//Calculate object seen by light
	matrix lightViewProj = mul(g_lightProjectionMatrix, g_lightViewMatrix);

	output.lightViewPosition = mul(input.position, lightViewProj);
	//output.lightViewPosition = mul(output.lightViewPosition, g_lightViewMatrix);
	//output.lightViewPosition = mul(output.lightViewPosition, g_lightProjectionMatrix);

	//Calculate light-object vector
	output.lightPos = g_lightPosition.xyz - worldPosition.xyz;
	output.lightPos = normalize(output.lightPos);

    return output;
}