#ifndef LUMIFILE_H
#define LUMIFILE_H

#include <iostream>

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#include <QString>
#include <QList>
#pragma GCC diagnostic pop

using namespace std;

class LumiError;
class TimeSeries;

class LumiFile
{

public:

	LumiFile(const char *FileName);
	const QString &fileName() const {return _fileName;}
	bool hasErrors() const {return _error.size() > 0;}
	int nSeries() const {return _series.size();}
	TimeSeries *series(int i) const {return _series[i];}
	bool isCSFile() const {return _isCSFile;}

private:

	void readLumicycle(const char *name);
	void readChronostar(const char *name);	
	void readTextFile (istream &file);
	QString _fileName;
	QList<TimeSeries*> _series;
	QList<LumiError*> _error;
	bool _isCSFile;
};

#endif // LUMIFILE_H
