#include "datapicker.h"

DataPickerModel::DataPickerModel(const QList<TimeSeries::Data>& Vec) ://, bool takeExcluded) : 
	QAbstractListModel(),
	_dataList()
{
/*	if (takeExcluded)
	{
		for (unsigned d = 0; d < TimeSeries::nData; d++) 
		{
			if (!Vec.contains(static_cast<TimeSeries::Data>(d))) 
			{
				_dataList.append(static_cast<TimeSeries::Data>(d));
			}
		}
	}
	else*/ _dataList = Vec;
}


Qt::ItemFlags DataPickerModel::flags(const QModelIndex& idx) const
{
	Qt::ItemFlags flg; //= QStringListModel::flags(index);
	if (idx.isValid()) flg =  Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
	else flg = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled  | Qt::ItemIsEnabled;
	return flg;
}

QVariant DataPickerModel::data ( const QModelIndex & idx, int role ) const
{
	if (!idx.isValid()) return QVariant();
	if (role != Qt::DisplayRole) return QVariant();
	return TimeSeries::name(_dataList[idx.row()]);
}


bool DataPickerModel::insertRows ( int row, int count, const QModelIndex & parentIndex)
{
	beginInsertRows(parentIndex, row, row+count-1);
	for (int i = 0; i < count; i++) _dataList.insert(row, TimeSeries::FILENAME); // insert anything
	endInsertRows();
	return true;
}
bool DataPickerModel::removeRows ( int row, int count, const QModelIndex & parentIndex)
{
	beginRemoveRows(parentIndex, row, row+count-1);
	for (int i = 0; i < count; i++) _dataList.removeAt(row);
	endRemoveRows();
	return true;
}

int DataPickerModel::rowCount ( const QModelIndex &) const {return _dataList.size();}

bool DataPickerModel::setData ( const QModelIndex & idx, const QVariant & value, int)
{
	qDebug() << "Picker::setData" << value;
	_dataList[idx.row()] = toData(value);
	emit(dataChanged(idx, idx));
	return true;
}
TimeSeries::Data DataPickerModel::toData (const QVariant &var)
{
	QString varName = var.toString();
	for (QList<TimeSeries::Data>::const_iterator it = TimeSeries::TableOffer.begin(); it < TimeSeries::TableOffer.end(); it++)
	{
		if (TimeSeries::name(*it) == varName) return *it;
	}
	return TimeSeries::FILENAME;
}
DataPicker::DataPicker(const QList< TimeSeries::Data >& vec) : 
	QListView(),
	model(new DataPickerModel(vec))
{
	setModel(model);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
}

DataPicker::~DataPicker()
{
	delete model;
}
