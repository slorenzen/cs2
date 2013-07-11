#include "cs2.h"

#pragma GCC diagnostic ignored "-Weffc++"
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QKeyEvent>
#include <QGridLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#pragma GCC diagnostic pop

//#include <stdint.h>
//#include <algorithm>
#include <qwt/qwt_plot_curve.h>
#include "scaledraw24.h"

void cs2::makeMenu()
{
	actDelete->setShortcut(QKeySequence("Del"));
	connect(actDelete, SIGNAL(triggered()), SLOT(del()));
	actDupSin->setShortcut(QKeySequence("Ctrl+D"));
	connect(actDupSin, SIGNAL(triggered()), SLOT(duplicateSingle()));
	actDupBlock->setShortcut(QKeySequence("Ctrl+V"));
	connect(actDupBlock, SIGNAL(triggered()), SLOT(duplicateBlock()));
	QAction *actOpen = new QAction("Open", this);
	actOpen->setShortcut(QKeySequence("Ctrl+O"));
	connect(actOpen, SIGNAL(triggered()), SLOT(open()));
	QAction *actSave = new QAction("Save", this);
	actSave->setShortcut(QKeySequence("Ctrl+S"));
	connect(actSave, SIGNAL(triggered()), SLOT(save()));
	QAction *actSaveAs = new QAction("Save as...", this);
	connect(actSaveAs, SIGNAL(triggered()), SLOT(saveAs()));
	QAction *actExcel = new QAction("Export Excel", this);
	connect(actExcel, SIGNAL(triggered()), this, SLOT(exportExcel()));
	QAction *actSingle = new QAction("Export single", this);
	connect(actSingle, SIGNAL(triggered()), SLOT(exportSingle()));
	QAction *actTrend = new QAction("Export Trend", this);
	connect(actTrend, SIGNAL(triggered()), this, SLOT(exportTrend()));
	QAction *actDetrend = new QAction("Export Detrended", this);
	connect(actDetrend, SIGNAL(triggered()), this, SLOT(exportDetrended()));
	QAction *actFit = new QAction("Export Fit", this);
	connect(actFit, SIGNAL(triggered()), this, SLOT(exportFit()));
	QAction *actParam = new QAction("Export Fit Parameters", this);
	connect(actParam, SIGNAL(triggered()), this, SLOT(exportParam()));
	QAction *actCustomize = new QAction("Customize table", this);
	connect(actCustomize, SIGNAL(triggered()), tsList, SLOT(customizeTable()));
	QAction *actEditSettings = new QAction("Edit defaults", this);
	connect(actEditSettings, SIGNAL(triggered()), SLOT(editSettings()));
	QAction *actQuit = new QAction("Quit", this);
	actQuit->setShortcut(QKeySequence("Ctrl+Q"));
	connect(actQuit, SIGNAL(triggered()), SLOT(close()));
	QAction *actAbout = new QAction("About", this);
	connect(actAbout, SIGNAL(triggered()), SLOT(showAbout()));
	QMenu *menFile = menuBar()->addMenu("File");
	menFile->addAction(actOpen);
	menFile->addAction(actSave);
	menFile->addAction(actSaveAs);
	menFile->addAction(actQuit);
	QMenu *men_export = menuBar()->addMenu("Export");
	men_export->addAction(actSingle);
	men_export->addAction(actExcel);
	men_export->addAction(actTrend);
	men_export->addAction(actDetrend);
	men_export->addAction(actFit);
	men_export->addAction(actParam);
	QMenu *men_edit = menuBar()->addMenu("Edit");
	men_edit->addAction(actDelete);
	men_edit->addAction(actDupSin);
	men_edit->addAction(actDupBlock);
	men_edit->addAction(actCustomize);
	men_edit->addAction(actEditSettings);
	QMenu *men_views = menuBar()->addMenu("Views");
	men_views->addAction(dockRaw->toggleViewAction());
	men_views->addAction(dockFit->toggleViewAction());
	men_views->addAction(dockCtrl->toggleViewAction());
	QMenu *menHelp = menuBar()->addMenu("Help");
	menHelp->addAction(actAbout);
}

void cs2::showAbout()
{
	QMessageBox::about ( this, "About ChronoStar", 
							QString::fromUtf8("ChronoStar\nAnalysis of circadian data sets\nCompiled: %1\nAuthor: Stephan Lorenzen\nMedizinische Statistik\nUniversity Medicine Göttingen, Germany\nstephlo1976@gmail.com").arg(__DATE__));
}

void cs2::makeDockWidgets()
{
	dockRaw->setObjectName("dockRaw");
	dockRaw->setFloating(false);
	addDockWidget(Qt::RightDockWidgetArea, dockRaw);
	dockFit->setObjectName("dockFit");
	dockFit->setFloating(false);
	addDockWidget(Qt::RightDockWidgetArea, dockFit);
	plotRaw->setAxisScaleDraw(QwtPlot::xBottom, new ScaleDraw24());
	plotFit->setAxisScaleDraw(QwtPlot::xBottom, new ScaleDraw24());	
}

void cs2::makeControlPanel()
{
	const QString outerFrame = 	"QGroupBox{font-weight: bold;border:2px solid gray;border-radius:5px;margin-top: 1ex;border-color:darkblue} "
								"QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;} ";
	const QString innerFrame = 	"QGroupBox{font-weight: normal;border:1px solid gray;border-radius:5px;margin-top: 1ex;} "
								"QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;} ";
	
	QGroupBox *grpSmooth = new QGroupBox("Smoothing");
	grpSmooth->setStyleSheet(innerFrame);
	QHBoxLayout *laySmooth = new QHBoxLayout(grpSmooth);
	laySmooth->addWidget(ctrlSmooth);
	laySmooth->addWidget(new QLabel("points"));
	
	QGroupBox *grpBorders = new QGroupBox("Borders");
	grpBorders->setStyleSheet(innerFrame);
	QGridLayout *layBorders = new QGridLayout(grpBorders);
	layBorders->addWidget(new QLabel("Left"),  0, 0);
	layBorders->addWidget(new QLabel("Right"), 0, 1);
	layBorders->addWidget(ctrlTrendFrom,      1, 0);
	layBorders->addWidget(ctrlTrendTo,        1, 1);
	
	QGroupBox *grpRaw = new QGroupBox("Raw data");
	grpRaw->setStyleSheet(outerFrame);
	QVBoxLayout *layRaw = new QVBoxLayout(grpRaw);
	layRaw->addWidget(grpSmooth);
	layRaw->addWidget(grpBorders);

	QWidget *widOneTwo = new QWidget;
	QVBoxLayout *layOneTwo = new QVBoxLayout(widOneTwo);
	layOneTwo->addWidget(ctrlOneStep);
	layOneTwo->addWidget(ctrlTwoSteps);
	
	QGroupBox *grpMethod = new QGroupBox("DetrendMethod");
	grpMethod->setStyleSheet(innerFrame);
	QGridLayout *layDetrMethod = new QGridLayout(grpMethod);
	layDetrMethod->addWidget(ctrlMethNone,        0, 0);
	layDetrMethod->addWidget(ctrlMethMean,        1, 0);
	layDetrMethod->addWidget(ctrlMeanHours,       1, 1);
	layDetrMethod->addWidget(new QLabel("h"),     1, 2);
	layDetrMethod->addWidget(ctrlMethPoly,        2, 0);
	layDetrMethod->addWidget(ctrlPolDeg,          2, 1);
	layDetrMethod->addWidget(new QLabel("order"), 2, 2);
	layDetrMethod->addWidget(ctrlMethFou,         3, 0);
	layDetrMethod->addWidget(ctrlFouDeg,          3, 1);
	layDetrMethod->addWidget(new QLabel("order"), 3, 2);
	layDetrMethod->addWidget(widOneTwo,           4, 0, 1, 3);
	
	QGroupBox *grpLog = new QGroupBox("SineFit");
	grpLog->setStyleSheet(innerFrame);
	QVBoxLayout *layLog = new QVBoxLayout(grpLog);
	layLog->addWidget(ctrlFitLog);
	
	QGroupBox *grpAbsRel = new QGroupBox("Output");
	grpAbsRel->setStyleSheet(innerFrame);
	QVBoxLayout *layAbsRel = new QVBoxLayout(grpAbsRel);
	layAbsRel->addWidget(ctrlAbsCounts);
	layAbsRel->addWidget(ctrlRelCounts);

	QGroupBox *grpAmpRef = new QGroupBox("Amplitude");
	grpAmpRef->setStyleSheet(innerFrame);
	QVBoxLayout *layAmpRef = new QVBoxLayout(grpAmpRef);
	layAmpRef->addWidget(new QLabel("Pick Amplitude at"));
	layAmpRef->addWidget(ctrlAmpRef);
	
	QGroupBox *grpDetFit = new QGroupBox("Detrending && Sine Fit");
	grpDetFit->setStyleSheet(outerFrame);
	QGridLayout *layDetFit = new QGridLayout(grpDetFit);
	layDetFit->addWidget(grpMethod, 0, 0, 3, 1);
	layDetFit->addWidget(grpLog,    0, 1);
	layDetFit->addWidget(grpAbsRel, 1, 1);
	layDetFit->addWidget(grpAmpRef, 2, 1);
	
	
	grpRaw->    setFixedWidth(grpRaw->     minimumSizeHint().width());
	grpRaw->    setFixedHeight(max(grpRaw->minimumSizeHint().height(), grpDetFit->minimumSizeHint().height()));
	grpDetFit-> setFixedWidth(grpDetFit->  minimumSizeHint().width());
	grpDetFit-> setFixedHeight(max(grpRaw->minimumSizeHint().height(), grpDetFit->minimumSizeHint().height()));
	
	
	ctrlSmooth->setEnabled(false);
	ctrlTrendFrom->setEnabled(false);
	ctrlTrendTo->setEnabled(false);
	ctrlMethNone->setEnabled(false);
	ctrlMethMean->setEnabled(false);
	ctrlMethPoly->setEnabled(false);
	ctrlMethFou->setEnabled(false);
	ctrlMeanHours->setEnabled(false);
	ctrlPolDeg->setEnabled(false);
	ctrlFouDeg->setEnabled(false);
	ctrlFitLog->setEnabled(false);
	ctrlOneStep->setEnabled(false);
	ctrlTwoSteps->setEnabled(false);
	ctrlAbsCounts->setEnabled(false);
	ctrlRelCounts->setEnabled(false);
	ctrlAmpRef->setEnabled(false);

	connect(ctrlSmooth,    SIGNAL(valueChanged(int)), SLOT(ctrlChangeSmooth(int)));
	connect(ctrlTrendFrom, SIGNAL(valueChanged(int)), SLOT(ctrlChangeTrendFrom(int)));
	connect(ctrlTrendTo,   SIGNAL(valueChanged(int)), SLOT(ctrlChangeTrendTo(int)));
	connect(ctrlMeanHours, SIGNAL(valueChanged(int)), SLOT(ctrlChangeMeanH(int)));
	connect(ctrlPolDeg,    SIGNAL(valueChanged(int)), SLOT(ctrlChangePolDeg(int)));
	connect(ctrlFouDeg,    SIGNAL(valueChanged(int)), SLOT(ctrlChangeFouDeg(int)));
	connect(ctrlAmpRef,    SIGNAL(valueChanged(int)), SLOT(ctrlChangeAmpRef(int)));
	connect(ctrlMethNone,  SIGNAL(toggled(bool)), SLOT(ctrlSetMethodNone(bool)));
	connect(ctrlMethMean,  SIGNAL(toggled(bool)), SLOT(ctrlSetMethodMean(bool)));
	connect(ctrlMethPoly,  SIGNAL(toggled(bool)), SLOT(ctrlSetMethodPoly(bool)));
	connect(ctrlMethFou,   SIGNAL(toggled(bool)), SLOT(ctrlSetMethodFou(bool)));
	connect(ctrlOneStep,   SIGNAL(toggled(bool)), SLOT(ctrlSetOneStep(bool)));
	connect(ctrlTwoSteps,  SIGNAL(toggled(bool)), SLOT(ctrlSetTwoSteps(bool)));
	connect(ctrlFitLog,    SIGNAL(toggled(bool)), SLOT(ctrlChangeFitLog(bool)));
	connect(ctrlAbsCounts, SIGNAL(toggled(bool)), SLOT(ctrlSetAbs(bool)));
	connect(ctrlRelCounts, SIGNAL(toggled(bool)), SLOT(ctrlSetRel(bool)));

	QWidget *wid = new QWidget();
	QHBoxLayout *layCtrl = new QHBoxLayout(wid);
	layCtrl->addWidget(grpRaw);
	layCtrl->addWidget(grpDetFit);
	wid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	wid->setMaximumHeight(wid->sizeHint().height());
	wid->setMaximumWidth(wid->sizeHint().width());
	dockCtrl->setObjectName("dockCtrl");
	dockCtrl->setFloating(false);
	dockCtrl->setWidget(wid);
	addDockWidget(Qt::RightDockWidgetArea, dockCtrl);
	wid->resize(wid->minimumSize());
	dockCtrl->show();
}

void cs2::editSettings()
{
	const QString outerFrame = 	"QGroupBox{font-weight: bold;border:2px solid gray;border-radius:5px;margin-top: 1ex;border-color:darkblue} "
								"QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;} ";
	const QString innerFrame = 	"QGroupBox{font-weight: normal;border:1px solid gray;border-radius:5px;margin-top: 1ex;} "
								"QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;} ";
	QRadioButton *editMethNone = new QRadioButton("None");
	QRadioButton *editMethMean = new QRadioButton("Running Average");
	QRadioButton *editMethPoly = new QRadioButton("Polynome");
	QRadioButton *editMethFou  = new QRadioButton("Fourier");
	QRadioButton *editOneStep  = new QRadioButton("OneStep (Trend&&Fit)");
	QRadioButton *editTwoSteps = new QRadioButton("TwoSteps (Trend->Fit)");
	QRadioButton *editAbsCounts = new QRadioButton("Abs. Counts");
	QRadioButton *editRelCounts = new QRadioButton("Rel. Counts");
	QSpinBox *editMeanH  = new QSpinBox();
	QSpinBox *editPolDeg = new QSpinBox();
	QSpinBox *editFouDeg = new QSpinBox();
	QCheckBox *editFitLog= new QCheckBox("Log Scale");
	
	QWidget *widOneTwo = new QWidget;
	QVBoxLayout *layOneTwo = new QVBoxLayout(widOneTwo);
	layOneTwo->addWidget(editOneStep);
	layOneTwo->addWidget(editTwoSteps);
	
	QGroupBox *grpMethod = new QGroupBox("DetrendMethod");
	grpMethod->setStyleSheet(innerFrame);
	QGridLayout *layDetrMethod = new QGridLayout(grpMethod);
	layDetrMethod->addWidget(editMethNone,        0, 0);
	layDetrMethod->addWidget(editMethMean,        1, 0);
	layDetrMethod->addWidget(editMeanH,           1, 1);
	layDetrMethod->addWidget(new QLabel("h"),     1, 2);
	layDetrMethod->addWidget(editMethPoly,        2, 0);
	layDetrMethod->addWidget(editPolDeg,          2, 1);
	layDetrMethod->addWidget(new QLabel("order"), 2, 2);
	layDetrMethod->addWidget(editMethFou,         3, 0);
	layDetrMethod->addWidget(editFouDeg,          3, 1);
	layDetrMethod->addWidget(new QLabel("order"), 3, 2);
	layDetrMethod->addWidget(widOneTwo,           4, 0, 1, 3);
	
	QGroupBox *grpLog = new QGroupBox("SineFit");
	grpLog->setStyleSheet(innerFrame);
	QVBoxLayout *layLog = new QVBoxLayout(grpLog);
	layLog->addWidget(editFitLog);
	
	QGroupBox *grpAbsRel = new QGroupBox("Output");
	grpAbsRel->setStyleSheet(innerFrame);
	QVBoxLayout *layAbsRel = new QVBoxLayout(grpAbsRel);
	layAbsRel->addWidget(editAbsCounts);
	layAbsRel->addWidget(editRelCounts);

	QPushButton *butOK = new QPushButton("OK");
	QPushButton *butCancel = new QPushButton("Cancel");
	
	if (TimeSeries::defaultTrendMethod == TimeSeries::NONE) editMethNone->setChecked(true);
	if (TimeSeries::defaultTrendMethod == TimeSeries::MEAN) editMethMean->setChecked(true);
	if (TimeSeries::defaultTrendMethod == TimeSeries::POLY) editMethPoly->setChecked(true);
	if (TimeSeries::defaultTrendMethod == TimeSeries::FOU)  editMethFou ->setChecked(true);
	if (TimeSeries::defaultTrendSteps  == TimeSeries::ONESTEP)  editOneStep->setChecked(true);
	if (TimeSeries::defaultTrendSteps  == TimeSeries::TWOSTEPS) editTwoSteps->setChecked(true);
	if (TimeSeries::defaultAbsRel      == TimeSeries::ABS) editAbsCounts->setChecked(true);
	if (TimeSeries::defaultAbsRel      == TimeSeries::REL) editRelCounts->setChecked(true);
	
	QDialog   *dia = new QDialog(this);
	dia->setWindowTitle("Edit defaults");
	//QGroupBox *grpDetFit = new QGroupBox("Detrending && Sine Fit");
	//grpDetFit->setStyleSheet(outerFrame);
	QGridLayout *layDetFit = new QGridLayout(dia);
	layDetFit->addWidget(grpMethod, 0, 0, 4, 1);
	layDetFit->addWidget(grpLog,    0, 1);
	layDetFit->addWidget(grpAbsRel, 1, 1);
	layDetFit->addWidget(butOK,     2, 1);
	layDetFit->addWidget(butCancel, 3, 1);

	editPolDeg->setRange(0, 10);
	editFouDeg->setRange(0, 10);
	editFitLog->setChecked(TimeSeries::defaultFitLog);
	editMeanH->setValue(static_cast<int>(round(TimeSeries::defaultWindowWidth_h)));
	editPolDeg->setValue(TimeSeries::defaultPolyDegree);
	editFouDeg->setValue(TimeSeries::defaultFouDegree);

	connect(butOK, SIGNAL(clicked()), dia, SLOT(accept()));
	connect(butCancel, SIGNAL(clicked()), dia, SLOT(reject()));
	int result = dia->exec();
	if (result == QDialog::Accepted)
	{
		if (editMethNone->isChecked()) TimeSeries::defaultTrendMethod = TimeSeries::NONE;
		if (editMethMean->isChecked()) TimeSeries::defaultTrendMethod = TimeSeries::MEAN;
		if (editMethPoly->isChecked()) TimeSeries::defaultTrendMethod = TimeSeries::POLY;
		if (editMethFou ->isChecked()) TimeSeries::defaultTrendMethod = TimeSeries::FOU;
		if (editOneStep ->isChecked()) TimeSeries::defaultTrendSteps  = TimeSeries::ONESTEP;
		if (editTwoSteps->isChecked()) TimeSeries::defaultTrendSteps  = TimeSeries::TWOSTEPS;
		if (editAbsCounts->isChecked()) TimeSeries::defaultAbsRel     = TimeSeries::ABS;
		if (editRelCounts->isChecked()) TimeSeries::defaultAbsRel     = TimeSeries::REL;
		TimeSeries::defaultFitLog = editFitLog->isChecked();
		TimeSeries::defaultWindowWidth_h = editMeanH->value();
		TimeSeries::defaultPolyDegree = editPolDeg->value();
		TimeSeries::defaultFouDegree  = editFouDeg->value();
	}
}

void cs2::setBackground(QWidget *widget, bool equal)
{
	if (equal) widget->setStyleSheet("QWidget:enabled {background-color: white; color:black; selection-color: red}");
	else       widget->setStyleSheet("QWidget:enabled {background-color: lightGray; color:black; selection-color: red}");
}

void cs2::ctrlChangeSmooth(int newSmooth) // user change in control panel
{
	tsList->setData(selectedContinuousRows(), TimeSeries::SMOOTH, newSmooth);
}
void cs2::ctrlChangeTrendFrom(int trend) // user change in control panel
{
	if (ctrlTrendTo->value() > trend) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDFROM, trend);
}
void cs2::ctrlChangeTrendTo(int trend) // user change in control panel
{
	if (ctrlTrendFrom->value() < trend) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDTO, trend);
}



void cs2::ctrlChangeFitLog(bool newFitLog) // user change in control panel
{
	tsList->setData(selectedContinuousRows(), TimeSeries::FITLOG, newFitLog);
}
void cs2::ctrlChangePolDeg(int newPolDeg) // user change in control panel
{
	tsList->setData(selectedContinuousRows(), TimeSeries::POLYDEG, newPolDeg);
	ctrlMethPoly->setChecked(true);
}
void cs2::ctrlChangeFouDeg(int newFouDeg) // user change in control panel
{
	tsList->setData(selectedContinuousRows(), TimeSeries::FOUDEG, newFouDeg);
	ctrlMethFou->setChecked(true);
}


cs2::cs2() :
	QMainWindow(),
	tsList(new TSList()),
	table(new QTableView(this)),
	dockRaw(new QDockWidget("Raw data", this)), dockFit(new QDockWidget("Fit data", this)), 
	dockCtrl(new QDockWidget("Control panel", this)),
	plotRaw(new QwtPlot()), plotFit(new QwtPlot()), 
	selected(),
	_lastOpened(),
	ctrlMethNone(new QRadioButton("None")),
	ctrlMethMean(new QRadioButton("Running Average")),
	ctrlMethPoly(new QRadioButton("Polynome")),
	ctrlMethFou(new QRadioButton("Fourier")),
	ctrlOneStep(new QRadioButton("OneStep (Trend&&Fit)")),
	ctrlTwoSteps(new QRadioButton("TwoSteps (Trend->Fit)")),
	ctrlAbsCounts(new QRadioButton("Abs. Counts")),
	ctrlRelCounts(new QRadioButton("Rel. Counts")), 
	ctrlSmooth(new DelayedSpinBox()),
	ctrlTrendFrom(new DelayedSpinBox()),
	ctrlTrendTo(new DelayedSpinBox()),
	ctrlMeanHours(new DelayedSpinBox()),
	ctrlPolDeg(new DelayedSpinBox()),
	ctrlFouDeg(new DelayedSpinBox()),
	ctrlAmpRef(new DelayedSpinBox()),
	ctrlFitLog(new QCheckBox("Log Scale")),
	currentCSFile(), 
	actDelete   (new QAction("Delete", this)),
	actDupSin   (new QAction("Duplicate single", this)),
	actDupBlock (new QAction("Duplicate block", this))
{
	setCorner(Qt::BottomRightCorner, Qt:: RightDockWidgetArea);
	table->setModel(tsList);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSortingEnabled(true);
	//table->verticalHeader()->show();
	setCentralWidget(table);
	makeMenu();
	makeDockWidgets();
	makeControlPanel();
	connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(tsList, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
	connect(tsList, SIGNAL(layoutChanged()), SLOT(layoutChanged()));
	//connect(tsList, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
	connect(tsList, SIGNAL(controlPanelValueChanged(TimeSeries::Data, const QVariant &)), SLOT(controlPanelValueChanged(TimeSeries::Data, const QVariant &)));
	connect(tsList, SIGNAL(curveChanged()), SLOT(curveChanged()));
	readSettings();
}

cs2::~cs2()
{
	writeSettings();
	delete tsList;
	delete plotRaw; 
	delete plotFit;
}

void cs2::readSettings()
{
	QSettings settings("stephlo", "cs2");
	tsList->readSettings(settings);
	TimeSeries::readSettings(settings);
	if (settings.contains("state")) restoreState(settings.value("state").toByteArray());
	if (settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
	setWindowState(static_cast<Qt::WindowState>(settings.value("winstate",Qt::WindowMaximized).toInt()));
	_lastOpened = settings.value("envDir", getenv("HOME")).toString();
}

void cs2::writeSettings()
{
	QSettings settings("stephlo", "cs2");
	tsList->writeSettings(settings);
	TimeSeries::writeSettings(settings);
	settings.setValue("winstate", static_cast<int>(windowState()));
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state",    saveState());
	settings.setValue("envDir", lastOpened());
}

void cs2::open()
{
	QStringList fList = QFileDialog::getOpenFileNames(this, "Open File", lastOpened());
	for (int fNr = 0; fNr < fList.size(); fNr++)
	{
		QString fn = fList[fNr];
		_lastOpened = QFileInfo(fn).absoluteFilePath();
		LumiFile *lumi = new LumiFile(fn.toUtf8().data());
		if (!lumi->hasErrors()) 
		{
			if (fList.size() == 1 && tsList->rowCount() == 0 && lumi->isCSFile()) currentCSFile = fn;
			else currentCSFile = "";
			tsList->insert(lumi);
		}
		else
		{
			QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(QFileInfo(fn).fileName()));
			return;
		}
		statusBar()->showMessage(QString("%1 with %2 samples loaded.").arg(QFileInfo(fn).fileName()).arg(lumi->nSeries()));
		delete lumi;
	}
}

void cs2::saveAs()
{
	QString filter = "ChronoStar files (*.cs2)";
	currentCSFile = QFileDialog::getSaveFileName(this, "Save File", lastOpenedPath(), filter, &filter);
	if (currentCSFile.isEmpty()) return;
	if (!currentCSFile.endsWith(".cs2")) currentCSFile += ".cs2";
	save();
}

void cs2::save()
{
	if (currentCSFile.isEmpty()) {saveAs(); return;}
	QFile file(currentCSFile);
	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(currentCSFile));
		return;
	}
	QDataStream out(&file);
	uint8_t start = 236;
	int magic = 04112010;
	out << start << magic << TimeSeries::FILEVERSION;
	for (int i = 0; i < tsList->rowCount(); i++) out << *(tsList->ts(i));
	file.close();
	statusBar()->showMessage(QString("Workbench saved as %1.").arg(currentCSFile));
}

bool cs2::checkExportIntegrity()
{
	if (selected.length() == 0)
	{
		QMessageBox::warning(this, "Export error", "No samples selected");
		return false;
	}
	TimeSeries *tsRef = selected[0];
	QModelIndexList mil = table->selectionModel()->selectedRows();
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		if (tsList->ts(*it)->nRaw() != tsRef->nRaw())
		{
			QMessageBox::warning(this, "Length error", "Selected files have different lengths and cannot be exported together");
			return false;
		}
	}
	return true;
}

void cs2::exportExcel()
{
	if (!checkExportIntegrity()) return;
	TimeSeries *tsRef = selected[0];
	QString filter = "MS Excel (*.xml)";
	QString fileName = QFileDialog::getSaveFileName(this, "Export Excel", lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".xml")) fileName += ".xml";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	out << "<?xml version=\"1.0\"?>" << endl;
	out << "<?mso-application progid=\"Excel.Sheet\"?>" << endl;
	out << "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"" << endl;
	out << " xmlns:o=\"urn:schemas-microsoft-com:office:office\"" << endl;
	out << " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"" << endl;
	out << " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"" << endl;
	out << " xmlns:html=\"http://www.w3.org/TR/REC-html40\">" << endl;
	QModelIndexList mil = table->selectionModel()->selectedRows();
	//
	// RAW
	//
	out << " <Worksheet ss:Name=\"Raw\">" << endl;
	out << "  <Table ss:ExpandedColumnCount=\"" << mil.length()+1 << "\" ss:ExpandedRowCount=\"" << tsRef->nRaw()+1 << "\" x:FullColumns=\"1\" x:FullRows=\"1\">" << endl;
	out << "  <Row>" << endl;
	out << "    <Cell><Data ss:Type=\"String\">time</Data></Cell>" << endl;
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) out << "    <Cell><Data ss:Type=\"String\">" << tsList->ts(*it)->sample() << "</Data></Cell>" << endl;
	out << "  </Row>" << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << "  <Row>" << endl;
		out << "    <Cell><Data ss:Type=\"Number\">" << tsRef->tRawDays(tNr)*24 << "</Data></Cell>" << endl;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			out << "    <Cell><Data ss:Type=\"Number\">" << tsList->ts(*it)->dRaw(tNr) << "</Data></Cell>" << endl;
		}
		out << "  </Row>" << endl;
	}
	out << "  </Table>" << endl;
	out << " </Worksheet>" << endl;
	//
	// SMOOTH
	//
	out << " <Worksheet ss:Name=\"Smooth\">" << endl;
	out << "  <Table ss:ExpandedColumnCount=\"" << mil.length()+1 << "\" ss:ExpandedRowCount=\"" << tsRef->nRaw()+1 << "\" x:FullColumns=\"1\" x:FullRows=\"1\">" << endl;
	out << "  <Row>" << endl;
	out << "    <Cell><Data ss:Type=\"String\">time</Data></Cell>" << endl;
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) out << "    <Cell><Data ss:Type=\"String\">" << tsList->ts(*it)->sample() << "</Data></Cell>" << endl;
	out << "  </Row>" << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << "  <Row>" << endl;
		out << "    <Cell><Data ss:Type=\"Number\">" << tsRef->tRawDays(tNr)*24 << "</Data></Cell>" << endl;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			if (tNr >= ts->skipSmoothBegin() && tNr < ts->nRaw()-ts->skipSmoothEnd())
			{
				out << "    <Cell><Data ss:Type=\"Number\">" << ts->dSmooth(tNr-ts->skipSmoothBegin()) << "</Data></Cell>" << endl;
			}
			else out << "<Cell><Data ss:Type=\"Error\">#N/A</Data></Cell>" << endl;
		}
		out << "  </Row>" << endl;
	}
	out << "  </Table>" << endl;
	out << " </Worksheet>" << endl;
	//
	// TREND
	//
	out << " <Worksheet ss:Name=\"Trend\">" << endl;
	out << "  <Table ss:ExpandedColumnCount=\"" << mil.length()+1 << "\" ss:ExpandedRowCount=\"" << tsRef->nRaw()+1 << "\" x:FullColumns=\"1\" x:FullRows=\"1\">" << endl;
	out << "  <Row>" << endl;
	out << "    <Cell><Data ss:Type=\"String\">time</Data></Cell>" << endl;
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) out << "    <Cell><Data ss:Type=\"String\">" << tsList->ts(*it)->sample() << "</Data></Cell>" << endl;
	out << "  </Row>" << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << "  <Row>" << endl;
		out << "    <Cell><Data ss:Type=\"Number\">" << tsRef->tRawDays(tNr)*24 << "</Data></Cell>" << endl;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd())
			{
				out << "    <Cell><Data ss:Type=\"Number\">" << ts->dTrend(tNr-ts->skipTrendBegin()) << "</Data></Cell>" << endl;
			}
			else out << "<Cell><Data ss:Type=\"Error\">#N/A</Data></Cell>" << endl;
		}
		out << "  </Row>" << endl;
	}
	out << "  </Table>" << endl;
	out << " </Worksheet>" << endl;
	//
	// DETRENDED
	//
	out << " <Worksheet ss:Name=\"Detrended\">" << endl;
	out << "  <Table ss:ExpandedColumnCount=\"" << mil.length()+1 << "\" ss:ExpandedRowCount=\"" << tsRef->nRaw()+1 << "\" x:FullColumns=\"1\" x:FullRows=\"1\">" << endl;
	out << "  <Row>" << endl;
	out << "    <Cell><Data ss:Type=\"String\">time</Data></Cell>" << endl;
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) out << "    <Cell><Data ss:Type=\"String\">" << tsList->ts(*it)->sample() << "</Data></Cell>" << endl;
	out << "  </Row>" << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << "  <Row>" << endl;
		out << "    <Cell><Data ss:Type=\"Number\">" << tsRef->tRawDays(tNr)*24 << "</Data></Cell>" << endl;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd())
			{
				out << "    <Cell><Data ss:Type=\"Number\">" << ts->dDetrended(tNr-ts->skipTrendBegin()) << "</Data></Cell>" << endl;
			}
			else out << "<Cell><Data ss:Type=\"Error\">#N/A</Data></Cell>" << endl;
		}
		out << "  </Row>" << endl;
	}
	out << "  </Table>" << endl;
	out << " </Worksheet>" << endl;
	//
	// FIT
	//
	out << " <Worksheet ss:Name=\"Fit\">" << endl;
	out << "  <Table ss:ExpandedColumnCount=\"" << TimeSeries::TextFileFormat.size() << "\" ss:ExpandedRowCount=\"" << mil.length()+1 << "\" x:FullColumns=\"1\" x:FullRows=\"1\">" << endl;
	out << "  <Row>" << endl;
	for (QList<TimeSeries::Data>::const_iterator it = TimeSeries::TextFileFormat.begin(); it < TimeSeries::TextFileFormat.end(); it++)
	{
		out << "    <Cell><Data ss:Type=\"String\">" << TimeSeries::name(*it) << "</Data></Cell>" << endl;
	}
	//for (int dNr = 0; dNr < TimeSeries::nData; dNr++) out << "    <Cell><Data ss:Type=\"String\">" << TimeSeries::name(static_cast<TimeSeries::Data>(dNr)) << "</Data></Cell>" << endl;
	out << "  </Row>" << endl;
	for (QModelIndexList::const_iterator itTS = mil.begin(); itTS < mil.end(); itTS++)
	{
		TimeSeries *ts = tsList->ts(*itTS);
		out << "  <Row>" << endl;
		for (QList<TimeSeries::Data>::const_iterator itDat = TimeSeries::TextFileFormat.begin(); itDat < TimeSeries::TextFileFormat.end(); itDat++)
		{
			switch (TimeSeries::type(*itDat))
			{
				case (TimeSeries::T_STRING) : 
				{
					out << "    <Cell><Data ss:Type=\"String\">" << ts->data(*itDat).toString() << "</Data></Cell>" << endl; 
					break;
				}
				case (TimeSeries::T_DOUBLE) : 
				{
					if (std::isnan(ts->data(*itDat).toDouble())) out << "    <Cell><Data ss:Type=\"Error\">#N/A</Data></Cell>" << endl;
					else out << "    <Cell><Data ss:Type=\"Number\">" << ts->data(*itDat).toDouble() << "</Data></Cell>" << endl;
					break;
				}
				case (TimeSeries::T_INT) : 
				{
					if (isnan(ts->data(*itDat).toInt())) out << "    <Cell><Data ss:Type=\"Error\">#N/A</Data></Cell>" << endl;
					else out << "    <Cell><Data ss:Type=\"Number\">" << ts->data(*itDat).toInt() << "</Data></Cell>" << endl;
					break;
				}
				case (TimeSeries::T_BOOL) :
				{
					out << "    <Cell><Data ss:Type=\"String\">" << (ts->data(*itDat).toBool()?'T':'F') << "</Data></Cell>" << endl; 
					break;
				}
				case (TimeSeries::T_TRENDMETHOD) :
				{
					out << "    <Cell><Data ss:Type=\"String\">" << TimeSeries::name(ts->trendMethod()) << "</Data></Cell>" << endl; 
					break;
				}
				case (TimeSeries::T_TRENDSTEPS) :
				{
					out << "    <Cell><Data ss:Type=\"String\">" << TimeSeries::name(ts->trendSteps()) << "</Data></Cell>" << endl; 
					break;
				}
				case (TimeSeries::T_ABSREL) :
				{
					out << "    <Cell><Data ss:Type=\"String\">" << TimeSeries::name(ts->absRel()) << "</Data></Cell>" << endl; 
					break;
				}
				default:{}
			}
		}
		out << "  </Row>" << endl;
	}
	out << "  </Table>" << endl;
	out << " </Worksheet>" << endl;
	out << "</Workbook>" << endl;
	file.close();
}

void cs2::exportSingle()
{
	QModelIndex idx = table->currentIndex();
	if (!idx.isValid())
	{
		QMessageBox::warning(this, "No sample selected", "You have to select a sample first to save it.");
		return;
	}
	TimeSeries *ts = tsList->ts(idx);
	QString filter = "CSV (*.csv)";
	QString fileName = QFileDialog::getSaveFileName(this, QString("Export Sample %1").arg(ts->sample()), lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".csv")) fileName += ".csv";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	
	// HEADER
	out << "t\tRaw\tSmooth\tTrend\tDetrended\tFit" << endl;
	for (int tNr = 0; tNr < ts->nRaw(); tNr++)
	{
		out << ts->tRawDays(tNr)*24.0 << "\t" << ts->dRaw(tNr) << "\t";
		if (tNr >= ts->skipSmoothBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dSmooth(tNr-ts->skipSmoothBegin());
		out << "\t";
		if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dTrend(tNr-ts->skipTrendBegin());
		out << "\t";
		if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dDetrended(tNr-ts->skipTrendBegin());
		out << "\t";
		if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dFit(tNr-ts->skipTrendBegin());
		out << endl;
	}
	file.close();
}
void cs2::exportTrend()
{
	// OPEN FILE
	if (!checkExportIntegrity()) return;
	TimeSeries *tsRef = selected[0];
	QString filter = "CSV (*.csv)";
	QString fileName = QFileDialog::getSaveFileName(this, "Export Trends", lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".csv")) fileName += ".csv";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	
	// HEADER
	QModelIndexList mil = table->selectionModel()->selectedRows();
	out << "t";
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		out << "\t" << tsList->ts(*it)->sample();
	}
	out << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << tsRef->tRawDays(tNr)*24.0;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			out << "\t";
			if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dTrend(tNr-ts->skipTrendBegin());
		}
		out << endl;
	}
	file.close();
}
void cs2::exportDetrended()
{
	// OPEN FILE
	if (!checkExportIntegrity()) return;
	TimeSeries *tsRef = selected[0];
	QString filter = "CSV (*.csv)";
	QString fileName = QFileDialog::getSaveFileName(this, "Export Detrended", lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".csv")) fileName += ".csv";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	
	// HEADER
	QModelIndexList mil = table->selectionModel()->selectedRows();
	out << "t";
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		out << "\t" << tsList->ts(*it)->sample();
	}
	out << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << tsRef->tRawDays(tNr)*24.0;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			out << "\t";
			if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dDetrended(tNr-ts->skipTrendBegin());
		}
		out << endl;
	}
	file.close();
}
void cs2::exportFit()
{
	// OPEN FILE
	if (!checkExportIntegrity()) return;
	TimeSeries *tsRef = selected[0];
	QString filter = "CSV (*.csv)";
	QString fileName = QFileDialog::getSaveFileName(this, "Export Fits", lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".csv")) fileName += ".csv";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	
	// HEADER
	QModelIndexList mil = table->selectionModel()->selectedRows();
	out << "t";
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		out << "\t" << tsList->ts(*it)->sample();
	}
	out << endl;
	for (int tNr = 0; tNr < tsRef->nRaw(); tNr++)
	{
		out << tsRef->tRawDays(tNr)*24.0;
		for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
		{
			TimeSeries *ts = tsList->ts(*it);
			out << "\t";
			if (tNr >= ts->skipTrendBegin() && tNr < ts->nRaw()-ts->skipTrendEnd()) out << ts->dFit(tNr-ts->skipTrendBegin());
		}
		out << endl;
	}
	file.close();
}
void cs2::exportParam()
{
	// OPEN FILE
	if (!checkExportIntegrity()) return;
	QString filter = "CSV (*.csv)";
	QString fileName = QFileDialog::getSaveFileName(this, "Export Parameters", lastOpenedPath(), filter, &filter);
	if (fileName.isEmpty()) return;
	if (!fileName.endsWith(".csv")) fileName += ".csv";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, "Error opening file", QString("Could not open file %1").arg(fileName));
		return;
	}
	QTextStream out(&file);
	
	// HEADER
	QModelIndexList mil = table->selectionModel()->selectedRows();
	out << "Sample\tComment\tSmooth\tMethod\tTrendFrom\tTrendTo\tPolyDeg\tFouDeg\tFouPer\tAmp\tPer\tPha\tDamp\tFVU" << endl;
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		TimeSeries *ts = tsList->ts(*it);
		out << ts->sample() <<"\t" << ts->comment() << "\t" << ts->smoothWidth() << "\t" << TimeSeries::name(ts->trendMethod()) << "\t" << ts->trendFrom_h() << "\t" << ts->trendTo_h() << "\t"
			<< ts->polyDegree() << "\t" << ts->fouDegree() << "\t" << ts->fouShortPer_h() << "\t" 
			<< ts->amp() << "\t" << 24.0*2*M_PI/ts->phi() << "\t" << 24.0*ts->theta()/(2*M_PI) << "\t" << ts->alpha() << "\t" << ts->fvu() << endl;
	}
	file.close();
}

void cs2::rowsAboutToBeRemoved(const QModelIndex &, int begin, int end)
{
	for (int i = begin; i <= end; i++) 
	{
		TimeSeries *ts = tsList->ts(i);
		ts->hide();
		selected.removeAll(ts);
	}
	statusBar()->showMessage(QString("TimeSeries deleted."));
}

// void cs2::setViewDiv(bool viewDiv)
// {
// 	tsList->setViewDiv(viewDiv);
// 	plotFit->replot();
// }

void cs2::blockSignals(bool block)
{
	ctrlAbsCounts->blockSignals(block);
	ctrlAmpRef->blockSignals(block);
	ctrlFitLog->blockSignals(block);
	ctrlFouDeg->blockSignals(block);
	ctrlMeanHours->blockSignals(block);
	ctrlMethFou->blockSignals(block);
	ctrlMethMean->blockSignals(block);
	ctrlMethNone->blockSignals(block);
	ctrlMethPoly->blockSignals(block);
	ctrlOneStep->blockSignals(block);
	ctrlPolDeg->blockSignals(block);
	ctrlRelCounts->blockSignals(block);
	ctrlTrendFrom->blockSignals(block);
	ctrlTrendTo->blockSignals(block);
	ctrlTwoSteps->blockSignals(block);
	ctrlSmooth->blockSignals(block);
}
void cs2::updateSelected() // nach selectionChanged, layoutChanged
{
	// blockSignals() to prevent signal from control panel to set values
	blockSignals(true);
	if (!selected.empty())
	{
		if (dockRaw->widget() == NULL) dockRaw->setWidget(plotRaw); // show if empty before
		if (dockFit->widget() == NULL) dockFit->setWidget(plotFit); // show if empty before
		TimeSeries *ref = tsList->ts(selectedSingleRows()[0]);
		ctrlSmooth->setValue(ref->smoothWidth());
		ctrlFitLog->setChecked(ref->fitLog());
		ctrlMeanHours->setValue(static_cast<int>(round(ref->windowWidth_h())));
		ctrlPolDeg->setValue(ref->polyDegree());
		ctrlFouDeg->setValue(ref->fouDegree());
		bool equalSmooth = true;
		bool equalTrendFrom = true;
		bool equalTrendTo = true;
		bool equalMethod = true;
		bool equalMeanH = true;
		bool equalPolDeg = true;
		bool equalFouDeg = true;
		bool equalFouPer = true;
		bool equalSteps = true;
		bool equalFitLog = true;
		bool equalAbsRel = true;
		bool equalAmpRef = true;
		int minFirst = ref->firstHour();
		int maxLast  = ref->lastHour();
		for (QList<TimeSeries*>::const_iterator it = selected.begin(); it < selected.end(); it++) // start with begin(), not begin()+1 since begin() does not have to be the top one!
		{
			minFirst = min(minFirst, (*it)->firstHour());
			maxLast  = max(maxLast,  (*it)->lastHour());
			equalSmooth &= ((*it)->smoothWidth() == ref->smoothWidth());
			equalFitLog &= ((*it)->fitLog() == ref->fitLog());
			equalMethod &= ((*it)->trendMethod() == ref->trendMethod());
			equalSteps  &= ((*it)->trendSteps() == ref->trendSteps());
			equalAbsRel &= ((*it)->absRel() == ref->absRel());
			equalPolDeg &= ((*it)->polyDegree() == ref->polyDegree());
			equalFouDeg &= ((*it)->fouDegree() == ref->fouDegree());
			equalMeanH &= (fabs((*it)->windowWidth_h() - ref->windowWidth_h()) < 0.25);
			equalTrendFrom &= (fabs((*it)->trendFrom_h() - ref->trendFrom_h()) < 0.25);
			equalTrendTo &= (fabs((*it)->trendTo_h() - ref->trendTo_h()) < 0.25);
			equalFouPer &= (fabs((*it)->fouShortPer_h() - ref->fouShortPer_h()) < 0.25);
			equalAmpRef &= (fabs((*it)->ampRef() - ref->ampRef()) < 0.25);
		}
		ctrlTrendFrom->setRange(minFirst, maxLast);
		ctrlTrendTo->setRange(minFirst, maxLast);
		setBackground(ctrlSmooth, equalSmooth);
		setBackground(ctrlTrendFrom, equalTrendFrom);
		setBackground(ctrlTrendTo, equalTrendTo);
		setBackground(ctrlMeanHours, equalMeanH);
		setBackground(ctrlPolDeg, equalPolDeg);
		setBackground(ctrlFouDeg, equalFouDeg);
		setBackground(ctrlAmpRef, equalAmpRef);
		if (equalMethod) 
		{
			if (ref->trendMethod()==TimeSeries::NONE) ctrlMethNone->setChecked(true);
			if (ref->trendMethod()==TimeSeries::MEAN) ctrlMethMean->setChecked(true);
			if (ref->trendMethod()==TimeSeries::POLY) ctrlMethPoly->setChecked(true);
			if (ref->trendMethod()==TimeSeries::FOU) ctrlMethFou->setChecked(true);
		}
		else
		{
			ctrlMethNone->setChecked(false);
			ctrlMethMean->setChecked(false);
			ctrlMethPoly->setChecked(false);
			ctrlMethFou->setChecked(false);
		}
		if (equalSteps)
		{
			if (ref->trendSteps()==TimeSeries::ONESTEP) ctrlOneStep->setChecked(true);
			if (ref->trendSteps()==TimeSeries::TWOSTEPS) ctrlTwoSteps->setChecked(true);
		}
		else
		{
			ctrlOneStep->setAutoExclusive(false);
			ctrlTwoSteps->setAutoExclusive(false);
			ctrlOneStep->setChecked(false);
			ctrlTwoSteps->setChecked(false);
			ctrlOneStep->setAutoExclusive(true);
			ctrlTwoSteps->setAutoExclusive(true);
		}
		if (equalAbsRel)
		{
			if (ref->absRel()==TimeSeries::ABS) ctrlAbsCounts->setChecked(true);
			if (ref->absRel()==TimeSeries::REL) ctrlRelCounts->setChecked(true);
		}
		else
		{
			ctrlAbsCounts->setAutoExclusive(false);
			ctrlRelCounts->setAutoExclusive(false);
			ctrlAbsCounts->setChecked(false);
			ctrlRelCounts->setChecked(false);
			ctrlAbsCounts->setAutoExclusive(true);
			ctrlRelCounts->setAutoExclusive(true);
		}
		if (equalFitLog) {ctrlFitLog->setTristate(false); ctrlFitLog->setChecked(ref->fitLog());}
		else {ctrlFitLog->setTristate(true); ctrlFitLog->setCheckState(Qt::PartiallyChecked);}
		if (ref->trendFrom_h() >= 0) ctrlTrendFrom->setValue(static_cast<int>(ref->trendFrom_h()));
		else ctrlTrendFrom->setValue(static_cast<int>(minFirst));
		if (ref->trendTo_h() >= 0) ctrlTrendTo->setValue(static_cast<int>(ref->trendTo_h()));
		else ctrlTrendTo->setValue(maxLast);
	}
	if (selected.empty())
	{
		ctrlFitLog->setTristate(true); ctrlFitLog->setCheckState(Qt::PartiallyChecked);
		dockRaw->setWidget(NULL);
		dockFit->setWidget(NULL);
	}
	plotRaw->replot();
	plotFit->replot();
	// blockSignals(false) to restore control panel behavior
	ctrlSmooth->setEnabled(!selected.isEmpty());
	ctrlTrendFrom->setEnabled(!selected.isEmpty());
	ctrlTrendTo->setEnabled(!selected.isEmpty());
	ctrlMethNone->setEnabled(!selected.isEmpty());
	ctrlMethMean->setEnabled(!selected.isEmpty());
	ctrlMethPoly->setEnabled(!selected.isEmpty());
	ctrlMethFou->setEnabled(!selected.isEmpty());
	ctrlOneStep->setEnabled(!selected.isEmpty());
	ctrlTwoSteps->setEnabled(!selected.isEmpty());
	ctrlFitLog->setEnabled(!selected.isEmpty());
	ctrlMeanHours->setEnabled(!selected.isEmpty());
	ctrlPolDeg->setEnabled(!selected.isEmpty());
	ctrlFouDeg->setEnabled(!selected.isEmpty());
	ctrlAmpRef->setEnabled(!selected.isEmpty());
	ctrlAbsCounts->setEnabled(!selected.isEmpty());
	ctrlRelCounts->setEnabled(!selected.isEmpty());
	blockSignals(false);
}

void cs2::selectionChanged(const QItemSelection& sel, const QItemSelection& desel)
{
	QModelIndexList mil;
	mil = sel.indexes();
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) if (it->column() == 0) 
	{
		TimeSeries *ts = tsList->ts(*it);
		selected.push_back(ts);
		ts->show(plotRaw, plotFit);
	}
	mil = desel.indexes();
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) if (it->column() == 0)
	{
		TimeSeries *ts = tsList->ts(*it);
		selected.removeAll(ts);
		ts->hide();
	}
	updateSelected();
}

void cs2::layoutChanged()
{
	QList<TimeSeries*> newSel;
	QModelIndexList mil = table->selectionModel()->selectedRows();
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++)
	{
		TimeSeries *ts = tsList->ts(*it);
		newSel.push_back(ts);
	}
	for (QList<TimeSeries*>::const_iterator it = selected.begin(); it < selected.end(); it++)
	{
		if (!newSel.contains(*it)) (*it)->hide();
	}
	for (QList<TimeSeries*>::const_iterator it = newSel.begin(); it < newSel.end(); it++)
	{
		if (!selected.contains(*it)) (*it)->show(plotRaw, plotFit);
	}
	selected = newSel;
	updateSelected();
}

const QList<int> cs2::selectedSingleRows() 
{
	QList<int> rows;
	QModelIndexList mil = table->selectionModel()->selectedRows();
	for (QModelIndexList::const_iterator it = mil.begin(); it < mil.end(); it++) rows.append(it->row());
	std::sort(rows.begin(), rows.end());
	return rows;
}

const QList<QList<int> > cs2::selectedContinuousRows()
{
	QList<QList<int> > result;
	QList<int> part;
	QList<int> sel = selectedSingleRows();
	for (QList<int>::const_iterator it = sel.begin(); it < sel.end(); it++)
	{
		if (part.size() > 0 && part.last() != *it-1)
		{
			result.append(part);
			part.clear();
		}
		part.append(*it);
	}
	result.append(part);
	return result;
}

void cs2::curveChanged()
{
	plotRaw->replot();
	plotFit->replot();
}

void cs2::controlPanelValueChanged(TimeSeries::Data d, const QVariant &newValue) // signal emitted from TSList after change (withion table OR control panel)
{
	statusBar()->showMessage(QString("Workbench modified."));
#pragma GCC diagnostic ignored "-Wswitch-enum"
	switch (d)
	{
		case (TimeSeries::SMOOTH) : 
		{
			ctrlSmooth->blockSignals(true); // prevent signal from control panel
			ctrlSmooth->setValue(newValue.toInt()); 
			ctrlSmooth->blockSignals(false); // restore control panel behavior
			setBackground(ctrlSmooth, true);
			break;
		}
		case (TimeSeries::FITLOG) : 
		{
			ctrlFitLog->blockSignals(true); // prevent signal from control panel
			ctrlFitLog->setTristate(false);
			ctrlFitLog->setChecked(newValue.toBool());
			ctrlFitLog->blockSignals(false); // restore control panel behavior
			break;
		}
		case (TimeSeries::TRENDFROM) : 
		{
			ctrlTrendFrom->blockSignals(true); // prevent signal from control panel
			ctrlTrendFrom->setValue(newValue.toInt());
			ctrlTrendFrom->blockSignals(false); // restore control panel behavior
			setBackground(ctrlTrendFrom, true);
			break;
		}
		case (TimeSeries::TRENDTO) : 
		{
			ctrlTrendTo->blockSignals(true); // prevent signal from control panel
			ctrlTrendTo->setValue(newValue.toInt());
			ctrlTrendTo->blockSignals(false); // restore control panel behavior
			setBackground(ctrlTrendTo, true);
			break;
		}
		case (TimeSeries::POLYDEG) : 
		{
			ctrlPolDeg->blockSignals(true); // prevent signal from control panel
			ctrlPolDeg->setValue(newValue.toInt());
			ctrlPolDeg->blockSignals(false); // restore control panel behavior
			setBackground(ctrlPolDeg, true);
			break;
		}
		case (TimeSeries::FOUDEG) : 
		{
			ctrlFouDeg->blockSignals(true); // prevent signal from control panel
			ctrlFouDeg->setValue(newValue.toInt());
			ctrlFouDeg->blockSignals(false); // restore control panel behavior
			setBackground(ctrlFouDeg, true);
			break;
		}
		default : break;
	}
#pragma GCC diagnostic pop
}

void cs2::del()
{
	tsList->remove(selectedContinuousRows());
}

void cs2::duplicateSingle()
{
	tsList->insertSingle(selectedSingleRows());
}

void cs2::duplicateBlock()
{
	tsList->insertBlock(selectedContinuousRows());
}

void cs2::ctrlSetMethodNone(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDMETHOD, TimeSeries::NONE);
}
void cs2::ctrlSetMethodMean(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDMETHOD, TimeSeries::MEAN);
}
void cs2::ctrlSetMethodPoly(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDMETHOD, TimeSeries::POLY);
}
void cs2::ctrlSetMethodFou(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDMETHOD, TimeSeries::FOU);
}
void cs2::ctrlSetOneStep(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDSTEPS, TimeSeries::ONESTEP);
}
void cs2::ctrlSetTwoSteps(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::TRENDSTEPS, TimeSeries::TWOSTEPS);
}
void cs2::ctrlSetAbs(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::ABSREL, TimeSeries::ABS);
}
void cs2::ctrlSetRel(bool checked)
{
	if (checked) tsList->setData(selectedContinuousRows(), TimeSeries::ABSREL, TimeSeries::REL);
}
void cs2::ctrlChangeAmpRef(int newAmpRef)
{
	tsList->setData(selectedContinuousRows(), TimeSeries::AMPREF, newAmpRef);
}
void cs2::ctrlChangeMeanH(int newMeanH)
{
	tsList->setData(selectedContinuousRows(), TimeSeries::MEANWIDTH, newMeanH);
}

// void cs2::keyPressEvent(QKeyEvent *ev)
// {
// 	//if (ev->key() == Qt::Key_Delete || (ev->key() == Qt::Key_X && ev->modifiers()&Qt::ControlModifier)) delete();
// 	//if (ev->key() == Qt::Key_D && ev->modifiers()&Qt::ControlModifier) duplicateSingle();
// 	//if (ev->key() == Qt::Key_V && ev->modifiers()&Qt::ControlModifier) duplicateBlock();
// }
