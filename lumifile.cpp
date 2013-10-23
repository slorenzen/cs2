#include "lumifile.h"

#include <fstream>
#include <ctime>
#include <sstream>
#include <vector>
#include <iostream>

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#include <QDateTime>
#include <QFileInfo>
#pragma GCC diagnostic pop

#include <stdint.h>
#include "lumierror.h"
#include "timeseries.h"

LumiFile::LumiFile(const char* FileName) :
	_fileName(QFileInfo(FileName).fileName()),
	_series(), 
	_error(),
	_isCSFile(false)
{
	ifstream in(FileName);
	if (!in) 
	{
		_error.append(new LumiError(QString("Could not open file %1").arg(fileName()))); 
		return;
	}
	switch (in.peek())
	{
		case ('\0') : {readLumicycle (FileName); break;}
		case (236)  : {readChronostar(FileName); break;}
		default     : {readTextFile  (in);}
	}
	in.close();
// 	if (nTimes() == 0) throw ZeroTimesInFile();
// 	if (nSeries() == 0) throw ZeroSeriesInFile();
// 	_td->makeTime24Data();
// 	double endTime = ceil(maxTime()*24.0)/24.0;
// 	for (unsigned int i = 0; i < nSeries(); i++) 
// 	{
// 		TimeSeries *ts = series(i);
// 		if (ts->stopMean()  < 0) ts->_stopMean  = endTime;
// 		if (ts->stopTrend() < 0) ts->_stopTrend = endTime;
// 		if (ts->stopFit()   < 0) ts->_stopFit   = endTime;
// 		ts->setLog(defaultLog, true); // smooth, fit and calcMean are called by this function
// 	}
}


void LumiFile::readLumicycle(const char *name)
{
	ifstream file(name, ios::binary|ios::in);
	if (!file) 
	{
		_error.append(new LumiError(QString("Could not open LumiCycle file %1").arg(fileName()))); 
		return;
	}
	file.seekg(0, std::ios::end);
	streampos size = file.tellg();
	file.seekg(0);
	char *content = new char[size];
	file.read (content, size);
	int offset = static_cast<int>(content[3]) + 4;
	QVector<double> t, raw;
	for (int i = 0; i < size/offset; i++)
	{
		long int x1 
		=                static_cast<unsigned char>(content[i*offset+19]) 
		+ 256l*          static_cast<unsigned char>(content[i*offset+18]) 
		+ 256l*256l*     static_cast<unsigned char>(content[i*offset+17]) 
		+ 256l*256l*256l*static_cast<unsigned char>(content[i*offset+16]);
		long int x2 
		=                static_cast<unsigned char>(content[i*offset+23]) 
		+ 256l*          static_cast<unsigned char>(content[i*offset+22]) 
		+ 256l*256l*     static_cast<unsigned char>(content[i*offset+21]) 
		+ 256l*256l*256l*static_cast<unsigned char>(content[i*offset+20]);
		double x3 = 1000.0*static_cast<double>(x1)/static_cast<double>(x2);
		raw.push_back(x3);
		int min   =   10 * (static_cast<int>(*(content+i*offset+54))-48)
		           +   1 * (static_cast<int>(*(content+i*offset+55))-48);
		int hour =    10 * (static_cast<int>(*(content+i*offset+51))-48)
		           +   1 * (static_cast<int>(*(content+i*offset+52))-48);
		int day =     10 * (static_cast<int>(*(content+i*offset+43))-48)
		           +   1 * (static_cast<int>(*(content+i*offset+44))-48);
		int month  =  10 * (static_cast<int>(*(content+i*offset+40))-48)
		           +   1 * (static_cast<int>(*(content+i*offset+41))-48);
				  // -   1; // MONTHS START AT 0!
		int year =  1000 * (static_cast<int>(*(content+i*offset+46))-48)
		           + 100 * (static_cast<int>(*(content+i*offset+47))-48)
		           +  10 * (static_cast<int>(*(content+i*offset+48))-48)
		           +   1 * (static_cast<int>(*(content+i*offset+49))-48);
				 //  -1900; // YEARS SINCE 1900!
		if (hour == 0 && min == 0) hour = 24; // CORRECT MIDNIGHT ERROR
		QDateTime dt(QDate(year, month, day), QTime(hour, min));
		t.push_back(static_cast<double>(dt.toMSecsSinceEpoch()) / 1000 / 60 / 60 / 24);
	}
	delete [] content;
	for (QVector<double>::iterator it = t.end()-1; it >= t.begin(); it--) *it -= t[0];
	_series.push_back(new TimeSeries(t, raw, fileName(), QString("%1").arg(nSeries()+1)));
	file.close();
}


void LumiFile::readTextFile(istream &file)
{
	string line, buf;
	vector<vector<double> > raw;
	vector<QString> sampleName, comment;
	while (getline(file, line, '\n'))
	{
		if (line.length()<2) continue;
		if (line.compare(0, 1, "#")==0) 
		{
			if (line.compare(0, 7, "#SAMPLE")==0)
			{
				stringstream ss(line);
				getline(ss, buf, '\t'); // SAMPLE
				getline(ss, buf, '\t');
				const unsigned int nr = atoi(buf.c_str()); 
				getline(ss, buf, '\t');
				if (sampleName.size()<nr+1) sampleName.resize(nr+1);
				sampleName[nr] = buf.c_str();
			}
			if (line.compare(0, 8, "#COMMENT")==0)
			{
				stringstream ss(line);
				getline(ss, buf, '\t'); // COMMENT
				getline(ss, buf, '\t');
				const unsigned int nr = atoi(buf.c_str()); 
				getline(ss, buf, '\t');
				if (comment.size()<nr+1) comment.resize(nr+1);
				comment[nr] = buf.c_str();
			}
			continue;
		}
		if (raw.size() == 0 && (line[0]=='t' || line[0]=='T'))
		{
			istringstream ss(line);
			string s;
			//while (ss >> s) sampleName.push_back(QString(s.c_str()));
			while (getline(ss, s, '\t')) sampleName.push_back(QString(s.c_str()));
			continue;
		}
		istringstream ss(line);
		vector<double> rawData;
		double d;
		while (ss >> d) rawData.push_back(d);
		raw.push_back(rawData);
	}
	if (raw.size() == 0)
	{
		_error.append(new LumiError("No data in file"));
		return;
	}
	size_t nTimes = raw[0].size();
	for (unsigned int row = 0; row < raw.size(); row++) 
	{
		if (raw[row].size() != nTimes) {_error.append(new LumiError(QString("Wrong data count in line %1").arg(row+1))); return;}
	}
	QVector<double> t;
	for (unsigned int row = 0; row < raw.size(); row++) t.push_back(raw[row][0]);
	sampleName.resize(nTimes);
	comment.resize(nTimes);
	for (unsigned int i = 0; i < nTimes; i++) if (sampleName[i]=="") sampleName[i] = QString("%1").arg(i);
	if (sampleName.size() > 0)
	{
		if (sampleName.size() != nTimes) {_error.append(new LumiError(QString("Header row count %1 does not match data count %2").arg(sampleName.size()).arg(nTimes))); return;}
		for (unsigned int col = 1; col < raw[0].size(); col++)
		{
			QVector<double> data;
			for (unsigned int row = 0; row < raw.size(); row++) data.push_back(raw[row][col]);
			_series.push_back(new TimeSeries(t, data, fileName(), sampleName[col], comment[col]));
		}
		return;
	}
	for (unsigned int col = 1; col < raw[0].size(); col++)
	{
		QVector<double> data;
		for (unsigned int row = 0; row < raw.size(); row++) data.push_back(raw[row][col]);
		_series.push_back(new TimeSeries(t, data, fileName(), QString("%1").arg(nSeries()+1)));
	}
}

void LumiFile::readChronostar(const char* name)
{
	_isCSFile = true;
	QFile file(name);
	if (!file.open(QIODevice::ReadOnly))
	{
		_error.append(new LumiError(QString("Could not open LumiCycle file %1").arg(fileName()))); 
		return;
	}
	QDataStream in(&file);
	uint8_t control;
	int magic, version;
	in >> control >> magic >> version;
	if (magic != 04112010)
	{
		_error.append(new LumiError(QString("%1 is not a ChronoStar file").arg(fileName()))); 
		return;
	}
	if (version > TimeSeries::FILEVERSION) 
	{
		_error.append(new LumiError(QString("%1 is a version %2 file, but this is version %3. It's time for an upgrade, my friend...").arg(fileName()).arg(version).arg(TimeSeries::FILEVERSION))); 
		return;
	}
	while (!in.atEnd()) _series.push_back(new TimeSeries(in, version));
	file.close();
}
