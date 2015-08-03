#version 430 core

#define MAX_BONES 100

uniform mat4 mvp;

uniform mat4 bones[MAX_BONES];

layout(location=0) in vec3 in_vertex;
layout(location=1) in vec2 in_texcoord;
layout(location=2) in vec3 in_normal;
layout(location=3) in vec3 in_tangent;
layout(location=4) in ivec4 in_bone_id;
layout(location=5) in vec4 in_bone_weight;

out vec2 texcoord;
out vec4 idx;
out vec4 weights;

void main()
{
  texcoord = in_texcoord;

  mat4 bone_transform = bones[in_bone_id[0]] * in_bone_weight[0];
  bone_transform     += bones[in_bone_id[1]] * in_bone_weight[1];
  bone_transform     += bones[in_bone_id[2]] * in_bone_weight[2];
  bone_transform     += bones[in_bone_id[3]] * in_bone_weight[3];

  idx = vec4( in_bone_id[0], in_bone_id[1], in_bone_id[2], in_bone_id[3] ) / 32;
  weights = in_bone_weight;

  gl_Position = mvp * bone_transform * vec4( in_vertex, 1 );
}
