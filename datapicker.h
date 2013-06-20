#ifndef DATAPICKER_H
#define DATAPICKER_H

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QAbstractListModel>
#include <QListView>
#pragma GCC diagnostic pop

#include "timeseries.h"

class DataPickerModel : public QAbstractListModel // for table configuration dialog
{

public:

	DataPickerModel(const QList<TimeSeries::Data> &Vec);//, bool takeExcluded);
	virtual QVariant data ( const QModelIndex & index, int role ) const;
	virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
	virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	virtual Qt::DropActions supportedDropActions () const {return Qt::MoveAction;}
	TimeSeries::Data toData (const QVariant &var);
	const QList<TimeSeries::Data> &dataList() const {return _dataList;}

private:

	QList<TimeSeries::Data> _dataList;
};


class DataPicker : public QListView
{

public:

	//enum PickerType{REST, TABLE};
	DataPicker(const QList<TimeSeries::Data> &Vec);//, PickerType pt);
	virtual ~DataPicker();
	const QList<TimeSeries::Data> &dataList() const {return model->dataList();}

private:

	DataPicker(const DataPicker &dp); // do not implement
	DataPicker&operator=(const DataPicker &dp); // do not implement
	DataPickerModel *model;

};

#endif // DATAPICKER_H
