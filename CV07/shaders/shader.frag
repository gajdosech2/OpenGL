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

const float PI = 3.1415926535897932;

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }

bool occlusion (vec4 pos, vec3 N, vec3 T, vec3 B, int i) {
    vec3 model_view_pos = pos.xyz/pos.w;

    float polar = random(i+1) * PI / 2.0;
    float azimuth = random(i*i) * PI * 2.0;
    
    float x = cos(azimuth);
    float y = sin(azimuth);
    float z = cos(polar);
    
    float scale = 0.2 * random(i*i+2);
    
    vec3 sample_pos = scale*(z*N + x*T + y*B);
    sample_pos += model_view_pos;
    
    vec4 sample_proj = camera_projection_matrix * vec4(sample_pos, 1.0);
    sample_proj /= sample_proj.w;
        
    // Transform to [0,1] range
    sample_proj = sample_proj * 0.5 + 0.5;
    
    // Get closest depth value from depth texture (using [0,1] range as coords)
    float closestDepth = texture(depth_tex, sample_proj.xy).r; 
    
    // Get depth of current sample
    float currentDepth = sample_proj.z;

    return (closestDepth < currentDepth);
}  
 
void main() {
    if (pass_number == 2) {
        vec3 lightDir = normalize(vec3(L_pos - VertexIn.ecPos.xyz));
        vec3 N = normalize(VertexIn.normal);
        float NdotL = max(dot(N, lightDir), 0.0);
        vec4 ambient_col = vec4(0.1, 0.1, 0.1, 0.0);
		
        if (enabled) {
            //tangent
            vec3 T = normalize(cross(N, vec3(0, 1, 0)));
            //bitangent
            vec3 B = normalize(cross(T, N));
            float kernelSize = 50.0;
            float occluded = 0;
            for (int i = 0; i < kernelSize; ++i) {
                if (occlusion (VertexIn.ecPos, N, T, B, i))
                    occluded += 1;
            }       
            occluded /= kernelSize;
            fragmentOut = ambient_col + diffuse_col * NdotL * (1.0 - occluded); 
        } else {
            fragmentOut = ambient_col + diffuse_col * NdotL;
        }
    }
}