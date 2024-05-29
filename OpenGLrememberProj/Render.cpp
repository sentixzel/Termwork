#include "Render.h"

#include <sstream>
#include <iostream>

#include <math.h>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;



	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


class Prism {
	GLdouble Points[8][3] = {
							{ 4,  0,  0},
							{ 11, 1,  0},
							{ 9,  8,  0},
							{ 13, 8,  0},
							{ 12, 15, 0},
							{ 7,  16, 0},
							{ 6,  9,  0},
							{ 0,  7,  0} };

	GLdouble PointsHigher[8][3] = {
									{ 4,  0,  10},
									{ 11, 1,  10},
									{ 9,  8,  10},
									{ 13, 8,  10},
									{ 12, 15, 10},
									{ 7,  16, 10},
									{ 6,  9,  10},
									{ 0,  7,  10} };
	
	GLdouble normal[3] = { 0,0,0 };

public:
	double getPoints(int i, int j)
	{
		return Points[i][j];
	}

	void setNormal(double* fst, double* scnd, double* thrd)
	{
		GLdouble ax = - scnd[0] + fst[0],
				 ay = - scnd[1] + fst[1],
				 az = - scnd[2] + fst[2],

				 bx = - thrd[0] + fst[0],
				 by = - thrd[1] + fst[1],
				 bz = - thrd[2] + fst[2];

		GLdouble x = ay * bz - by * az;
		GLdouble y = bx * az - ax * bz;
		GLdouble z = ax * by - bx * ay;

		GLdouble size = x * x + y * y + z * z;
		size = sqrt(size);

		normal[0] = x / size;
		normal[1] = y / size;
		normal[2] = z / size;

		glNormal3d(normal[0], normal[1], normal[2]);
	}

	void render()
	{
		glBegin(GL_TRIANGLES);
			//main "square"
			//1st
			setNormal(Points[1], Points[0], Points[2]);
			for (int i = 0; i < 3; i++)
				glVertex3dv(Points[i]);
			//2nd - special

			glVertex3dv(Points[3]);

			for (double j = -0.14; j < 3.14; j += 0.06)
			{
				GLdouble NextCirPoint[] = { 12.5 + 3.53 * sin(j) , 11.5 + 3.53 * cos(j) , 0 };
				glVertex3dv(Points[2]);
				glVertex3dv(NextCirPoint);

				GLdouble PastCirPoint[3];

				for (int i = 0; i < 3; i++)
					PastCirPoint[i] = NextCirPoint[i];

				glVertex3dv(PastCirPoint);
			}
			glVertex3dv(Points[4]);
			glVertex3dv(Points[2]);

			//3rd
			for (int i = 4; i < 6; i++)
				glVertex3dv(Points[i]);
			glVertex3dv(Points[2]);


			//4th
			glVertex3dv(Points[5]);
			glVertex3dv(Points[6]);
			glVertex3dv(Points[2]);

			for (int i = 6; i < 8; i++)
				glVertex3dv(Points[i]);
			glVertex3dv(Points[2]);

			//6th - spedcial
			glVertex3dv(Points[7]);
			for (double j = 0.7; j < 1.5; j += 0.04)
			{
				double NextCirPoint[] = { -5 + 9.01 * sin(j) , -0.5 + 9.01 * cos(j) , 0 };
				glVertex3dv(NextCirPoint);
				glVertex3dv(Points[2]);

				double PastCirPoint[3];
				for (int i = 0; i < 3; i++)
					PastCirPoint[i] = NextCirPoint[i];

				glVertex3dv(PastCirPoint);
			}
			glVertex3dv(Points[0]);
			glVertex3dv(Points[2]);


			setNormal(PointsHigher[0], PointsHigher[1], PointsHigher[2]);
			for (int i = 0; i < 3; i++)
				glVertex3dv(PointsHigher[i]);


			glVertex3dv(PointsHigher[3]);
			for (double j = -0.14; j < 3.14; j += 0.06)
			{
				double NextCirPoint[] = { 12.5 + 3.53 * sin(j) , 11.5 + 3.53 * cos(j) , 10 };
				glVertex3dv(PointsHigher[2]);
				glVertex3dv(NextCirPoint);

				double PastCirPoint[3];
				for (int i = 0; i < 3; i++)
					PastCirPoint[i] = NextCirPoint[i];

				glVertex3dv(PastCirPoint);
			}
			glVertex3dv(PointsHigher[2]);
			glVertex3dv(PointsHigher[4]);


			for (int i = 4; i < 6; i++)
				glVertex3dv(PointsHigher[i]);
			glVertex3dv(PointsHigher[2]);


			for (int i = 5; i < 7; i++)
				glVertex3dv(PointsHigher[i]);
			glVertex3dv(PointsHigher[2]);

			//5th
			for (int i = 6; i < 8; i++)
				glVertex3dv(PointsHigher[i]);
			glVertex3dv(PointsHigher[2]);

			for (int i = 6; i < 8; i++)
				glVertex3dv(PointsHigher[i]);
			glVertex3dv(PointsHigher[2]);


			glVertex3dv(PointsHigher[7]);
			for (double j = 0.7; j < 1.5; j += 0.04)
			{
				double NextCirPoint[] = { -5 + 9.01 * sin(j) , -0.5 + 9.01 * cos(j) , 10 };
				glVertex3dv(NextCirPoint);
				glVertex3dv(PointsHigher[2]);

				GLdouble PastCirPoint[3];
				for (int i = 0; i < 3; i++)
					PastCirPoint[i] = NextCirPoint[i];

				glVertex3dv(PastCirPoint);
			}
			glVertex3dv(PointsHigher[0]);
			glVertex3dv(PointsHigher[2]);

		glEnd();

		//faces

		glBegin(GL_QUADS);
		double bufPoint[3];
		for (int i = 0; i < 7; i++)
		{
			glVertex3dv(Points[i]);
			glVertex3dv(PointsHigher[i]);
			for (int k = 0; k < 3; k++)
				bufPoint[k] = Points[i][k];

			if (i == 3)
			{
				double PastCirPoint[3];
				for (double j = -0.139; j < 3.1; j += 0.06)
				{
					double NextCirPoint[] = { 12.5 + 3.53 * sin(j) , 11.5 + 3.53 * cos(j) , 10 };
					glVertex3dv(NextCirPoint);

					for (int k = 0; k < 3; k++)
						PastCirPoint[k] = NextCirPoint[k];

					NextCirPoint[2] = 0;
					setNormal(bufPoint, PastCirPoint, NextCirPoint);
					glVertex3dv(NextCirPoint);
					glVertex3dv(NextCirPoint);
					NextCirPoint[2] = 10;
					glVertex3dv(NextCirPoint);
					for (int k = 0; k < 3; k++)
						bufPoint[k] = NextCirPoint[k];
				}

			}
			glVertex3dv(PointsHigher[i + 1]);
			setNormal(PointsHigher[i], bufPoint, PointsHigher[i + 1]);
			glVertex3dv(Points[i + 1]);
		}

		glVertex3dv(Points[7]);
		glVertex3dv(PointsHigher[7]);
		for (int k = 0; k < 3; k++)
			bufPoint[k] = Points[7][k];

		double PastCirPoint[3];

		for (double j = 0.7; j < 1.5; j += 0.04)
		{
			double NextCirPoint[] = { -5 + 9.01 * sin(j) , -0.5 + 9.01 * cos(j) , 10 };
			glVertex3dv(NextCirPoint);

			for (int k = 0; k < 3; k++)
				PastCirPoint[k] = NextCirPoint[k];

			NextCirPoint[2] = 0;
			setNormal(NextCirPoint, PastCirPoint, bufPoint);
			glVertex3dv(NextCirPoint);
			glVertex3dv(NextCirPoint);
			NextCirPoint[2] = 10;
			glVertex3dv(NextCirPoint);
			for (int k = 0; k < 3; k++)
				bufPoint[k] = NextCirPoint[k];
		}
		setNormal(PointsHigher[0], bufPoint, Points[0]);
		glVertex3dv(PointsHigher[0]);
		glVertex3dv(Points[0]);
		glEnd();
	}
};


void Render(OpenGL *ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	Prism prism;
	prism.render();

	////Начало рисования квадратика станкина
	//double A[2] = { -4, -4 };
	//double B[2] = { 4, -4 };
	//double C[2] = { 4, 4 };
	//double D[2] = { -4, 4 };

	//glBindTexture(GL_TEXTURE_2D, texId);


	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//	glNormal3d(0, 0, 1);
	//	glTexCoord2d(0, 0);
	//	glVertex2dv(A);
	//	glTexCoord2d(1, 0);
	//	glVertex2dv(B);
	//	glTexCoord2d(1, 1);
	//	glVertex2dv(C);
	//	glTexCoord2d(0, 1);
	//	glVertex2dv(D);

	//glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}