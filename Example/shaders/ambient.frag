#version 400
 
in VertexData {
    vec3 normal;
    vec2 texcoord;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L_pos;
uniform float L_inten;

//camera position
uniform mat4 camera_model_view_matrix;
uniform mat4 camera_projection_matrix;

uniform sampler2D depth_tex;  //texturna jednotka s depth texturou
uniform sampler2D color_tex;  //texturna jednotka s UV mapou

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

bool occlusion (vec4 pos, vec3 N, vec3 T, vec3 B, int i)
{

    vec3 model_view_pos = pos.xyz/pos.w;

    float polar = random(i+1) * PI / 2.0;
    float azimuth = random(i*i) * PI * 2.0;
    
    //ToDo skonstruujte suradnice x, y, z jednotkoveho vektora v tangencialnom priestore
    // z azimutu a polary
    float x = cos(azimuth);
    float y = sin(azimuth);
    float z = cos(polar);
    
    float scale = 0.2 * random(i*i+2);
    
    vec3 sample_pos = scale*(z*N + x*T + y*B);
    
    //ToDo posunte skonstruovany vektor aby zacinal na pozicii fragmentu v kamorovych suradniciach
    sample_pos += model_view_pos;
    
    vec4 sample_proj = camera_projection_matrix * vec4(sample_pos, 1.0);
    sample_proj /= sample_proj.w;
        
    // Transform to [0,1] range
    //ToDo transformujte vektor z priestoru [-1,1]x[-1,1]x[-1,1]x[-1,1] do [0,1]x[0,1]x[0,1]x[0,1]
    sample_proj = sample_proj * 0.5 + 0.5;
    
    // Get closest depth value from depth texture (using [0,1] range as coords)
    float closestDepth = texture(depth_tex, sample_proj.xy).r; 
    
    // Get depth of current sample
    // ToDo ziskajte Z-tovu suradnicu pozicie fragmentu transformovanu na interval [0,1]
    float currentDepth = sample_proj.z;

    //porovnajte ziskanu suradnicu Z s hodnotou v hlbkovej texture 
    return currentDepth > closestDepth;
}  
 
void main()
{
    
    if (pass_number == 2){
        // Compute the ligt direction
        vec3 lightDir = normalize(vec3(L_pos - VertexIn.ecPos.xyz));
        
        // normal
        vec3 N = normalize(VertexIn.normal);
        
        // Compute the dot product between normal and lightDir 
        float NdotL = max(dot(N, lightDir), 0.0);

        vec4 ambient_col = vec4(0.1, 0.1, 0.1, 0.0);
        
        vec4 color;
        
        vec4 diffuse_col = texture(color_tex, VertexIn.texcoord).rgba;
        
        if (enabled){
        
          //tangent
          //ToDo skonstruujte tangentu a bitangentu k normale N
          vec3 T = normalize(cross(N, lightDir));
          
          //bitangent
          vec3 B =  normalize(cross(N, T));
          
          float kernelSize = 50.0;
          
          float occluded = 0; 
          
          for (int i = 0; i < kernelSize; ++i) {
            if(occlusion (VertexIn.ecPos, N, T, B, i))
                occluded += 1.0;
          }
          
          occluded /= kernelSize;
          
          color = ambient_col + diffuse_col * NdotL * (1.0 - occluded) * L_inten; 
        
        } else {
        
          color = ambient_col + diffuse_col * NdotL * L_inten;
          
        }
        
        
      
        fragmentOut = color;
    }
}