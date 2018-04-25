#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;

out vec3 Normal;
out vec3 Position;
out vec3 temp;

uniform mat4 model;
uniform mat4 modelView;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
    gl_Position = projection * modelView * model *vec4(position,1.0);
    Position = vec3(modelView *vec4(position,1.0));
    Normal = normalMatrix * normal;
}
