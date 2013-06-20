#ifndef cs2_H
#define cs2_H

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QtGui/QMainWindow>
#include <QTableView>
#include <QDockWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileInfo>
#include <QRadioButton>
#include <qwt/qwt_plot.h>
#pragma GCC diagnostic pop


#include "tslist.h"
#include "delayedspinbox.h"

class cs2 : public QMainWindow
{

Q_OBJECT

public:

	cs2();
	virtual ~cs2();
	const QString &lastOpened() const {return _lastOpened;}
	const QString lastOpenedPath() const {return QFileInfo(lastOpened()).absolutePath();}


private slots:

	void editSettings();
	void open();
	void save();
	void saveAs();
	void exportSingle();
	void exportExcel();
	void exportTrend();
	void exportDetrended();
	void exportFit();
	void exportParam();
	void del();
	void duplicateSingle();
	void duplicateBlock();
	void selectionChanged(const QItemSelection &sel, const QItemSelection &desel);
	void controlPanelValueChanged(TimeSeries::Data d, const QVariant &newValue);
	void curveChanged();
	void layoutChanged();
	void rowsAboutToBeRemoved(const QModelIndex &, int, int);
	void ctrlChangeSmooth(int newSmooth);
	void ctrlChangeMeanH(int newSmooth);
	void ctrlChangeFitLog(bool newFitLog);
	void ctrlChangePolDeg(int newPolDeg);
	void ctrlChangeFouDeg(int newFouDeg);
	void ctrlChangeAmpRef(int newFouDeg);
	void ctrlChangeTrendFrom(int tr);
	void ctrlChangeTrendTo  (int tr);
	void ctrlSetMethodNone(bool);
	void ctrlSetMethodMean(bool);
	void ctrlSetMethodPoly(bool);
	void ctrlSetMethodFou(bool);
	void ctrlSetOneStep(bool);
	void ctrlSetTwoSteps(bool);
	void ctrlSetAbs(bool);
	void ctrlSetRel(bool);
	void showAbout();
	
private:

	cs2(const cs2 &cs); // do not implement
	cs2 &operator=(const cs2 &cs); // do not implement
	void readSettings();
	void writeSettings();
	void makeMenu();
	void makeControlPanel();
	void makeDockWidgets();
	void updateSelected();
	void setBackground(QWidget *widget, bool equal);
	bool checkExportIntegrity();
	const QList<int> selectedSingleRows();
	const QList<QList<int> > selectedContinuousRows();
	void blockSignals(bool block);
	TSList *tsList;
	QTableView *table;
	QDockWidget *dockRaw, *dockFit, *dockCtrl;
	QwtPlot *plotRaw, *plotFit;
	QList<TimeSeries*> selected;
	QString _lastOpened;
	QRadioButton *ctrlMethNone, *ctrlMethMean, *ctrlMethPoly, *ctrlMethFou, *ctrlOneStep, *ctrlTwoSteps, *ctrlAbsCounts, *ctrlRelCounts;
	DelayedSpinBox *ctrlSmooth, *ctrlTrendFrom, *ctrlTrendTo, *ctrlMeanHours, *ctrlPolDeg, *ctrlFouDeg, *ctrlAmpRef;
	QCheckBox *ctrlFitLog;
	QString currentCSFile;
	QAction *actDelete, *actDupSin, *actDupBlock;
};

#endif // cs2_H
