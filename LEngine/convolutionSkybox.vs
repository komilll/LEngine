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

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;    
};

struct PixelInputType
{
    float4 positionSV : SV_POSITION;
    float4 position : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

	output.position = input.position;
	input.position.w = 1.0f;

    output.positionSV = mul(input.position, worldMatrix).xyww;
    //output.positionSV = mul(output.positionSV, viewMatrix).xyww;
    //output.positionSV = mul(output.positionSV, projectionMatrix).xyww;

    return output;
}