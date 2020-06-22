#version 400
 
in VertexData {
    vec3 normal;
    vec4 ecPos;
} VertexIn;
 
uniform vec3 L1_pos = vec3(0, 0, -100);
uniform vec4 L1_diffuse_col = vec4(1, 1, 1, 1);

uniform vec3 L2_pos = vec3(0, 0, 100);
uniform vec4 L2_diffuse_col = vec4(0, 0, 0.8, 1);

uniform sampler2D color_tex;
uniform bool enabled = true;
uniform int pass_number;

layout(location = 0) out vec4 fragmentOut;

void main() {
	vec4 color = vec4(0, 0, 0, 0);
	
	if (pass_number == 1) {
		vec4 dif_col = vec4(0, 0, 0, 0);
		vec3 N = normalize(VertexIn.normal);

		vec3 lightDir = normalize(vec3(L1_pos - VertexIn.ecPos.xyz));
		float NdotL = max(dot(N, lightDir), 0.0);
		dif_col += L1_diffuse_col * NdotL;
		
		vec3 H = normalize(lightDir + N);
		
		float specular = pow(dot(H, N), 150.0);
		vec4 spec_col = clamp(specular, 0.0, 1.0) * vec4(0.5, 0.5, 0.5, 1);
		
		lightDir = normalize(vec3(L2_pos - VertexIn.ecPos.xyz));
		NdotL = max(dot(N, lightDir), 0.0);
		dif_col += L2_diffuse_col * NdotL;
		
		color = vec4(0.1, 0.1, 0.1, 1.0) + spec_col + dif_col;
		color.a = specular;
	}
	
	if (pass_number == 2) {
		vec4 fragPos = VertexIn.ecPos;
		fragPos = fragPos * 0.5 + 0.5;
		color = texture(color_tex, fragPos.xy);
		
		if (enabled) {      
            float kernelSize = 0.03;
          
            int sample_no = 100;
          
            float delta = kernelSize / sample_no;
          
            float glow = 0;
          
            for (int i = 0; i < sample_no; i++) {  
                vec2 uv = fragPos.xy + vec2(i*delta, i*delta);
                glow += texture(color_tex, uv).a;
              
                uv = fragPos.xy + vec2(-i*delta, i*delta);
                glow += texture(color_tex, uv).a;
              
                uv = fragPos.xy + vec2(i*delta, -i*delta);
                glow += texture(color_tex, uv).a;
              
				uv = fragPos.xy + vec2(-i*delta, -i*delta);
                glow += texture(color_tex, uv).a;		  
            }
          
            glow /= sample_no;       
    
            color += glow * vec4(3, 3, 2, 1);
        }   
	}
	fragmentOut = color;
}