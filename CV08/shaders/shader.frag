#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L_pos;
uniform vec4 diffuse_col;

uniform sampler2D tex;

uniform int pass_number;

uniform bool enabled;

layout(location = 0) out vec4 fragmentOut;

 
void main() {
    vec4 color;
    
    if (pass_number == 1) {
        // Compute the ligt direction
        vec3 L = normalize(vec3(L_pos - VertexIn.ecPos.xyz));
        
        // normal
        vec3 N = normalize(VertexIn.normal);
      
        // Compute the dot product between normal and lightDir 
        float NdotL = max(dot(N, L), 0.0);

        vec4 ambient_col = vec4(0.1, 0.1, 0.1, 0.0);
    
        //ToDo vypocitajte Blin-Phong farbu a ulozte ju do "color"
        vec3 H = normalize(L + N);
		
		float NdotH = max(dot(N, H), 0.0);
		
		float specular = pow(NdotH, 150.0);
		
		color = ambient_col + diffuse_col * NdotL + vec4(1.0, 1.0, 1.0, 0.0) * specular;
      
        //ToDo do "alpha" zlozky vo vektore "color" ulozte len intenzitu zrhadlovej zlozky
        color.a = specular;
    }
    
    if (pass_number == 2) {
        vec4 fragPos =  VertexIn.ecPos;
          
        //fragPos /= fragPos.w; //netreba, lebo sme nasobili len s MV maticou vo vertex shadery
          
        fragPos = fragPos * 0.5 + 0.5;
        
        //ToDo vyberte farbu z textury
        color = texture(tex, fragPos.xy);
       
        if (enabled) {      
            float kernelSize = 0.015;
          
            int sample_no = 10;
          
            float delta = kernelSize / sample_no;
          
            float glow = 0;
          
            for (int i = 0; i < sample_no; i++) {  
                //vzorka v smere uv vektora (1,1)
                vec2 uv = fragPos.xy + vec2(i*delta, i*delta);
                glow += texture(tex, uv).a;
              
                //ToDo priratajte dalsie vzorky v smeroch (-1,-1), (-1,1) a (1,-1)
                uv = fragPos.xy + vec2(-i*delta, i*delta);
                glow += texture(tex, uv).a;
              
                uv = fragPos.xy + vec2(i*delta, -i*delta);
                glow += texture(tex, uv).a;
              
				uv = fragPos.xy + vec2(-i*delta, -i*delta);
                glow += texture(tex, uv).a;		  
            }
          
            glow /= sample_no;       
    
            color += glow * vec4(0.5, 0.5, 0.5, 0.0);
        }   
    }
    
    fragmentOut = color;
}