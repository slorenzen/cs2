#include "nopack.h"

#include <math.h>

NoPack::NoPack(const QVector<double> *T, const QVector<double> *Y) :
MinPack(T, Y)
{
	func.n   = 2;
	func.f   = f;
	func.df  = df;
	func.fdf = fdf;
	func.params = reinterpret_cast<void*>(this);
}

NoPack::~NoPack()
{
}

double NoPack::f (const gsl_vector *x, void *params)
{
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	NoPack *np = reinterpret_cast<NoPack*>(params);
	double pss = 0, psc = 0, pcc = 0;
	double pys = 0, pyc = 0;
	for (int tNr = 0; tNr < np->t->size(); tNr++)
	{
		const double  eat = exp(alpha * np->t->at(tNr));
		const double seat = sin(phi   * np->t->at(tNr)) * eat;
		const double ceat = cos(phi   * np->t->at(tNr)) * eat;
		pys += np->y->at(tNr) * seat;
		pyc += np->y->at(tNr) * ceat;
		pss += seat * seat;
		psc += seat * ceat;
		pcc += ceat * ceat;
	}
	const double nom = 2*psc*pys*pyc - pcc*pys*pys - pss*pyc*pyc;
	const double denom = pss*pcc - psc*psc;
	return np->yy + nom / denom;
}

void NoPack::df (const gsl_vector *x, void *params, gsl_vector *df)
{
	double dummy;
	fdf(x, params, &dummy, df);
}

void NoPack::fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df)
{
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	NoPack *np = reinterpret_cast<NoPack*>(params);
	double pss = 0, psc = 0, pcc = 0;
	double pys = 0, pyc = 0;
	double ptss = 0, ptsc = 0, ptcc = 0;
	double ptys = 0, ptyc = 0;
	for (int tNr = 0; tNr < np->t->size(); tNr++)
	{
		const double  eat = exp(alpha * np->t->at(tNr));
		const double seat = sin(phi   * np->t->at(tNr)) * eat;
		const double ceat = cos(phi   * np->t->at(tNr)) * eat;
		pys += np->y->at(tNr) * seat;
		pyc += np->y->at(tNr) * ceat;
		pss += seat * seat;
		psc += seat * ceat;
		pcc += ceat * ceat;
		ptys += np->t->at(tNr) * np->y->at(tNr) * seat;
		ptyc += np->t->at(tNr) * np->y->at(tNr) * ceat;
		ptss += np->t->at(tNr) * seat * seat;
		ptsc += np->t->at(tNr) * seat * ceat;
		ptcc += np->t->at(tNr) * ceat * ceat;
	}
	
	const double nom = 2*psc*pys*pyc - pcc*pys*pys - pss*pyc*pyc;
	const double denom = pss*pcc-psc*psc;
	const double func = np->yy + nom / denom;
	const double dNom = (ptcc-ptss)*pys*pyc + psc*ptyc*pyc - psc*pys*ptys + ptsc*pys*pys - pcc*pys*ptyc - ptsc*pyc*pyc + pss*pyc*ptys;
	const double dDenom = ptsc*pcc - pss*ptsc - (ptcc-ptss)*psc;
	const double dFunc = 2 * (dNom/denom - nom*dDenom/(denom*denom));
	const double dNomA = 2*ptsc*pys*pyc + psc*ptys*pyc + psc*pys*ptyc - ptcc*pys*pys - pcc*pys*ptys - ptss*pyc*pyc - pss*pyc*ptyc;
	const double dDenomA = ptss*pcc + pss*ptcc - 2*psc*ptsc;
	const double dFuncA = 2 * (dNomA/denom - nom*dDenomA/(denom*denom));

	*f = func;
	gsl_vector_set(df, 0, dFunc);
	gsl_vector_set(df, 1, dFuncA);
}


void NoPack::setValues(double phi, double alpha, double *amp, double *theta, double *trendIntegral, QVector<double> *, QVector<double> *dFit)
{
	double pss = 0, psc = 0, pcc = 0;
	double pys = 0, pyc = 0;
	for (int tNr = 0; tNr < t->size(); tNr++)
	{
		const double  eat = exp(alpha * t->at(tNr));
		const double seat = sin(phi   * t->at(tNr)) * eat;
		const double ceat = cos(phi   * t->at(tNr)) * eat;
		pys += y->at(tNr) * seat;
		pyc += y->at(tNr) * ceat;
		pss += seat * seat;
		psc += seat * ceat;
		pcc += ceat * ceat;
	}
	const double omega = (pys*pcc-pyc*psc) / (pyc*pss-pys*psc);
	const double ac    = (pss*pyc-psc*pys) / (pcc*pss-psc*psc);
	*theta = atan(omega);
	*amp = ac/cos(*theta);
	// do not touch trend!!
	dFit->resize(t->size());
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		dFit->data()[tNr] = *amp * exp(alpha * t->at(tNr)) * cos(phi * t->at(tNr) - *theta);
	}
	*trendIntegral = e_integral(*amp, alpha) / *trendIntegral;
}