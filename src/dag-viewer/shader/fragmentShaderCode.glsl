#version 430
in vec3 Normal;
in vec3 Position;
out vec4 color;

uniform samplerCube ourTexture;

void main()
{             
    float ratio = 1.00/1.52;
    vec3 I = normalize(Position);
    vec3 R = refract(I,normalize(Normal),ratio);
    vec3 nR = vec3(R.x,-R.y,R.z);
    vec4 refractColor = texture(ourTexture, nR);

    R =  reflect(I, normalize(Normal));
    nR = vec3(R.x,-R.y,R.z);;
    vec4 reflectColor = texture(ourTexture,nR);

    float fresnel = max(0.1,pow(1.0-abs(dot(I,normalize(Normal))),2.0));
    color =mix(reflectColor,refractColor,fresnel);
    color.a = clamp( smoothstep(0.0, 1.0, fresnel ) / 3.f + 0.5,0.0f,1.0f);
}
