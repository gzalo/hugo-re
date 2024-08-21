#version 330 core

uniform sampler2D tex;
uniform float time;

in vec2 uvs;
out vec4 f_color;

void main() {
    vec2 sample_pos = vec2(uvs.x + sin(uvs.y * 10 + time) * 0.01, uvs.y);
    f_color = vec4(texture(tex, sample_pos).rgb, 1.0);
}
