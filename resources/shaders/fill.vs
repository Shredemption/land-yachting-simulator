#version 410

// Program Uniforms
uniform vec4 State;
uniform mat4 Transform;
uniform vec4 Scalar4[2];
uniform vec4 Vector[8];
uniform uint ClipSize;
uniform mat4 Clip[8];

uniform bool flipY;

// Uniform Accessor Functions
float Time()
{
  return State[0];
}
float ScreenWidth()
{
  return State[1];
}
float ScreenHeight()
{
  return State[2];
}
float ScreenScale()
{
  return State[3];
}
float Scalar(uint i)
{
  if(i < 4u)
    return Scalar4[0][i];
  else
    return Scalar4[1][i - 4u];
}
vec4 sRGBToLinear(vec4 val)
{
  return vec4(val.xyz * (val.xyz * (val.xyz * 0.305306011 + 0.682171111) + 0.012522878), val.w);
}

// Vertex Attributes
layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in vec2 in_ObjCoord;
layout(location = 4) in vec4 in_Data0;
layout(location = 5) in vec4 in_Data1;
layout(location = 6) in vec4 in_Data2;
layout(location = 7) in vec4 in_Data3;
layout(location = 8) in vec4 in_Data4;
layout(location = 9) in vec4 in_Data5;
layout(location = 10) in vec4 in_Data6;

// Out Params
out vec4 ex_Color;
out vec2 ex_TexCoord;
out vec4 ex_Data0;
out vec4 ex_Data1;
out vec4 ex_Data2;
out vec4 ex_Data3;
out vec4 ex_Data4;
out vec4 ex_Data5;
out vec4 ex_Data6;
out vec2 ex_ObjectCoord;
out vec2 ex_ScreenCoord;

void main(void)
{
  ex_ObjectCoord = in_ObjCoord;
  gl_Position = Transform * vec4(in_Position, 0.0, 1.0);
  ex_Color = in_Color;
  ex_TexCoord = in_TexCoord;
  if(flipY)
    ex_TexCoord.y = 1.0 - ex_TexCoord.y;
  ex_Data0 = in_Data0;
  ex_Data1 = in_Data1;
  ex_Data2 = in_Data2;
  ex_Data3 = in_Data3;
  ex_Data4 = in_Data4;
  ex_Data5 = in_Data5;
  ex_Data6 = in_Data6;
}