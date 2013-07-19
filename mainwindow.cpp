#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wpencoder.h"
#include "editdialog.h"

#include <QtGui/QPainter>
#include <QFileDialog>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp; // make the same cv::Mat
     cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
     QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     QImage dest2(dest);
     dest2.detach(); // enforce deep copy
     return dest2;
}

cv::Mat QImage2Mat(QImage const& src)
{
     cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar*)src.bits(), src.bytesPerLine());
     cv::Mat result(tmp); // deep copy just in case (my lack of knowledge with open cv)
     cvtColor(tmp, result, CV_BGR2RGB);
     return result;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    size_(0),
    text_("hello"),
    thres_(0),
    update_(false)
{
    ui->setupUi(this);

    ui->showWidget->setVisible(false);
    ui->showWidget->hide();

    this->resize(this->width(), this->height() - ui->showWidget->height());


    fontfamily_ = ui->fontComboBox->currentFont().family();
    size_ = ui->fontsizeSlider->value();
    thres_ = ui->sizeSpinBox->value();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));
    timer->start(20);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::fontChanged()
{
    QFont font = ui->fontComboBox->currentFont();
    fontfamily_ = font.family();
    //qreal spacing = font.letterSpacing();
    int spacing = ui->fontspaceSlider->value();
    //ui->fontspaceSlider->setValue(spacing);
    font.setLetterSpacing(QFont::PercentageSpacing, spacing + 100);
    ui->spaceSpinBox->setValue((double)(spacing + 100) / 100.0);
    //font.
    ui->widget->setFont(font);
    updateStatus();
}

void MainWindow::textChanged()
{
    text_ = ui->lineEdit->text();
    ui->widget->setText(text_);
    updateStatus();
}

void MainWindow::updateStatus()
{

    update_ = true;
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(fontfamily_ + " (" + QString::number(size_) + "): " + text_);

    if (ui->showButton->isChecked())
    {
        if (!timer->isActive())
        {
            timer->start(20);
        }
    }
}

void MainWindow::getFontSize(int sz)
{
    size_ = sz;
    ui->sizeSpinBox->setValue(int(sz * 255 / 200));
    ui->widget->setFontSize(sz);
    updateStatus();
}

void MainWindow::getSpaceSize(int sz)
{
    //space_size_ = sz;
    ui->spaceSpinBox->setValue(double(sz + 100.0)/ 100.0);
    ui->widget->setSpaceSize(sz + 100);
    updateStatus();
}

void MainWindow::getThres(int thres)
{
    thres_ = thres;
    updateStatus();
}

void MainWindow::clearImage()
{
    ui->widget->setShowType(ImageArea::CLEAR_IMAGE);
    ui->widget->setImage("");
    timer->stop();
}

void MainWindow::timerDone()
{
    bool checked = ui->showButton->isChecked();
    if (checked)
    {
        if (update_)
        {
            transformImage();
            update_ = false;
        }
        bool reverse = ui->reverseBox->isChecked();
        if (reverse)
        {
            ui->showWidget->setWaterDrop(reverse_bitmap_);
        }
        else
        {
            ui->showWidget->setWaterDrop(out_bitmap_);
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open Image Files"),
                                "",
                                tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {
        fname_ = fileName;
        ui->widget->setShowType(ImageArea::LOAD_IMAGE);
        ui->widget->setImage(fileName);
        update_ = true;
        //transformImage();
    }

}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionSave_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("Save WaterPrinter Files"),
                                "out.3dwp",
                                tr("3D WaterPrinter Files (*.3dwp)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {

        //transformImage();

        bool encrypt = ui->encryptBox->isChecked();

        WaterPrinter::WPEncoder encoder;
        encoder.setCryptType(encrypt);
        encoder.setOutfile(fileName.toStdString());
        std::string out_str;
        if (!ui->reverseBox->isChecked())
        {
            out_str = out_bitmap_.toStdString();
        }
        else
        {
            out_str = reverse_bitmap_.toStdString();
        }

        std::stringstream sstr;
        sstr.str("");
        for (size_t i = 0; i < out_str.size(); i += 8)
        {
            char ch = 0;
            for (size_t j = 0; j < 8; ++j)
            {
                ch += (out_str[i+j] - '0') << (7 - j);
            }
            sstr << (char) ch;
        }
        encoder.setBitmap(sstr.str());
        encoder.encodeBits();
    }
}

void MainWindow::showPreview(bool preview)
{
    if (preview)
    {
        update_ = true;
        this->resize(this->width(), this->height() + ui->showWidget->height());
        ui->showWidget->setVisible(true);
    }
    else
    {
        ui->showWidget->setVisible(false);
        this->resize(this->width(), this->height() - ui->showWidget->height());
    }
}

void MainWindow::transformImage()
{
    cv::Mat full_matrix;
    cv::Mat orig_img;

    QPixmap pxmap = ui->widget->getImage();
    QImage image = pxmap.toImage().convertToFormat(QImage::Format_RGB888);
    orig_img = QImage2Mat(image);

    cv::Mat gray_img(orig_img.size(), CV_8U);

    int col = WaterPrinter::g_col;
    int row = WaterPrinter::g_row;
    if (!ui->fixedBox->isChecked())
    {
        row = (int) (col * 1.0/ orig_img.cols * orig_img.rows);
    }

    full_matrix.create(row, col, CV_8U);
    cv::cvtColor(orig_img, gray_img, CV_BGR2GRAY);
    cv::threshold(gray_img, gray_img, thres_, 255, CV_THRESH_BINARY);
    cv::resize(gray_img, full_matrix, cv::Size(col, row));

    std::stringstream sstr;
    sstr.str("");
    for (size_t j = 0; j < full_matrix.rows; ++j)
    {
        uchar * pRow = full_matrix.ptr<uchar>(j);
        for (size_t i = 0; i < full_matrix.cols; ++i)
        {
            if (pRow[i] == 0)
                sstr << '1';
            else
                sstr << '0';
        }

    }
    out_bitmap_ = QString::fromStdString(sstr.str());
    reverseBitmap(out_bitmap_, reverse_bitmap_);
}

void MainWindow::on_actionEditMatrix_triggered()
{
    //transformImage();
    if (!out_bitmap_.isEmpty())
    {
        bool reverse = ui->reverseBox->isChecked();
        EditDialog *editdialog;
        if (reverse)
        {
            editdialog = new EditDialog(reverse_bitmap_);
        }
        else
        {
            editdialog = new EditDialog(out_bitmap_);
        }
        editdialog->showBitmap();
        if (editdialog->exec() == QDialog::Accepted)
        {
            if (reverse)
            {
                reverse_bitmap_ = editdialog->outputString();
                reverseBitmap(reverse_bitmap_, out_bitmap_);
            }
            else
            {
                out_bitmap_ = editdialog->outputString();
                reverseBitmap(out_bitmap_, reverse_bitmap_);
            }
        }
        delete editdialog;
    }

}

void MainWindow::reverseBitmap(const QString& in, QString& out)
{
    out = in;
    for (int i = 0; i < in.size(); ++i)
    {
        QCharRef bit = out[i];
        if (in[i] == '1')
            bit = '0';
        else if (in[i] == '0')
            bit = '1';
    }
}
