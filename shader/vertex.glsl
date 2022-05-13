#version 330

//ZDEFINIOWANE RÓWNIEŻ W FRAGMENT SHADERZE
#define NUMBER_OF_LIGHTS 3

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex;
in vec4 normal; 
in vec2 texCoord0;

//Zmienne interpolowane
out vec4 l[NUMBER_OF_LIGHTS];
out vec4 n;
out vec4 v;
out vec2 iTexCoord0;
out float dist[NUMBER_OF_LIGHTS];


void main(void) {

	// Definicja źródeł światła (ich pozycje)
	vec4 lp[NUMBER_OF_LIGHTS];
	lp[0] = vec4(0, 10, 0, 1); //pozcyja głównego światła, przestrzeń świata
	lp[1] = vec4(0, 0.5, -5, 1); //pozcyja światła za graczem 1
	lp[2] = vec4(0, 0.5, 5, 1); //pozcyja światła dla gracza 2

	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		l[i] = normalize(V * lp[i] - V*M*vertex); //wektor do światła w przestrzeni oka
		dist[i] = distance((V*lp[0]).xyz,(V*M*vertex).xyz);
	}
	v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
	n = normalize(V * M * normal); //wektor normalny w przestrzeni oka

    iTexCoord0 = texCoord0;
    
    gl_Position=P*V*M*vertex;
}
