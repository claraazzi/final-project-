


uniform sampler2D diffuse;      // Texture
uniform vec2 lightPosition;     // Player position (light source)

varying vec2 texCoordVar;       // Texture coordinates
varying vec2 varPosition;       // Position from vertex shader

float attenuate(float dist, float a, float b) {
    return 1.0 / (1.0 + (a * dist) + (b * dist * dist)); // Inverse square attenuation
}

void main()
{
    vec4 color = texture2D(diffuse, texCoordVar); // Sample texture
    float dist = distance(lightPosition, varPosition); // Distance to light source
    float brightness = attenuate(dist, 1.0, 0.1); // Adjust attenuation parameters
    gl_FragColor = vec4(color.rgb * brightness, color.a); // Apply brightness
}

