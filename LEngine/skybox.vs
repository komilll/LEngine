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
    float4 position : POSITION;
};

RasterizerState NoCull
{
	CullMode = None;
};

DepthStencilState LessEqualDSS
{
    // Make sure the depth function is LESS_EQUAL and not just LESS.  
    // Otherwise, the normalized depth values at z = 1 (NDC) will 
    // fail the depth test if the depth buffer was cleared to 1.
    DepthFunc = LESS_EQUAL;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

	input.position.w = 1.0f;
	output.position = input.position;

    output.positionSV = mul(input.position, worldMatrix).xyww;
    output.positionSV = mul(output.positionSV, viewMatrix).xyww;
    output.positionSV = mul(output.positionSV, projectionMatrix).xyww;

    return output;
}