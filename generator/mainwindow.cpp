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
#include <cmath>

#include <fstream>

static QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp; // make the same cv::Mat
     cvtColor(src, temp, CV_BGR2RGB);
     QImage dest((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     QImage dest2(dest);
     dest2.detach(); // enforce deep copy
     return dest2;
}

static cv::Mat QImage2Mat(QImage const& src)
{
     cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar*)src.bits(), src.bytesPerLine());
     cv::Mat result(tmp); // deep copy just in case
     cvtColor(tmp, result, CV_BGR2RGB);
     return result;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    size_(0),
    text_("hello"),
    thres_(0)
{
    ui->setupUi(this);

    ui->showWidget->stopWaterDrop();
    ui->showWidget->setVisible(false);
    ui->showWidget->hide();

    this->resize(this->width(), this->height() - ui->showWidget->height());

    fontfamily_ = ui->fontComboBox->currentFont().family();
    size_ = ui->fontsizeSlider->value();
    thres_ = ui->sizeSpinBox->value();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fontChanged()
{
    QFont font = ui->fontComboBox->currentFont();
    fontfamily_ = font.family();

    int spacing = ui->fontspaceSlider->value();
    font.setLetterSpacing(QFont::PercentageSpacing, spacing + 100);
    ui->spaceSpinBox->setValue((double)(spacing + 100) / 100.0);

    int stretch = ui->scaleSlider->value();
    font.setStretch(stretch);
    ui->widget->setFont(font);

    updateAll();
    //update_ = true;
    //updateStatus(true);
}

void MainWindow::textChanged()
{
    text_ = ui->lineEdit->text();
    ui->widget->setText(text_);
    updateAll();
}

void MainWindow::updateStatus()
{
    ui->statusBar->clearMessage();
    ImageArea::ImageType imagetype = ui->widget->getShowType();

    switch (imagetype)
    {
    case ImageArea::LOAD_IMAGE:
        ui->statusBar->showMessage("Load Image: " + fname_ );
        break;
    case ImageArea::SHOW_IMAGE:
        ui->statusBar->showMessage("Show Image: " + fname_ + "with prop" + QString::number(ui->spinBox->value()));
        break;
    case ImageArea::DRAW_TEXT:
    case ImageArea::SHOW_TEXT:
        ui->statusBar->showMessage("Show Text: " + text_ + " in " + fontfamily_ + " with size " + QString::number(size_));
        break;
    case ImageArea::CLIP_IMAGE:
        ui->statusBar->showMessage("Clip Image!!!");
        break;
    case ImageArea::CLEAR_IMAGE:
        ui->statusBar->showMessage("Clear Image.");
        break;
    default:
        ui->statusBar->showMessage("Do Noting, Waiting for texts or images");
        break;
    }

}

void MainWindow::updatePreview()
{

    if (ui->showButton->isChecked())
    {
        if (ui->reverseBox->isChecked())
        {

            ui->showWidget->setWaterDrop(reverse_bitmap_);
        }
        else
        {
            ui->showWidget->setWaterDrop(out_bitmap_);
        }
    }
}

void MainWindow::getFontSize(int sz)
{
    size_ = sz;
    ui->sizeSpinBox->setValue(int(sz * 255 / 200));
    ui->widget->setFontSize(sz);
    updateAll();
}

void MainWindow::getSpaceSize(int sz)
{
    ui->spaceSpinBox->setValue(double(sz + 100.0)/ 100.0);
    ui->widget->setSpaceSize(sz + 100);
    updateAll();
}

void MainWindow::getStretch(int sz)
{
    ui->widget->setStretch(sz);
    updateAll();
}

void MainWindow::getThres(int thres)
{
    thres_ = thres;
    updateAll();
}

void MainWindow::clearImage()
{
    ui->widget->setShowType(ImageArea::CLEAR_IMAGE);
    ui->widget->setImage("");
    ui->showWidget->stopWaterDrop();
}

void MainWindow::timerDone()
{
    QTimer::singleShot(50, this, SLOT(updatePreview()));
}

void MainWindow::updateAll()
{
    updateStatus();
    transformImage();
    updatePreview();
}

void MainWindow::on_actionOpen_triggered()
{
//FIXME: the program crashed when load the image initially.
    //   but it is ok when add some text.

    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open Image Files"),
                                work_dir_,
                                tr("Image files (*.jpg *.jpeg *.png *.gif *.bmp)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {
        QDir curr_dir(fileName);
        work_dir_ = curr_dir.filePath(fileName);
        fname_ = fileName;
        ui->widget->setShowType(ImageArea::LOAD_IMAGE);
        ui->widget->setImage(fileName);
        //ui->widget->update();
        updateAll();
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

void MainWindow::on_actionSaveRaw_triggered()
{
    // FIXME: 16
    const int line_num = 16;
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("Save WaterPrinter Files"),
                                "out.3draw",
                                tr("3D WaterPrinter Files (*.3draw)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {
        std::ofstream sstr(fileName.toStdString().c_str());
        std::string out_str;
        if (!ui->reverseBox->isChecked())
        {
            out_str = out_bitmap_.toStdString();
        }
        else
        {
            out_str = reverse_bitmap_.toStdString();
        }
        for (size_t i = 0; i < out_str.size(); ++i)
        {
//            if (i % WaterPrinter::g_col >= line_num)
//            {
//                if (i < out_str.size() - WaterPrinter::g_col) sstr << "\n";
//                i += WaterPrinter::g_col - line_num - 1;
//                continue;
//            }
//            if (i % line_num == 0) sstr << "0B";
//            sstr << (char)out_str[i];
//            if (i % line_num == 7) sstr << ",0B";
//           // else if (i % 16 == 15 && i < out_str.size() - 64) sstr << "\n";
            sstr << (char)out_str[i];
            if (i % WaterPrinter::g_col == WaterPrinter::g_col - 1)
            {
                sstr << "\n";
            }
            else if (i % line_num == line_num - 1)
            {
                sstr << ",";
            }
        }
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open WaterPrinter Files"),
                                work_dir_,
                                tr("3D WaterPrinter Files (*.3dwp)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {
        QDir curr_dir(fileName);
        work_dir_ = curr_dir.filePath(fileName);
        clearImage();
        bool encrypt = ui->encryptBox->isChecked();

        WaterPrinter::WPEncoder encoder;
        encoder.setCryptType(encrypt);
        encoder.setOutfile(fileName.toStdString());
        encoder.decodeBits();

        std::string bitmap = encoder.getBitmap();

        std::stringstream sstr;
        sstr.str("");

        for (size_t i = 0; i < bitmap.size(); ++i)
        {
            int idx = (bitmap.size() / 8 - i / 8 - 1) * 8 + i % 8;
            for (size_t j = 0; j < 8; ++j)
            {
                if ((bitmap[idx] >> (7-j)) & 0x01)
                    sstr << '1';
                else
                    sstr << '0';
            }
        }
//        const char* pbit = bitmap.data();

//        for (int i = bitmap.size() - 8; i >= 0; i -= 8)
//        {
//            sstr << std::string(pbit + i, 8);
//        }

        out_bitmap_ = QString::fromStdString(sstr.str());
        updatePreview();
    }
}

void MainWindow::showPreview(bool preview)
{
    if (preview)
    {
        //update_ = true;
        this->resize(this->width(), this->height() + ui->showWidget->height());
        ui->showWidget->setVisible(true);
        updatePreview();
    }
    else
    {
        ui->showWidget->setVisible(false);
        this->resize(this->width(), this->height() - ui->showWidget->height());
        ui->showWidget->stopWaterDrop();
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
    int row = (int) (col * 1.0/ orig_img.cols * orig_img.rows);
    if (ui->fixedBox->isChecked())
    {
        int prop = ui->spinBox->value();
        if (prop > 0 ) row = (int) (row / prop);
        if (row < WaterPrinter::g_row / 4) row = WaterPrinter::g_row / 4;
    }

    full_matrix.create(row, col, CV_8U);

    cv::cvtColor(orig_img, gray_img, CV_BGR2GRAY);
    cv::threshold(gray_img, gray_img, thres_, 255, CV_THRESH_BINARY);

    printf("gray_img: col = %d, row = %d\n", gray_img.cols, gray_img.rows);

    cv::Mat dot_img(gray_img.size(), CV_8U);

    int cur_row = 0;
    int segment = 4;
    int dot_row = 0;

    if (!ui->fixedBox->isChecked())
    {
        row = WaterPrinter::g_row;
        int step = gray_img.rows / segment;
        for (int i = 0; i < segment; ++i)
        {

            cv::Rect roi(0, cur_row, gray_img.cols, step);
            cv::Mat seg_img = gray_img(roi);

            int seg_row = (int) step * 2.0 / (3 * sqrt(i * 1.0) + 2);

            cv::Rect roi_2(0, dot_row, gray_img.cols, seg_row);
            cv::Mat seg_dot_img = dot_img(roi_2);

            cv::Mat after_img(seg_img.size(), CV_8U);
            cv::resize(seg_img, after_img, cv::Size(gray_img.cols, seg_row));

            after_img.copyTo(seg_dot_img);
            cur_row += step;
            dot_row += after_img.rows;
        }
        cv::resize(gray_img, gray_img, cv::Size(gray_img.cols, dot_row));
        cv::Rect roi(0, 0, gray_img.cols, dot_row);
        //printf("%d-%d\n", col, dot_row );
        cv::Mat img = dot_img(roi);
        img.copyTo(gray_img);
    }

    size_t top_blank = 0;
    size_t bot_blank = gray_img.rows;

    for (size_t j = 0; j < gray_img.rows; ++j)
    {
        uchar *pRow = gray_img.ptr<uchar>(j);
        for (size_t i = 0; i < gray_img.cols; ++i)
        {
            if (pRow[i] == 0)
            {
                top_blank = j;
                break;
            }
        }
        if (top_blank != 0)
            break;
        //break;
    }
    for (size_t j = gray_img.rows; j > 0; --j)
    {
        uchar *pRow = gray_img.ptr<uchar>(j);
        for (size_t i = 0; i < gray_img.cols; ++i)
        {
            if (pRow[i] == 0)
            {
                bot_blank = gray_img.rows - j;
                break;
            }
        }
        if (bot_blank != gray_img.rows)
            break;
    }

    printf("top_blank = %d, bot_blank = %d, total height = %d\n", top_blank, bot_blank, gray_img.rows);

    cv::Mat recenter;
    recenter.create(gray_img.size(), CV_8U);

    for (size_t j = 0; j < recenter.rows; ++j)
    {
        uchar *pRow = recenter.ptr<uchar>(j);
        for (size_t i = 0; i < recenter.cols; ++i)
        {
            pRow[i] = 255;
        }
    }



    cv::Rect roi(0, (top_blank + bot_blank) / 2, gray_img.cols, gray_img.rows - top_blank - bot_blank);
    cv::Mat center = recenter(roi);

    cv::Rect roi_gray(0, top_blank, gray_img.cols, gray_img.rows - top_blank - bot_blank);
    cv::Mat gray_center = gray_img(roi_gray);

    gray_center.copyTo(center);

    recenter.copyTo(gray_img);


    cv::resize(gray_img, full_matrix, cv::Size(col, row));

    std::stringstream sstr;
    sstr.str("");
    for (size_t j = 0; j < full_matrix.rows; ++j)
    {
        uchar * pRow = full_matrix.ptr<uchar>(j);
        //uchar bit = 0;
        for (size_t i = 0; i < full_matrix.cols; ++i)
        {
//            if (i % 8 == 7)
//            {
//                sstr << (char) bit;
//                bit = 0;
//            }
//            bit |= pRow[i] << (7 - i % 8);
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
    ui->showWidget->stopWaterDrop();
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
    updatePreview();
}

void MainWindow::reverseBitmap(const QString& in, QString& out)
{
    out = in;
    for (int i = 0; i < in.size(); ++i)
    {
        //out[i] = ~in[i];
        QCharRef bit = out[i];
        if (in[i] == '1')
            bit = '0';
        else if (in[i] == '0')
            bit = '1';
    }
}
