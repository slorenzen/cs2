#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#include <QtGui/QApplication>
#include <qfile.h>
#pragma GCC diagnostic pop

#include "cs2.h"
#include <iostream>
#include <fstream>

#include "lumifile.h"

void example();
void example()
{
	ofstream trendM("trendMean.out");
	ofstream detrendedM("detrendedMean.out");
	ofstream fitM("fitMean.out");
	LumiFile lf("/home/stephan/tex/poster/001_stuttgart/bsp3.txt");
	trendM << "t";
	fitM << "t";
	detrendedM << "t";
	TimeSeries *ts0 = lf.series(0);
	for (int i = 0; i < ts0->nTrend(); i++) {trendM << "\t" << ts0->tTrendDays(i); fitM << "\t" << ts0->tTrendDays(i); detrendedM << "\t" << ts0->tTrendDays(i);}
	trendM << endl;
	detrendedM << endl;
	fitM << endl;
	for (int i = 1; i < lf.nSeries(); i++)
	{
		TimeSeries *ts = lf.series(i);
		detrendedM << i;
		trendM << i;
		fitM << i;
		for (int k = 0; k < ts->nTrend(); k++) 
		{
			trendM << "\t" << ts->dTrend(k);
			detrendedM << "\t" << ts->dDetrended(k);
			fitM << "\t" << ts->dFit(k);
		}
		trendM << endl;
		detrendedM << endl;
		fitM << endl;
	}
	trendM.close();
	detrendedM.close();
	fitM.close();
	ofstream trend("trend.out");
	ofstream detrended("detrended.out");
	ofstream fit("fit.out");
	trend << "t";
	fit << "t";
	detrended << "t";
	for (int i = 0; i < lf.nSeries(); i++) lf.series(i)->setData(TimeSeries::TRENDMETHOD, TimeSeries::POLY);
	for (int i = 0; i < ts0->nTrend(); i++) {trend << "\t" << ts0->tTrendDays(i); fit << "\t" << ts0->tTrendDays(i); detrended << "\t" << ts0->tTrendDays(i);}
	trend << endl;
	detrended << endl;
	fit << endl;
	for (int deg = 1; deg < 10; deg++) 
	{
		for (int i = 0; i < lf.nSeries(); i++) lf.series(i)->setData(TimeSeries::POLYDEG, deg);
		for (int i = 1; i < lf.nSeries(); i++)
		{
			TimeSeries *ts = lf.series(i);
			detrended << i;
			trend << i;
			fit << i;
			for (int k = 0; k < ts->nTrend(); k++) 
			{
				trend << "\t" << ts->dTrend(k);
				detrended << "\t" << ts->dDetrended(k);
				fit << "\t" << ts->dFit(k);
			}
			trend << endl;
			detrended << endl;
			fit << endl;
		}
	}
	for (int i = 0; i < lf.nSeries(); i++) lf.series(i)->setData(TimeSeries::TRENDMETHOD, TimeSeries::FOU);
	for (int deg = 1; deg < 10; deg++) 
	{
		for (int i = 0; i < lf.nSeries(); i++) lf.series(i)->setData(TimeSeries::FOUDEG, deg);
		for (int i = 1; i < lf.nSeries(); i++)
		{
			TimeSeries *ts = lf.series(i);
			detrended << i;
			trend << i;
			fit << i;
			for (int k = 0; k < ts->nTrend(); k++) 
			{
				trend << "\t" << ts->dTrend(k);
				detrended << "\t" << ts->dDetrended(k);
				fit << "\t" << ts->dFit(k);
			}
			trend << endl;
			detrended << endl;
			fit << endl;
		}
	}
	detrended.close();
	trend.close();
	fit.close();
	exit(0);
}

int main(int argc, char **argv)
{
	
	//example();;
	QApplication app(argc, argv);
	cs2 *foo = new cs2;
	foo->show();
	app.exec();
	delete foo;
	//LumiFile *lf = new LumiFile("/home/slorenz/work/bert/110411MaMTC_data_P#10.txt");
	//if (lf->hasErrors()) cerr << "ERRORS" << endl; else cerr << "Ales klar" << endl;
}
