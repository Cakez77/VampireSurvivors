
layout (location = 0) out int blackFlag;

layout(std430, binding = 0) buffer TransfromSBO
{
    Transform transforms[];
};

void main()
{
  vec2 vertices[6] = 
  {
    vec2(-0.1,  0.1),
    vec2(-0.1, -0.1),
    vec2( 0.1,  0.1),
    vec2( 0.1,  0.1),
    vec2(-0.1, -0.1),
    vec2( 0.1, -0.1)
  };

  int instanceIdx = gl_InstanceID;

  Transform t = transforms[gl_InstanceID];

  blackFlag = t.blackFlag;

  gl_Position = vec4(vertices[gl_VertexID] * t.size + t.pos, 0.0, 1.0);
}