#ifndef MINPACK_H
#define MINPACK_H

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#include <QtCore/QVector>
#pragma GCC diagnostic pop

#include <gsl/gsl_multimin.h>

using namespace std;

class MinPack
{
public:

	MinPack(const QVector<double> *T, const QVector<double> *Y);
	virtual ~MinPack() {}
	gsl_multimin_function_fdf func;
	virtual void setValues(double phi, double alpha, double *amp, double *theta, double *trendIntegral, QVector<double> *dTrend, QVector<double> *dFit) = 0;
	void testDerivative();
	double ssTot() const {return yy-ySum*ySum/y->size();}
	double e_integral(double amp, double alpha) const {return fabs(amp)/alpha*(exp(alpha*t->last())-exp(alpha*t->first()));}

protected:

	const QVector<double> *t, *y;
	double yy, ySum;

private:

	MinPack(const MinPack &mp); // do not implement
	MinPack&operator=(const MinPack &mp); // do not implement

};

#endif // MINPACK_H
