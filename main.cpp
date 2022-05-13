/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define X_VELOCITY 0.1
#define Y_VELOCITY 0.1
#define Z_VELOCITY 0.1
#define DESTRUCTION_SPEED 0.4
#define MAX_DESTRUCTION 20

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "piece.h"

//Modele
#include "models/pawn.h"
#include "models/queen.h"
#include "models/rook.h"
#include "models/king.h"
#include "models/bishop.h"
#include "models/knight.h"
#include "models/chessboard.h"
#include "models/table.h"

#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

std::vector<Piece*> pieces;

//-----------TUTAJ zaczyna sie rysowanie---------------------
//-----------------------------------------------------------

float angle_x = PI / 2;
float angle_y = PI / 6;
float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;

bool reset = 0;
bool pause = 1;

// zmienna lights przechowuje dane na temat tego jak mają zachować się światła po bokach
// jest to zależne od wyników rozgrywki oraz od tego czyja przebiega tura
// 0 = ruch białych
// 1 = ruch czarnych
// 2 = remis
// 3 = wygrana białych
// 4 = wygrana czarnych
int lights = 0;

ShaderProgram *sp;
ShaderProgram *sp_destruction;

GLuint tex0;
GLuint tex1;
GLuint tex2;
GLuint tex3;
GLuint tex4;
GLuint tex5;
GLuint tex6;
GLuint tex7;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obsługi klawiszy
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
		if (key == GLFW_KEY_UP) speed_y = PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_y = 0;
		if (key == GLFW_KEY_DOWN) speed_y = 0;
		if (key == GLFW_KEY_R) reset = 1;
		if (key == GLFW_KEY_SPACE) pause = !pause;
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}


GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	sp = new ShaderProgram("shader/vertex.glsl", NULL, "shader/fragment.glsl");
	sp_destruction = new ShaderProgram("shader/vertex_d.glsl", "shader/geometry_d.glsl", "shader/fragment_d.glsl");
	// specjalny shader program do efektu niszczenia figur, geometry shader niszczył oświetlenie

	tex0 = readTexture("textures/whitewood.png");		// tesktura białych figur
	tex1 = readTexture("textures/blackwood.png");		// tesktura czarnych figur
	tex2 = readTexture("textures/chessboard.png");		// tesktura szachownicy
	tex3 = readTexture("textures/table.png");			// tesktura stołu
	tex4 = readTexture("textures/whitewood_spec.png");	// specular map białych figur
	tex5 = readTexture("textures/blackwood_spec.png");	// specular map czarnych figur
	tex6 = readTexture("textures/chessboard_spec.png");	// specular map szachownicy
	tex7 = readTexture("textures/table_spec.png");		// specular map stołu
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	// Usuwanie textur z zasobów
	glDeleteTextures(1, &tex0);
	glDeleteTextures(1, &tex1);
	glDeleteTextures(1, &tex2);
	glDeleteTextures(1, &tex3);
	glDeleteTextures(1, &tex4);
	glDeleteTextures(1, &tex5);
	glDeleteTextures(1, &tex6);
	glDeleteTextures(1, &tex7);

	delete sp;
	delete sp_destruction;
}



//funkcja do wczytywania modeli
void load(glm::mat4 M, int texture, int spec, float* vert, float* norm, float* tex, int count, float dest) {
	if (dest == 0) {
		sp->use();//Aktywacja programu cieniującego
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
		glUniform1i(sp->u("textureMap0"), texture);
		glUniform1i(sp->u("textureMap1"), spec);
		glUniform1i(sp->u("lights"), lights);

		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, vert);
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, norm);
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, tex);

		glDrawArrays(GL_TRIANGLES, 0, count);
	}
	else {
		sp_destruction->use();	//Aktywacja drugiego programu cieniującego
		glUniformMatrix4fv(sp_destruction->u("M"), 1, false, glm::value_ptr(M));
		glUniform1i(sp_destruction->u("textureMap0"), texture);
		glUniform1i(sp_destruction->u("textureMap1"), spec);
		glUniform1f(sp_destruction->u("destruction"), dest);
		glUniform1i(sp_destruction->u("lights"), lights);

		glVertexAttribPointer(sp_destruction->a("vertex"), 4, GL_FLOAT, false, 0, vert);
		glVertexAttribPointer(sp_destruction->a("normal"), 4, GL_FLOAT, false, 0, norm);
		glVertexAttribPointer(sp_destruction->a("texCoord0"), 2, GL_FLOAT, false, 0, tex);

		glDrawArrays(GL_TRIANGLES, 0, count);
	}
}


//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Macierz widoku
	glm::mat4 V = glm::lookAt(
		glm::vec3(sin(angle_x) * 10 * cos(angle_y), 10 * sin(angle_y), cos(angle_x) * 10 * cos(angle_y)),
		glm::vec3(0, 0, 0), 
		glm::vec3(0.0f, 1.0f, 0.0f)); 

	// Macierz perspektywy
	glm::mat4 P = glm::perspective(50.0f*PI / 180.0f, aspectRatio, 0.01f, 50.0f);	

	// Macierz modelu
	glm::mat4 M = glm::mat4(1.0f);	

	// Aktywacja textur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex5);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex6);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tex7);
	glActiveTexture(GL_TEXTURE8);

	//Przygotowanie shaderów
	//Shader główny
	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	glEnableVertexAttribArray(sp->a("vertex"));
	glEnableVertexAttribArray(sp->a("normal"));
	glEnableVertexAttribArray(sp->a("texCoord0"));

	//Shader do destrukcji
	sp_destruction->use();
	glUniformMatrix4fv(sp_destruction->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp_destruction->u("V"), 1, false, glm::value_ptr(V));

	glEnableVertexAttribArray(sp_destruction->a("vertex"));
	glEnableVertexAttribArray(sp_destruction->a("normal"));
	glEnableVertexAttribArray(sp_destruction->a("texCoord0"));


	//Rysowanie obiektów
	//Szachownica
	load(M, 2, 6, chessboardVertices, chessboardNormals, chessboardTexCoords, chessboardVertexCount, 0);

	//Stół
	load(M, 3, 7, tableVertices, tableNormals, tableTexCoords, tableVertexCount, 0);

	//Figury
	for (int i = 0; i < pieces.size(); i++) {
		int type = pieces[i]->type;
		bool color = pieces[i]->color;
		glm::mat4 M1 = glm::translate(M, glm::vec3(pieces[i]->posX - 4.5, pieces[i]->posY, pieces[i]->posZ - 4.5));
		M1 = glm::rotate(M1, color*PI + PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));

		if (type == 0) {		// pionek
			load(M1, color, color + 4, pawnVertices, pawnNormals, pawnTexCoords, pawnVertexCount, pieces[i]->destruct);
		}

		else if (type == 1) {	// wieża
			load(M1, color, color + 4, rookVertices, rookNormals, rookTexCoords, rookVertexCount, pieces[i]->destruct);
		}

		else if (type == 2) {	// koń
			load(M1, color, color + 4, knightVertices, knightNormals, knightTexCoords, knightVertexCount, pieces[i]->destruct);
		}

		else if (type == 3) {	// goniec
			load(M1, color, color + 4, bishopVertices, bishopNormals, bishopTexCoords, bishopVertexCount, pieces[i]->destruct);
		}

		else if (type == 4) {	// hetman
			load(M1, color, color + 4, queenVertices, queenNormals, queenTexCoords, queenVertexCount, pieces[i]->destruct);
		}

		else {					// król
			load(M1, color, color + 4, kingVertices, kingNormals, kingTexCoords, kingVertexCount, pieces[i]->destruct);
		}
	}

	// Wyłączanie przesyłu danych do atrybutów w shaderach
	// Główny shader
	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normals"));
	glDisableVertexAttribArray(sp->a("texCoord0"));

	// Shader do destrukcji
	glDisableVertexAttribArray(sp_destruction->a("vertex"));
	glDisableVertexAttribArray(sp_destruction->a("normals"));
	glDisableVertexAttribArray(sp_destruction->a("texCoord0"));

	glfwSwapBuffers(window); //Przerzuć tylny bufor na przedni
}

//----------------MECHANIKA SZACHÓW---------------------
//-------------------------------------------------------

//Funkcja do ustawiania figur na początku rozgrywki
void ustawFigury() {
	//Czyszczenie wektora pieces
	pieces.clear();
	//Dodawanie wszystkich figur i ustawianie ich na miejsce
	//Typy figur:
	// 0 = pionek
	// 1 = wieża
	// 2 = koń
	// 3 = goniec
	// 4 = hetman
	// 5 = król
	//Kolory:
	// 0 = biały
	// 1 = czarny
	for (int j = 0; j < 2; j++) {						// Taaa, wiem, napisałem to w dziwny sposób
		for (int i = 1; i < 9; i++) {					// nie chciałem wstawiać żadnych if'ów itp
														// pozycja Z ma wynosić 2 dla białych i 7 dla czarnych pionków
			pieces.push_back(new Piece(0, i, 0, 5 * j + 2, j));// Piece(typ figury, X, Z, kolor)
		}
		for (int i = 1; i < 9; i = i + 7) {
			// pozycja Z ma wynosić 1 dla białych i 8 dla czarnych wież
			pieces.push_back(new Piece(1, i, 0, 7 * j + 1, j));
		}
		for (int i = 2; i < 9; i = i + 5) {
			pieces.push_back(new Piece(2, i, 0, 7 * j + 1, j));
		}
		for (int i = 3; i < 7; i = i + 3) {
			pieces.push_back(new Piece(3, i, 0, 7 * j + 1, j));
		}
		pieces.push_back(new Piece(4, 5, 0, 7 * j + 1, j));
		pieces.push_back(new Piece(5, 4, 0, 7 * j + 1, j));
	}
}

//funkcja do interpretowania nazw pól
int liczba_numer(string pole) {
	if (pole == "a") return 8;
	else if (pole == "b") return 7;
	else if (pole == "c") return 6;
	else if (pole == "d") return 5;
	else if (pole == "e") return 4;
	else if (pole == "f") return 3;
	else if (pole == "g") return 2;
	else if (pole == "h") return 1;
}

//funkcja do podnoszenia figur
void podnies(Piece* figura, GLFWwindow* window) {
	bool flagY = true;
	float vY = Y_VELOCITY;
	while (flagY) {
		if (pause == 0) {
			if (abs(figura->posY - 1.5) <= abs(vY)) {
				figura->posY = 1.5;
				flagY = false;
			}
			else {
				figura->posY += vY;
			}
		}
		if (reset == 1 || glfwWindowShouldClose(window)) {
			break;
		}

		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		glfwSetTime(0);

		//Obrót kamery działa najlepiej dla kątów z przedziału (-PI/2, PI/2)
		angle_y = min(angle_y, PI / 2 - 0.00001f);
		angle_y = max(angle_y, -PI / 2 + 0.00001f);

		drawScene(window, angle_x, angle_y);
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
}

//funkcja do opuszczania figur
void opusc(Piece* figura, GLFWwindow* window) {
	bool flagY = true;
	float vY = -Y_VELOCITY;
	while (flagY) {
		if (pause == 0) {
			if (abs(figura->posY) <= abs(vY)) {
				figura->posY = 0;
				flagY = false;
			}
			else {
				figura->posY += vY;
			}
		}
		if (reset == 1 || glfwWindowShouldClose(window)) {
			break;
		}

		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		glfwSetTime(0);

		//Obrót kamery działa najlepiej dla kątów z przedziału (-PI/2, PI/2)
		angle_y = min(angle_y, PI / 2 - 0.00001f);
		angle_y = max(angle_y, -PI / 2 + 0.00001f);

		drawScene(window, angle_x, angle_y);
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
}

//funkcja do niszczenia figur
void destroy(Piece* figura, GLFWwindow* window) {
	while (figura->destruct*-1 < MAX_DESTRUCTION) {
		if (pause == 0) {
			figura->destruct -= DESTRUCTION_SPEED;
		}
		if (reset == 1 || glfwWindowShouldClose(window)) {
			break;
		}

		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		glfwSetTime(0); //Zeruj timer

		//Obrót kamery działa najlepiej dla kątów z przedziału (-PI/2, PI/2)
		angle_y = min(angle_y, PI / 2 - 0.00001f);
		angle_y = max(angle_y, -PI / 2 + 0.00001f);

		drawScene(window, angle_x, angle_y);
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
	for (int i = 0; i < pieces.size(); i++) {
		if (pieces[i] == figura) {
			pieces.erase(pieces.begin() + i);
			break;
		}
	}
}

//funkcja do przesuwania figur w osiach X i Z
void przesun(Piece* figura, float x, float z, GLFWwindow* window) {
	if (figura->type == 2)	podnies(figura, window);
	float vX, vZ;
	if (x >= figura->posX) vX = X_VELOCITY;
	else vX = -X_VELOCITY;

	if (z >= figura->posZ) vZ = Z_VELOCITY;
	else vZ = -Z_VELOCITY;

	bool flagX = true;
	bool flagZ = true;

	while (flagX || flagZ) {
		if (pause == 0) {
			if (abs(figura->posX - x) <= abs(vX)) {
				figura->posX = x;
				flagX = false;
			}
			else {
				figura->posX += vX;
			}

			if (abs(figura->posZ - z) <= abs(vZ)) {
				figura->posZ = z;
				flagZ = false;
			}
			else {
				figura->posZ += vZ;
			}
		}
		if (reset == 1 || glfwWindowShouldClose(window)) {
			break;
		}

		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		glfwSetTime(0);

		//Obrót kamery działa najlepiej dla kątów z przedziału (-PI/2, PI/2)
		angle_y = min(angle_y, PI / 2 - 0.00001f);
		angle_y = max(angle_y, -PI / 2 + 0.00001f);

		drawScene(window, angle_x, angle_y);
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
	if (figura->type == 2)	opusc(figura, window);
}

void ruch_pion(string start, string koniec, bool bicie, string ep_pos, string figura, GLFWwindow* window, bool ismat) {
	int startX = liczba_numer(start.substr(0, 1));
	int startZ = stoi(start.substr(1, 1));
	int koniecX = liczba_numer(koniec.substr(0, 1));
	int koniecZ = stoi(koniec.substr(1, 1));

	if (figura == "W") figura = "wieze.";
	else if (figura == "S") figura = "skoczka.";
	else if (figura == "G") figura = "gonca.";
	else if (figura == "H") figura = "hetmana.";

	Piece* pion = pieces[0];

	for (int i = 0; i < pieces.size(); i++) {
		if (pieces[i]->posX == startX && pieces[i]->posZ == startZ) {
			pion = pieces[i];
			break;
		}
	}

	if (bicie) {
		if (ep_pos != "0") {
			int epX = liczba_numer(ep_pos.substr(0, 1));
			int epZ = stoi(ep_pos.substr(1, 1));
			int bity = 0;

			for (int i = 0; i < pieces.size(); i++) {
				if (pieces[i]->posX == epX && pieces[i]->posZ == epZ) {
					bity = i;
					break;
				}
			}

			cout << "Ruch piona z " << start << " na " << koniec << ". Bicie w przelocie piona na " << ep_pos << endl;
			przesun(pion, koniecX, koniecZ, window);
			destroy(pieces[bity], window);
		}
		else {
			int bity = 0;

			for (int i = 0; i < pieces.size(); i++) {
				if (pieces[i]->posX == koniecX && pieces[i]->posZ == koniecZ) {
					bity = i;
					break;
				}
			}

			if (figura != "0") {
				cout << "Bicie piona z " << start << " na " << koniec << ". Promocja piona na " << figura << endl;
				przesun(pion, koniecX, koniecZ, window);
				destroy(pieces[bity], window);
				if (figura == "W")	pion->type = 1;
				else if (figura == "S") pion->type = 2;
				else if (figura == "G") pion->type = 3;
				else pion->type = 4;
			}
			else {
				cout << "Bicie piona z " << start << " na " << koniec << endl;
				przesun(pion, koniecX, koniecZ, window);
				destroy(pieces[bity], window);
			}
		}
	}
	else {
		if (figura != "0") {
			cout << "Ruch piona z " << start << " na " << koniec << ". Promocja piona na " << figura << endl;
			przesun(pion, koniecX, koniecZ, window);
			if (figura == "W")	pion->type = 1;
			else if (figura == "S") pion->type = 2;
			else if (figura == "G") pion->type = 3;
			else pion->type = 4;
		}
		else {
			cout << "Ruch piona z " << start << " na " << koniec << endl;
			przesun(pion, koniecX, koniecZ, window);
		}
	}

	if (ismat) cout << "MAT." << endl;
}

void ruch_figura(string start, string koniec, string nazwa, bool bicie, GLFWwindow* window, bool ismat) {
	int startX = liczba_numer(start.substr(0, 1));
	int startZ = stoi(start.substr(1, 1));
	int koniecX = liczba_numer(koniec.substr(0, 1));
	int koniecZ = stoi(koniec.substr(1, 1));

	Piece* figura = pieces[0];

	for (int i = 0; i < pieces.size(); i++) {
		if (pieces[i]->posX == startX && pieces[i]->posZ == startZ) {
			figura = pieces[i];
			break;
		}
	}

	if (bicie) {
		int bity = 0;

		for (int i = 0; i < pieces.size(); i++) {
			if (pieces[i]->posX == koniecX && pieces[i]->posZ == koniecZ) {
				bity = i;
				break;
			}
		}

		cout << "Bicie " << nazwa << " z " << start << " na " << koniec << endl;
		przesun(figura, koniecX, koniecZ, window);
		destroy(pieces[bity], window);
	}
	else {
		cout << "Ruch " << nazwa << " z " << start << " na " << koniec << endl;
		przesun(figura, koniecX, koniecZ, window);
	}

	if (ismat) cout << "MAT." << endl;
}

void roszada(bool dluga, bool biale, GLFWwindow* window) {
	Piece* krol = pieces[0];
	Piece* wieza = pieces[0];

	for (int i = 0; i < pieces.size(); i++) {
		if (pieces[i]->color != biale && pieces[i]->type == 5) {
			krol = pieces[i];
			break;
		}
	}

	podnies(krol, window);

	if (dluga) {
		//Zależnie od tego czy roszada jest długa czy krótka to używa się innej wieży
		for (int i = 0; i < pieces.size(); i++) {
			if (pieces[i]->color != biale && pieces[i]->type == 1 && pieces[i]->posX == 8) {
				wieza = pieces[i];
				break;
			}
		}
		if (biale) {
			cout << "Biale roszuja (dluga roszada)" << endl;
			przesun(krol, 6, 1, window);
			przesun(wieza, 5, 1, window);
		}
		else {
			cout << "Czarne roszuja (dluga roszada)" << endl;
			przesun(krol, 6, 8, window);
			przesun(wieza, 5, 8, window);
		}
	}
	else {
		for (int i = 0; i < pieces.size(); i++) {
			if (pieces[i]->color != biale && pieces[i]->type == 1 && pieces[i]->posX == 1) {
				wieza = pieces[i];
				break;
			}
		}
		if (biale) {
			cout << "Biale roszuja (krotka roszada)" << endl;
			przesun(krol, 2, 1, window);
			przesun(wieza, 3, 1, window);
		}
		else {
			cout << "Czarne roszuja (krotka roszada)" << endl;
			przesun(krol, 2, 8, window);
			przesun(wieza, 3, 8, window);
		}
	}
	opusc(krol, window);
}

void wczytaj(GLFWwindow* window, float angle_x, float angle_y) {
	//tablica tura przechowuje ruch bialych w komorce 0 i czarnych w komorce 1
	string tura[2];
	string line;
	ifstream myfile("partia.txt");
	if (myfile.is_open()) {
		//wzorce ruchów
		regex pion("^[a-h].+");
		regex wieza("^W.+");
		regex skoczek("^S.+");
		regex goniec("^G.+");
		regex krol("^K.+");
		regex hetman("^H.+");
		regex bicie(".+:.+");
		regex en_passant(".+e\\.p\\.");
		regex promocja("^[a-h].+[WSGH]");
		regex roszada_krotka("^0-0$");
		regex roszada_dluga("^0-0-0$");
		regex mat(".*#$");
		regex rezygnacja("^!$");
		regex komentarz("^//.+");

		//zmienna biale mowi czy teraz nastepuje ruch bialych czy czarnych
		bool biale = true;
		//zmienna pos przechowuje pozycje w linijce, na ktorej znajduje sie spacja (granica miedzy ruchem bialych a czarnych)
		int pos;
		bool ismat = false;
		bool resigned = false;
		// zmienna result przechowuje dane na temat tego, który gracz wygrał
		// 0 = remis
		// 1 = wygrana białych
		// 2 = wygrana czanych
		char result = 0;
		while (getline(myfile, line)) {
			if (regex_search(line, komentarz)) continue;
			if (reset == 1 || glfwWindowShouldClose(window)) {
				break;
			}
			cout << line << endl;
			//znalezienie spacji
			pos = line.find(" ");
			//podzial na ruch bialych
			tura[0] = line.substr(0, pos);
			//i czarnych
			tura[1] = line.substr(pos + 1, line.length() - pos);
			smatch wynik;
			int x = 1;
			if (regex_search(tura[0], wynik, mat)) {
				x = 0;
				ismat = true;
				result = 1;
			}
			else if (regex_search(tura[1], wynik, mat)) {
				ismat = true;
				result = 2;
			}
			else if (regex_search(tura[0], wynik, rezygnacja)) {
				resigned = true;
				x = 0;
				result = 2;
			}
			else if (regex_search(tura[1], wynik, rezygnacja)) {
				resigned = true;
				result = 1;
			}
			for (int i = 0; i <= x; i++) {
				line = tura[i];
				//wykryto ruch pionem z promocją
				if (regex_search(line, wynik, promocja)) {
					string ruch = wynik.str();
					string start = ruch.substr(0, 2);
					string koniec = ruch.substr(3, 2);
					string figura = ruch.substr(5, 1);
					ruch_pion(start, koniec, regex_search(line, bicie), "0", figura, window, ismat);
				}
				//wykryto ruch pionem
				else if (regex_search(line, wynik, pion)) {
					string ruch = wynik.str();
					string start = ruch.substr(0, 2);
					string koniec = ruch.substr(3, 2);
					//wykryto bicie w przelocie
					if (regex_search(line, en_passant)) {
						string bity;
						int wiersz = stoi(koniec.substr(1, 1));
						if (biale) {
							wiersz--;
							bity = koniec.substr(0, 1) + to_string(wiersz);
						}
						else {
							wiersz++;
							bity = koniec.substr(0, 1) + to_string(wiersz);
						}
						ruch_pion(start, koniec, regex_search(line, bicie), bity, "0", window, ismat);
					}
					else {
						ruch_pion(start, koniec, regex_search(line, bicie), "0", "0", window, ismat);
					}
				}
				//wykryto ruch wieżą
				else if (regex_search(line, wynik, wieza)) {
					string ruch = wynik.str();
					string start = ruch.substr(1, 2);
					string koniec = ruch.substr(4, 2);
					ruch_figura(start, koniec, "wieza", regex_search(line, bicie), window, ismat);
				}
				//wykryto ruch skoczkiem
				else if (regex_search(line, wynik, skoczek)) {
					string ruch = wynik.str();
					string start = ruch.substr(1, 2);
					string koniec = ruch.substr(4, 2);
					ruch_figura(start, koniec, "skoczkiem", regex_search(line, bicie), window, ismat);
				}
				//wykryto ruch gońcem
				else if (regex_search(line, wynik, goniec)) {
					string ruch = wynik.str();
					string start = ruch.substr(1, 2);
					string koniec = ruch.substr(4, 2);
					ruch_figura(start, koniec, "goncem", regex_search(line, bicie), window, ismat);
				}
				//wykryto ruch królem
				else if (regex_search(line, wynik, krol)) {
					string ruch = wynik.str();
					string start = ruch.substr(1, 2);
					string koniec = ruch.substr(4, 2);
					ruch_figura(start, koniec, "krolem", regex_search(line, bicie), window, ismat);
				}
				//wykryto ruch hetmanem
				else if (regex_search(line, wynik, hetman)) {
					string ruch = wynik.str();
					string start = ruch.substr(1, 2);
					string koniec = ruch.substr(4, 2);
					ruch_figura(start, koniec, "hetmanem", regex_search(line, bicie), window, ismat);
				}
				//wykryto krótką roszadę
				else if (regex_search(line, roszada_krotka)) {
					roszada(false, biale, window);
				}
				//wykryto długą roszadę
				else if (regex_search(line, roszada_dluga)) {
					roszada(true, biale, window);
				}
				//przejscie z ruchu bialych na ruch czarnych lub odwrotnie
				lights = biale;
				biale = !biale;
			}
		}
		myfile.close();

		if (result == 1) {
			cout << "Biale wygrywaja";
			if (resigned) cout << " walkowerem! (rezygnacja czarnych)" << endl;
			else cout << "!" << endl;
		}
		else if (result == 2) {
			cout << "Czarne wygrywaja";
			if (resigned) cout << " walkowerem! (rezygnacja bialych)" << endl;
			else cout << "!" << endl;
		}
		else cout << "Rozgrywka nie rozstrzygnieta!" << endl;

		// nieważne jaki był wynik ustawiamy wartość lights na 2
		lights = 2;
		// a potem jedynie dodajemy do niej wartość result
		lights += result;
	}
}

//--------------MAIN--------------------
//--------------------------------------

int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące
	ustawFigury();

	//Główna pętla
	glfwSetTime(0); //Zeruj timer
	wczytaj(window, angle_x, angle_y);

	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		glfwSetTime(0); //Zeruj timer

		//Obrót kamery działa najlepiej dla kątów z przedziału (-PI/2, PI/2)
		angle_y = min(angle_y, PI / 2 - 0.00001f);
		angle_y = max(angle_y, -PI / 2 + 0.00001f);

		//Obsługa funkcji resetu
		if (reset == 1) {
			ustawFigury();
			cout << endl << "----==============----" << endl << "Zresetowano rozgrywke!" << endl << "----==============----" << endl << endl;
			reset = 0;
			lights = 0;
			wczytaj(window, angle_x, angle_y);
		}

		drawScene(window, angle_x, angle_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
