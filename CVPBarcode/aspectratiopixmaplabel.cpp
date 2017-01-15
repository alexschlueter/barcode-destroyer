#include "aspectratiopixmaplabel.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(const QPixmap &pixmap, QWidget *parent) :
    QLabel(parent)
{
    QLabel::setPixmap(pixmap);
    setScaledContents(true);
    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    policy.setHeightForWidth(true);
    this->setSizePolicy(policy);
}

int AspectRatioPixmapLabel::heightForWidth(int width) const
{
    if (width > pixmap()->width()) {
        return pixmap()->height();
    } else {
        return ((qreal)pixmap()->height()*width)/pixmap()->width();
    }
}
