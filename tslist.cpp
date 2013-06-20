#include "tslist.h"

#include "gslErrors.h"

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QStringListModel>
#include <QListView>
#include <QPushButton>
#include <QFile>
#pragma GCC diagnostic pop

#include "datapicker.h"
#include <algorithm>

TimeSeries::Data TSList::sortData = TimeSeries::SAMPLE;

TSList::TSList(): 
	QAbstractTableModel(),
	_ts(), 
	header()
{
}


Qt::ItemFlags TSList::flags (const QModelIndex & idx) const 
{
	if (TimeSeries::TableEditable.contains(header[idx.column()])) return QAbstractTableModel::flags(idx) | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(idx);
}

int TSList::columnCount(const QModelIndex&) const
{
	return header.size();
}

QVariant TSList::data(const QModelIndex& idx, int role) const
{
	TimeSeries::Data d = header[idx.column()];
	switch (TimeSeries::type(d))
	{
		case (TimeSeries::TRENDMETHOD) : {if (role == Qt::DisplayRole) return TimeSeries::name(ts(idx)->trendMethod()); if (role == Qt::EditRole) return ts(idx)->data(d); break;}
		case (TimeSeries::TRENDSTEPS)  : {if (role == Qt::DisplayRole) return TimeSeries::name(ts(idx)->trendSteps());  if (role == Qt::EditRole) return ts(idx)->data(d); break;}
		case (TimeSeries::T_ABSREL)    : {if (role == Qt::DisplayRole) return TimeSeries::name(ts(idx)->absRel());      if (role == Qt::EditRole) return ts(idx)->data(d); break;}
		case (TimeSeries::T_STRING) :
		case (TimeSeries::T_DOUBLE) :
		case (TimeSeries::T_INT) :
		case (TimeSeries::T_BOOL) :
		default:                       {if (role == Qt::DisplayRole || role == Qt::EditRole) return ts(idx)->data(d); break;}
	}
	return QVariant();
}

int TSList::rowCount(const QModelIndex&) const
{
	return static_cast<int>(_ts.size());
}

void TSList::insert(const LumiFile* lf)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount()+lf->nSeries()-1);
	for (int i = 0; i < lf->nSeries(); i++) _ts.push_back(lf->series(i));
	endInsertRows();
}

void TSList::insertSingle(const QList<int> &rows)
{
	for (QList<int>::const_iterator it = rows.end()-1; it >= rows.begin(); it--)
	{
		beginInsertRows(QModelIndex(), *it, *it);
		_ts.insert(*it, new TimeSeries(*_ts[*it]));
		endInsertRows();
	}
}


void TSList::insertBlock(const QList< QList< int > >& idx)
{
	for (QList< QList< int > >::const_iterator it1 = idx.end()-1; it1 >= idx.begin(); it1--)
	{
		beginInsertRows(QModelIndex(), it1->first(), it1->last());
		for (QList<int>::const_iterator it2 = it1->end()-1; it2 >= it1->begin(); it2--)
		{
			_ts.insert(it1->first(), new TimeSeries(*ts(it1->last())));
		}
		endInsertRows();
	}
}



void TSList::remove(const QList<QList<int> > &rows)
{
	if (rows.empty()) return;
	for (QList<QList<int> >::const_iterator it1 = rows.end()-1; it1 >= rows.begin(); it1--)
	{
		beginRemoveRows(QModelIndex(), it1->first(), it1->last());
		for (QList<int>::const_iterator it2 = it1->end()-1; it2 >= it1->begin(); it2--)
		{
			delete _ts.takeAt(*it2);
		}
		endRemoveRows();
	}
}

QVariant TSList::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical) 
	{
		if (role == Qt::DisplayRole) return section;
		return QVariant();
	}
	if (role == Qt::DisplayRole) return TimeSeries::name(header[section]);
	if (role == Qt::EditRole) return static_cast<int>(header[section]);
	return QVariant();
}

void TSList::customizeTable()
{
	QDialog dia;
	QGridLayout *lay = new QGridLayout();
	QList<TimeSeries::Data> rest;
	for (QList<TimeSeries::Data>::const_iterator it = TimeSeries::TableOffer.begin(); it < TimeSeries::TableOffer.end(); it++) 
	{
		if (!header.contains(*it)) rest.append(*it);
	}
	DataPicker *restList = new DataPicker(rest);
	DataPicker *tabList  = new DataPicker(header);
	QPushButton *butOK = new QPushButton("OK");
	QPushButton *butCancel = new QPushButton("Cancel");
	connect(butOK, SIGNAL(clicked()), &dia, SLOT(accept()));
	connect(butCancel, SIGNAL(clicked()), &dia, SLOT(reject()));
	lay->addWidget(new QLabel("Available"), 0, 0);
	lay->addWidget(new QLabel("Your table"), 0, 1); 
	lay->addWidget(restList, 1, 0);
	lay->addWidget(tabList, 1, 1);
	lay->addWidget(butOK, 2, 0);
	lay->addWidget(butCancel, 2, 1);
	dia.setLayout(lay);
	int result = dia.exec();
	QList<TimeSeries::Data> lNew = tabList->dataList();
	delete restList;
	delete tabList;
	if (result != QDialog::Accepted) return;
	beginRemoveColumns(QModelIndex(), 0, header.size()-1);
	header.clear();
	endRemoveColumns();
	beginInsertColumns(QModelIndex(), 0, lNew.size()-1);
	header = lNew;
	endInsertColumns();
}

void TSList::readSettings(const QSettings& settings)
{
	if (settings.contains("header")) 
	{
		QList<QVariant> oldHeader = settings.value("header").toList();
		beginInsertColumns(QModelIndex(), 0, oldHeader.size()-1);
		for (QList<QVariant>::const_iterator it = oldHeader.begin(); it < oldHeader.end(); it++) header.append(static_cast<TimeSeries::Data>(it->toInt()));
		endInsertColumns();
	}
	else
	{
		beginRemoveColumns(QModelIndex(), 0, header.size()-1);
		header.clear();
		endRemoveColumns();
		beginInsertColumns(QModelIndex(), 0, 2);
		header.append(TimeSeries::FILENAME);
		header.append(TimeSeries::SAMPLE);
		header.append(TimeSeries::COMMENT);
		header.append(TimeSeries::AMP);
		header.append(TimeSeries::PER_H);
		header.append(TimeSeries::PHA_H);
		header.append(TimeSeries::DAMP);
		header.append(TimeSeries::FVU);
		header.append(TimeSeries::INTEGRAL);
		header.append(TimeSeries::PVALUE);
		endInsertColumns();
	}
}


void TSList::writeSettings(QSettings& settings)
{
	QList<QVariant> hList;
	for (QList<TimeSeries::Data>::const_iterator it = header.begin(); it < header.end(); it++) hList.append(static_cast<int>(*it));
	settings.setValue("header", hList);
}

bool TSList::setData(const QModelIndex& idx, const QVariant& value, int) // user changed settings in table; only one row affected
{
	QList<int> dummyList;
	dummyList.append(idx.row());
	QList<QList<int> > dummyArray;
	dummyArray.append(dummyList);
	setData(dummyArray, header[idx.column()], value);
	return true;
}

void TSList::setData(const QList<QList<int> > &rows, TimeSeries::Data d, const QVariant & value) // user changed settings in ControlPanel (called directly) or table (called via setData(Data, Variant)
{
	for (QList<QList<int> >::const_iterator it1 = rows.begin(); it1 < rows.end(); it1++)
	{
		bool changed = false;
		for (QList<int>::const_iterator it2 = it1->begin(); it2 < it1->end(); it2++)
		{
			changed |= ts(*it2)->setData(d, value);
		}
		if (!changed) continue;
		//emitDataChanged(*it1, d); // emit blockwise signal to update visible table
		//emit dataChanged(index(it1->first(), col), index(it1->last(), col)); // emit THIS data change
		if (d == TimeSeries::COMMENT) emit controlPanelValueChanged(d, value);
		else
		{
			emit controlPanelValueChanged(d, value); 
			emit curveChanged();
			emit dataChanged(index(it1->first(), 0),      index(it1->last(), header.size()-1));
		}
	}
}


void TSList::sort (int column, Qt::SortOrder order)
{
	if (column >= header.size()) return;
	emit layoutAboutToBeChanged();
	sortData = header[column];
	switch (TimeSeries::type(sortData))
	{
		case (TimeSeries::T_STRING) :
		{
			if (order == Qt::AscendingOrder) std::stable_sort(_ts.begin(), _ts.end(), sortStringA);
			else                             std::stable_sort(_ts.begin(), _ts.end(), sortStringD);
			break;
		}
		case (TimeSeries::T_DOUBLE) :
		{
			if (order == Qt::AscendingOrder) std::stable_sort(_ts.begin(), _ts.end(), sortDoubleA);
			else                             std::stable_sort(_ts.begin(), _ts.end(), sortDoubleD);
			break;
		}
		case (TimeSeries::T_INT) :
		case (TimeSeries::T_BOOL) :
		case (TimeSeries::T_TRENDMETHOD) :
		case (TimeSeries::T_TRENDSTEPS) :
		case (TimeSeries::T_ABSREL) :
		{
			if (order == Qt::AscendingOrder) std::stable_sort(_ts.begin(), _ts.end(), sortIntA);
			else                             std::stable_sort(_ts.begin(), _ts.end(), sortIntD);
			break;
		}
		default: break;
	}
	emit layoutChanged();
}
