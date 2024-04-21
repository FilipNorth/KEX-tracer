#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 Normal;
out vec3 color;
out vec2 texCoord;
out vec3 crntPos;

in DATA{
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
    mat4 projection;
    vec3 crntPos;
} data_in[];

// Default main function
void main()
{
  vec3 p1 = gl_in[0].gl_Position.xyz;
  vec3 p2 = gl_in[1].gl_Position.xyz;
  vec3 p3 = gl_in[2].gl_Position.xyz;

  vec3 tempNormal = abs( cross( p2-p1, p3-p1 ) );

  for(int i = 0; i < 3; i++){

    crntPos = data_in[i].crntPos;
    Normal = data_in[i].Normal;
    color = data_in[i].color;
    texCoord = data_in[i].texCoord;

    vec3 p = ( p1 + ( gl_in[i].gl_Position.xyz - p1 ) );
    if( tempNormal.x > tempNormal.y && tempNormal.x > tempNormal.z ){
      gl_Position = vec4( p.yz, 0, 1 );
    } else if (tempNormal.y > tempNormal.z){
      gl_Position = vec4( p.xz, 0, 1 );
    } else {
      gl_Position = vec4( p.xy, 0, 1 );
    }
    EmitVertex();
  }
  EndPrimitive();
}


// "Explosion" main function
/* void main()
{
   vec3 vector0 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
   vec3 vector1 = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
   vec4 surfaceNormal = vec4(normalize(cross(vector0, vector1)), 0.0f);

   gl_Position = data_in[0].projection * (gl_in[0].gl_Position + surfaceNormal);
   Normal = data_in[0].Normal;
   color = data_in[0].color;
   texCoord = data_in[0].texCoord;
   crntPos = data_in[0].crntPos;
   EmitVertex();

   gl_Position = data_in[1].projection * (gl_in[1].gl_Position + surfaceNormal);
   Normal = data_in[1].Normal;
   color = data_in[1].color;
   texCoord = data_in[1].texCoord;
   crntPos = data_in[1].crntPos;
   EmitVertex();

   gl_Position = data_in[2].projection * (gl_in[2].gl_Position + surfaceNormal);
   Normal = data_in[2].Normal;
   color = data_in[2].color;
   texCoord = data_in[2].texCoord;
   crntPos = data_in[2].crntPos;
   EmitVertex();

   EndPrimitive();
}
 */
