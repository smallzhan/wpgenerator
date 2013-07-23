#include <QtGui>
#include <iostream>

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
}

void ShowArea::setWaterDrop(const QString& bitmap)
{
    waterdrop_ ++;

    row_ = bitmap.size() / col_;

    if (waterdrop_ > row_ + WaterPrinter::g_row)
    {
        waterdrop_ = 0;
        return;
    }
    else
    {
        float h_step = width() * 1.0 / WaterPrinter::g_col;
        float v_step = height() * 1.0 / WaterPrinter::g_row / WaterPrinter::g_row;
        std::string str = bitmap.toStdString();
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
//        for (int i = 0; i < waterdrop_; ++i)
//        {
//            for (int j = 0; j < 64; ++j)
//            {
//                if (str[(31 - i) * 64 + j] == '1')
//                {
//                    QPoint p((j + 0.5) * h_step,
//                             ((waterdrop_ - i - 1) * (waterdrop_ - i - 1) + 0.5) * v_step);
//                    pointlist_.append(p);
//                }
//            }
//        }
        update();
    }
}

void ShowArea::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::black);

    for (int i = 0; i < pointlist_.size(); ++i)
    {
        painter.drawEllipse(pointlist_[i], 2, 2);
    }
}
