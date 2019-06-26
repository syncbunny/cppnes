#version 150 core
in vec4 position;
out vec2 texcoord;

void main() {
	gl_Position = position;
	//texcoord = vec2(gl_VertexID%2, gl_VertexID/2);
	texcoord = vec2(gl_VertexID/2, gl_VertexID%2);
}
