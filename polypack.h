#ifndef POLYPACK_H
#define POLYPACK_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>

#include "minpack.h"

using namespace std;

class PolyPack: public MinPack
{

public:

	PolyPack(const QVector<double> *T, const QVector<double> *Y, int PolyDegree);
	~PolyPack();
	virtual void setValues(double phi, double alpha, double *amp, double *theta, double *trendIntegral, QVector<double> *dDetrended, QVector<double> *dFit);
	double setBefTrend(QVector<double> *dTrend);

private:

	PolyPack(const PolyPack &f); // do not implement
	PolyPack &operator=(const PolyPack &f); // do not implement
	static double f   (const gsl_vector *x, void *params);
	static void   df  (const gsl_vector *x, void *params,            gsl_vector *df);
	static void   fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df);
	int polyDegree;
	QVector<QVector<double> > pow;
	gsl_matrix *mat_LU;
	gsl_permutation *per;
	gsl_vector *Dy;
	double Wyy;

};



#endif // POLYPACK_H
