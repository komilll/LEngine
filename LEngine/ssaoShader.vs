////////////////////////////////////////////////////////////////////////////////
// Filename: ssaoShader.vs
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

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex: TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 projection_1 : TEXCOORD1;
	float4 projection_2 : TEXCOORD2;
	float4 projection_3 : TEXCOORD3;
	float4 projection_4 : TEXCOORD4;
	float4 view_1 : TEXCOORD5;
	float4 view_2 : TEXCOORD6;
	float4 view_3 : TEXCOORD7;
	float4 view_4 : TEXCOORD8;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = input.position;
    //output.position = mul(input.position, worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);
	
	output.tex = input.tex;
	matrix matrixToPass = mul(viewMatrix, projectionMatrix);

	output.projection_1 = projectionMatrix[0];
	output.projection_2 = projectionMatrix[1];
	output.projection_3 = projectionMatrix[2];
	output.projection_4 = projectionMatrix[3];

	output.view_1 = viewMatrix[0];
	output.view_2 = viewMatrix[1];
	output.view_3 = viewMatrix[2];
	output.view_4 = viewMatrix[3];

    return output;
}