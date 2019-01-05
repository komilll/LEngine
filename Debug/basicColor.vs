////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////

//////////////
// TYPEDEFS //
//////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    //float4 color : COLOR;	
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;
    
	input.position.w = 1.0f;
	output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	output.position.x *= 0.1f;
	output.position.y *= 0.1f;
	output.position.z = 0.0f;
	
	//output.position.z = 1.0f;
	//output.position = input.position;

    output.color = float4(0.0, 1.0, 0.0, 1.0);
    
    return output;
}