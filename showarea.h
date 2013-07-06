#ifndef SHOWAREA_H
#define SHOWAREA_H
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

//! [0]
class ShowArea : public QWidget
{
    Q_OBJECT

public:

    ShowArea(QWidget *parent = 0);


public slots:
    void setWaterDrop(const QString& bitmap);
    //

protected:
    void paintEvent(QPaintEvent *event);

private:
    int waterdrop_;
    QList<QPoint> pointlist_;

    int col_;
    int row_;
};
#endif // SHOWAREA_H
