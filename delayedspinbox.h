#ifndef DELAYEDSPINBOX_H
#define DELAYEDSPINBOX_H

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include <QSpinBox>
#pragma GCC diagnostic pop

using namespace std;

class DelayedSpinBox : public QSpinBox
{

Q_OBJECT

public:

	DelayedSpinBox(QWidget *parent = NULL);

private slots:

	void valueChanged(int);
	void timerFires();

signals:

	void delayedChange(int);

private:

	int newValue, timerCount;

};

#endif // DELAYEDSPINBOX_H
