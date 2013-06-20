#ifndef TSLIST_H
#define TSLIST_H

#include <vector>
#include <functional>

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QSettings>
#include <QTableView>
#include <QHeaderView>
#pragma GCC diagnostic pop

#include "lumifile.h"
#include "timeseries.h"

class TSList: public QAbstractTableModel
{

Q_OBJECT

public:

	TSList();
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex & idx) const;
	virtual bool setData(const QModelIndex & idx, const QVariant & value, int role = Qt::EditRole);
	void setData(const QList<QList<int> > &rows, TimeSeries::Data d, const QVariant & value);
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	TimeSeries *ts(const QModelIndex &qmi) const {return _ts[qmi.row()];}
	TimeSeries *ts(int row) const {return _ts[row];}
	void insert (const LumiFile *lf);
	void readSettings(const QSettings &settings);
	void writeSettings(QSettings &settings);
	
	void remove(const QList<QList<int> > &idx); // use sorted list!
	void insertSingle (const QList<int> &idx); // use sorted list!
	void insertBlock (const QList<QList<int> > &idx); // use sorted list!

signals:

	void controlPanelValueChanged(TimeSeries::Data, const QVariant &newValue);
	void curveChanged();

public slots:

	void customizeTable();

private:

	static QList<TimeSeries::Data> Editable;
	QList<TimeSeries*> _ts;
	QList<TimeSeries::Data> header;
	bool (*sortMethod(TimeSeries::Data d, Qt::SortOrder order)) (const TimeSeries *ts1, const TimeSeries *ts2);
	static TimeSeries::Data sortData;
	static bool sortStringA (const TimeSeries *ts1, const TimeSeries *ts2)
	{
		if (ts1->data(sortData).toInt() == ts2->data(sortData).toInt()) return ts1->data(sortData).toString() < ts2->data(sortData).toString(); 
		return ts1->data(sortData).toInt() < ts2->data(sortData).toInt();
	}
	static bool sortStringD (const TimeSeries *ts1, const TimeSeries *ts2)
	{
		if (ts1->data(sortData).toInt() == ts2->data(sortData).toInt()) return ts1->data(sortData).toString() > ts2->data(sortData).toString(); 
		return ts1->data(sortData).toInt() > ts2->data(sortData).toInt();
	}
	static bool sortIntA (const TimeSeries *ts1, const TimeSeries *ts2)
	{
		return ts1->data(sortData).toInt() < ts2->data(sortData).toInt();
	}
	static bool sortIntD (const TimeSeries *ts1, const TimeSeries *ts2)
	{
		return ts1->data(sortData).toInt() > ts2->data(sortData).toInt();
	}
	static bool sortDoubleA (const TimeSeries *ts1, const TimeSeries *ts2) 
	{
		return ((std::isnan(ts1->data(sortData).toDouble())?0:ts1->data(sortData).toDouble())) < (std::isnan(ts2->data(sortData).toDouble())?0:ts2->data(sortData).toDouble());
	}
	static bool sortDoubleD (const TimeSeries *ts1, const TimeSeries *ts2)
	{
		return ((std::isnan(ts1->data(sortData).toDouble())?1:ts1->data(sortData).toDouble())) > (std::isnan(ts2->data(sortData).toDouble())?1:ts2->data(sortData).toDouble());
	}
};

#endif // TSLIST_H
