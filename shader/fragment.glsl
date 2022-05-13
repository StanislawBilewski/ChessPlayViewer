#version 330

//ZDEFINIOWANE RÓWNIEŻ W VERTEX SHADERZE
#define NUMBER_OF_LIGHTS 3

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;
uniform int lights;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec4 n;
in vec4 l[NUMBER_OF_LIGHTS];
in vec4 v;
in vec2 iTexCoord0;
in float dist[NUMBER_OF_LIGHTS];


void main(void) {
	//Moc poszczególnych świateł
	float[NUMBER_OF_LIGHTS] lpower;
	lpower[0] = 10;
	lpower[1] = 3.5;
	lpower[2] = 3.5;

	//Kolory poszczególnych świateł
	vec4[NUMBER_OF_LIGHTS] col;
	col[0] = vec4(1,1,1,1);
	if(lights==0){
		col[1] = vec4(0,1,0,1);
		col[2] = vec4(0,0,0,1);
	}
	else if(lights==1){
		col[1] = vec4(0,0,0,1);
		col[2] = vec4(0,1,0,1);
	}
	else if(lights==2){
		col[1] = vec4(0,0,1,1);
		col[2] = vec4(0,0,1,1);
	}
	else if(lights==3){
		col[1] = vec4(0,1,0,1);
		col[2] = vec4(1,0,0,1);
	}
	else{
		col[1] = vec4(1,0,0,1);
		col[2] = vec4(0,1,0,1);
	}


	//Znormalizowane interpolowane wektory
	vec4[NUMBER_OF_LIGHTS] ml;
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		ml[i] = normalize(l[i]);
	}
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);

	//Parametry powierzchni
	vec4 kd = texture(textureMap0,iTexCoord0);
	vec4 ks = texture(textureMap1,iTexCoord0);

	vec4[NUMBER_OF_LIGHTS] mr;
	float[NUMBER_OF_LIGHTS] nl;
	float[NUMBER_OF_LIGHTS] rv;
	vec4 color = vec4(0,0,0,kd.a);
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		//Wektor odbity
		mr[i] = reflect(-ml[i], mn);
		
		//Obliczenie modelu oświetlenia
		nl[i] = clamp(dot(mn, ml[i]), 0, 1);
		rv[i] = pow(clamp(dot(mr[i], mv), 0, 1), 50);

		//Sumowanie światła ze źródeł, mnożenie razy wektor koloru światła
		//oraz kalkulacja osłabienia światła wraz ze wzrostem dystansu (attenuation)
		color += (vec4(kd.rgb * nl[i], 0) + vec4(ks.rgb*rv[i], 0)) * col[i] / (1+dist[i]) * lpower[i];
	} 
	
	//Przypisanie wartości koloru pikselowi
	pixelColor = color;

	//alternatywna wersja uwzględniająca tylko pierwsze źródło światła
	//pixelColor = vec4(kd.rgb * nl[0], kd.a) + vec4(ks.rgb*rv, 0);
}
