#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L_pos;
uniform vec4 diffuse_col;

//camera position
uniform mat4 camera_model_view_matrix;
uniform mat4 camera_projection_matrix;

//light position
uniform mat4 light_model_view_matrix;
uniform mat4 light_projection_matrix;

uniform sampler2D depth_tex;

uniform int pass_number;

layout(location = 0) out vec4 fragmentOut;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    //ToDo transformujte projCoords vzhladom na rozsah texturnych uv suradnice
    //projCoords = ...
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depth_tex, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    //ToDo priradte Z-tovu suradnicu fragmentu v kamerovom priestore svetla
    //float currentDepth = ...
    // Check whether current frag pos is in shadow
    float bias = 0.0009;
    float shadow = currentDepth - bias <= closestDepth  ? 1.0 : 0.5;  

    return shadow;
}  
 
void main() {
    if (pass_number == 2) {
        // Compute the ligt direction
        vec3 lightDir = vec3(L_pos - VertexIn.ecPos.xyz);
        
        // Compute the dot product between normal and lightDir 
        float NdotL = max(dot(normalize(VertexIn.normal), normalize(lightDir)), 0.0);
        
        mat4 camModelInv = inverse(camera_model_view_matrix);
        
        vec4 fragPosInLightSpace = light_projection_matrix * light_model_view_matrix * camModelInv * (VertexIn.ecPos/VertexIn.ecPos.w);
        
		//ToDo vypocitajte farbu fragmentu z difuznej farby a tiena
        // Setting Each Pixel 
		// fragmentOut =...
    }
}