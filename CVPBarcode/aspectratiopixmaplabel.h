#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AspectRatioPixmapLabel(const QPixmap &pixmap, QWidget *parent = 0);
    virtual int heightForWidth(int width) const;
    virtual bool hasHeightForWidth() { return true; }
    virtual QSize sizeHint() const { return pixmap()->size(); }
    virtual QSize minimumSizeHint() const { return QSize(0, 0); }
};

#endif // ASPECTRATIOPIXMAPLABEL_H
