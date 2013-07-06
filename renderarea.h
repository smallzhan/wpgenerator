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

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void loadImage(const QString& fname);


public slots:
    void setFont(const QFont &font_);
    void setText(const QString &text_);
    void setFontSize(const int sz);
    void setPen(const QPen &pen_);
    void setBrush(const QBrush &brush_);
    void setAntialiased(bool antialiased_);
    void setTransformed(bool transformed_);
    void setLoadImage(const QString& fname);
    void setClearImage();


    //

protected:
    void paintEvent(QPaintEvent *event);

private:
    //Shape shape;
    QPen pen_;
    QFont font_;
    QBrush brush_;
    bool antialiased_;
    bool transformed_;
    QString text_;
    QString fname_;
    bool loadImg_;
    bool clear_;
    int size_;
};
#endif // IMAGEAREA_H
