#include "editdialog.h"
#include "ui_editdialog.h"

#include "wpdefines.h"

#include <QPainter>
#include <QtGui>
#include <QScrollArea>
#include <QHBoxLayout>

static const int g_top_margin = 10;

ShowWidget::ShowWidget(QWidget *parent) :
    QWidget(parent)
{
    col_size_ = WaterPrinter::g_col;
    row_size_ = WaterPrinter::g_row;
    dot_size_ = WaterPrinter::g_dot_size;

    edit_state_ = kEDIT_ADD;
}


ShowWidget::~ShowWidget()
{

}

void ShowWidget::setString(const QString &bitmap)
{
    edit_str_ = bitmap;
    row_size_ = bitmap.size() / col_size_;
}

QPoint ShowWidget::global2map(const QPoint& gpos) const
{
    return global2map(gpos.x(), gpos.y());
}

QPoint ShowWidget::global2map(int i, int j) const
{
    return QPoint(i / dot_size_,
                  j / dot_size_);
}
QPoint ShowWidget::map2global(const QPoint& mpos) const
{
    return map2global(mpos.x(), mpos.y());
}

QPoint ShowWidget::map2global(int i, int j) const
{
    return QPoint(dot_size_ / 2 + j * dot_size_,
                  dot_size_ / 2 + i * dot_size_);
}


void ShowWidget::showBitmap()
{
    this->resize((col_size_ + 2) * dot_size_, (row_size_ + 1) * dot_size_);
}

void ShowWidget::paintEvent(QPaintEvent *)
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

void ShowWidget::mousePressEvent(QMouseEvent *event)
{
    select_point_ = QPoint(event->x(), event->y());
}

void ShowWidget::mouseMoveEvent(QMouseEvent *event)
{

    static QPoint oldpoint;
    QPoint point = global2map(event->pos());

    if (point != oldpoint)
    {
        if (point.x() >= 0 && point.x() <= col_size_
                &&  point.y() >= 0 && point.y() <= row_size_)
        {
            QCharRef bit = edit_str_[point.y() * col_size_ + point.x()];
            if (edit_state_ == kEDIT_ADD)
            {
                if (bit == '0')
                {
                    bit = '1';
                }
            }
            else if (edit_state_ == kEDIT_REM)
            {
                if (bit == '1')
                {
                    bit = '0';
                }
            }
        }
        update();
        oldpoint = point;
    }
}

void ShowWidget::mouseReleaseEvent(QMouseEvent *event)
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
        if (edit_state_ == kEDIT_ADD)
        {
            if (bit == '0')
            {
                bit = '1';
            }
        }
        else if (edit_state_ == kEDIT_REM)
        {
            if (bit == '1')
            {
                bit = '0';
            }
        }
    }
    update();
}



/////////////////////////////////////////////////////////////
/// \brief EditDialog::EditDialog
/// \param bitmap the bitmap representation of the image
/// \param parent = 0
/////////////////////////////////////////////////////////////

EditDialog::EditDialog(const QString& bitmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDialog)
{
    ui->setupUi(this);

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setWidgetResizable(false);

    ui->widget->setString(bitmap);

    width_ = ui->scrollArea->width();
    height_ = ui->scrollArea->height();

    this->resize(this->size().width(), height_ + 2 * g_top_margin);
}

EditDialog::~EditDialog()
{

}

void EditDialog::adjustSize()
{
    if (ui->widget->height() < height_)
    {
        ui->scrollArea->resize(width_, ui->widget->height());
        this->resize(this->size().width(), ui->scrollArea->height() + 2 * g_top_margin);
    }
    else
    {
        ui->scrollArea->resize(width_, height_);
        this->resize(this->width(), height_ + 2 * g_top_margin);
    }

}

void EditDialog::showBitmap()
{
    ui->widget->showBitmap();
    adjustSize();
}

QString EditDialog::outputString() const
{
    return ui->widget->outputString();
}

void EditDialog::markEditAdd()
{
    ui->widget->setEditType(ShowWidget::kEDIT_ADD);
}

void EditDialog::markEditRemove()
{
    ui->widget->setEditType(ShowWidget::kEDIT_REM);
}


