#ifndef FQTERMIMAGE_H
#define FQTERMIMAGE_H

#include <QWidget>
#include <QString>

namespace FQTerm 
{
#define ICON_SOURCE				"pic/ViewerButtons/"
    
    class FQTermImage : public QWidget {

    public:
	FQTermImage(QWidget * parent, Qt::WindowFlags f);
	virtual void adjustItemSize() = 0;
	virtual void scrollTo(const QString &) = 0;
	virtual void updateImage(const QString &) = 0;
    };

}

#endif
