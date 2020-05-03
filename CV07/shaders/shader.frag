#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L_pos;
uniform vec4 diffuse_col;

uniform mat4 camera_model_view_matrix;
uniform mat4 camera_projection_matrix;

uniform sampler2D depth_tex;

uniform int pass_number;

uniform bool enabled;

layout(location = 0) out vec4 fragmentOut;

void main() {
    if (pass_number == 2) {
        vec3 lightDir = normalize(vec3(L_pos - VertexIn.ecPos.xyz));
        vec3 N = normalize(VertexIn.normal);
        float NdotL = max(dot(N, lightDir), 0.0);
        vec4 ambient_col = vec4(0.1, 0.1, 0.1, 0.0);
        fragmentOut = ambient_col + diffuse_col * NdotL;;
    }
}