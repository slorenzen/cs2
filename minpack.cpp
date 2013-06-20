#include "minpack.h"
#include <iostream>

MinPack::MinPack(const QVector<double> *T, const QVector<double> *Y) : 
	func(), t(T), y(Y), yy(0), ySum(0)
{
	for (QVector<double>::const_iterator it = Y->begin(); it != Y->end(); it++) 
	{
		ySum += *it;
		yy += *it * *it;
	}
}

void MinPack::testDerivative()
{
	const int PHISTEPS = 25;
	const int ALPSTEPS = 25;
	const double MINALP = -0.75;
	const double MAXALP = -0.25;
	const double MINPHI = 1.5*M_PI;
	const double MAXPHI = 2.5*M_PI;
	gsl_vector *x = gsl_vector_alloc(2);
	gsl_vector *dx = gsl_vector_alloc(2);
	QVector<double> trend, fit;
	double amp, theta;
	double dummy;
	for (int nAlp = 0; nAlp <= ALPSTEPS; nAlp++)
	{
		double alp = MINALP + (MAXALP-MINALP)*nAlp/ALPSTEPS;
		gsl_vector_set(x, 1, alp);
		for (int nPhi = 0; nPhi <= PHISTEPS; nPhi++)
		{
			double phi = MINPHI + (MAXPHI-MINPHI)*nPhi/PHISTEPS;
			gsl_vector_set(x, 0, phi);
			double f;
			func.fdf(x, this, &f, dx);
			setValues(phi, alp, &amp, &theta, &dummy, &trend, &fit);
			double errSum = 0;
			for (int tNr = 0; tNr < t->size(); tNr++) 
			{
				const double err = y->at(tNr) - trend.at(tNr) - fit.at(tNr);
				errSum += err*err;
			}
			cout << "\t" << f << "\t" << errSum << "\t" << dx->data[0] << "\t" << dx->data[1];
		}
		cout << endl;
	}
	for (int nPhi = 0; nPhi <= PHISTEPS; nPhi++)
	{
		double phi = MINPHI + (MAXPHI-MINPHI)*nPhi/PHISTEPS;
		gsl_vector_set(x, 0, phi);
		for (int nAlp = 0; nAlp <= ALPSTEPS; nAlp++)
		{
			double alp = MINALP + (MAXALP-MINALP)*nAlp/ALPSTEPS;
			gsl_vector_set(x, 1, alp);
			double f;
			func.fdf(x, this, &f, dx);
			setValues(phi, alp, &amp, &theta, &dummy, &trend, &fit);
			double errSum = 0;
			for (int tNr = 0; tNr < t->size(); tNr++) 
			{
				const double err = y->at(tNr) - trend.at(tNr) - fit.at(tNr);
				errSum += err*err;
			}
			cout << "\t" << f << "\t" << errSum << "\t" << dx->data[0] << "\t" << dx->data[1];
		}
		cout << endl;
	}
	gsl_vector_free(x);
	gsl_vector_free(dx);
}
