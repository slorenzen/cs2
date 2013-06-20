#include "timeseries.h"
#include "gslErrors.h"
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_cdf.h>
#include <iostream>
#include <vector>
#include <limits>
#include <time.h>
#include <gsl/gsl_statistics.h>

#include "nopack.h"
#include "polypack.h"
#include "foupack.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  DEFAULT VALUES AND STATIC CONSTANTS
//
//
int  TimeSeries::giveUpTime        = 3;
bool TimeSeries::defaultFitLog     = false;
int  TimeSeries::defaultSmooth     = 0;
int  TimeSeries::defaultPolyDegree = 5;
int  TimeSeries::defaultFouDegree  = 4;
double TimeSeries::defaultWindowWidth_h = 24.0;
double TimeSeries::defaultFouShortPer_h = 48.0;
double TimeSeries::default_alpha_start  = -0.5;
double TimeSeries::default_phi_start    = 2*M_PI;
TimeSeries::TrendMethod TimeSeries::defaultTrendMethod = TimeSeries::FOU;
TimeSeries::TrendSteps  TimeSeries::defaultTrendSteps  = TimeSeries::ONESTEP;
TimeSeries::AbsRel      TimeSeries::defaultAbsRel      = TimeSeries::ABS;
const int TimeSeries::FILEVERSION = 2;

QList<TimeSeries::Data> TimeSeries::TableOffer = 
	{FILENAME, SAMPLE, COMMENT, SMOOTH, TRENDMETHOD, TRENDSTEPS, FITLOG, TRENDFROM, TRENDTO, POLYDEG, FOUDEG, ABSREL, AMP, PER_H, PHA_H, DAMP, INTEGRAL, FVU, PVALUE, CC, ERROR};
QList<TimeSeries::Data> TimeSeries::TextFileFormat = 
	{FILENAME, SAMPLE, COMMENT, SMOOTH, TRENDMETHOD, TRENDSTEPS, FITLOG, TRENDFROM, TRENDTO, POLYDEG, FOUDEG, ABSREL, AMP, PER_H, PHA_H, DAMP, INTEGRAL, FVU, PVALUE, CC, ERROR};
QList<TimeSeries::Data> TimeSeries::TableEditable = 
	{COMMENT, SMOOTH, FITLOG, TRENDFROM, TRENDTO, POLYDEG, FOUDEG};

TimeSeries::Type TimeSeries::type(Data d)
{
	switch (d)
	{
		case (TRENDMETHOD): return T_TRENDMETHOD;
		case (TRENDSTEPS) : return T_TRENDSTEPS;
		case (ABSREL)     : return T_ABSREL;
		case (FITLOG)     : return T_BOOL;
		case (SMOOTH)     : return T_INT;
		case (TRENDFROM)  : return T_INT;
		case (TRENDTO)    : return T_INT;
		case (POLYDEG)    : return T_INT;
		case (FOUDEG)     : return T_INT;
		case (FOUPER)     : return T_INT;
		case (AMP)        : return T_DOUBLE;
		case (PER_H)      : return T_DOUBLE;
		case (PHA_H)      : return T_DOUBLE;
		case (DAMP)       : return T_DOUBLE;
		case (INTEGRAL)   : return T_DOUBLE;
		case (FVU)        : return T_DOUBLE;
		case (PVALUE)     : return T_DOUBLE;
		case (CC)         : return T_DOUBLE;
		case (ERROR)      : return T_DOUBLE;
		case (AMPREF)     : return T_DOUBLE;
		case (MEANWIDTH)  : return T_DOUBLE;
		case (FILENAME)   : return T_STRING;
		case (SAMPLE)     : return T_STRING;
		case (COMMENT)    : return T_STRING;
		default           : return T_STRING;
	}
}

QString TimeSeries::name(Data data)
{
	switch (data)
	{
		case (FILENAME)    : return "File";
		case (SAMPLE)      : return "Sample";
		case (COMMENT)     : return "Comment";
		case (SMOOTH)      : return "Smooth";
		case (FITLOG)      : return "FitLog";
		case (TRENDMETHOD) : return "Method";
		case (TRENDSTEPS)  : return "Steps";
		case (ABSREL)      : return "Abs/Rel";
		case (TRENDFROM)   : return "TrendFrom";
		case (TRENDTO)     : return "TrendTo";
		case (POLYDEG)     : return "PolDeg";
		case (FOUDEG)      : return "FouDeg";
		case (FOUPER)      : return "FouPer";
		case (AMP)         : return "Amp";
		case (PER_H)       : return "Per";
		case (PHA_H)       : return "Pha";
		case (DAMP)        : return "Damp";
		case (INTEGRAL)    : return "Integral";
		case (FVU)         : return "FVU";
		case (PVALUE)      : return "p";
		case (CC)          : return "CC";
		case (ERROR)       : return "Error";
		case (AMPREF)      : return "AmpRef";
		case (MEANWIDTH)   : return "MeanWidth";
		default            : return "Unknown";
	}
	return "Unknown";
}

QString TimeSeries::name(TrendMethod m)
{
	switch (m)
	{
		case (NONE) : return "None";
		case (MEAN) : return "Average";
		case (POLY) : return "Poly";
		case (FOU)  : return "Fou";
		default     : return "Unknown";
	}
	return "Unknown";
}

QString TimeSeries::name(TrendSteps m)
{
	switch (m)
	{
		case (ONESTEP)  : return "One";
		case (TWOSTEPS) : return "Two";
		default         : return "Unknown";
	}
	return "Unknown";
}

QString TimeSeries::name(AbsRel m)
{
	switch (m)
	{
		case (ABS) : return "Abs";
		case (REL) : return "Rel";
		default    : return "Unknown";
	}
	return "Unknown";
}

QDataStream &operator >> (QDataStream &ds, TimeSeries::TrendMethod & m )
{
	int i;
	ds >> i;
	m = static_cast<TimeSeries::TrendMethod>(i);
	return ds;
}


QDataStream &operator<<(QDataStream &ds, const TimeSeries &ts)
{
	ds << ts.tRawDays();
	ds << ts.dRaw();
	ds << ts.skipSmoothBegin();
	ds << ts.skipSmoothEnd();
	ds << ts.skipTrendBegin();
	ds << ts.skipTrendEnd();
	ds << ts._tSmoothDays;
	ds << ts._dSmooth;
	ds << ts._tTrendDays;
	ds << ts._dTrend;
	ds << ts._dDetrended;
	ds << ts._dFit;
	ds << ts._fileName;
	ds << ts._sample;
	ds << ts._comment;
	ds << ts._smooth;
	ds << ts._fitLog;
	ds << ts._trendMethod;
	ds << ts._windowWidth_h;
	ds << ts._polyDegree;
	ds << ts._fouDegree;
	ds << ts._fouShortPer_h;
	ds << ts._trendFrom_h;
	ds << ts._trendTo_h;
	ds << ts._alpha;
	ds << ts._phi;
	ds << ts._amp;
	ds << ts._theta;
	ds << ts._fitError;
	ds << ts.fvu();
	ds << ts._integral;
	ds << ts._pValue;
	ds << ts._divScale;
	ds << ts.cc();
	return ds;
}



TimeSeries::TimeSeries(const QVector< double >& TRawDays, const QVector< double >& DRaw, const QString &FileName, const QString &Sample, const QString &Comment) :
	_fileName(FileName), _sample(Sample), _comment(Comment), 
	_tRawDays(TRawDays), _tSmoothDays(), _tTrendDays(), 
	_dRaw(DRaw), _dSmooth(), _dTrend(), _dDetrended(), _dFit(),
	_fitLog(defaultFitLog), _smooth(defaultSmooth), 
	_polyDegree(defaultPolyDegree), _fouDegree(defaultFouDegree),
	_skipSmoothBegin(), _skipSmoothEnd(), _skipTrendBegin(), _skipTrendEnd(), 
	_windowWidth_h(defaultWindowWidth_h), _fouShortPer_h(defaultFouShortPer_h), 
	_trendFrom_h(-1), _trendTo_h(-1), 
	_alpha(), _phi(), _amp(), _theta(), _fitError(), _fvu(), _integral(), _pValue(), _cc(), 
	_ampRef(), _divScale(), 
	_trendMethod(defaultTrendMethod), _trendSteps(defaultTrendSteps), _absRel(defaultAbsRel),
	curveSmoothed(new QwtPlotCurve()), curveTrend(new QwtPlotCurve()), curveDetrended(new QwtPlotCurve()), curveFit(new QwtPlotCurve())
{
	curveTrend->setPen(QPen(Qt::blue));
	curveFit->setPen(QPen(Qt::blue));
	cutAndSmooth();
}

TimeSeries::TimeSeries(const TimeSeries& ts) :
	_fileName(ts.fileName()), _sample(ts.sample()), _comment(ts.comment()), 
	_tRawDays(ts.tRawDays()), _tSmoothDays(ts.tSmoothDays()), _tTrendDays(ts.tTrendDays()), 
	_dRaw(ts.dRaw()), _dSmooth(ts.dSmooth()), _dTrend(ts.dTrend()), _dDetrended(ts.dDetrended()), _dFit(ts.dFit()),
	_fitLog(ts.fitLog()), _smooth(ts.smoothWidth()), 
	_polyDegree(ts.polyDegree()), _fouDegree(ts.fouDegree()),
	_skipSmoothBegin(ts.skipSmoothBegin()), _skipSmoothEnd(ts.skipSmoothEnd()), _skipTrendBegin(ts.skipTrendBegin()), _skipTrendEnd(ts.skipTrendEnd()), 
	_windowWidth_h(ts.windowWidth_h()), _fouShortPer_h(ts.fouShortPer_h()), 
	_trendFrom_h(ts.trendFrom_h()), _trendTo_h(ts.trendTo_h()), 
	_alpha(ts.alpha()), _phi(ts.phi()), _amp(ts.amp()), _theta(ts.theta()), _fitError(ts.fitError()), _fvu(ts.fvu()), _integral(ts.integral()), _pValue(ts.pValue()), _cc(ts.cc()), 
	_ampRef(), _divScale(ts.divScale()), 
	_trendMethod(ts.trendMethod()), _trendSteps(ts.trendSteps()), _absRel(ts.absRel()),
	curveSmoothed(new QwtPlotCurve()), curveTrend(new QwtPlotCurve()), curveDetrended(new QwtPlotCurve()), curveFit(new QwtPlotCurve())
{
	curveTrend->setPen(QPen(Qt::blue));
	curveFit->setPen(QPen(Qt::blue));
	curveSmoothed->setSamples(tSmoothDays(), dSmooth());
	curveTrend->setSamples(tTrendDays(), dTrend());
	curveDetrended->setSamples(tTrendDays(), dDetrended());
	curveFit->setSamples(tTrendDays(), dFit());
}

TimeSeries::TimeSeries(QDataStream& ds, int version) :
	_fileName(), _sample(), _comment(), 
	_tRawDays(), _tSmoothDays(), _tTrendDays(), 
	_dRaw(), _dSmooth(), _dTrend(), _dDetrended(), _dFit(),
	_fitLog(defaultFitLog), _smooth(defaultSmooth), 
	_polyDegree(defaultPolyDegree), _fouDegree(defaultFouDegree),
	_skipSmoothBegin(), _skipSmoothEnd(), _skipTrendBegin(), _skipTrendEnd(), 
	_windowWidth_h(defaultWindowWidth_h), _fouShortPer_h(defaultFouShortPer_h), 
	_trendFrom_h(-1), _trendTo_h(-1), 
	_alpha(), _phi(), _amp(), _theta(), _fitError(), _fvu(), _integral(), _pValue(), _cc(), 
	_ampRef(), _divScale(), 
	_trendMethod(defaultTrendMethod), _trendSteps(defaultTrendSteps), _absRel(defaultAbsRel),
	curveSmoothed(new QwtPlotCurve()), curveTrend(new QwtPlotCurve()), curveDetrended(new QwtPlotCurve()), curveFit(new QwtPlotCurve())
{
	int dummy;
	ds >> _tRawDays;
	ds >> _dRaw;
	ds >> _tSmoothDays;
	ds >> _dSmooth;
	ds >> _tTrendDays;
	ds >> _dTrend;
	ds >> _dDetrended;
	ds >> _dFit;
	ds >> _fileName;
	ds >> _sample;
	ds >> _comment;
	ds >> _smooth;
	ds >> _fitLog;
	ds >> _trendMethod;
	ds >> dummy;
	ds >> _windowWidth_h;
	ds >> _polyDegree;
	ds >> _fouDegree;
	ds >> _fouShortPer_h;
	ds >> _trendFrom_h;
	ds >> _trendTo_h;
	ds >> _alpha;
	ds >> _phi;
	ds >> _amp;
	ds >> _theta;
	ds >> _fitError;
	ds >> _fvu;
	ds >> _integral;
	ds >> _pValue;
	ds >> _divScale;
	if (version >= 2) ds >> _cc;
	curveTrend->setPen(QPen(Qt::blue));
	curveFit->setPen(QPen(Qt::blue));
	curveSmoothed->setSamples(_tSmoothDays, _dSmooth);
	curveTrend->setSamples(_tTrendDays, _dTrend);
	curveDetrended->setSamples(_tTrendDays, _dDetrended);
	curveFit->setSamples(_tTrendDays, _dFit);
}

TimeSeries::~TimeSeries()
{
	curveSmoothed->detach();
	delete curveSmoothed;
	curveTrend->detach();
	delete curveTrend;
	curveDetrended->detach();
	delete curveDetrended;
	curveFit->detach();
	delete curveFit;
}

bool TimeSeries::cutAndSmooth()
{
	_tSmoothDays = _tRawDays;
	_skipSmoothBegin = 0;
	_skipSmoothEnd = 0;
	if (trendFrom_h() >= 0) while (_tSmoothDays.first() < trendFrom_h()/24) {_tSmoothDays.pop_front(); _skipSmoothBegin++;}
	if (trendTo_h() >= 0)   while (_tSmoothDays.last()  > trendTo_h()  /24) {_tSmoothDays.pop_back();  _skipSmoothEnd++;}

	if (smoothWidth() > 0)
	{
		for (int i = 0; i < smoothWidth(); i++) {_tSmoothDays.pop_front(); _tSmoothDays.pop_back();}
		_dSmooth.clear();
		int smoothW = 2*smoothWidth()+1;
		double d = 0;
		for (int i = skipSmoothBegin(); i < smoothW-1+skipSmoothBegin(); i++) d += _dRaw[i];
		for (int i = smoothW-1+skipSmoothBegin(); i < _dRaw.size()-skipSmoothEnd(); i++)
		{
			d += _dRaw[i];
			_dSmooth.push_back(d/smoothW);
			d -= _dRaw[i-smoothW+1];
		}
		_skipSmoothBegin += smoothWidth();
		_skipSmoothEnd += smoothWidth();
	}
	else 
	{
		_dSmooth = _dRaw;
		for (int i = 0; i < skipSmoothBegin(); i++) _dSmooth.pop_front();
		for (int i = 0; i < skipSmoothEnd();   i++) _dSmooth.pop_back();
	}
	curveSmoothed->setSamples(tSmoothDays(), dSmooth());
	if (tSmoothDays().size() < 10) return false;
	return detrend();
}


bool TimeSeries::detrend()
{
	if (fitLog())
	{
		for (QVector<double>::iterator it = _dSmooth.begin(); it < _dSmooth.end(); it++) *it = log(*it);
	}
	_tTrendDays.clear();
	_dTrend.clear();
	_dDetrended.clear();
	_skipTrendBegin = skipSmoothBegin();
	_skipTrendEnd = skipSmoothEnd();
	switch (trendMethod())
	{
		case (NONE) :
		{
			_tTrendDays = _tSmoothDays;
			_dTrend = _dSmooth;
			_dDetrended = _dSmooth;
			_integral = 0;
			for (int tNr = 0; tNr < nTrend(); tNr++) _integral += dTrend(tNr);
			break;
		}
		case (MEAN) :
		{
			const int halfPeriodWidth = static_cast<int>(round((nSmooth()-1) / (_tSmoothDays.last()-_tSmoothDays.first()) * 12.0/windowWidth_h()));
			_skipTrendBegin += halfPeriodWidth;
			_skipTrendEnd   += halfPeriodWidth;
			double sum = 0;
			if (dSmooth().size() < 2*halfPeriodWidth) return false;
			for (int tNr = 0; tNr < 2*halfPeriodWidth; tNr++) sum += _dSmooth[tNr];
			_integral = 0;
			for (int tNr = halfPeriodWidth; tNr < nSmooth()-halfPeriodWidth; tNr++)
			{
				sum += _dSmooth[tNr+halfPeriodWidth];
				_tTrendDays.append(_tSmoothDays[tNr]);
				const double dT = sum/(2*halfPeriodWidth+1);
				_dTrend.append(dT);
				_integral += dT;
				_dDetrended.append(_dSmooth[tNr]-dT);
				sum -= _dSmooth[tNr-halfPeriodWidth];
			}
			if (tTrendDays().size() < 10) return false;
			break;
		}
		case (POLY) :
		{
			if (trendSteps() == ONESTEP) break;
			_integral = PolyPack(&tSmoothDays(), &dSmooth(), polyDegree()).setBefTrend(&_dTrend);
			getDetrended(); // discard RMSD
			break;
		}
		case (FOU) :
		{
			if (trendSteps() == ONESTEP) break;
			_integral = FouPack(&tSmoothDays(), &dSmooth(), fouDegree(), fouShortPer_h()).setBefTrend(&_dTrend);
			getDetrended(); // discard RMSD
			break;
		}
		default : return false;
	}
	return fit();
}

double TimeSeries::getDetrended()
{
	_tTrendDays = _tSmoothDays;
	_dDetrended.resize(nTrend());
	double result = 0;
	for (int tNr = 0; tNr < nTrend(); tNr++) 
	{
		_dDetrended[tNr] = dSmooth(tNr)-dTrend(tNr);
		result += dDetrended(tNr)*dDetrended(tNr);
	}
	return result;
}




// bool TimeSeries::testFouBefore()
// {
// 	const int FOUSTEPS = 100;
// 	const double FOUMIN_D = 1.0;
// 	const double FOUMAX_D = 3.0;
// 	_dFit.clear();
// 	size_t iter = 0;
// 	int status;
// 	const gsl_multimin_fdfminimizer_type *T = gsl_multimin_fdfminimizer_conjugate_fr;
// 	gsl_multimin_fdfminimizer *m = gsl_multimin_fdfminimizer_alloc (T, 2);
// 	gsl_vector *x= gsl_vector_alloc (2);
// 	gsl_vector_set (x, 0, default_phi_start); // period 24 h
// 	gsl_vector_set (x, 1, default_alpha_start);
// 	for (int fouStep = 0; fouStep <= FOUSTEPS; fouStep++)
// 	{
// 		_fouShortestPeriod_h = 24.0*(FOUMIN_D + (FOUMAX_D-FOUMIN_D)*fouStep/FOUSTEPS);
// 		MinPack *pack = new FouPack (&tSmoothDays(), &dSmooth(), fouDegree(), fouShortPer_h());
// 		gsl_multimin_fdfminimizer_set (m, &pack->func, x, 0.01, 1e-6);
// 		do
// 		{
// 			iter++;
// 			status = gsl_multimin_fdfminimizer_iterate (m);
// 			if (status) break;
// 			status = gsl_multimin_test_gradient (m->gradient, 1e-6);
// 		}
// 		while (status == GSL_CONTINUE && iter < 1000);
// 		cout << "\t" << m->f;
// 		delete pack;
// 	}
// 	cout << endl;
// 	gsl_multimin_fdfminimizer_free (m);
// 	gsl_vector_free (x);
// 	return true;
// }


bool TimeSeries::fit()
{
	_dFit.clear();
	size_t iter = 0;
	int status;
	const gsl_multimin_fdfminimizer_type *T = gsl_multimin_fdfminimizer_vector_bfgs2;
	gsl_multimin_fdfminimizer *m = gsl_multimin_fdfminimizer_alloc (T, 2);
	gsl_vector *x= gsl_vector_alloc (2);
	gsl_vector_set (x, 0, default_phi_start);
	gsl_vector_set (x, 1, default_alpha_start); 
	MinPack *pack = NULL;
	switch (trendSteps())
	{
		case (TWOSTEPS) : {pack = new NoPack  (&tTrendDays(),  &dDetrended()); break;}
		case (ONESTEP) :
		switch (trendMethod())
		{
			case (NONE) : {pack = new NoPack  (&tTrendDays(),  &dDetrended()); break;}
			case (MEAN) : {pack = new NoPack  (&tTrendDays(),  &dDetrended()); break;}
			case (POLY) : {pack = new PolyPack(&tSmoothDays(), &dSmooth(), polyDegree()); break;}
			case (FOU)  : {pack = new FouPack (&tSmoothDays(), &dSmooth(), fouDegree(), fouShortPer_h()); break;}
			default: break;
		}
		default: break;
	}
	gsl_multimin_fdfminimizer_set (m, &pack->func, x, 0.01/*step size*/, 0.1/*tol*/);
	time_t t0 = time(0);
	time_t t1 = t0;
	do
	{
		iter++;
		status = gsl_multimin_fdfminimizer_iterate (m);
		if (status) break;
		status = gsl_multimin_test_gradient (m->gradient, 1);
		t1 = time(0);
	}
	while (status == GSL_CONTINUE && (t1-t0) < giveUpTime && iter < 100);

	_phi      = gsl_vector_get(gsl_multimin_fdfminimizer_x(m), 0);
	_alpha    = gsl_vector_get(gsl_multimin_fdfminimizer_x(m), 1);
	_fitError = gsl_multimin_fdfminimizer_minimum(m);
	gsl_multimin_fdfminimizer_free (m);
	gsl_vector_free (x);
	pack->setValues(phi(), alpha(), &_amp, &_theta, &_integral, &_dTrend, &_dFit); // integral must have been set before for none, mean and bef!!
	if (amp() < 0) {_amp = -amp(); _theta += M_PI;}
	if (theta() < 0) _theta += 2*M_PI;
	_fvu = fitError() / pack->ssTot();
	_pValue = numeric_limits<double>::quiet_NaN();
	switch (trendMethod())
	{
		case (NONE) :
		case (MEAN) :
		{
			double eSum = 0;
			double tSum = 0;
			for (int tNr = 0; tNr < nTrend(); tNr++) {eSum += amp()*exp(alpha()*tTrendDays(tNr)); tSum += dTrend(tNr);}
			_integral = eSum/tSum;
			break;
		}
		case (POLY) : 
		case (FOU)  : 
		{
			if (trendSteps() == TWOSTEPS) break;
			const int MOREPARAM = 4; //additional parameters: amp, per, pha, damp
			const int FULLPARAM = (trendMethod()==POLY?(polyDegree()+1):(fouDegree()*2+1)) + MOREPARAM;
			const double nestedError = getDetrended();
			const double F = nestedError / fitError() - 1.0;
			_pValue = gsl_cdf_fdist_Q(F, MOREPARAM, nTrend()-FULLPARAM-1);
			break;
		}
		default: break;
	}
	if (fitLog())
	{
		for (QVector<double>::iterator it = _dSmooth.begin();    it < _dSmooth.end();    it++) *it = exp(*it);
		for (QVector<double>::iterator it = _dTrend.begin();     it < _dTrend.end();     it++) *it = exp(*it);
		for (QVector<double>::iterator it = _dDetrended.begin(); it < _dDetrended.end(); it++) *it = exp(*it);
		for (QVector<double>::iterator it = _dFit.begin();       it < _dFit.end();       it++) *it = exp(*it);
	}
	_divScale = (tTrendDays().last()-tTrendDays().first()) * integral() / pack->e_integral(amp(), alpha());
	if (!fitLog() && absRel()==REL)  // no scaling if logarithmized
	{
		for (int tNr = 0; tNr < nTrend(); tNr++) 
		{
			_dFit[tNr]       = 1 + dFit(tNr)      *_divScale;
			_dDetrended[tNr] = 1 + dDetrended(tNr)*_divScale;
		}
	}
	delete pack;
	_cc = gsl_stats_correlation (dDetrended().data(), 1, dFit().data(), 1, dFit().size());
	curveFit->setSamples(_tTrendDays, _dFit);
	curveTrend->setSamples(_tTrendDays, _dTrend);
	curveDetrended->setSamples(_tTrendDays, _dDetrended);
	return true;
}

// void TimeSeries::setViewDiv(bool viewDiv)
// {
// 	if (fit_as_div() == viewDiv) return;
// 	_fit_as_div = viewDiv;
// 	if (fitLog()) return;// no scaling if logarithmized
// 	if (fit_as_div()) 
// 	{
// 		for (int tNr = 0; tNr < nTrend(); tNr++) 
// 		{
// 			_dFit[tNr] = 1 + dFit(tNr)*_divScale;
// 			_dDetrended[tNr] = 1 + dDetrended(tNr)*_divScale;
// 		}
// 	}
// 	else
// 	{
// 		for (int tNr = 0; tNr < nTrend(); tNr++) 
// 		{
// 			_dFit[tNr] = (dFit(tNr)-1)/_divScale;
// 			_dDetrended[tNr] = (dDetrended(tNr)-1) / _divScale;
// 		}
// 	}
// 	curveFit->setSamples(_tTrendDays, _dFit);
// 	curveDetrended->setSamples(_tTrendDays, _dDetrended);
// }

bool TimeSeries::setData(TimeSeries::Data d, const QVariant& value)
{
#pragma GCC diagnostic ignored "-Wswitch-enum"
	switch (d)
	{
		case (COMMENT)   : {_comment = value.toString(); return true;}
		case (SMOOTH)    : return setSmooth(value.toInt());
		case (TRENDFROM) : return setTrendFrom_h(value.toInt());
		case (TRENDTO)   : return setTrendTo_h(value.toInt());
		case (TRENDMETHOD) : return setTrendMethod(static_cast<TrendMethod>(value.toInt()));
		case (TRENDSTEPS) : return setTrendSteps(static_cast<TrendSteps>(value.toInt()));
		case (MEANWIDTH) : return setMeanWidth(value.toInt());
		case (POLYDEG)   : return setPolyDegree(value.toInt());
		case (FOUDEG)    : return setFouDegree(value.toInt());
		case (FOUPER)    : return setFouPer_h(value.toInt());
		case (FITLOG)    : return setFitLog(value.toBool());
		case (ABSREL)    : return setAbsRel(static_cast<AbsRel>(value.toInt()));
		case (AMPREF)    : return setAmpRef(value.toInt());
		default: return false;
	}
#pragma GCC diagnostic pop
	return false;
}

void TimeSeries::show(QwtPlot* plotRaw, QwtPlot *plotFit)
{
	curveSmoothed->attach(plotRaw);
	curveTrend->attach(plotRaw);
	curveDetrended->attach(plotFit);
	curveFit->attach(plotFit);
}

void TimeSeries::hide()
{
	curveSmoothed->detach();
	curveTrend->detach();
	curveDetrended->detach();
	curveFit->detach();
}


QVariant TimeSeries::data(TimeSeries::Data d) const
{
	switch (d)
	{
		case (FILENAME)    : return fileName();
		case (SAMPLE)      : return sample();
		case (COMMENT)     : return comment();
		case (SMOOTH)      : return smoothWidth();
		case (FITLOG)      : return fitLog();
		case (TRENDMETHOD) : return trendMethod();
		case (TRENDSTEPS)  : return trendSteps();
		case (ABSREL)      : return absRel();
		case (TRENDFROM)   : return trendFrom_h();
		case (TRENDTO)     : return trendTo_h();
		case (POLYDEG)     : return polyDegree();
		case (FOUDEG)      : return fouDegree();
		case (FOUPER)      : return fouShortPer_h();
		case (AMP)         : {double result = amp()*exp(alpha()*ampRef()); if (fitLog()||absRel()==ABS) return result; else return result*_divScale;}
		case (PER_H)       : return 24.0*2*M_PI/phi();
		case (PHA_H)       : return 24.0*theta()/(2*M_PI);
		case (DAMP)        : return alpha();
		case (INTEGRAL)    : return integral();
		case (FVU)         : return fvu();
		case (PVALUE)      : return pValue();
		case (CC)          : return cc();
		case (ERROR)       : return fitError();
		case (AMPREF)      : return ampRef();
		case (MEANWIDTH)   : return windowWidth_h();
		default : return "Invalid";
	}
	return "Invalid";
}


void TimeSeries::readSettings(const QSettings& settings)
{
	if (settings.contains("smooth")) defaultSmooth = settings.value("smooth").toInt();
	if (settings.contains("fouDeg")) defaultFouDegree = settings.value("fouDeg").toInt();
	if (settings.contains("polDeg")) defaultPolyDegree = settings.value("polDeg").toInt();
	if (settings.contains("fouPer")) defaultFouShortPer_h = settings.value("fouPer").toInt();
	if (settings.contains("method")) defaultTrendMethod = static_cast<TrendMethod>(settings.value("method").toInt());
	if (settings.contains("steps")) defaultTrendSteps = static_cast<TrendSteps>(settings.value("steps").toInt());
}

void TimeSeries::writeSettings(QSettings& settings)
{
	settings.setValue("smooth", defaultSmooth);
	settings.setValue("fouDeg", defaultFouDegree);
	settings.setValue("polDeg", defaultPolyDegree);
	settings.setValue("fouPer", defaultFouShortPer_h);
	settings.setValue("method", static_cast<int>(defaultTrendMethod));
	settings.setValue("steps", static_cast<int>(defaultTrendSteps));
}

bool TimeSeries::setFitLog(bool sl)  
{
	if (fitLog() == sl) return false; 
	_fitLog = sl; 
	return detrend();
}

bool TimeSeries::setTrendMethod(TrendMethod me)
{
	if (trendMethod() == me) return false; 
	_trendMethod = me; 
	return detrend();
}

bool TimeSeries::setTrendSteps(TrendSteps me)
{
	if (trendSteps() == me) return false; 
	_trendSteps = me;
	if (trendMethod() == NONE || trendMethod() == MEAN) return true;
	return detrend();
}

bool TimeSeries::setSmooth     (int sm) 
{
	if (smoothWidth() == sm) return false;
	_smooth = sm;
	return cutAndSmooth();
}
bool TimeSeries::setPolyDegree (int pd) 
{
	if (polyDegree()  == pd) return false;
	_polyDegree = pd; 
	if (trendMethod() != POLY) return true; 
	return detrend();
}

bool TimeSeries::setFouDegree  (int pd) 
{
	if (fouDegree()  == pd)  return false; 
	_fouDegree = pd;  
	if (trendMethod() != FOU)  return true; 
	return detrend();
}

bool TimeSeries::setTrendFrom_h(int tr) 
{
	if (fabs(trendFrom_h()        -tr) < 0.25 || (tr < firstHour() && trendFrom_h() < 0)) return false; 
	_trendFrom_h = tr; 
	return cutAndSmooth();
}
bool TimeSeries::setTrendTo_h  (int tr) 
{
	if (fabs(trendTo_h()          -tr) < 0.25 || (tr > lastHour()  && trendTo_h()   < 0)) return false; 
	_trendTo_h   = tr; 
	return cutAndSmooth();
}
bool TimeSeries::setFouPer_h   (int pd) 
{
	if (fabs(fouShortPer_h()-pd) < 0.25) return false; 
	_fouShortPer_h = pd; 
	if (trendMethod() != FOU) return true; 
	return detrend();
}
bool TimeSeries::setMeanWidth(int w)
{
	if (fabs(w-windowWidth_h()) < .25) return false;
	_windowWidth_h = w;
	if (trendMethod() != MEAN) return true;
	return detrend();
}
bool TimeSeries::setAbsRel(AbsRel ar)
{
	if (absRel() == ar) return false;
	_absRel = ar;
	if (fitLog()) return true;
	if (absRel()==REL)  // no scaling if logarithmized
	{
		for (int tNr = 0; tNr < nTrend(); tNr++) 
		{
			_dFit[tNr]       = 1 + dFit(tNr)      *_divScale;
			_dDetrended[tNr] = 1 + dDetrended(tNr)*_divScale;
		}
	}
	else
	{
		for (int tNr = 0; tNr < nTrend(); tNr++) 
		{
			_dFit[tNr]       = (dFit(tNr)-1)/      _divScale;
			_dDetrended[tNr] = (dDetrended(tNr)-1)/_divScale;
		}
	}
	curveDetrended->setSamples(tTrendDays(), dDetrended());
	curveFit->setSamples(tTrendDays(), dFit());
	return true;
}
bool TimeSeries::setAmpRef(int ar)
{
	if (fabs(ampRef()-ar) < .25) return false;
	_ampRef = ar;
	return true;
}
