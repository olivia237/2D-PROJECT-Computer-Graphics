//Elaborato 1 computer Grafics
//Author: Olivia Rannick Nguemo

#include <iostream>

#include "ShaderMaker.h"
#include "lib.h"

#include <GL/glew.h>
#include <GL/freeglut.h>



static unsigned int programId, programId_text;
bool Coll = FALSE;
#define  PI   3.14159265358979323846
int frame_animazione = 0; // usato per animare la fluttuazione
unsigned int VAO_Text, VBO_Text;
float angolo = 0.0f;
float s = 1, dx = 0, dy = 0, dx_f = 0, dy_f = 0;
mat4 Projection;
GLuint MatProj, MatModel, loctime, locres;
float posx_Proiettile, posy_Proiettile, angoloCannone = 0.0;

int nv_P;
// viewport size
int width = 1280;
int height = 720;
float w_update, h_update;

bool drawBB = FALSE;

int posx = (float)width / 2.0, posy = (float)height / 4.0;

//STRUTTURA FIGURA
typedef struct {
	GLuint VAO;
	GLuint VBO_G;
	GLuint VBO_C;
	int nTriangles;
	// Vertici
	vector<vec3> vertici;
	vector<vec4> colors;
	// Numero vertici
	int nv;
	//Matrice di Modellazione: Traslazione*Rotazione*Scala
	mat4 Model;
	vec4 corner_b_obj;
	vec4 corner_t_obj;
	vec4 corner_b;
	vec4 corner_t;
	bool alive;
} Figura;

//SCENA
vector<Figura> Scena;

Figura Proiettile = {};
Figura Brico = {};
Figura Cielo = {};
Figura Fuoco = {};
Figura Luna = {};
Figura Stella = {};

void crea_VAO_Vector(Figura* fig)
{
	glGenVertexArrays(1, &fig->VAO);
	glBindVertexArray(fig->VAO);
	//Genero , rendo attivo, riempio il VBO della geometria dei vertici
	glGenBuffers(1, &fig->VBO_G);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_G);
	glBufferData(GL_ARRAY_BUFFER, fig->vertici.size() * sizeof(vec3), fig->vertici.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	//Genero , rendo attivo, riempio il VBO dei colori
	glGenBuffers(1, &fig->VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_C);
	glBufferData(GL_ARRAY_BUFFER, fig->colors.size() * sizeof(vec4), fig->colors.data(), GL_STATIC_DRAW);
	//Adesso carico il VBO dei colori nel layer 2
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

}
void costruisci_cielo(vec4 color_top, vec4 color_bot, Figura* cielo)
{
	cielo->vertici.push_back(vec3(0.0, 0.0, 0.0));
	cielo->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_top.a));
	cielo->vertici.push_back(vec3(1.0, 0.0, 0.0));
	cielo->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_bot.a));
	cielo->vertici.push_back(vec3(0.0, 1.0, 0.0));
	cielo->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	cielo->vertici.push_back(vec3(1.0, 1.0, 0.0));
	cielo->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));

	cielo->nv = cielo->vertici.size();

	//Costruzione matrice di Modellazione Sole, che rimane la stessa(non si aggiorna)
	cielo->Model = mat4(1.0);
	cielo->Model = translate(cielo->Model, vec3(0.0, float(height) / 2, 0.0));
	cielo->Model = scale(cielo->Model, vec3(float(width), float(height) / 2, 1.0));
}

void costruisci_fuoco(vec4 color_top, vec4 color_bot, Figura* mare)
{
	mare->vertici.push_back(vec3(0.0, 0.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(0.0, 1.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(1.0, 0.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(1.0, 1.0, 0.0));
	mare->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_bot.a));

	mare->nv = mare->vertici.size();

	//Costruzione matrice di Modellazione Prato, che rimane la stessa(non si aggiorna)
	mare->Model = mat4(1.0);
	mare->Model = scale(mare->Model, vec3(float(width), float(height) / 2, 1.0));
}


void costruisci_brico(float cx, float cy, float raggiox, float raggioy, Figura* fig) {
	int i;
	float stepA = (PI) / fig->nTriangles;
	float t;
	float xmax = 0;
	float xmin = 0;

	float ymax = 0;
	float ymin = 0;


	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(255.0/255.0, 128.0/255.0, 191.0/255.0,1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA ;
		fig->vertici.push_back(vec3(cx + raggiox * cos(t), cy + raggioy * sin(t), 0.0));
		//Colore 
		fig->colors.push_back(vec4(0.0, 0.0, 102.0/255.0, 1.0));
	}



	fig->nv = fig->vertici.size();

	for (i = 1; i < fig->nv; i++)
	{

		if (fig->vertici[i].x < xmin)
			xmin = fig->vertici[i].x;
	}

	for (i = 1; i < fig->nv; i++)
	{

		if (fig->vertici[i].x > xmax)
			xmax = fig->vertici[i].x;
	}
	for (i = 1; i < fig->nv; i++)
	{
		if (fig->vertici[i].y <= ymin)
			ymin = fig->vertici[i].y;
	}

	for (i = 1; i < fig->nv; i++)
	{
		if (fig->vertici[i].y > ymax)
			ymax = fig->vertici[i].y;
	}
	//Aggiorno i valori del corner più in basso a sinistra (corner_b) e del corner più in alto a destra (conrner_t)

	fig->corner_b_obj = vec4(xmin, ymin, 0.0, 1.0);
	fig->corner_t_obj = vec4(xmax, ymax, 0.0, 1.0);
	//Aggiungo i vertici della spezzata per costruire il bounding box
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));

	//Aggiorno il numero dei vertici della figura
	fig->nv = fig->vertici.size();

}


void costruisci_proiettile(float cx, float cy, float raggiox, float raggioy, Figura* fig) {

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;
	float xmax = 0;
	float xmin = 0;

	float ymax = 0;
	float ymin = 0;


	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(0.0, 0.0, 102.0 / 255.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * cos(t), cy + raggioy * sin(t), 0.0));
		//Colore 
		fig->colors.push_back(vec4(255.0 / 255.0, 128.0 / 255.0, 191.0 / 255.0, 1.0));


	}
	fig->nv = fig->vertici.size();

	//Calcolo di xmin, ymin, xmax, ymax

	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].x <= xmin)
			xmin = fig->vertici[i].x;


	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].x > xmax)
			xmax = fig->vertici[i].x;

	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].y <= ymin)
			ymin = fig->vertici[i].y;


	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].y > ymax)
			ymax = fig->vertici[i].y;

	//Aggiorno i valori del corner più in basso a sinistra (corner_b) e del corner più in alto a destra (conrner_t)

	fig->corner_b_obj = vec4(xmin, ymin, 0.0, 1.0);
	fig->corner_t_obj = vec4(xmax, ymax, 0.0, 1.0);
	//Aggiungo i vertici della spezzata per costruire il bounding box
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));

	//Aggiorno il numero dei vertici della figura
	fig->nv = fig->vertici.size();
}
void costruisci_luna(float cx, float cy, float raggiox, float raggioy, Figura* fig) {

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;


	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(1.0, 1.0, 1.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * (sin(t) * 3) / 4, cy + raggioy * (0.5 - cos(2 * t) - cos(t)) / 4, 0.0));
		//Colore 
		fig->colors.push_back(vec4(1.0, 1.0, 1.0, 0.0));


	}
	fig->nv = fig->vertici.size();

}
void costruisci_stella(Figura* fig) {

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;


	fig->vertici.push_back(vec3(0.0, 0.0, 0.0));

	fig->colors.push_back(vec4(1.0, 0.8627, 0.0, 1.0000));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;

		if (i % 2 == 0)
			fig->vertici.push_back(vec3(cos(t), sin(t), 0.0));
		else
			fig->vertici.push_back(vec3(0.5 * cos(t), 0.5 * sin(t), 0.0));
		//Colore 
		fig->colors.push_back(vec4(1.0, 0.9, 0.9, 0.8));


	}
	fig->nv = fig->vertici.size();

}

void INIT_SHADER(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);



}

void INIT_VAO(void)
{
	Proiettile.nTriangles = 180;
	costruisci_proiettile(0.0, 0.0, 1.0, 1.0, &Proiettile);
	crea_VAO_Vector(&Proiettile);
	Proiettile.alive = TRUE;
	Scena.push_back(Proiettile);

	Brico.nTriangles = 180;
	costruisci_brico(0.0, 0.0, 1.0, 1.0, &Brico);
	crea_VAO_Vector(&Brico);
	Scena.push_back(Brico);

	
	vec4 col_top = { 0.0, 0.0,0.1,1.0 };
	vec4 col_bottom = { 0.0, 0.0,0.2,1.0 };
	costruisci_cielo(col_top, col_bottom, &Cielo);
	crea_VAO_Vector(&Cielo);
	Scena.push_back(Cielo);

	col_top = vec4{ 0.0, 0.0,0.1,1.0 };
	col_bottom = vec4{ 0.0, 0.0,0.2,1.0 };
	costruisci_fuoco(col_top, col_bottom, &Fuoco);
	crea_VAO_Vector(&Fuoco);
	Scena.push_back(Fuoco);


	Luna.nTriangles = 180;
	costruisci_luna(0.0, 0.0, 1.0, 1.0, &Luna);
	crea_VAO_Vector(&Luna);
	Scena.push_back(Luna);


	Stella.nTriangles = 10;
	costruisci_stella(&Stella);
	crea_VAO_Vector(&Stella);
	Scena.push_back(Stella);

	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
	loctime = glGetUniformLocation(programId, "time");
	locres = glGetUniformLocation(programId, "resolution");
}

void INIT_VAO_Text(void)
{

	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO_Text);
	glGenBuffers(1, &VBO_Text);
	glBindVertexArray(VAO_Text);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
bool checkCollision(Figura obj1, Figura obj2) {
	// guardo collisioni su asse x
	bool collisionX = obj1.corner_b.x <= obj2.corner_t.x &&
		obj1.corner_t.x >= obj2.corner_b.x;

	// guardo collisioni su asse y
	bool collisionY = obj1.corner_b.y <= obj2.corner_t.y &&
		obj1.corner_t.y >= obj2.corner_b.y;
	//Si ha collisione se c'è collisione sia nella direzione x che nella direzione y

	return collisionX && collisionY;
}
double  degtorad(double angle) {
	return angle * PI / 180;
}
void disegno_proiettile(int value)
{

	Scena[0].Model = mat4(1.0);
	Scena[0].Model = translate(Scena[0].Model, vec3((900.0 - dx_f) / 2, 700.0 - dy_f, 0.0));
	Scena[0].Model = scale(Scena[0].Model, vec3(30.5, 30.5, 1.0));

	Scena[0].corner_b = Scena[0].corner_b_obj;
	Scena[0].corner_t = Scena[0].corner_t_obj;
	//printf("Farfalla \n");
	Scena[0].corner_b = Scena[0].Model * Scena[0].corner_b;
	//std::cout << glm::to_string(Scena[0].corner_b) << std::endl;
	Scena[0].corner_t = Scena[0].Model * Scena[0].corner_t;
	//std::cout << glm::to_string(Scena[0].corner_t) << std::endl;

	Coll = checkCollision(Scena[0], Scena[1]);

	if (Coll == TRUE)
		Scena[0].alive = FALSE;

	//Disegno il proiettile fino a quando non sia avvenuta la prima collisione con la palla del brico
	if (Scena[0].alive == TRUE)
	{

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[0].Model));
		glBindVertexArray(Scena[0].VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[0].nv - 6);
		if (drawBB == TRUE)
			//Disegno Bounding Box
			glDrawArrays(GL_LINE_STRIP, Scena[0].nv - 6, 6);
		glBindVertexArray(0);
	}
	
}
void disegno_proiettile_m(int value) {
	disegno_proiettile(0);
	glutTimerFunc(1000, disegno_proiettile_m, 0);
	glutPostRedisplay();
}

void drawScene(void)
{
	int i = 0, j = 0;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(14.0);
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	vec2 resolution = vec2(w_update, h_update);

	glUniform1f(loctime, time);
	glUniform2f(locres, resolution.x, resolution.y);
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));


	//Disegna Cielo e fuoco
	for (i = 2; i <4; i++)
	{

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[i].Model));
		glBindVertexArray(Scena[i].VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, Scena[i].nv);
		glBindVertexArray(0);
	}
	
	for (int k = 0; k < 5; k++) {
		//disegno luna e mat di modellazione

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[4].Model));
		glBindVertexArray(Scena[4].VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[4].nv);

		//modelazione di luna 
		Scena[4].Model = mat4(1.0);
		Scena[4].Model = translate(Scena[4].Model, vec3((500.0f) * (float(k)/2), 650.0f , 0.0f));
		Scena[4].Model = scale(Scena[4].Model, vec3(50.0f * s, 50.0f * s, 1.0f));
		/*Matrice di modellazione del brico */
		glBindVertexArray(0);
	}
	for (int g = 0; g < 6; g++) {
		//disegno stella e mat di modellazione

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[5].Model));
		glBindVertexArray(Scena[5].VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[5].nv);

		//modelazione di stella 
		Scena[5].Model = mat4(1.0);
		Scena[5].Model = translate(Scena[5].Model, vec3((300.0f)*g, (610.0f ), 0.0f));
		Scena[5].Model = scale(Scena[5].Model, vec3(7.5f * s, 7.5f * s, 1.0f));
		glBindVertexArray(0);
	}
	/*Matrice di modellazione del brico */
	Scena[1].Model = mat4(1.0);
	Scena[1].Model = translate(Scena[1].Model, vec3(600.0 + dx, 300.0 + dy, 0.0));


	

	//std::cout << glm::to_string(Scena[2].Model) << std::endl;
	Scena[1].corner_b = Scena[1].corner_b_obj;
	Scena[1].corner_t = Scena[1].corner_t_obj;
	Scena[1].corner_b = Scena[1].Model * Scena[1].corner_b;
	//printf("Proiettile \n");
	//std::cout << glm::to_string(Scena[2].corner_b) << std::endl;
	Scena[1].corner_t = Scena[1].Model * Scena[1].corner_t;
	//std::cout << glm::to_string(Scena[2].corner_t) << std::endl;
	 /*Update Scala Matrice di modellazione del brico */

	Scena[1].Model = scale(Scena[1].Model, vec3(80.0, 80.5, 1.0));


	if (drawBB == TRUE)
	{
		//Disegno Bounding Box
		glDrawArrays(GL_LINE_STRIP, Scena[1].nv - 6, 6);
		glBindVertexArray(0);
	}


	//Disegno brico

	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[1].Model));
	glBindVertexArray(Scena[1].VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[1].nv-6);
	glBindVertexArray(0);


	/*Matrice di modellazione proiettile */

	disegno_proiettile(0);

		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[5].Model));
		glBindVertexArray(Scena[5].VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, Scena[5].nv);

	glutSwapBuffers();



}


void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	{
		switch (key)
		{
		case 'b':
			drawBB = FALSE;
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}
void myKeyboard(unsigned char key, int x, int y)
{
	{
		switch (key)
		{


		case 'a':
			dx -= 1;
			break;

		case 'd':
			dx += 1;
			break;
		case 's':
			dy -= 1;
			break;

		case 'w':
			dy += 1;
			break;

		case 'b':
			drawBB = TRUE;
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}
void update_c(int value)
{
	dx_f = dx_f + 0.1;
	dy_f = dy_f + 0.5;
	glutTimerFunc(25, update_c, 0);
	glutPostRedisplay();
}



void reshape(int w, int h)
{
	Projection = ortho(0.0f, (float)width, 0.0f, (float)height);

	float AspectRatio_mondo = (float)(width) / (float)(height); //Rapporto larghezza altezza di tutto ciò che è nel mondo
	 //Se l'aspect ratio del mondo è diversa da quella della finestra devo mappare in modo diverso 
	 //per evitare distorsioni del disegno
	if (AspectRatio_mondo > w / h)   //Se ridimensioniamo la larghezza della Viewport
	{
		glViewport(0, 0, w, w / AspectRatio_mondo);
		w_update = (float)w;
		h_update = w / AspectRatio_mondo;
	}
	else {  //Se ridimensioniamo la larghezza della viewport oppure se l'aspect ratio tra la finestra del mondo 
			//e la finestra sullo schermo sono uguali
		glViewport(0, 0, h * AspectRatio_mondo, h);
		w_update = h * AspectRatio_mondo;
		h_update = (float)h;
	}




}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Catch the ball");
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(myKeyboard);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyboardReleasedEvent);
	glutTimerFunc(25, update_c, 0);
	glutTimerFunc(1000, disegno_proiettile_m, 0);

	glewExperimental = GL_TRUE;
	glewInit();
	INIT_SHADER();
	INIT_VAO();

	//Gestione della Trasparenza
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}


