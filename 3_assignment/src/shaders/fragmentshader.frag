
#version 430
#define KERNEL_SIZE 9
float kernel[KERNEL_SIZE];
float width = 512.0;
float height = 512.0;
float step_w = 1.0/width;
float step_h = 1.0/height;
vec2 offset[KERNEL_SIZE];
                         

// Reference: GLSL CookBook, Chapter 2.

in vec3 LightIntensity;
layout( location = 0 ) out vec4 FragColor;
layout(binding=0) uniform sampler2D RenderTex;
uniform int ShaderType;
uniform int ImgType;
uniform float EdgeThreshold;
uniform int Pass;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 VarLightIntensity;

struct LightInfo {
	vec4 Position; // Light position in eye coords.
	vec3 La; // Ambient light intensity
	vec3 Ld; // Diffuse light intensity
	vec3 Ls; // Specular light intensity
};

uniform LightInfo Light;
struct MaterialInfo {
	vec3 Ka; // Ambient reflectivity
	vec3 Kd; // Diffuse reflectivity
	vec3 Ks; // Specular reflectivity
	float Shininess; // Specular shininess factor
};

uniform MaterialInfo Material;
uniform float Weight[5];
uniform vec3 StripeColor;
uniform vec3 BackColor;
uniform float Width;
uniform float Fuzz;
uniform float Scale;
uniform vec2 Threshold;
float alpha = 3.5;
vec3 AvgLuminance = vec3(0.5, 0.5, 0.5);
const vec3 lumCoeff = vec3 (0.2125, 0.7154, 0.0721);


vec3 ads( vec3 col )
{
	vec3 n = normalize( Normal );
	vec3 s = normalize( vec3(Light.Position) - Position );
	vec3 v = normalize(vec3(-Position));
	vec3 r = reflect( -s, n );
	return VarLightIntensity * ( col * ( Material.Ka + Material.Kd * max( dot(s, n), 0.0 ) ) + 
		Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess ) );
}

vec3 Phong( vec3 position, vec3 n, vec4 TexColor) {
    vec3 col = TexColor.rgb;
    vec3 ambient = Light.La * col; // instead of using Ka, using the texture mapping
    vec3 s = normalize( Light.Position.xyz - position );
    float sDotN = max( dot(s,n), 0.0 );
    vec3 diffuse = col * sDotN; // col

    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 ) {
        vec3 v = normalize(-position.xyz);
        vec3 r = reflect( -s, n );
        spec = Material.Ks *
                pow( max( dot(r,v), 0.0 ), Material.Shininess );
    }
    return ambient + Light.Ls * (diffuse + spec);
}

void Passing1() {
    if( ShaderType == 1 ) {
		FragColor = vec4( LightIntensity, 1.0 );
	} else if( ShaderType == 2 ) {
		FragColor = vec4( ads( vec3(1.0f, 1.0f, 1.0f ) ), 1.0 );
	} else if( ShaderType == 3 ) { 

		float scaled_t = fract( TexCoord.t * Scale );
        float frac1 = clamp( scaled_t / Fuzz, 0.0, 1.0 );
        float frac2 = clamp( ( scaled_t - Width ) / Fuzz, 0.0, 1.0 );
     
	    frac1 = frac1 * ( 1.0 - frac2 );
        frac1 = frac1 * frac1 * ( 3.0 - ( 2.0 * frac1 ) );
        vec3 finalColor = mix( BackColor, StripeColor, frac2 );
   
		FragColor = vec4( ads( finalColor ), 1.0 );
	} else if( ShaderType == 4 ) {

		float ss = fract( TexCoord.s * Scale );
		float tt = fract( TexCoord.t * Scale );
		if( ( ss > Threshold.s ) && ( tt > Threshold.t ) ) discard;
		FragColor = vec4( ads( vec3(1.0f, 1.0f, 1.0f ) ), 1.0 );
	}else if( ShaderType == 5 || ShaderType == 6 || ShaderType == 7) {
        FragColor = vec4(Phong(Position, normalize(Normal), texture( RenderTex, TexCoord )), 1.0 );
    }
}


const vec3 lum = vec3(0.2126, 0.7152, 0.0722);
float luminance( vec3 color ) {
    return dot(lum,color);
}

void per_pixel(){
        if( ImgType == 1 ) {
        vec4  TexColor  = texture2D( RenderTex, TexCoord.st);
        FragColor = TexColor * alpha;
    }else if( ImgType == 2 ) {
        vec3  TexColor  = vec3(texture2D( RenderTex, TexCoord.st));
        vec3 color = AvgLuminance*(1.0-alpha) +  vec3(TexColor)*(alpha);
        FragColor = vec4 (color, 1.0);
    }else if( ImgType == 3 ) {
        vec3  TexColor  = vec3(texture2D( RenderTex, TexCoord.st));
        vec3 intensity = vec3(dot(TexColor.rgb, lumCoeff));
        vec3 color = intensity*(1.0-alpha) + vec3(TexColor)*(alpha);
        FragColor = vec4 (color, 1.0);
    }
}


vec4 edge()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);
    float s00 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,1)).rgb);
    float s10 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,0)).rgb);
    float s20 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,1)).rgb);
    float s21 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(0,-1)).rgb);
    float s02 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,1)).rgb);
    float s12 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,0)).rgb);
    float s22 = luminance(texelFetchOffset(RenderTex, pix, 0, ivec2(1,-1)).rgb);

    float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
    float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

    float g = sx * sx + sy * sy;

    if( g > EdgeThreshold )
        return vec4(1.0);
    else
        return vec4(0.0,0.0,0.0,1.0);
}


// float weights[5], sum, sigma2 = 8.0f;
//
//    // Compute and sum the weights
//    weights[0] = gauss(0,sigma2);
//    sum = weights[0];
//    for( int i = 1; i < 5; i++ ) {
//        weights[i] = gauss(float(i), sigma2);
//        sum += 2 * weights[i];
//    }
//
//	    // Normalize the weights and set the uniform
//    for( int i = 0; i < 5; i++ ) {
//		std::stringstream uniName;
//		uniName << "Weight[" << i << "]";
//        float val = weights[i] / sum;
//        prog.setUniform(uniName.str().c_str(), val);
//    }
//


vec4 blur()
{
    ivec2 pix = ivec2( gl_FragCoord.xy );

    vec4 sum = texelFetch(RenderTex, pix, 0) * Weight[0];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,1) ) * Weight[1];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-1) ) * Weight[1];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,2) ) * Weight[2];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-2) ) * Weight[2];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,3) ) * Weight[3];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-3) ) * Weight[3];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,4) ) * Weight[4];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-4) ) * Weight[4];
    return sum;
}



vec4 sharpening()
{
    ivec2 pix = ivec2( gl_FragCoord.xy );
    vec4 sum = texelFetch(RenderTex, pix, 0) * Weight[0];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-0.5) ) * Weight[1];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,0) ) * Weight[1];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-0.5) ) * Weight[2];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,3) ) * Weight[2];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-0.5) ) * Weight[3];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,0) ) * Weight[3];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,-0.5) ) * Weight[4];
    sum += texelFetchOffset( RenderTex, pix, 0, ivec2(0,0) ) * Weight[4];
    return sum;
}


void main()
{
    if( Pass == 1 ) {
	Passing1();
	 if (ImgType ==1 || ImgType ==2 || ImgType ==3){
            per_pixel();
        }
	}
    if( Pass == 2){
        if( ImgType == 4) {
            FragColor = edge();
        }
		if( ImgType == 5 ){
		     FragColor = blur();
		}
		if( ImgType == 6 ){
		     FragColor = sharpening();
		}
    }
}
