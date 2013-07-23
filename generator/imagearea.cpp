#include <QtGui>

#include "imagearea.h"
#include "clipdialog.h"

ImageArea::ImageArea(QWidget *parent)
    : QWidget(parent),
      antialiased_(true),
      size_(100),
      type_(IMAGE_NONE)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    this->font_.setWeight(QFont::Black);
    this->font_.setPixelSize(size_);
}


void ImageArea::setShowType(const ImageType &type)
{
    type_  = type;
}

ImageArea::ImageType ImageArea::getShowType() const
{
    return type_;
}

void ImageArea::setPen(const QPen &pen)
{
    this->pen_ = pen;
    //type_ = DRAW_TEXT;
    //update();
}


void ImageArea::setFont(const QFont &font)
{
    this->font_ = font;
    this->font_.setPixelSize(size_);
    this->font_.setLetterSpacing(QFont::PercentageSpacing, space_size_);
    textUpdate();

}


void ImageArea::setBrush(const QBrush &brush)
{
    this->brush_ = brush;
    update();
}

void ImageArea::setAntialiased(bool antialiased)
{
    this->antialiased_ = antialiased;
    update();
}

void ImageArea::setText(const QString &text)
{
    this->text_ = text;
    textUpdate();
}

void ImageArea::setFontSize(const int sz)
{
    this->size_ = sz;
    if (sz > 0)
    {
        this->font_.setPixelSize(sz);
        textUpdate();
    }
}

void ImageArea::setSpaceSize(const int sz)
{
    this->space_size_ = sz;
    if (sz > 0)
    {
        this->font_.setLetterSpacing(QFont::PercentageSpacing, sz);
        textUpdate();
    }
}

void ImageArea::textUpdate()
{
    if (type_ == IMAGE_NONE || type_ == SHOW_TEXT)
    {
        type_ = DRAW_TEXT;
        update();
        pxmap_ = QPixmap::grabWidget(this, QRect(1, 1, width()-2, height()-2));
        //pxmap_.save("pxmap.png");
    }
}

void ImageArea::setImage(const QString& fname)
{
    fname_ = fname;
    update();
}

void ImageArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    QRect full_rect(0, 0, width(), height());

    switch(type_)
    {
    case DRAW_TEXT:
    {
        painter.setFont(font_);
        if (antialiased_)
        {
            painter.setRenderHint(QPainter::Antialiasing, true);
        }
        painter.drawText(full_rect, Qt::AlignCenter, text_);

        type_ = SHOW_TEXT;
        break;
    }
    case LOAD_IMAGE:
    {
        pxmap_ = QPixmap(fname_);
        painter.drawPixmap(full_rect, pxmap_.scaled(this->size(), Qt::KeepAspectRatio));
        type_ = SHOW_IMAGE;
        break;
    }
    case CLIP_IMAGE:
    {
        int topX = beg_.x();
        int topY = beg_.y();
        int w = abs(end_.x() - beg_.x());
        int h = abs(end_.y() - beg_.y());

        if (end_.x() < topX) topX = end_.x();
        if (end_.y() < topY) topY = end_.y();

        QRect clip_rect(topX, topY, w, h);
        painter.setPen(Qt::DashLine);
        painter.drawPixmap(full_rect, pxmap_.scaled(this->size(), Qt::KeepAspectRatio));
        painter.drawRect(clip_rect);
        break;
    }
    case SHOW_IMAGE:
    case SHOW_TEXT:
    {
        painter.drawPixmap(full_rect, pxmap_.scaled(this->size(), Qt::KeepAspectRatio));
        break;
    }
    case CLEAR_IMAGE:
    {
        type_ = IMAGE_NONE;
        break;
    }
    }
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(full_rect);
}


void ImageArea::mousePressEvent(QMouseEvent *event)
{
    if (event->x() >= 0 && event->y() >= 0
            && event->x() <= this->width() && event->y() <= this->height())
    {
        beg_.setX(event->x());
        beg_.setY(event->y());
        end_.setX(event->x());
        end_.setY(event->y());
        type_ = IMAGE_NONE;
    }

}

void ImageArea::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        end_.setX(event->x());
        end_.setY(event->y());

        if ((abs(end_.x() - beg_.x()) > 1) && (abs(end_.y() - beg_.y()) > 1))
        {
            type_ = CLIP_IMAGE;

            update();
        }

    }
}

void ImageArea::mouseReleaseEvent(QMouseEvent *event)
{
    //type_ = IMAGE_NONE;
    end_.setX(event->x());
    end_.setY(event->y());

    int w = abs(end_.x() - beg_.x());
    int h = abs(end_.y() - beg_.y());

    if (w > 10 && h > 10)
    {
        int topX = beg_.x();
        int topY = beg_.y();


        if (end_.x() < topX) topX = end_.x();
        if (end_.y() < topY) topY = end_.y();
        QRect rect(topX, topY, w, h);
        type_ = SHOW_IMAGE;
        QPixmap pxmap = QPixmap::grabWidget(this, rect);
        ClipDialog clipdialog(pxmap);
        clipdialog.showClipped();
        type_ = IMAGE_NONE;

        if (clipdialog.exec() == QDialog::Accepted)
        {
            pxmap_ = pxmap;
            type_ = SHOW_IMAGE;
            update();
            //setImage();
            emit(getFinished());
        }
    }
}
