#version 330 core

uniform sampler2D tex;
uniform float time;
uniform float wavyness;

in vec2 uvs;
out vec4 f_color;

void main() {
    vec2 sample_pos = vec2(uvs.x + sin(uvs.y * 10 + time) * wavyness, uvs.y);
    f_color = vec4(texture(tex, sample_pos).rgb, 1.0);
}
