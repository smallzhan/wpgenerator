#ifndef IMAGEAREA_H
#define IMAGEAREA_H
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

//! [0]
class ImageArea : public QWidget
{
    Q_OBJECT

public:

    ImageArea(QWidget *parent = 0);

    enum ImageType
    {
        IMAGE_NONE = 0,
        DRAW_TEXT,
        SHOW_TEXT,
        LOAD_IMAGE,
        CLIP_IMAGE,
        SHOW_IMAGE,
        CLEAR_IMAGE
    };

    void setShowType(const ImageType& type);
    ImageType getShowType() const;

    QPixmap getImage() const {return pxmap_;}


signals:
    void getFinished(bool);

public slots:
    void setFont(const QFont &font_);
    void setText(const QString &text_);
    void setFontSize(const int sz);
    void setSpaceSize(const int sz);
    void setStretch(const int stretch);
    void setPen(const QPen &pen_);
    void setBrush(const QBrush &brush_);
    void setAntialiased(bool antialiased_);
    void setImage(const QString& fname);


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void textUpdate();

private:

    ImageType type_;

    QPen pen_;
    QFont font_;
    QBrush brush_;
    bool antialiased_;

    QString text_;
    QString fname_;

    int size_;
    int space_size_;
    int stretch_;

    QPoint beg_;
    QPoint end_;

    QPixmap pxmap_;
};
#endif // IMAGEAREA_H
