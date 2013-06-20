#include "polypack.h"

#include <math.h>
#include <iostream>

#include <gsl/gsl_linalg.h>

PolyPack::PolyPack(const QVector<double> *T, const QVector<double> *Y, int PolyDegree) :
	MinPack(T, Y),
	polyDegree(PolyDegree),
	pow(polyDegree+1, QVector<double>(t->size(), 1)),
	mat_LU (gsl_matrix_alloc     (polyDegree+1, polyDegree+1)),
	per    (gsl_permutation_alloc(polyDegree+1)),
	Dy     (gsl_vector_alloc     (polyDegree+1)), 
	Wyy(0)
{
	gsl_vector *wy = gsl_vector_calloc (polyDegree+1); // set to 0
	for (int tNr = 0; tNr < y->size(); tNr++) wy->data[0] += y->at(tNr);
	gsl_matrix_set(mat_LU, 0, 0, t->size());
	for (int p = 1; p <= polyDegree; p++) 
	{
		double powSum = 0;
		double powSumY = 0;
		for (int tNr = 0; tNr < t->size(); tNr++) 
		{
			pow[p][tNr] = pow[p-1][tNr]*t->at(tNr);
			powSum     += pow[p][tNr];
			powSumY    += pow[p][tNr] * y->at(tNr);
		}
		gsl_vector_set(wy, p, powSumY);
		for (int col = 0; col <= p; col++) gsl_matrix_set(mat_LU, col, p-col, powSum);
	}
	QVector<double> tmp = pow[polyDegree];
	for (int p = 1; p <= polyDegree; p++)
	{
		double powSum = 0;
		for (int tNr = 0; tNr < t->size(); tNr++) 
		{
			tmp[tNr] = tmp[tNr] * t->at(tNr);
			powSum += tmp[tNr];
		}
		for (int col = p; col <= polyDegree; col++) gsl_matrix_set(mat_LU, col, polyDegree+p-col, powSum);
	}
	int sign;
	gsl_linalg_LU_decomp (mat_LU, per, &sign);
	gsl_linalg_LU_solve(mat_LU, per, wy, Dy);
	for (int nPol = 0; nPol <= polyDegree; nPol++) Wyy += Dy->data[nPol] * wy->data[nPol];
	gsl_vector_free(wy);
	func.n   = 2;
	func.f   = f;
	func.df  = df;
	func.fdf = fdf;
	func.params = reinterpret_cast<void*>(this);
}

PolyPack::~PolyPack()
{
	gsl_matrix_free(mat_LU);
	gsl_permutation_free(per);
	gsl_vector_free(Dy);
}


double PolyPack::f (const gsl_vector *x, void *params)
{
	PolyPack *pp = reinterpret_cast<PolyPack*>(params);
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	gsl_vector *wc = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	gsl_vector *ws = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	double pyc = 0, pys = 0, pss = 0, psc = 0, pcc = 0;
	for (int tNr = 0; tNr < pp->t->size(); tNr++)
	{
		const double  eat = exp(alpha * pp->t->at(tNr));
		const double seat = sin(phi   * pp->t->at(tNr)) * eat;
		const double ceat = cos(phi   * pp->t->at(tNr)) * eat;
		pys += pp->y->at(tNr) * seat;
		pyc += pp->y->at(tNr) * ceat;
		pss += seat * seat;
		psc += seat * ceat;
		pcc += ceat * ceat;
		for (int nPol = 0; nPol <= pp->polyDegree; nPol++)
		{
			ws->data[nPol] += pp->pow[nPol][tNr] * seat;
			wc->data[nPol] += pp->pow[nPol][tNr] * ceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(pp->polyDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(pp->polyDegree+1);
	
	gsl_linalg_LU_solve(pp->mat_LU, pp->per, ws, Ds);
	gsl_linalg_LU_solve(pp->mat_LU, pp->per, wc, Dc);
	
	double Wys = 0, Wyc = 0, Wss = 0, Wsc = 0, Wcc = 0;
	for (int nPol = 0; nPol <= pp->polyDegree; nPol++)
	{
		Wys += pp->Dy->data[nPol] * ws->data[nPol];
		Wyc += pp->Dy->data[nPol] * wc->data[nPol];
		Wss +=     Ds->data[nPol] * ws->data[nPol];
		Wsc +=     Ds->data[nPol] * wc->data[nPol];
		Wcc +=     Dc->data[nPol] * wc->data[nPol];
	}
	
	const double nom = 
		    (pcc-Wcc)*(pys-Wys)*(pys-Wys)
		- 2*(psc-Wsc)*(pys-Wys)*(pyc-Wyc) 
		+   (pss-Wss)*(pyc-Wyc)*(pyc-Wyc)
	;
	const double den = 
		  (pcc-Wcc)*(pss-Wss)
		- (psc-Wsc)*(psc-Wsc)
	;
	const double value = pp->yy - pp->Wyy - nom/den;
	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
	return value;
}
void PolyPack::df (const gsl_vector *x, void *params, gsl_vector *df)
{
	double dummy;
	fdf(x, params, &dummy, df);
}
void PolyPack::fdf (const gsl_vector *x, void *params, double *f, gsl_vector *df)
{
	PolyPack *pp = reinterpret_cast<PolyPack*>(params);
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	gsl_vector *wc  = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	gsl_vector *ws  = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	gsl_vector *wTc = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	gsl_vector *wTs = gsl_vector_calloc(pp->polyDegree+1); // set to 0
	double pyc  = 0, pys  = 0, pss  = 0, psc  = 0, pcc  = 0;
	double pTyc = 0, pTys = 0, pTss = 0, pTsc = 0, pTcc = 0;
	for (int tNr = 0; tNr < pp->t->size(); tNr++)
	{
		const double  eat = exp(alpha * pp->t->at(tNr));
		const double seat = sin(phi   * pp->t->at(tNr)) * eat;
		const double ceat = cos(phi   * pp->t->at(tNr)) * eat;
		pys += pp->y->at(tNr) * seat;
		pyc += pp->y->at(tNr) * ceat;
		pss += seat * seat;
		psc += seat * ceat;
		pcc += ceat * ceat;
		pTys += pp->t->at(tNr) * pp->y->at(tNr) * seat;
		pTyc += pp->t->at(tNr) * pp->y->at(tNr) * ceat;
		pTss += pp->t->at(tNr) * seat * seat;
		pTsc += pp->t->at(tNr) * seat * ceat;
		pTcc += pp->t->at(tNr) * ceat * ceat;
		for (int nPol = 0; nPol <= pp->polyDegree; nPol++)
		{
			ws->data[nPol]  +=                  pp->pow[nPol][tNr] * seat;
			wc->data[nPol]  +=                  pp->pow[nPol][tNr] * ceat;
			wTs->data[nPol] += pp->t->at(tNr) * pp->pow[nPol][tNr] * seat;
			wTc->data[nPol] += pp->t->at(tNr) * pp->pow[nPol][tNr] * ceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(pp->polyDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(pp->polyDegree+1);
	
	gsl_linalg_LU_solve(pp->mat_LU, pp->per, ws,  Ds);
	gsl_linalg_LU_solve(pp->mat_LU, pp->per, wc,  Dc);
	
	double Wys  = 0, Wyc  = 0, Wss  = 0, Wsc  = 0, Wcc  = 0;
	double WTys = 0, WTyc = 0, WTss = 0, WTsc = 0, WTcs = 0, WTcc = 0;
	for (int nPol = 0; nPol <= pp->polyDegree; nPol++)
	{
		Wys   += pp->Dy->data[nPol] * ws->data[nPol];
		Wyc   += pp->Dy->data[nPol] * wc->data[nPol];
		Wss   +=     Ds->data[nPol] * ws->data[nPol];
		Wsc   +=     Ds->data[nPol] * wc->data[nPol];
		Wcc   +=     Dc->data[nPol] * wc->data[nPol];
		WTys  += pp->Dy->data[nPol] * wTs->data[nPol];
		WTyc  += pp->Dy->data[nPol] * wTc->data[nPol];
		WTss +=      Ds->data[nPol] * wTs->data[nPol];
		WTsc +=      Dc->data[nPol] * wTs->data[nPol];
		WTcs +=      Ds->data[nPol] * wTc->data[nPol];
		WTcc +=      Dc->data[nPol] * wTc->data[nPol];
	}
	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
	gsl_vector_free(wTs);
	gsl_vector_free(wTc);
	const double nom = 
	        (pcc-Wcc)*(pys-Wys)*(pys-Wys)
		- 2*(psc-Wsc)*(pys-Wys)*(pyc-Wyc) 
		+   (pss-Wss)*(pyc-Wyc)*(pyc-Wyc)
	;
	const double nom_A = 
	2* (
		  (  pTcc     -WTcc)      * (pys -Wys)  * (pys -Wys)
		+ (  pcc      -Wcc)       * (pTys-WTys) * (pys -Wys)
		- (2*pTsc     -WTsc-WTcs) * (pys -Wys)  * (pyc -Wyc) 
		- (  psc      -Wsc)       * (pTys-WTys) * (pyc -Wyc) 
		- (  psc      -Wsc)       * (pys -Wys)  * (pTyc-WTyc) 
		+ (  pTss     -WTss)      * (pyc -Wyc)  * (pyc -Wyc)
		+ (  pss      -Wss)       * (pTyc-WTyc) * (pyc -Wyc)
	);
	const double nom_P = 
	2 * (
		- (  pTsc     -WTsc)      * (pys -Wys)  * (pys -Wys)
		+ (  pcc      -Wcc)       * (pTyc-WTyc) * (pys -Wys)
		- (  pTcc-pTss-WTcc+WTss) * (pys -Wys)  * (pyc -Wyc) 
		- (  psc      -Wsc)       * (pTyc-WTyc) * (pyc -Wyc) 
		+ (  psc      -Wsc)       * (pys -Wys)  * (pTys-WTys) 
		+ (  pTsc     -WTcs)      * (pyc -Wyc)  * (pyc -Wyc)
		- (  pss      -Wss)       * (pTys-WTys) * (pyc -Wyc)
	);
	const double den = 
		  (pcc-Wcc)*(pss-Wss)
		- (psc-Wsc)*(psc-Wsc)
	;
	const double den_A = 
	2 * (
		  (pTcc-WTcc) * (pss -Wss)
		+ (pcc -Wcc)  * (pTss-WTss)
		- (pTsc-WTsc) * (psc -Wsc)
		- (psc -Wsc)  * (pTsc-WTsc)
	);
	const double den_P = 
	2 * (
		- (pTsc     -WTsc)      * (pss -Wss)
		+ (pcc      -Wcc)       * (pTsc-WTcs)
		- (pTcc-pTss-WTcc+WTss) * (psc -Wsc)
	);
	const double value  = pp->yy - pp->Wyy - nom / den;
	const double dVal_A = - (nom_A*den-nom*den_A) / (den*den);
	const double dVal_P = - (nom_P*den-nom*den_P) / (den*den);
	*f = value;
	gsl_vector_set(df, 0, dVal_P);
	gsl_vector_set(df, 1, dVal_A);
}


void PolyPack::setValues(double phi, double alpha, double *amp, double *theta, double *trendIntegral, QVector<double> *dTrend, QVector<double> *dFit)
{
	gsl_vector *wc = gsl_vector_calloc(polyDegree+1); // set to 0
	gsl_vector *ws = gsl_vector_calloc(polyDegree+1); // set to 0
	double pyc = 0, pys = 0, pss = 0, psc = 0, pcc = 0;
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
		for (int nPol = 0; nPol <= polyDegree; nPol++)
		{
			ws->data[nPol] += pow[nPol][tNr] * seat;
			wc->data[nPol] += pow[nPol][tNr] * ceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(polyDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(polyDegree+1);
	
	gsl_linalg_LU_solve(mat_LU, per, ws, Ds);
	gsl_linalg_LU_solve(mat_LU, per, wc, Dc);
	
	double Wys = 0, Wyc = 0, Wss = 0, Wsc = 0, Wcc = 0;
	for (int nPol = 0; nPol <= polyDegree; nPol++)
	{
		Wys += Dy->data[nPol] * ws->data[nPol];
		Wyc += Dy->data[nPol] * wc->data[nPol];
		Wss += Ds->data[nPol] * ws->data[nPol];
		Wsc += Ds->data[nPol] * wc->data[nPol];
		Wcc += Dc->data[nPol] * wc->data[nPol];
	}
	const double denom = (pcc-Wcc)*(pss-Wss) - (psc-Wsc)*(psc-Wsc);
	const double asNom = (pcc-Wcc)*(pys-Wys) - (psc-Wsc)*(pyc-Wyc);
	const double acNom = (pss-Wss)*(pyc-Wyc) - (psc-Wsc)*(pys-Wys);
	const double as = asNom / denom;
	const double ac = acNom / denom;
	const double omega = asNom / acNom;
	vector<double> poly(polyDegree+1);
	for (int pNr = 0; pNr <= polyDegree; pNr++) poly[pNr] = Dy->data[pNr] - as*Ds->data[pNr] - ac*Dc->data[pNr];
	*theta = atan(omega);
	*amp = ac/cos(*theta);
	*dTrend = QVector<double>(t->size(), 0); // set to zero
	dFit->resize(t->size());
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		for (int pNr = 0; pNr <= polyDegree; pNr++) 
		{
			dTrend->data()[tNr] += poly[pNr]*pow[pNr][tNr];
		}
		dFit->data()[tNr] = *amp * exp(alpha * t->at(tNr)) * cos(phi * t->at(tNr) - *theta);
	}
	*trendIntegral = 0;
	for (int pNr = 0; pNr <= polyDegree; pNr++) 
	{
		*trendIntegral += poly[pNr]/(pNr+1) * (pow[pNr][t->size()-1]*t->last() - pow[pNr][0]*t->first());
	}
	*trendIntegral = e_integral(*amp, alpha) / *trendIntegral;
	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
}

double PolyPack::setBefTrend(QVector<double> *dTrend)
{
	*dTrend = QVector<double>(t->size(), 0); // set to zero
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		for (int pNr = 0; pNr <= polyDegree; pNr++) 
		{
			dTrend->data()[tNr] += Dy->data[pNr]*pow[pNr][tNr];
		}
	}
	double result = 0;
	for (int pNr = 0; pNr <= polyDegree; pNr++) 
	{
		result += Dy->data[pNr]/(pNr+1) * (pow[pNr][t->size()-1]*t->last() - pow[pNr][0]*t->first());
	}
	return result;
}
