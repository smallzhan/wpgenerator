#ifndef SHOWAREA_H
#define SHOWAREA_H
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

//! [0]
class QTimer;

class ShowArea : public QWidget
{
    Q_OBJECT

public:

    ShowArea(QWidget *parent = 0);
    ~ShowArea();

    void stopWaterDrop();

signals:
    void mapOver();

public slots:
    void setWaterDrop(const QString& bitmap);

    void timerDone();


    //

protected:
    void paintEvent(QPaintEvent *event);

private:
    int waterdrop_;
    QList<QPoint> pointlist_;

    int col_;
    int row_;

    bool show_static_;

    QTimer *p_timer_;

    QString show_strs_;
};
#endif // SHOWAREA_H
