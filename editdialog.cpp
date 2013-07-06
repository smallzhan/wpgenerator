#include "editdialog.h"
#include "ui_editdialog.h"

#include "wpdefines.h"

#include <QPainter>
#include <QtGui>


EditDialog::EditDialog(const QString& bitmap, QWidget *parent) :
    edit_str_(bitmap),
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);
    ui->widget->setMouseTracking(true);
    col_size_ = WaterPrinter::g_col;
    row_size_ = bitmap.size() / col_size_;
    dot_size_ = WaterPrinter::g_dot_size;
}

EditDialog::~EditDialog()
{
    delete ui;
}

QPoint EditDialog::global2map(const QPoint& gpos) const
{
    return global2map(gpos.x(), gpos.y());
}

QPoint EditDialog::global2map(int i, int j) const
{
    return QPoint((i - begin_.x()) / dot_size_,
                  (j - begin_.y()) / dot_size_);
}
QPoint EditDialog::map2global(const QPoint& mpos) const
{
    return map2global(mpos.x(), mpos.y());
}

QPoint EditDialog::map2global(int i, int j) const
{
    return QPoint(begin_.x() + dot_size_ / 2 + j * dot_size_,
                  begin_.y() + dot_size_ / 2 + i * dot_size_);
}


void EditDialog::showBitmap()
{
    ui->widget->resize((col_size_ + 2) * dot_size_, (row_size_ + 2) * dot_size_);
    this->resize(ui->widget->width() + dot_size_ + ui->buttonBox->width(), ui->widget->height() + 2 * dot_size_);
    center_ = QPoint(ui->widget->width() / 2,
                     ui->widget->height() / 2);
    begin_ = QPoint(center_.x() - dot_size_ * col_size_ / 2,
                    center_.y() - dot_size_ * row_size_ / 2);
    update();
}

QString EditDialog::outputString()
{
    return edit_str_;
}

void EditDialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::black);

    for (int i = 0; i < row_size_; ++i)
    {
        for (int j = 0; j < col_size_; ++j)
        {
            QPoint point = map2global(i, j);
            QCharRef bit = edit_str_[i * col_size_ + j];
            if (bit == '1')
            {
                painter.setBrush(Qt::black);

            }
            else if (bit == '0')
            {
                painter.setBrush(Qt::lightGray);
                //printer.draw
            }
            else if (bit >= '2')
            {
                painter.setBrush(Qt::darkGray);
            }
            painter.drawEllipse(point, dot_size_ / 2, dot_size_ / 2);
            //edit_str_[i * g_col + j] = '2';
        }
    }

}

void EditDialog::mousePressEvent(QMouseEvent *event)
{
    select_point_ = QPoint(event->x(), event->y());
}

void EditDialog::mouseMoveEvent(QMouseEvent *event)
{

    static QPoint oldpoint;
    QPoint point = global2map(event->pos());

    if (point != oldpoint)
    {
    if (point.x() >= 0 && point.x() <= col_size_
        &&  point.y() >= 0 && point.y() <= row_size_)
    {
        QCharRef bit = edit_str_[point.y() * col_size_ + point.x()];
        if (bit == '1')
        {
            bit = '0';
        }
        else if (bit == '0')
        {
            bit = '1';
        }
        //edit_str_[point.y() * col_size_ + point.x()] = QChar(bit);
    }
    update();
    oldpoint = point;
    }

}

void EditDialog::mouseReleaseEvent(QMouseEvent *event)
{

    if (event->pos() != select_point_)
    {
        return;
    }

    QPoint point = global2map(event->pos());
    if (point.x() >= 0 && point.x() <= col_size_
        &&  point.y() >= 0 && point.y() <= row_size_)
    {
        QCharRef bit = edit_str_[point.y() * col_size_ + point.x()];
        if (bit == '1')
        {
            bit = '0';
        }
        else if (bit == '0')
        {
            bit = '1';
        }
        else if (bit >= '2')
        {
            QChar cbit = bit.toAscii() - 2;
            bit = QChar(cbit);
        }
    }
    update();
}
