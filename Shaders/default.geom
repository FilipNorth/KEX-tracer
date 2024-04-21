#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec3 Normal;
out vec3 color;
out vec2 texCoord;
out vec3 crntPos;  // New output variable

in DATA
{
    vec3 Normal;
    vec3 color;
    vec2 texCoord;
    vec3 crntPos;
    //mat4 projection;
} data_in[];

//voxelization:
void main(){
  vec3 p1 = gl_in[0].gl_Position.xyz;
  vec3 p2 = gl_in[1].gl_Position.xyz;
  vec3 p3 = gl_in[2].gl_Position.xyz;

  vec3 projs = abs( cross( p2-p1, p3-p1 ) );

  for(int i = 0; i < 3; i++){

    crntPos = data_in[i].crntPos;
    Normal = data_in[i].Normal;
    color = data_in[i].color;
    texCoord = data_in[i].texCoord;

    vec3 p = ( p1 + ( gl_in[i].gl_Position.xyz - p1 ) );
    if( projs.x > projs.y && projs.x > projs.z ){
      gl_Position = vec4( p.yz, 0, 1 );
    } else if (projs.y > projs.z){
      gl_Position = vec4( p.xz, 0, 1 );
    } else {
      gl_Position = vec4( p.xy, 0, 1 );
    }
    EmitVertex();
  }
  EndPrimitive();
}




// Default main function, changes nothing
/*
void main()
{
    // Output the custom variable instead of gl_Position
    //crntPos = vec3(data_in[0].projection * gl_in[0].gl_Position);
    gl_Position = data_in[0].projection * gl_in[0].gl_Position;
    Normal = data_in[0].Normal;
    color = data_in[0].color;
    texCoord = data_in[0].texCoord;
    EmitVertex();

    //crntPos = vec3(data_in[1].projection * gl_in[1].gl_Position);
    gl_Position = data_in[1].projection * gl_in[1].gl_Position;
    Normal = data_in[1].Normal;
    color = data_in[1].color;
    texCoord = data_in[1].texCoord;
    EmitVertex();

    //crntPos = vec3(data_in[2].projection * gl_in[2].gl_Position);
    gl_Position = data_in[2].projection * gl_in[2].gl_Position;
    Normal = data_in[2].Normal;
    color = data_in[2].color;
    texCoord = data_in[2].texCoord;
    EmitVertex();

    EndPrimitive();
}*/



// "Explosion" main function
/*void main()
{
    vec3 vector0 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
    vec3 vector1 = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
    vec4 surfaceNormal = vec4(normalize(cross(vector0, vector1)), 0.0f);
     

      crntPos = vec3(data_in[0].projection * gl_in[0].gl_Position);

      gl_Position = data_in[0].projection * (gl_in[0].gl_Position + surfaceNormal);
      Normal = data_in[0].Normal;
      color = data_in[0].color;
      texCoord = data_in[0].texCoord;
      EmitVertex();
      
      crntPos = vec3(data_in[0].projection * gl_in[1].gl_Position);

      gl_Position = data_in[1].projection * (gl_in[1].gl_Position + surfaceNormal);
      Normal = data_in[1].Normal;
      color = data_in[1].color;
      texCoord = data_in[1].texCoord;
      EmitVertex();

      crntPos = vec3(data_in[2].projection * gl_in[1].gl_Position);

      gl_Position = data_in[2].projection * (gl_in[2].gl_Position + surfaceNormal);
      Normal = data_in[2].Normal;
      color = data_in[2].color;
      texCoord = data_in[2].texCoord;
      EmitVertex();
  
      EndPrimitive();
  }
  */
