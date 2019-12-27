#include "AspectRatioPixmapLabel.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent)
    : QLabel(parent)
{
    this->setMinimumSize(1,1);
    setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap ( const QPixmap & p)
{
    pix = p;
    QLabel::setPixmap(scaledPixmap());
}

int AspectRatioPixmapLabel::heightForWidth( int width ) const
{
    return pix.isNull() ? this->height() : ((qreal)pix.height()*width)/pix.width();
}

int AspectRatioPixmapLabel::widthForHeight(int height) const
{
    return pix.isNull() ? this->width() : ((qreal)pix.width()*height)/pix.height();
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
    if (pix.width() > this->width())
    {
        int w = this->width();
        return QSize( w, heightForWidth(w) );
    }
    else if (pix.height() > this->height())
    {
        int h = this->height();
        return QSize(widthForHeight(h),h);
    }

    return this->size();
}

QPixmap AspectRatioPixmapLabel::scaledPixmap() const
{
    if (pix.width() > this->width() || pix.height() > this->height())
    {
        return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return pix;
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
{
    Q_UNUSED(e);
    if(!pix.isNull())
    {
        QLabel::setPixmap(scaledPixmap());
    }
}
