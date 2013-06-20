#ifndef FOUPACK_H
#define FOUPACK_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>

#include "minpack.h"

using namespace std;

class FouPack: public MinPack
{

public:

	FouPack(const QVector<double> *T, const QVector<double> *Y, int FouDegree, double FouShortestPeriod_h);
	~FouPack();
	virtual void setValues(double phi, double alpha, double *amp, double *theta, double *trendIntegral, QVector<double> *dTrend, QVector<double> *dFit);
	double setBefTrend(QVector<double> *dTrend);

private:

	FouPack(const FouPack &f); // do not implement
	FouPack &operator=(const FouPack &f); // do not implement
	static double f   (const gsl_vector *x, void *params);
	static void   df  (const gsl_vector *x, void *params,            gsl_vector *df);
	static void   fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df);
	int fouDegree;
	double fouShortestPeriod_h;
	double Pyy;
	QVector<QVector<double> > s, c;
	//double Y, YY;
	//QVector<double> s, c, ys, yc;
	//QVector<QVector<double> > S, C, ss, sc, cc;
	gsl_matrix *mat_LU;
	gsl_permutation *per;
	gsl_vector *Dy;

};


#endif // FOUPACK_H
