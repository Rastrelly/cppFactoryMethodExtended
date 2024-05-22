#include <iostream>
#include <vector>
#include <fstream>
#include <GL/freeglut.h>
#include <cmath>

enum SignalType
{
	SIG_Sinusoid,
	SIG_Linear,
	SIG_Quadratic,
	SIG_Cubic
};

struct point
{
	float x, y;
};

class Singleton
{
private:
	//pointer to a Singleton instance
	static Singleton *instance;
	//private constructor
	Singleton()
	{
		data.clear();
	}

	//data fields
	std::vector<point> data;

public:
	//getters and setters
	std::vector<point> getData() { return data; }
	void setData(std::vector<point> val) { data = val; }

	void findDataExtremes(float &xmin, float &xmax, float &ymin, float &ymax)
	{
		int l = data.size();
		xmin = data[0].x;
		xmax = data[l - 1].x;
		ymin = data[0].y;
		ymax = data[0].y;
		for (int i = 0; i < l; i++)
		{
			if (data[i].y > ymax) ymax = data[i].y;
			if (data[i].y < ymin) ymin = data[i].y;
		}
	}

	//constructor returns static instance by pointer
	static Singleton *getInstance()
	{
		if (!instance)
			instance = new Singleton;
		return instance;
	}
};

//Singleton intialization to null
Singleton *Singleton::instance = 0;

//signal gen and children
class signalGen
{
public:
	float vArg; 
	std::vector<float>vC;
	void setupCalc(float arg, std::vector<float>c)
	{
		vArg = arg;
		vC = c;
	}
	void setArg(float val) { vArg = val; }
	point calcSigPoint()
	{
		return { vArg, calcSignal(vArg, vC) };
	}
	virtual float calcSignal(float arg, std::vector<float>c) = 0;
	static signalGen* Create(SignalType signal);
};

class sgSin :public signalGen
{
	float calcSignal(float arg, std::vector<float>c)
	{
		return c[0] * sin(c[1]*arg + c[2]);
	}
};

class sgLin:public signalGen
{
	float calcSignal(float arg, std::vector<float>c)
	{
		return c[0] * arg + c[1];
	}
};

class sgQuad :public signalGen
{
	float calcSignal(float arg, std::vector<float>c)
	{
		return c[0] * pow(arg,2) + c[1]*arg + c[2];
	}
};

class sgCub :public signalGen
{
	float calcSignal(float arg, std::vector<float>c)
	{
		return c[0] * pow(arg,3) + c[1]*pow(arg,2) + c[2]*arg + c[3];
	}
};

signalGen * signalGen::Create(SignalType signal)
{
	switch (signal)
	{
	case SIG_Sinusoid:
		return new sgSin();
		break;
	case SIG_Linear:
		return new sgLin();
		break;
	case SIG_Quadratic:
		return new sgQuad();
		break;
	case SIG_Cubic:
		return new sgCub();
		break;
	default:
		break;
	}
}

//main class
class dataGen
{
private:
	float fXmin, fXmax;
	int fNSteps;
	std::vector<point> dataSet;
	std::vector<signalGen*> signals;
public:
	dataGen(float xmin, float xmax, int nSteps)
	{
		fXmin = xmin;
		fXmax = xmax;
		fNSteps = nSteps;
	}

	void addGenerator(SignalType st, float arg, std::vector<float>c)
	{
		signals.push_back(signalGen::Create(st));
		int id = signals.size() - 1;
		signals[id]->setupCalc(arg,c);
	}

	void makeDataSet()
	{
		dataSet.clear();
		int ng = signals.size();
		float dx = (fXmax - fXmin) / fNSteps;
		for (int i = 0; i < fNSteps; i++)
		{
			point cData = {0,0};
			float cx = fXmin + dx * i;
			float cy = 0;
			for (int j = 0; j < ng; j++)
			{
				signals[j]->setArg(cx);

				point tempData = signals[j]->calcSigPoint();
				cy = tempData.y;

				cData.x += cx;
				cData.y += cy;
			}
			dataSet.push_back(cData);
		}
	}

	void saveDataSet()
	{
		std::ofstream writer("dataOut.csv");
		for (point cp : dataSet)
		{
			writer << (int)cp.x*10 << ";" << (int)cp.y*10 << "\n";
		}
		writer.close();
	}

	std::vector<point> getDataSet() { return dataSet; };
};

//freeglut ops
void cbReshape(int x, int y)
{
	glViewport(0,0,x,y);
}


float normalize(float val, float min, float max)
{
	return (val - min) / (max-min);
}

float lerp(float a, float b, float t)
{
	return a + t * (b-a);
}

void cbDisplay()
{

	float xmin, xmax, ymin, ymax;
	
	Singleton * sg = sg->getInstance();
	sg->findDataExtremes(xmin, xmax, ymin, ymax);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(
		xmin-0.05*(xmax-xmin), xmax+0.05*(xmax-xmin), 
		ymin-0.05*(ymax-ymin), ymax+0.05*(ymax-ymin)
	);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(3);
	
	glBegin(GL_LINE_STRIP);

	for (int i = 0; i < sg->getData().size(); i++)
	{
		float cx = sg->getData()[i].x;
		float cy = sg->getData()[i].y;
		
		float cyN = normalize(cy, ymin, ymax);
		float r = lerp(ymin,ymax,cyN);
		float g = lerp(ymax, ymin, cyN);
		float b = 0.0f;

		glColor3f(r, g, b);
		glVertex2f(cx, cy);
	}

	glEnd();

	glutSwapBuffers();

}


//main
int main(int argc, char **argv)
{
	//init singleton
	Singleton * sg = sg->getInstance();

	//init freeglut
	glutInit(&argc, argv);

	//make data
	dataGen DG(-10, 10, 1000);
	DG.addGenerator(SIG_Sinusoid, 0, {10,10,0});
	DG.addGenerator(SIG_Linear, 0, { 0.3f,0 });
	DG.addGenerator(SIG_Quadratic, 0, { 1,10,0 });

	DG.makeDataSet();
	DG.saveDataSet();

	sg->setData(DG.getDataSet());

	//draw data
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("GLUT window :D");

	glutReshapeFunc(cbReshape);
	glutDisplayFunc(cbDisplay);

	glutMainLoop();

	//the end
	system("pause");
}