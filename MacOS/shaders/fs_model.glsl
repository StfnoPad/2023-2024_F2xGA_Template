#version 410 core

out vec4 color;

in VS_OUT
{
    vec3 vertex;
    vec3 normals;
    vec2 tc;
} fs_in;

uniform sampler2D tex;
uniform mat4 model_matrix;

void main(void){
  //color = vec4(1.0,1.0,1.0,1.0);
  //color = vec4(fs_in.normals, 1.0);

  color = texture(tex, fs_in.tc);
}
