#ifndef SCALEDRAW24_H
#define SCALEDRAW24_H

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wpacked"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wconversion"
#include <qwt/qwt_scale_draw.h>
#pragma GCC diagnostic pop

class ScaleDraw24 : public QwtScaleDraw
{
	virtual QwtText label (double d) const {return QwtScaleDraw::label(d*24.0);}
};

#endif // SCALEDRAW24_H
