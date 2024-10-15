#version 300 es
precision mediump float;

uniform sampler2D tex;
uniform float time;
uniform float wavyness;
uniform float bitcrush;
uniform float hue_shift;
uniform float scale;
uniform float chromatic_aberration;
uniform float line_glitch;

in vec2 uvs;
out vec4 f_color;

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    float redOffset   =  -chromatic_aberration * 0.015;
    float greenOffset =  0.0;
    float blueOffset  =  chromatic_aberration * 0.02;

    vec2 centered_uvs = uvs - 0.5;
    vec2 sample_pos = (centered_uvs * scale) + 0.5;
    sample_pos.x += sin(uvs.y * 10.0 + time) * wavyness * scale;

    if (rand(vec2(time, floor(sample_pos.y * 480.0) / 480.0)) < line_glitch) {
        discard;
    }

    vec2 direction = vec2(sin(time), cos(time));
    vec3 color = vec3(0,0,0);
    color.r  = texture(tex, sample_pos + (direction * vec2(redOffset  ))).r;
    color.g  = texture(tex, sample_pos + (direction * vec2(greenOffset))).g;
    color.b = texture(tex, sample_pos + (direction * vec2(blueOffset ))).b;

    vec3 hsv = rgb2hsv(color);
    hsv.x = fract(hsv.x + hue_shift);
    color = hsv2rgb(hsv);

    color = floor(color * bitcrush) / bitcrush;
    f_color = vec4(color, 1.0);
}
