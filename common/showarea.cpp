#include <QtGui>
#include <iostream>
#include <QTimer>

#include "showarea.h"
#include "wpdefines.h"


ShowArea::ShowArea(QWidget *parent)
    : QWidget(parent)
{
    waterdrop_ = 0;

    col_ = WaterPrinter::g_col;
    row_ = WaterPrinter::g_row;

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    p_timer_ = new QTimer(this);

    connect(p_timer_, SIGNAL(timeout()), this, SLOT(timerDone()));
}

ShowArea::~ShowArea()
{
    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }

    delete(p_timer_);
}

void ShowArea::timerDone()
{

    row_ = show_strs_.size() / col_;

    if (waterdrop_ > row_ + WaterPrinter::g_row)
    {
        waterdrop_ = 0;
        emit mapOver();
        printf("timerDone, mapOver...\n");
        //return;
        if (p_timer_->isActive())
        {
            p_timer_->stop();
        }
    }
    else
    {

        float h_step = width() * 1.0 / WaterPrinter::g_col;
        float v_step = height() * 1.0 / WaterPrinter::g_row / WaterPrinter::g_row;
        std::string str = show_strs_.toStdString();
        //std::cout << "str length : " << str.size() << std::endl;
        pointlist_.clear();

        int beg = row_ - waterdrop_;
        int end = beg + WaterPrinter::g_row;

        for (int i = beg; i < end; ++i)
        {
            for (int j = 0; j < WaterPrinter::g_col; ++j)
            {
                if (i < 0 || i >= row_)
                {
                    continue;
                }
                else if (str[i * WaterPrinter::g_col + j] == '1')
                {
                    QPoint p ((j + 0.5) * h_step,
                              ((i - beg) * (i - beg) + 0.5) * v_step);
                    pointlist_.append(p);
                }
            }
        }

        //update();
        if (p_timer_->isActive())
        {
            p_timer_->stop();
        }
        update();
        p_timer_->start(20);
    }

    //update();
    ++ waterdrop_;
}

void ShowArea::setWaterDrop(const QString& bitmap)
{
    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }

    waterdrop_ = 0;

    show_strs_ = bitmap;

    timerDone();
    p_timer_->start(20);
}

void ShowArea::stopWaterDrop()
{
    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }
    waterdrop_ = 0;
}

void ShowArea::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::black);

    float h_step = width() * 1.0 / WaterPrinter::g_col;
    int radius = (int) h_step / 2;
    for (int i = 0; i < pointlist_.size(); ++i)
    {

        painter.drawEllipse(pointlist_[i], radius, radius);
    }
}
