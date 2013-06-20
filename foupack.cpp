#include "foupack.h"

#include <iostream>
#include <math.h>
#include <vector>
#include <gsl/gsl_linalg.h>

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QDebug>
#pragma GCC diagnostic pop

FouPack::FouPack(const QVector< double >* T, const QVector< double >* Y, int FouDegree, double FouShortestPeriod_h) : 
	MinPack(T, Y),
	fouDegree(FouDegree),
	fouShortestPeriod_h(FouShortestPeriod_h),
	Pyy(0), 
	s(fouDegree+1, QVector<double>(t->size(), 1)),
	c(fouDegree+1, QVector<double>(t->size(), 1)),
	mat_LU (gsl_matrix_alloc     (2*fouDegree+1, 2*fouDegree+1)),
	per    (gsl_permutation_alloc(2*fouDegree+1)),
	Dy     (gsl_vector_alloc     (2*fouDegree+1))
{
	gsl_vector *wy = gsl_vector_calloc (2*fouDegree+1); // set to 0
	for (int tNr = 0; tNr < y->size(); tNr++) wy->data[0] += y->at(tNr);
	gsl_matrix_set(mat_LU, 0, 0, t->size());
	vector<double> sSum(2*fouDegree+1);
	vector<double> cSum(2*fouDegree+1);
	sSum[0] = 0;
	cSum[0] = t->size();
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		const double tPha = 2*M_PI * 24.0 * t->at(tNr) / fouDegree / fouShortestPeriod_h;
		for (int deg = 1; deg <= fouDegree; deg++) 
		{
			s[deg][tNr] = sin(deg*tPha);
			c[deg][tNr] = cos(deg*tPha);
			sSum[deg] += s[deg][tNr];
			cSum[deg] += c[deg][tNr];
			wy->data[deg]           += y->at(tNr)*s[deg][tNr];
			wy->data[deg+fouDegree] += y->at(tNr)*c[deg][tNr];
		}
		for (int deg = fouDegree+1; deg <= 2*fouDegree; deg++) 
		{
			sSum[deg] += sin(deg*tPha);
			cSum[deg] += cos(deg*tPha);
		}
	}
	for (int deg = 1; deg <= fouDegree; deg++)
	{
		gsl_matrix_set(mat_LU, 0, deg,           sSum[deg]);
		gsl_matrix_set(mat_LU, 0, deg+fouDegree, cSum[deg]);
		gsl_matrix_set(mat_LU, deg,           0, sSum[deg]);
		gsl_matrix_set(mat_LU, deg+fouDegree, 0, cSum[deg]);
		for (int deg2 = 1; deg2 < deg; deg2++)
		{
			gsl_matrix_set(mat_LU, deg,           deg2,           0.5*(cSum[deg-deg2]-cSum[deg+deg2]));
			gsl_matrix_set(mat_LU, deg+fouDegree, deg2,           0.5*(sSum[deg+deg2]-sSum[deg-deg2])); // sin changes sign
			gsl_matrix_set(mat_LU, deg,           deg2+fouDegree, 0.5*(sSum[deg+deg2]+sSum[deg-deg2])); // sin changes sign
			gsl_matrix_set(mat_LU, deg+fouDegree, deg2+fouDegree, 0.5*(cSum[deg-deg2]+cSum[deg+deg2]));
		}
		for (int deg2 = deg; deg2 <= fouDegree; deg2++)
		{
			gsl_matrix_set(mat_LU, deg,           deg2,           0.5*(cSum[deg2-deg]-cSum[deg2+deg]));
			gsl_matrix_set(mat_LU, deg+fouDegree, deg2,           0.5*(sSum[deg2+deg]+sSum[deg2-deg])); // sin changes sign
			gsl_matrix_set(mat_LU, deg,           deg2+fouDegree, 0.5*(sSum[deg2+deg]-sSum[deg2-deg])); // sin changes sign
			gsl_matrix_set(mat_LU, deg+fouDegree, deg2+fouDegree, 0.5*(cSum[deg2-deg]+cSum[deg2+deg]));
		}
	}
	int sign;
	gsl_linalg_LU_decomp (mat_LU, per, &sign);
	gsl_linalg_LU_solve(mat_LU, per, wy, Dy);
	const double NMyy = Dy->data[0] * wy->data[0];
	double Psyys = 0, Pcyyc = 0;
	for (int deg = 1; deg <= fouDegree; deg++)
	{
		Psyys += Dy->data[deg]           * wy->data[deg];
		Pcyyc += Dy->data[deg+fouDegree] * wy->data[deg+fouDegree];
	}
	Pyy = yy - NMyy - Psyys - Pcyyc;
	gsl_vector_free(wy);
	func.n   = 2;
	func.f   = f;
	func.df  = df;
	func.fdf = fdf;
	func.params = reinterpret_cast<void*>(this);
}

FouPack::~FouPack()
{
	gsl_matrix_free(mat_LU);
	gsl_permutation_free(per);
	gsl_vector_free(Dy);
}

double FouPack::f(const gsl_vector* x, void* params)
{
	FouPack *fp = reinterpret_cast<FouPack*>(params);
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	gsl_vector *wc = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	gsl_vector *ws = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	double Nys = 0, Nyc = 0, Nss = 0, Ncc = 0, Nsc = 0;
	for (int tNr = 0; tNr < fp->t->size(); tNr++)
	{
		const double  eat = exp(alpha * fp->t->at(tNr));
		const double seat = sin(phi   * fp->t->at(tNr)) * eat;
		const double ceat = cos(phi   * fp->t->at(tNr)) * eat;
		Nys += fp->y->at(tNr) * seat;
		Nyc += fp->y->at(tNr) * ceat;
		Nss += seat * seat;
		Nsc += seat * ceat;
		Ncc += ceat * ceat;
		ws->data[0] += seat;
		wc->data[0] += ceat;
		for (int deg = 1; deg <= fp->fouDegree; deg++)
		{
			ws->data[deg]               += fp->s[deg][tNr]*seat;
			ws->data[deg+fp->fouDegree] += fp->c[deg][tNr]*seat;
			wc->data[deg]               += fp->s[deg][tNr]*ceat;
			wc->data[deg+fp->fouDegree] += fp->c[deg][tNr]*ceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(2*fp->fouDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(2*fp->fouDegree+1);
	
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, ws, Ds);
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, wc, Dc);
	
	const double NMys = fp->Dy->data[0]*ws->data[0];
	const double NMyc = fp->Dy->data[0]*wc->data[0];
	const double NMss = Ds->data[0]*ws->data[0];
	//const double NMsc = Ds->data[0]*wc->data[0];
	const double NMcs = Dc->data[0]*ws->data[0];
	const double NMcc = Dc->data[0]*wc->data[0];
	//const double NMsy = Ds->data[0]*fp->wy->data[0]; // remove?
	//const double NMcy = Dc->data[0]*fp->wy->data[0]; // remove?
	
	double Psyss = 0, Psysc = 0, Pcycs = 0, Pcycc = 0;
	double Pssss = 0, /*Psssc = 0,*/ Pcscs = 0/*, Pcscc = 0*/;
	double Pscss = 0, Pscsc = 0, Pcccs = 0, Pcccc = 0;
	//double Pssys = 0, Pcsyc = 0, Pscys = 0, Pccyc = 0; // remove these four?
	for (int deg = 1; deg <= fp->fouDegree; deg++)
	{
		Psyss += fp->Dy->data[deg]               * ws->data[deg];
		Psysc += fp->Dy->data[deg]               * wc->data[deg];
		Pcycs += fp->Dy->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcycc += fp->Dy->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		Pssss +=     Ds->data[deg]               * ws->data[deg];
		//Psssc +=     Ds->data[deg]               * wc->data[deg];
		Pcscs +=     Ds->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		//Pcscc +=     Ds->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		Pscss +=     Dc->data[deg]               * ws->data[deg];
		Pscsc +=     Dc->data[deg]               * wc->data[deg];
		Pcccs +=     Dc->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcccc +=     Dc->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		//Pssys +=     Ds->data[deg]               * fp->wy->data[deg];
		//Pcsyc +=     Ds->data[deg+fp->fouDegree] * fp->wy->data[deg+fp->fouDegree];
		//Pscys +=     Dc->data[deg]               * fp->wy->data[deg];
		//Pccyc +=     Dc->data[deg+fp->fouDegree] * fp->wy->data[deg+fp->fouDegree];
	}
	
	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
	
	const double SS   = Nss - NMss - Pssss - Pcscs;
	const double CC   = Ncc - NMcc - Pcccc - Pscsc;
	const double SC   = Nsc - NMcs - Pcccs - Pscss; // (C eingesetzt, mit S multipliziert) = CS
	//const double CS   = Nsc - NMsc - Psssc - Pcscc; // (S eingesetzt, mit C multipliziert) = SC
	const double SYSS = Nys - NMys - Psyss - Pcycs;
	//const double SSYS = Nys - NMsy - Pssys - Pcsyc; // =SYSS
	const double CYCC = Nyc - NMyc - Psysc - Pcycc; 
	//const double CCYC = Nyc - NMcy - Pscys - Pccyc; // =CYCC
	
// 	for (int i = 0; i <= 2*fp->fouDegree; i++) printf("%8.3g ", ws->data[i]); printf("\n");
// 	for (int i = 0; i <= 2*fp->fouDegree; i++) printf("%8.3g ", wc->data[i]); printf("\n");
// 	for (int i = 0; i <= 2*fp->fouDegree; i++) printf("%8.3g ", Ds->data[i]); printf("\n");
// 	for (int i = 0; i <= 2*fp->fouDegree; i++) printf("%8.3g ", Dc->data[i]); printf("\n");
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", Psyss, Psysc, Pcycs, Pcycc);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", Pssss, Psssc, Pcscs, Pcscc);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", Pscss, Pscsc, Pcccs, Pcccc);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", SS, CC, SC, CS);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", NMsy, NMys, NMcy, NMyc);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", Pssys, Psyss, Pccyc, Pcycc);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", Pscys, Psysc, Pcsyc, Pcycs);
// 	printf("%8.3g %8.3g %8.3g %8.3g\n", SYSS, SSYS, CYCC, CCYC);
	
	//const double on = SYSS*CC - CYCC*SC;
	//const double od = CYCC*SS - SYSS*CS;
	//const double den =  CC*SS -   SC*CS;
	//const double as = on / den;
	//const double ac = od / den;
	
	//const double val = fp->yy - Psyys - Pcyyc - NMyy - 2*as*SSYS - 2*ac*CCYC + as*as*SS + ac*ac*CC + 2*as*ac*SC;
	const double val = fp->Pyy - (CC*SYSS*SYSS  - 2*CYCC*SC*SYSS + CYCC*CYCC*SS) / (CC*SS - SC*SC);
	//cout << "F\t" << phi << "\t" << alpha << "\t" << val << "\tNA\tNA" << endl;
	return val;
}

void FouPack::df(const gsl_vector* x, void* params, gsl_vector* df)
{
	double dummy;
	fdf(x, params, &dummy, df);
}

void FouPack::fdf(const gsl_vector* x, void* params, double* f, gsl_vector* df)
{
	FouPack *fp = reinterpret_cast<FouPack*>(params);
	const double phi   = gsl_vector_get(x, 0);
	const double alpha = gsl_vector_get(x, 1);
	gsl_vector *wc = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	gsl_vector *ws = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	gsl_vector *wTc = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	gsl_vector *wTs = gsl_vector_calloc(2*fp->fouDegree+1); // set to 0
	double Nys   = 0, Nyc   = 0, Nss   = 0, Ncc   = 0, Nsc   = 0;
	double Nys_A = 0, Nyc_A = 0, Nss_A = 0, Ncc_A = 0, Nsc_A = 0;
	double Nys_P = 0, Nyc_P = 0, Nss_P = 0, Ncc_P = 0, Nsc_P = 0;
	for (int tNr = 0; tNr < fp->t->size(); tNr++)
	{
		const double  eat = exp(alpha * fp->t->at(tNr));
		const double seat = sin(phi   * fp->t->at(tNr)) * eat;
		const double ceat = cos(phi   * fp->t->at(tNr)) * eat;
		const double Tseat = fp->t->at(tNr) * seat;
		const double Tceat = fp->t->at(tNr) * ceat;
		Nys   += fp->y->at(tNr) * seat;
		Nyc   += fp->y->at(tNr) * ceat;
		Nys_A += fp->y->at(tNr) * Tseat;
		Nyc_A += fp->y->at(tNr) * Tceat;
		Nys_P += fp->y->at(tNr) * Tceat;
		Nyc_P +=-fp->y->at(tNr) * Tseat;
		Nss   +=   seat * seat;
		Nsc   +=   seat * ceat;
		Ncc   +=   ceat * ceat;
		Nss_A += 2*seat * Tseat;
		Nsc_A +=   seat * Tceat + Tseat * ceat;
		Ncc_A += 2*ceat * Tceat;
		Nss_P += 2*seat * Tceat;
		Nsc_P +=  -seat * Tseat + Tceat * ceat;
		Ncc_P +=-2*ceat * Tseat;
		ws->data[0]  += seat;
		wc->data[0]  += ceat;
		wTs->data[0] += Tseat;
		wTc->data[0] += Tceat;
		for (int deg = 1; deg <= fp->fouDegree; deg++)
		{
			 ws->data[deg]               += fp->s[deg][tNr]*seat;
			 ws->data[deg+fp->fouDegree] += fp->c[deg][tNr]*seat;
			 wc->data[deg]               += fp->s[deg][tNr]*ceat;
			 wc->data[deg+fp->fouDegree] += fp->c[deg][tNr]*ceat;
			wTs->data[deg]               += fp->s[deg][tNr]*Tseat;
			wTs->data[deg+fp->fouDegree] += fp->c[deg][tNr]*Tseat;
			wTc->data[deg]               += fp->s[deg][tNr]*Tceat;
			wTc->data[deg+fp->fouDegree] += fp->c[deg][tNr]*Tceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(2*fp->fouDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(2*fp->fouDegree+1);
	gsl_vector *DTs = gsl_vector_alloc(2*fp->fouDegree+1);
	gsl_vector *DTc = gsl_vector_alloc(2*fp->fouDegree+1);
	
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, ws, Ds);
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, wc, Dc);
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, wTs, DTs);
	gsl_linalg_LU_solve(fp->mat_LU, fp->per, wTc, DTc);
	
	const double NMys   = fp->Dy->data[0]*ws->data[0];
	const double NMyc   = fp->Dy->data[0]*wc->data[0];
	const double NMss   =     Ds->data[0]*ws->data[0];
	const double NMcs   =     Dc->data[0]*ws->data[0];
	const double NMcc   =     Dc->data[0]*wc->data[0];
	const double NMys_A = fp->Dy->data[0]*wTs->data[0];
	const double NMyc_A = fp->Dy->data[0]*wTc->data[0];
	const double NMss_A =     Ds->data[0]*wTs->data[0] + DTs->data[0]*ws->data[0];
	const double NMcs_A =     Dc->data[0]*wTs->data[0] + DTc->data[0]*ws->data[0];
	const double NMcc_A =     Dc->data[0]*wTc->data[0] + DTc->data[0]*wc->data[0];
	const double NMys_P = fp->Dy->data[0]*wTc->data[0];
	const double NMyc_P =-fp->Dy->data[0]*wTs->data[0];
	const double NMss_P =     Ds->data[0]*wTc->data[0] + DTc->data[0]*ws->data[0];
	const double NMcs_P =     Dc->data[0]*wTc->data[0] - DTs->data[0]*ws->data[0];
	const double NMcc_P =    -Dc->data[0]*wTs->data[0] - DTs->data[0]*wc->data[0];
	
	double Psyss   = 0, Psysc   = 0, Pcycs   = 0, Pcycc   = 0;
	double Pssss   = 0, Pcscs   = 0, Pscss   = 0, Pscsc   = 0, Pcccs   = 0, Pcccc   = 0;
	double Psyss_A = 0, Psysc_A = 0, Pcycs_A = 0, Pcycc_A = 0;
	double Pssss_A = 0, Pcscs_A = 0, Pscss_A = 0, Pscsc_A = 0, Pcccs_A = 0, Pcccc_A = 0;
	double Psyss_P = 0, Psysc_P = 0, Pcycs_P = 0, Pcycc_P = 0;
	double Pssss_P = 0, Pcscs_P = 0, Pscss_P = 0, Pscsc_P = 0, Pcccs_P = 0, Pcccc_P = 0;
	for (int deg = 1; deg <= fp->fouDegree; deg++)
	{
		Psyss   += fp->Dy->data[deg]               * ws->data[deg];
		Psysc   += fp->Dy->data[deg]               * wc->data[deg];
		Pcycs   += fp->Dy->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcycc   += fp->Dy->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		Pssss   +=     Ds->data[deg]               * ws->data[deg];
		Pcscs   +=     Ds->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pscss   +=     Dc->data[deg]               * ws->data[deg];
		Pscsc   +=     Dc->data[deg]               * wc->data[deg];
		Pcccs   +=     Dc->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcccc   +=     Dc->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		Psyss_A += fp->Dy->data[deg]               * wTs->data[deg];
		Psysc_A += fp->Dy->data[deg]               * wTc->data[deg];
		Pcycs_A += fp->Dy->data[deg+fp->fouDegree] * wTs->data[deg+fp->fouDegree];
		Pcycc_A += fp->Dy->data[deg+fp->fouDegree] * wTc->data[deg+fp->fouDegree];
		Pssss_A +=     Ds->data[deg]               * wTs->data[deg]               + DTs->data[deg]               * ws->data[deg];
		Pcscs_A +=     Ds->data[deg+fp->fouDegree] * wTs->data[deg+fp->fouDegree] + DTs->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pscss_A +=     Dc->data[deg]               * wTs->data[deg]               + DTc->data[deg]               * ws->data[deg];
		Pscsc_A +=     Dc->data[deg]               * wTc->data[deg]               + DTc->data[deg]               * wc->data[deg];
		Pcccs_A +=     Dc->data[deg+fp->fouDegree] * wTs->data[deg+fp->fouDegree] + DTc->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcccc_A +=     Dc->data[deg+fp->fouDegree] * wTc->data[deg+fp->fouDegree] + DTc->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
		Psyss_P += fp->Dy->data[deg]               * wTc->data[deg];
		Psysc_P +=-fp->Dy->data[deg]               * wTs->data[deg];
		Pcycs_P += fp->Dy->data[deg+fp->fouDegree] * wTc->data[deg+fp->fouDegree];
		Pcycc_P +=-fp->Dy->data[deg+fp->fouDegree] * wTs->data[deg+fp->fouDegree];
		Pssss_P +=     Ds->data[deg]               * wTc->data[deg]               + DTc->data[deg]               * ws->data[deg];
		Pcscs_P +=     Ds->data[deg+fp->fouDegree] * wTc->data[deg+fp->fouDegree] + DTc->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pscss_P +=     Dc->data[deg]               * wTc->data[deg]               - DTs->data[deg]               * ws->data[deg];
		Pscsc_P +=    -Dc->data[deg]               * wTs->data[deg]               - DTs->data[deg]               * wc->data[deg];
		Pcccs_P +=     Dc->data[deg+fp->fouDegree] * wTc->data[deg+fp->fouDegree] - DTs->data[deg+fp->fouDegree] * ws->data[deg+fp->fouDegree];
		Pcccc_P +=    -Dc->data[deg+fp->fouDegree] * wTs->data[deg+fp->fouDegree] - DTs->data[deg+fp->fouDegree] * wc->data[deg+fp->fouDegree];
	}
	
	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
	gsl_vector_free(DTs);
	gsl_vector_free(DTc);
	gsl_vector_free(wTs);
	gsl_vector_free(wTc);
	
	const double SS     = Nss   - NMss   - Pssss   - Pcscs;
	const double CC     = Ncc   - NMcc   - Pcccc   - Pscsc;
	const double SC     = Nsc   - NMcs   - Pcccs   - Pscss;
	const double SYSS   = Nys   - NMys   - Psyss   - Pcycs;
	const double CYCC   = Nyc   - NMyc   - Psysc   - Pcycc;
	const double SS_A   = Nss_A - NMss_A - Pssss_A - Pcscs_A;
	const double CC_A   = Ncc_A - NMcc_A - Pcccc_A - Pscsc_A;
	const double SC_A   = Nsc_A - NMcs_A - Pcccs_A - Pscss_A;
	const double SYSS_A = Nys_A - NMys_A - Psyss_A - Pcycs_A;
	const double CYCC_A = Nyc_A - NMyc_A - Psysc_A - Pcycc_A;
	const double SS_P   = Nss_P - NMss_P - Pssss_P - Pcscs_P;
	const double CC_P   = Ncc_P - NMcc_P - Pcccc_P - Pscsc_P;
	const double SC_P   = Nsc_P - NMcs_P - Pcccs_P - Pscss_P;
	const double SYSS_P = Nys_P - NMys_P - Psyss_P - Pcycs_P;
	const double CYCC_P = Nyc_P - NMyc_P - Psysc_P - Pcycc_P;
	
	
	//const double on = SYSS*CC - CYCC*SC;
	//const double od = CYCC*SS - SYSS*CS;
	//const double den =  CC*SS -   SC*CS;
	//const double as = on / den;
	//const double ac = od / den;
	
	//const double val = fp->yy - Psyys - Pcyyc - NMyy - 2*as*SSYS - 2*ac*CCYC + as*as*SS + ac*ac*CC + 2*as*ac*SC;
	const double nom = CC*SYSS*SYSS  - 2*CYCC*SC*SYSS + CYCC*CYCC*SS;
	const double den = CC*SS - SC*SC;
	const double val = fp->Pyy - nom / den;
	const double nom_A = CC_A*SYSS*SYSS + 2*CC*SYSS_A*SYSS  - 2*CYCC_A*SC*SYSS - 2*CYCC*SC_A*SYSS - 2*CYCC*SC*SYSS_A + 2*CYCC_A*CYCC*SS + CYCC*CYCC*SS_A;
	const double den_A = CC_A*SS + CC*SS_A - 2*SC_A*SC;
	const double val_A = -(nom_A*den-nom*den_A) / (den*den);
	const double nom_P = CC_P*SYSS*SYSS + 2*CC*SYSS_P*SYSS  - 2*CYCC_P*SC*SYSS - 2*CYCC*SC_P*SYSS - 2*CYCC*SC*SYSS_P + 2*CYCC_P*CYCC*SS + CYCC*CYCC*SS_P;
	const double den_P = CC_P*SS + CC*SS_P - 2*SC_P*SC;
	const double val_P = -(nom_P*den-nom*den_P) / (den*den);
	*f = val;
	gsl_vector_set(df, 0, val_P);
	gsl_vector_set(df, 1, val_A);
	//cout << "FDF\t" << phi << "\t" << alpha << "\t" << val << "\t" << val_P << "\t" << val_A << endl;
}

void FouPack::setValues(double phi, double alpha, double* amp, double* theta, double *trendIntegral, QVector< double >* dTrend, QVector< double >* dFit)
{

	gsl_vector *wc = gsl_vector_calloc(2*fouDegree+1); // set to 0
	gsl_vector *ws = gsl_vector_calloc(2*fouDegree+1); // set to 0
	double Nys = 0, Nyc = 0, Nss = 0, Ncc = 0, Nsc = 0;
	for (int tNr = 0; tNr < t->size(); tNr++)
	{
		const double  eat = exp(alpha * t->at(tNr));
		const double seat = sin(phi   * t->at(tNr)) * eat;
		const double ceat = cos(phi   * t->at(tNr)) * eat;
		Nys += y->at(tNr) * seat;
		Nyc += y->at(tNr) * ceat;
		Nss += seat * seat;
		Nsc += seat * ceat;
		Ncc += ceat * ceat;
		ws->data[0] += seat;
		wc->data[0] += ceat;
		for (int deg = 1; deg <= fouDegree; deg++)
		{
			ws->data[deg]           += s[deg][tNr]*seat;
			ws->data[deg+fouDegree] += c[deg][tNr]*seat;
			wc->data[deg]           += s[deg][tNr]*ceat;
			wc->data[deg+fouDegree] += c[deg][tNr]*ceat;
		}
	}
	gsl_vector *Ds = gsl_vector_alloc(2*fouDegree+1);
	gsl_vector *Dc = gsl_vector_alloc(2*fouDegree+1);
	
	gsl_linalg_LU_solve(mat_LU, per, ws, Ds);
	gsl_linalg_LU_solve(mat_LU, per, wc, Dc);
	
	const double NMys = Dy->data[0]*ws->data[0];
	const double NMyc = Dy->data[0]*wc->data[0];
	const double NMss = Ds->data[0]*ws->data[0];
	const double NMcs = Dc->data[0]*ws->data[0];
	const double NMcc = Dc->data[0]*wc->data[0];
	
	double Psyss = 0, Psysc = 0, Pcycs = 0, Pcycc = 0;
	double Pssss = 0, /*Psssc = 0, */Pcscs = 0/*, Pcscc = 0*/;
	double Pscss = 0, Pscsc = 0, Pcccs = 0, Pcccc = 0;
	for (int deg = 1; deg <= fouDegree; deg++)
	{
		Psyss += Dy->data[deg]           * ws->data[deg];
		Psysc += Dy->data[deg]           * wc->data[deg];
		Pcycs += Dy->data[deg+fouDegree] * ws->data[deg+fouDegree];
		Pcycc += Dy->data[deg+fouDegree] * wc->data[deg+fouDegree];
		Pssss += Ds->data[deg]           * ws->data[deg];
		//Psssc += Ds->data[deg]           * wc->data[deg];
		Pcscs += Ds->data[deg+fouDegree] * ws->data[deg+fouDegree];
		//Pcscc += Ds->data[deg+fouDegree] * wc->data[deg+fouDegree];
		Pscss += Dc->data[deg]           * ws->data[deg];
		Pscsc += Dc->data[deg]           * wc->data[deg];
		Pcccs += Dc->data[deg+fouDegree] * ws->data[deg+fouDegree];
		Pcccc += Dc->data[deg+fouDegree] * wc->data[deg+fouDegree];
	}
	
	
	const double SS   = Nss - NMss - Pssss - Pcscs;
	const double CC   = Ncc - NMcc - Pcccc - Pscsc;
	const double SC   = Nsc - NMcs - Pcccs - Pscss; 
	//const double CS   = Nsc - NMsc - Psssc - Pcscc; // (S eingesetzt, mit C multipliziert) = SC
	const double SYSS = Nys - NMys - Psyss - Pcycs;
	//const double SSYS = Nys - NMsy - Pssys - Pcsyc; // =SYSS
	const double CYCC = Nyc - NMyc - Psysc - Pcycc;
	//const double CCYC = Nyc - NMcy - Pscys - Pccyc; // =CYCC
		
	const double on = SYSS*CC - CYCC*SC;
	//const double od = CYCC*SS - SYSS*CS;
	const double od = CYCC*SS - SYSS*SC;
	//const double den =  CC*SS -   SC*CS;
	const double den =  CC*SS -   SC*SC;
	const double as = on / den;
	const double ac = od / den;
	const double omega = on / od;
	vector<double> fouS(fouDegree+1);
	vector<double> fouC(fouDegree+1);
	double M = Dy->data[0]-as*Ds->data[0]-ac*Dc->data[0];
	for (int deg = 1; deg <= fouDegree; deg++) 
	{
		fouS[deg] = Dy->data[deg]           - as*Ds->data[deg]           - ac*Dc->data[deg];
		fouC[deg] = Dy->data[deg+fouDegree] - as*Ds->data[deg+fouDegree] - ac*Dc->data[deg+fouDegree];
	}
	*theta = atan(omega);
	*amp = ac / cos(*theta);
	*dTrend = QVector<double>(t->size(), M); // set to M
	dFit->resize(t->size());
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		for (int deg = 1; deg <= fouDegree; deg++) 
		{
			dTrend->data()[tNr] += fouS[deg]*s[deg][tNr] + fouC[deg]*c[deg][tNr];
		}
		dFit->data()[tNr] = *amp * exp(alpha * t->at(tNr)) * cos(phi * t->at(tNr) - *theta);
	}
	*trendIntegral = M*(t->last()-t->first());
	for (int deg = 1; deg <= fouDegree; deg++) 
	{
		*trendIntegral += fouS[deg]*c[deg][t->size()-1] - fouC[deg]*s[deg][t->size()-1] - fouS[deg]*c[deg][0] + fouC[deg]*s[deg][0];
	}
	*trendIntegral = e_integral(*amp, alpha) / *trendIntegral;

	gsl_vector_free(Ds);
	gsl_vector_free(Dc);
	gsl_vector_free(ws);
	gsl_vector_free(wc);
}



double FouPack::setBefTrend(QVector<double> *dTrend)
{
	*dTrend = QVector<double>(t->size(), Dy->data[0]); // set to zero
	for (int tNr = 0; tNr < t->size(); tNr++) 
	{
		for (int deg = 1; deg <= fouDegree; deg++) 
		{
			dTrend->data()[tNr] += Dy->data[deg]*s[deg][tNr] + Dy->data[deg+fouDegree]*c[deg][tNr];
		} 
	}
	double result = Dy->data[0]*(t->last()-t->first());
	for (int deg = 1; deg <= fouDegree; deg++) 
	{
		result += Dy->data[deg]*c[deg][t->size()-1] - Dy->data[deg+fouDegree]*s[deg][t->size()-1] - Dy->data[deg]*c[deg][0] + Dy->data[deg+fouDegree]*s[deg][0];
	}
	return result;
}

