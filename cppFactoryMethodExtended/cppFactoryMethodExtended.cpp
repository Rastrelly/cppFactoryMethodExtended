#include <iostream>
#include <vector>
#include <GL/freeglut.h>

//TYPEDEFS
typedef std::vector<float> fVector;
struct dataPoint { float x, y; };
typedef std::vector<dataPoint> dpVector;

//ENUMS
enum signalType
{
	ST_Sin,
	ST_Lin,
	ST_Quad,
	ST_Cub
};

//SIGNAL GENERATION
class signalGen
{
private:
	float vArg;
	fVector coeffs;
public:
	//getters and setters
	float getArg() { return vArg; }
	void setArg(float val) { vArg = val; }

	virtual fVector getCoeffs() { return coeffs; }
	virtual void setCoeffs(fVector val)
	{
		coeffs.clear(); coeffs = val;
	}

	float calcSignalVal() { return calcSignal(getArg(), getCoeffs()); }

	//factory components
	virtual float calcSignal(float arg, fVector c) = 0;
	static signalGen* Create(signalType st);
};

class sgSin : public signalGen
{
	float calcSignal(float arg, fVector c)
	{
		if (c.size() >= 4)
			return c[0] * sin(c[1] * arg + c[2]) + c[3];
		else
			return 0;
	}
};

class sgLin : public signalGen
{
	float calcSignal(float arg, fVector c)
	{
		if (c.size() >= 2)
			return c[0] * arg + c[1];
		else
			return 0;
	}
};


class sgQuad : public signalGen
{
	float calcSignal(float arg, fVector c)
	{
		if (c.size() >= 3)
			return c[0] * pow(arg, 2) + c[1] * arg + c[2];
		else
			return 0;
	}
};

class sgCub : public signalGen
{
	float calcSignal(float arg, fVector c)
	{
		if (c.size() >= 4)
			return c[0] * pow(arg, 3) + c[1] * pow(arg, 2) + c[2] * arg + c[3];
		else
			return 0;
	}
};

signalGen * signalGen::Create(signalType st)
{
	switch (st)
	{
	case ST_Sin:
		return new sgSin();
		break;
	case ST_Lin:
		return new sgLin();
		break;
	case ST_Quad:
		return new sgQuad();
		break;
	case ST_Cub:
		return new sgCub();
		break;
	default:
		return NULL;
		break;
	}
}

//data management
class dataGen
{
private:
	float xMin, xMax, yMin, yMax;
	int nSteps;
	dpVector dataSet;
	std::vector<signalGen*> genSet;
public:
	//polymorphic constructors
	dataGen()
	{
		xMin = -10;
		xMax = 10;
		nSteps = 100;
	}
	dataGen(float xMn, float xMx, int ns)
	{
		xMin = xMn;
		xMax = xMx;
		nSteps = ns;
	}

	dpVector getDataSet() { return dataSet; }
	float getXMin() { return xMin; }
	float getXMax() { return xMax; }
	float getYMin() { return yMin; }
	float getYMax() { return yMax; }

	void addGenerator(signalType st, fVector coeffs)
	{
		genSet.push_back(signalGen::Create(st));
		genSet[genSet.size() - 1]->setCoeffs(coeffs);
	}

	void genData()
	{
		dataSet.resize(nSteps + 1);
		int ng = genSet.size(); //amount of generators
		float dataStep = (xMax - xMin) / nSteps;

		for (int i = 0; i <= nSteps; i++)
		{
			float cx = xMin + i * dataStep;
			float cy = 0;
			for (int j = 0; j < ng; j++)
			{
				genSet[j]->setArg(cx);
				cy += genSet[j]->calcSignalVal();
			}
			dataSet[i] = { cx, cy };
		}
	}

	void extractExtremes()
	{
		yMin = dataSet[0].y;
		yMax = dataSet[0].y;
		for (dataPoint cp : dataSet)
		{
			if (cp.y > yMax) yMax = cp.y;
			if (cp.y < yMin) yMin = cp.y;
		}
	}
};

class Singleton
{
private:
	static Singleton * instance;
	Singleton() {}
public:
	dataGen * refDG;
	static Singleton *getInstance()
	{
		if (!instance)
			instance = new Singleton;
		return instance;
	}
} *Singleton::instance = 0;


//GRAPHICS
void cbReshape(int x, int y)
{
	glViewport(0, 0, x, y);
}

float normalize(float coord, float axMin, float axMax)
{
	return (coord - axMin) / (axMax - axMin);
}

float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

void cbDisplay()
{
	//prepwork
	Singleton *s = s->getInstance();
	float xMax = s->refDG->getXMax(),
		xMin = s->refDG->getXMin(),
		yMax = s->refDG->getYMax(),
		yMin = s->refDG->getYMin();
	//render
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(xMin, xMax, yMin, yMax);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(3);

	int l = s->refDG->getDataSet().size();

	glBegin(GL_LINE_STRIP);

	for (int i = 0; i < l; i++)
	{

		float cx = s->refDG->getDataSet()[i].x;
		float cy = s->refDG->getDataSet()[i].y;

		float cyNorm = normalize(cy, yMin, yMax);
		float r = lerp(0, 1, cyNorm);
		float g = lerp(1, 0, cyNorm);
		float b = 0.0f;

		glColor3f(r, g, b);

		glVertex2f(cx, cy);
	}

	glEnd();

	glutSwapBuffers();
}


void initOGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Chart View");

	glutReshapeFunc(cbReshape);
	glutDisplayFunc(cbDisplay);

	glutMainLoop();
}

int main(int argc, char **argv)
{
	Singleton *s = s->getInstance();
	dataGen DG(-100, 100, 1000);
	s->refDG = &DG;

	DG.addGenerator(ST_Sin, { 20,1,0,0 });
	DG.addGenerator(ST_Sin, { 20,0.1,0,0 });
	DG.addGenerator(ST_Quad, { 0.01,0,0 });
	DG.genData();
	DG.extractExtremes();

	initOGL(argc, argv);
}