#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex;
in vec4 normal; 
in vec2 texCoord0;

//Zmienne interpolowane
out vec4 l;
out vec4 n;
out vec4 v;
out vec2 iTexCoord0;
out vec4 gVertex;


void main(void) {

	gVertex = vertex;

	vec4 lp = vec4(0, 10, 0, 1); //pozcyja głównego światła, przestrzeń świata

	l = normalize(V * lp - V*M*vertex); //wektor do światła w przestrzeni oka
	v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
	n = normalize(V * M * normal); //wektor normalny w przestrzeni oka

    iTexCoord0 = texCoord0;
    
    gl_Position=P*V*M*vertex;
}
