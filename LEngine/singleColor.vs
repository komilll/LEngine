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

cbuffer LightMatrixBuffer
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

cbuffer LightBuffer
{
	float3 g_lightPosition;
	float g_lightBufferPadding;
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
	float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;

    input.position.w = 1.0f;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	//Calculate position based on light position
	output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

	worldPosition = mul(input.position, worldMatrix);
	output.lightPos = g_lightPosition.xyz - worldPosition.xyz;
	output.lightPos = normalize(output.lightPos);

    return output;
}