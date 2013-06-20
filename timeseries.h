#ifndef TIMESERIES_H
#define TIMESERIES_H

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QVector>
#include <QSettings>
#include <qwt/qwt_plot_curve.h>
#pragma GCC diagnostic pop

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix_double.h>
#include <gsl/gsl_permutation.h>
#include <cmath>
#include <initializer_list>

using namespace std;

class TimeSeries
{

	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  TYPES
	//
	//
	public:
	enum Type       {T_STRING, T_DOUBLE, T_INT, T_BOOL, T_TRENDMETHOD, T_TRENDSTEPS, T_ABSREL};
	enum TrendMethod{NONE, MEAN, POLY, FOU};
	enum TrendSteps {ONESTEP, TWOSTEPS};
	enum AbsRel     {ABS, REL};
	enum Data
	{
		FILENAME, SAMPLE, COMMENT,                                                                            // ANNOTATION
		SMOOTH, TRENDMETHOD, TRENDSTEPS, FITLOG, TRENDFROM, TRENDTO, MEANWIDTH, POLYDEG, FOUDEG, FOUPER, ABSREL, AMPREF, // INPUT
		AMP, PER_H, PHA_H, DAMP, INTEGRAL, FVU, PVALUE, CC, ERROR                                             // OUTPUT
	};
	static Type    type(Data d);
	static QString name(Data data);
	static QString name(TrendMethod method);
	static QString name(TrendSteps  steps);
	static QString name(AbsRel      ar);
	static QList<Data> TableOffer, TextFileFormat, TableEditable;
	static const int FILEVERSION;
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  INPUT AND OUTPUT
	//
	//
	friend QDataStream &operator >> (QDataStream &ds, TrendMethod & m);
	friend QDataStream &operator >> (QDataStream &ds, TrendSteps  & m);
	friend QDataStream &operator >> (QDataStream &ds, AbsRel      & m);
	friend QDataStream &operator << (QDataStream &ds, const TimeSeries &ts);
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  CONSTRUCTOR, DESTRUCTOR ETC.
	//
	//
	public:
	TimeSeries(const QVector<double> &T, const QVector<double> &Raw, const QString &FileName, const QString &Sample, const QString &comment="");
	TimeSeries(const TimeSeries &ts);
	TimeSeries(QDataStream &ds, int version);
	TimeSeries &operator=(const TimeSeries &ts); // do not implement
	virtual ~TimeSeries();
	static void readSettings (const QSettings &settings);
	static void writeSettings (QSettings &settings);
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  ELEMENT ACCESS
	//
	//
	public:
	QVariant data(Data d)  const;
	bool fitLog()          const {return _fitLog;}
	int nRaw()             const {return _tRawDays.size();}
	int nSmooth()          const {return _tSmoothDays.size();}
	int nTrend()           const {return _tTrendDays.size();}
	int polyDegree()       const {return _polyDegree;}
	int fouDegree()        const {return _fouDegree;}
	int skipSmoothBegin()  const {return _skipSmoothBegin;}
	int skipSmoothEnd()    const {return _skipSmoothEnd;}
	int skipTrendBegin()   const {return _skipTrendBegin;}
	int skipTrendEnd()     const {return _skipTrendEnd;}
	int smoothWidth()      const {return _smooth;}
	int firstHour()        const {return static_cast<int>(floor(24.0*_tRawDays.first()));}
	int lastHour()         const {return static_cast<int>(ceil (24.0*_tRawDays.last()));}
	double trendFrom_h()   const {return _trendFrom_h;}
	double trendTo_h()     const {return _trendTo_h;}
	double windowWidth_h() const {return _windowWidth_h;}
	double fouShortPer_h() const {return _fouShortPer_h;}
	double phi()           const {return _phi;}
	double alpha()         const {return _alpha;}
	double amp()           const {return _amp;}
	double theta()         const {return _theta;}
	double fitError()      const {return _fitError;}
	double fvu()           const {return _fvu;}
	double integral()      const {return _integral;}
	double pValue()        const {return _pValue;}
	double cc()            const {return _cc;}
	double divScale()      const {return _divScale;}
	double ampRef()        const {return _ampRef;}
	double tRawDays  (int tNr) const {return _tRawDays  [tNr];}
	double tTrendDays(int tNr) const {return _tTrendDays[tNr];}
	double dRaw      (int tNr) const {return _dRaw      [tNr];}
	double dSmooth   (int tNr) const {return _dSmooth   [tNr];}
	double dTrend    (int tNr) const {return _dTrend    [tNr];}
	double dDetrended(int tNr) const {return _dDetrended[tNr];}
	double dFit      (int tNr) const {return _dFit      [tNr];}
	const QString &fileName() const {return _fileName;}
	const QString &sample()   const {return _sample;}
	const QString &comment()  const {return _comment;}
	TrendMethod trendMethod() const {return _trendMethod;}
	TrendSteps  trendSteps()  const {return _trendSteps;}
	AbsRel      absRel()      const {return _absRel;}
	const QVector<double> &tRawDays()    const {return _tRawDays;}
	const QVector<double> &tSmoothDays() const {return _tSmoothDays;}
	const QVector<double> &tTrendDays()  const {return _tTrendDays;}
	const QVector<double> &dRaw()        const {return _dRaw;}
	const QVector<double> &dSmooth()     const {return _dSmooth;}
	const QVector<double> &dTrend()      const {return _dTrend;}
	const QVector<double> &dDetrended()  const {return _dDetrended;}
	const QVector<double> &dFit()        const {return _dFit;}
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  DEFAULT VALUES AND STATIC CONSTANTS
	//
	//
	public:
	static int  giveUpTime;
	static bool defaultFitLog;
	static int  defaultSmooth;
	static int  defaultPolyDegree;
	static int  defaultFouDegree;
	static double defaultWindowWidth_h;
	static double defaultFouShortPer_h;
	static double default_alpha_start;
	static double default_phi_start;
	static TrendMethod defaultTrendMethod;
	static TrendSteps  defaultTrendSteps;
	static AbsRel      defaultAbsRel;
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  FIT AND CHANGES
	//
	//
	public:
	bool setData(Data d, const QVariant &value);
	private:
	bool cutAndSmooth();
	bool detrend();
	bool fit();
	bool setFitLog(bool sl);
	bool setTrendMethod(TrendMethod me);
	bool setTrendSteps(TrendSteps st);
	bool setSmooth     (int sm);
	bool setPolyDegree (int pd);
	bool setFouDegree  (int pd);
	bool setTrendFrom_h(int tr);
	bool setTrendTo_h  (int tr);
	bool setFouPer_h   (int pd);
	double getDetrended(); // return rmsd of nested model
	bool setMeanWidth(int w);
	bool setAbsRel(AbsRel ar);
	bool setAmpRef(int ar);

	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  SHOW AND HIDE
	//
	//
	public:
	void show (QwtPlot *plotRaw, QwtPlot *plotFit);
	void hide();
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  MEMBER DATA
	//
	//
	private:
	QString _fileName, _sample, _comment;
	QVector<double> _tRawDays, _tSmoothDays, _tTrendDays, _dRaw, _dSmooth, _dTrend, _dDetrended, _dFit;
	bool _fitLog;
	int _smooth;
	int _polyDegree, _fouDegree;
	int _skipSmoothBegin, _skipSmoothEnd, _skipTrendBegin, _skipTrendEnd;
	double _windowWidth_h;
	double _fouShortPer_h;
	double _trendFrom_h, _trendTo_h;
	double _alpha, _phi, _amp, _theta, _fitError, _fvu, _integral, _pValue, _cc;
	double _ampRef;
	double _divScale;
	TrendMethod _trendMethod;
	TrendSteps  _trendSteps;
	AbsRel      _absRel;
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	//  CURVES
	//
	//
	QwtPlotCurve *curveSmoothed, *curveTrend, *curveDetrended, *curveFit;
};

#endif // TIMESERIES_H
