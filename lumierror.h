#ifndef LUMIERROR_H
#define LUMIERROR_H

#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wpacked"
#include <QString>
#include <QDebug>
#pragma GCC diagnostic pop

class LumiError
{

public:

	LumiError(const QString &Message) : _message(Message) {qDebug() << "LumiError" << message();}
	const QString &message() const {return _message;}

private:

	QString _message;
};

#endif // LUMIERROR_H
