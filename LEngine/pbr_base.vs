////////////////////////////////////////////////////////////////////////////////
// Filename: pbr_base.vs
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

cbuffer CameraBuffer
{
	float3 cameraPosition;
	float padding;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex: TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex: TEXCOORD0;
    float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the input color for the pixel shader to use.
    output.tex = input.tex;

	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	//Calculate view direction for pixels
	worldPosition = mul(input.position, worldMatrix);

	output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
	output.viewDirection = normalize(output.viewDirection);

	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);

	output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);
    
    return output;
}