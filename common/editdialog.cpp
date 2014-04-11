#include "editdialog.h"
#include "ui_editdialog.h"

#include "wpdefines.h"

#include <cassert>

#include <QtGui>
#include <QScrollArea>

#include <QMessageBox>

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

QPoint ShowWidget::global2map(const int i, const int j) const
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

//const int ShowWidget::bit4map(const int i, const int j)
//{
//    return (edit_str_[i * col_size_ / 8 + j / 8] >> (7 - j % 8)) & 1;
//}

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
            QChar bit = edit_str_[i * col_size_ + j];

            //QChar bit = bit4map(i, j);
            if (bit == '0')
            {
                painter.setBrush(Qt::lightGray);
                //printer.draw
            }
            else if (bit == '1')
            {
                painter.setBrush(Qt::black);
            }
            else if (bit >= '2')
            {
                painter.setBrush(Qt::darkGray);
            }
            else
            {
                painter.setBrush(Qt::white);
            }
            painter.drawEllipse(point, dot_size_ / 2, dot_size_ / 2);
            //edit_str_[i * g_col + j] = '2';
        }
    }

}

void ShowWidget::drawPoint(const QPoint& point)
{
    QCharRef bit = edit_str_[point.y() * col_size_ + point.x()];
    switch (edit_state_)
    {
    case kEDIT_ADD:
        if (bit == '0')
        {
            bit = '1';
        }
        break;
    case kEDIT_REM:
        if (bit == '1')
        {
            bit = '0';
        }
        break;
    case kEDIT_LINE:
    {
        //QCharRef linebit = edit_str_[point.y() * col_size_];
        for (int i = 0; i < col_size_; ++i)
        {
            QCharRef linebit = edit_str_[point.y() * col_size_ + i];
            //printf("%d:%c->", i, QChar(linebit));
            if (linebit == '0')
            {
                linebit = '2';
            }
            else if (linebit == '1')
            {
                linebit = '3';
            }
            else if (linebit == '2')
            {
                linebit = '0';
            }
            else if (linebit == '3')
            {
                linebit = '1';
            }
            //printf("%d:%c\n", i, QChar(linebit));
        }
    }
        break;
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
            drawPoint(point);
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
        drawPoint(point);
    }
    update();
}

void ShowWidget::lineAddEdit()
{
    int beg = -1;
    int end = -1;
    bool inside = false;

    for (size_t i = 0; i< edit_str_.size(); ++i)
    {
        if (edit_str_[i] >= '2')
        {
            if (!inside)
            {
                beg = i;
                inside = true;
            }
        }
        else if (edit_str_[i] <= '1')
        {
            if (inside)
            {
                end = i;
                inside = false;
            }
        }
    }

    if (inside)
    {
        end = edit_str_.size();
    }

    if (beg != -1)
    {
        QString zeros;
        zeros.resize(col_size_);
        for (size_t i = 0; i < col_size_; ++i)
        {
            zeros[i] = QChar('2');
        }
        edit_str_.insert(end, zeros);
        row_size_ += 1;
    }
    showBitmap();
}

void ShowWidget::lineDelEdit()
{
    std::vector<int> beg;
    std::vector<int> end;
    bool inside = false;
    for (size_t i = 0; i < edit_str_.size(); ++i)
    {
        if (edit_str_[i] >= '2')
        {
            if (!inside)
            {
                beg.push_back(i);
                inside = true;
            }
        }
        else if (edit_str_[i] <= '1')
        {
            if (inside)
            {
                end.push_back(i);
                inside = false;
            }
        }
    }

    if (inside) end.push_back(edit_str_.size());
    assert (beg.size() == end.size());
    if (beg.empty()) return;
    int remove_len = 0;
    for (size_t i = 0; i < beg.size(); ++i)
    {
        edit_str_.remove(beg[i] - remove_len, end[i] - beg[i]);
        remove_len += end[i] - beg[i];
        row_size_ -= (end[i] - beg[i]) / col_size_;
    }

    showBitmap();
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
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->setWidgetResizable(false);

    ui->widget->setString(bitmap);

    width_ = ui->scrollArea->width();
    height_ = ui->scrollArea->height();

    this->resize(this->size().width(), height_ + 2 * g_top_margin);


    connect(ui->lineAdd, SIGNAL(pressed()), ui->widget, SLOT(lineAddEdit()));
    connect(ui->lineRemove, SIGNAL(pressed()), ui->widget, SLOT(lineDelEdit()));

}

EditDialog::~EditDialog()
{

}

void EditDialog::adjustSize()
{
    if (ui->widget->height() < height_)
    {
        ui->scrollArea->resize(width_, ui->widget->height());
        this->resize(this->size().width(), ui->scrollArea->height() + ui->buttonBox->height() + 2 * g_top_margin);
        ui->buttonBox->setGeometry(g_top_margin, ui->scrollArea->height() + g_top_margin,
                                   ui->buttonBox->width(), ui->buttonBox->height());
    }
    else
    {
        ui->scrollArea->resize(width_, height_);
        this->resize(this->width(), height_ + ui->buttonBox->height() + 2 * g_top_margin);
        ui->buttonBox->setGeometry(g_top_margin, height_ + g_top_margin,
                                   ui->buttonBox->width(), ui->buttonBox->height());
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

void EditDialog::markEditLine()
{
    ui->widget->setEditType(ShowWidget::kEDIT_LINE);
}


