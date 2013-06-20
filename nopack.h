#ifndef NOPACK_H
#define NOPACK_H

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>

#include "minpack.h"

class NoPack: public MinPack
{

public:

	NoPack(const QVector<double> *T, const QVector<double> *Y);
	~NoPack();
	virtual void setValues(double phi, double alpha, double *amp, double *theta, double* trendIntegral, QVector<double> *dTrend, QVector<double> *dFit);
	
private:

	NoPack(const NoPack &f); // do not implement
	NoPack &operator=(const NoPack &f); // do not implement
	static double f   (const gsl_vector *x, void *params);
	static void   df  (const gsl_vector *x, void *params,            gsl_vector *df);
	static void   fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df);

};

#endif // NOPACK_H
