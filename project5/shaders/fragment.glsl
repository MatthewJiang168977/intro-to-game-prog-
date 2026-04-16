#version 330

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float hp_ratio; // 0.0 to 1.0
uniform float level_darkness; // 0.0 to ~0.3

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 color = texelColor * colDiffuse * fragColor;

    // Convert to grayscale
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec3 grayscale = vec3(gray);

    if (hp_ratio < 0.25)
    {
        // Near death: almost fully desaturated + red tint
        vec3 tinted = grayscale * vec3(1.15, 0.85, 0.85);
        color.rgb = mix(grayscale, tinted, 0.8);
    }
    else if (hp_ratio < 0.5)
    {
        // Hurt: partial desaturation
        float factor = (0.5 - hp_ratio) / 0.25; // 0 at 50%, 1 at 25%
        color.rgb = mix(color.rgb, grayscale, factor * 0.6);
    }
    // else: full color, no effect

    color.rgb *= (1.0 - level_darkness);

    finalColor = color;
}
