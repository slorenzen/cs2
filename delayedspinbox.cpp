#include "delayedspinbox.h"

#include <QTimer>

DelayedSpinBox::DelayedSpinBox(QWidget* par) : 
	QSpinBox(par),
	newValue(0), 
	timerCount(0)
{
	connect(this, SIGNAL(valueChanged(int)), SLOT(valueChanged(int)));
}

void DelayedSpinBox::valueChanged(int newVal)
{
	newValue = newVal;
	timerCount++;
	QTimer::singleShot(500, this, SLOT(timerFires()));
}

void DelayedSpinBox::timerFires()
{
	timerCount--;
	if (timerCount == 0) emit(delayedChange(newValue));
}
