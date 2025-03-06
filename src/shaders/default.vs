#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 lightDir;
    vec3 color;
}
vs_out;

const int MAX_BONES = 50;
const int MAX_BONE_INFLUENCE = 4;

uniform vec3 lightPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normal;

uniform vec4 location_plane;

uniform mat4 u_boneTransforms[MAX_BONES];
uniform mat4 u_inverseOffsets[MAX_BONES];

void main()
{
    // Initialize the final position of the vertex
    // vec4 finalPosition = vec4(aPos, 1.0);
    // vec3 finalNormal = vec3(aNormal);
    vec4 finalPosition = vec4(aPos, 1.0);
    vec3 finalNormal = vec3(aNormal);

    // Apply the bone transforms based on the weights and bone IDs
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        int boneID = aBoneIDs[i];
        float weight = aWeights[i];

        if(weight > 0.0)
        {
            // Apply the bone transform to the vertex position and normal
            mat4 boneTransform = u_boneTransforms[boneID];
            mat4 inverseOffset = u_inverseOffsets[boneID]; // Move vertex into bone-local space

            vec4 boneSpacePos = inverseOffset * vec4(aPos, 0.0);

            finalPosition += boneTransform * boneSpacePos * weight;
            finalNormal += mat3(boneTransform) * aNormal * weight; // Use the rotation part of the matrix for normal
        }
    }

    vec4 worldPosition = u_model * finalPosition;

    gl_ClipDistance[0] = dot(worldPosition, location_plane);

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = worldPosition.xyz;
    vs_out.Normal = normalize(mat3(u_normal) * finalNormal);
    vs_out.lightDir = normalize(lightPos - worldPosition.xyz);

    vs_out.color = vec3(aBoneIDs[0]/9.0);

    gl_Position = u_projection * u_view * worldPosition;
}