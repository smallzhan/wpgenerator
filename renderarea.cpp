#include <QtGui>

#include "renderarea.h"

//! [0]
RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    //shape = Polygon;
    antialiased_ = false;
    transformed_ = false;
    clear_ = false;
    loadImg_ = false;
    size_ = 0;

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    this->font_.setWeight(QFont::Black);
}
//! [0]

//! [1]
QSize RenderArea::minimumSizeHint() const
{
    return QSize(100, 100);
}
//! [1]

//! [2]
QSize RenderArea::sizeHint() const
{
    return QSize(400, 200);
}
//! [2]

//! [3]

void RenderArea::setPen(const QPen &pen)
{
    this->pen_ = pen;
    update();
}
//! [4]

void RenderArea::setFont(const QFont &font)
{
    this->font_ = font;
    this->font_.setPixelSize(size_);
    update();
}

//! [5]
void RenderArea::setBrush(const QBrush &brush)
{
    this->brush_ = brush;
    update();
}
//! [5]

//! [6]
void RenderArea::setAntialiased(bool antialiased)
{
    this->antialiased_ = antialiased;
    update();
}
//! [6]

void RenderArea::setText(const QString &text)
{
    this->text_ = text;
    update();
}

void RenderArea::setFontSize(const int sz)
{
    this->size_ = sz;
    if (sz > 0)
    {
        this->font_.setPixelSize(sz);
        update();
        //this->font_.setBold(true);
        //this->font_.setWeight(QFont::Black);
    }
}

//! [7]
void RenderArea::setTransformed(bool transformed)
{
    this->transformed_ = transformed;
    update();
}
//! [7]

void RenderArea::setLoadImage(const QString &fname)
{
    loadImg_ = true;
    fname_ = fname;
    update();
}

void RenderArea::setClearImage()
{
    clear_ = true;
    loadImg_ = false;
    update();
}

void RenderArea::loadImage(const QString &fname)
{
    QPainter painter(this);
    QRect target(10, 20, this->width() - 20, this->height() - 40);

    QPixmap pxmap(fname, "jpg");
    QRect source(0, 0, pxmap.width(), pxmap.height());
    painter.drawPixmap(target, pxmap, source);
}

//! [8]
void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    if (clear_)
    {
        clear_ = false;
        return;
    }

    painter.setFont(font_);
    if (antialiased_)
        painter.setRenderHint(QPainter::Antialiasing, true);
    //QRect rect(10, 20, 80, 60);
    QRect rect(10, 20, width() - 20, height() - 40);

    if (loadImg_)
    {
        //loadImg_ = false;
        QPixmap pxmap(fname_);
        painter.drawPixmap(rect, pxmap);
    }
    else
    {
        painter.drawText(rect, Qt::AlignCenter, text_);
    }
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
    //repaint();
}
//! [13]

