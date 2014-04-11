#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schemaeditor.h"

#include "wpsocket.h"
#include "wpencoder.h"
#include "wpdefines.h"

#include <string>
//#include <cstdlib>
#include <cstdio>

#include <sstream>
#include <fstream>

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace WaterPrinter;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket_(NULL),
    is_connected_(false),
    conf_name_("control.conf"),
    sketch_name_("sketch.jpg")
{
    ui->setupUi(this);

    ui->disconnButton->setEnabled(false);

    readSchema();

    p_timer_ = new QTimer(this);

    connect(p_timer_, SIGNAL(timeout()), this, SLOT(timerDone()));


    //socket_ = new TCPSocket;
}

MainWindow::~MainWindow()
{
    if (socket_)
    {
        disconnServer();
        delete socket_;
    }
    delete ui;

    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }

    delete(p_timer_);
}

void MainWindow::connectServer()
{
    QString address = ui->ipLineEdit->text();
    QString port = ui->portLineEdit->text();

    if (address.isEmpty())
    {
        address = "localhost";
    }
    if (port.isEmpty())
    {
        port = "9001";
    }

    if (socket_ == NULL)
    {
        socket_ = new TCPSocket(address.toStdString(), port.toInt());

        std::string peer_addr;
        unsigned short peer_port;
        socket_->getPeerInfo(peer_addr, peer_port);

        QString status = QString("Connected to ") + QString::fromStdString(peer_addr) + " at port " + QString::number(peer_port);
        ui->statusBar->showMessage(status);
        //ui->connButton->setText(QApplication::translate("MainWindow", "断开连接", 0, QApplication::UnicodeUTF8));
        ui->connButton->setEnabled(false);
        ui->disconnButton->setEnabled(true);
        is_connected_ = true;
        char init_msg[16];
        sprintf(init_msg, "#I9@Connected");
        socket_->send(init_msg, 13);
    }
}

void MainWindow::disconnServer()
{
    if (socket_)
    {
        char msg[16];
        sprintf(msg, "#C4@%s", "exit");
        socket_->send(msg, 8);
        delete socket_;
        socket_ = NULL;
        ui->statusBar->showMessage("Disconnect from Server");
        is_connected_ = false;
        //ui->connButton->setText(QApplication::translate("MainWindow", "连接", 0, QApplication::UnicodeUTF8));
        ui->connButton->setEnabled(true);
        ui->disconnButton->setEnabled(false);
    }
}

void MainWindow::setPlay()
{
    if (socket_ && is_connected_)
    {
        char msg[16];
        sprintf(msg, "#C4@%s", "play");
        socket_->send(msg, 8);
    }
    else
    {
        ui->statusBar->showMessage("Connection not established");
    }
}


void MainWindow::setPause()
{
    if (socket_ && is_connected_)
    {
        char msg[16];
        sprintf(msg, "#C5@%s", "pause");
        socket_->send(msg, 9);
    }
    else
    {
        ui->statusBar->showMessage("Connection not established!");
    }

    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }
}

void MainWindow::setSendFile()
{
    if (socket_ && is_connected_)
    {
        QFileDialog::Options options;
        QString selectedFilter;
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open WaterPrinter Files"),
                                                        work_dir_,
                                                        tr("WaterPrinter files (*.3dwp)"),
                                                        &selectedFilter,
                                                        options);
        if (!fileName.isEmpty())
        {
            QDir curr_dir(fileName);
            //ui->statusBar->showMessage("Current dir:" + curr_dir.dirName() );
            work_dir_ = curr_dir.filePath(fileName);

            int ret = QMessageBox::question(NULL, QString("Confirm"), QString("Are you sure to send the file ") + curr_dir.dirName(),
                                            //QApplication::translate("MainWindow", "确定发送该文件？", 0, QApplication::UnicodeUTF8),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (ret == QMessageBox::Yes)
            {

                sendFile(fileName);
            }
        }
    }
}

void MainWindow::editSchema()
{
    ui->editButton->setEnabled(false);
    SchemaEditor sch_editor(conf_name_);
    if (sch_editor.exec() == QDialog::Accepted)
    {
        QString contents = sch_editor.getContents();

        QFile file(conf_name_);
        file.open(QFile::WriteOnly);
        QTextStream out(&file);
        out << contents;
        file.close();
        readSchema();
    }
    ui->editButton->setEnabled(true);

}

void MainWindow::readSchema()
{
    QFile file(conf_name_);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    QString line;
    do {
        line = in.readLine();

        if (line.size() > 3)
        {
            QStringList list = line.split("=", QString::SkipEmptyParts);
            if (list.size() == 2)
            {
                if (list[0].startsWith("timetick"))
                {
                    timetick_ = list[1];
                    //printf("timetick_ = %s\n", timetick_.toStdString().c_str());
                }
                else if (list[0].startsWith("schema_1"))
                {
                    schema_1_ = list[1];
                }
                else if (list[0].startsWith("schema_2"))
                {
                    schema_2_  = list[1];
                }
                else if (list[0].startsWith("schema_3"))
                {
                    schema_3_ = list[1];
                }
            }
        }

    } while(!line.isNull());
}

void MainWindow::sendSchemaString(const QString &str)
{
    if (p_timer_->isActive())
    {
        p_timer_->stop();
    }

    std::string std_str = str.toStdString();

    size_t beg = std_str.find_first_not_of(" \n\t\r");
    size_t end = std_str.find_last_not_of(" \n\t\r");

    std_str = std_str.substr(beg, end - beg + 1);

    std::stringstream sstr;
    sstr.str("");
    sstr << "$L" << std_str.size() << "@" << std_str;

    std::string send_str = sstr.str();
    socket_->send(send_str.c_str(), send_str.size());
}

void MainWindow::sendTimeTick()
{
    sendSchemaString(timetick_);
}

void MainWindow::sendSchema_1()
{
    sendSchemaString(schema_1_);
}

void MainWindow::sendSchema_2()
{
    sendSchemaString(schema_2_);
}

void MainWindow::sendSchema_3()
{
    sendSchemaString(schema_3_);
}

void MainWindow::setSketch()
{
    setPause();

    p_timer_->start(1000); // 1s
}

void MainWindow::sendFile(const QString &fname)
{
    setPause();
    std::ifstream in(fname.toStdString().c_str(), std::ifstream::binary);
    if (in)
    {
        QDir curr_dir(fname);

        in.seekg(0, in.end);
        int length = in.tellg();
        in.seekg(0, in.beg);

        char *buffer = new char[length];
        in.read(buffer, length);

        std::stringstream sstr;
        sstr << "$F" << length + curr_dir.dirName().size() + 1 << "@" << curr_dir.dirName().toStdString()
             << ":" << std::string(buffer, length);

        std::string send_msg = sstr.str();

        ui->statusBar->showMessage(curr_dir.dirName() + " with size " + QString::number(length) + " sended ");
        socket_->send(send_msg.c_str(), send_msg.size());

        in.close();
        delete [] buffer;
    }
}

void MainWindow::timerDone()
{
    QString sketch_file("_sketch.3dwp");
    transformImage(sketch_name_, sketch_file);
    sendFile(sketch_file);
}

void MainWindow::transformImage(const QString &image, const QString &wpfile)
{
    cv::Mat full_matrix;
    cv::Mat orig_img = cv::imread(image.toStdString());

    cv::Mat gray_img(orig_img.size(), CV_8U);

    int col = WaterPrinter::g_col;
    int row = WaterPrinter::g_row;
    //if (!ui->fixedBox->isChecked())
    //{
    //    row = (int) (col * 1.0/ orig_img.cols * orig_img.rows);
    //}

    full_matrix.create(row, col, CV_8U);

    cv::cvtColor(orig_img, gray_img, CV_BGR2GRAY);
    cv::threshold(gray_img, gray_img, 200, 255, CV_THRESH_BINARY);

    printf("gray_img: col = %d, row = %d\n", gray_img.cols, gray_img.rows);

    cv::Mat dot_img(gray_img.size(), CV_8U);

    int cur_row = 0;
    int segment = 4;
    int dot_row = 0;

    //    if (ui->fixedBox->isChecked())
    //    {
    //        int step = gray_img.rows / segment;
    //        for (int i = 0; i < segment; ++i)
    //        {

    //            cv::Rect roi(0, cur_row, gray_img.cols, step);
    //            cv::Mat seg_img = gray_img(roi);

    //            int seg_row = (int) step * 2.0 / (3 * sqrt(i * 1.0) + 2);

    //            cv::Rect roi_2(0, dot_row, gray_img.cols, seg_row);
    //            cv::Mat seg_dot_img = dot_img(roi_2);

    //            cv::Mat after_img(seg_img.size(), CV_8U);
    //            cv::resize(seg_img, after_img, cv::Size(gray_img.cols, seg_row));

    //            after_img.copyTo(seg_dot_img);
    //            cur_row += step;
    //            dot_row += after_img.rows;
    //        }
    //        cv::resize(gray_img, gray_img, cv::Size(gray_img.cols, dot_row));
    //        cv::Rect roi(0, 0, gray_img.cols, dot_row);
    //        //printf("%d-%d\n", col, dot_row );
    //        cv::Mat img = dot_img(roi);
    //        img.copyTo(gray_img);
    //    }

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
            //if (i % 8 == 7)
            //{
            //    sstr << (char) bit;
            //    bit = 0;
            //}
            //bit |= pRow[i] << (7 - i % 8);
                        if (pRow[i] == 0)
                            sstr << '1';
                        else
                            sstr << '0';
        }

    }
    std::string out_str = sstr.str();

    WaterPrinter::WPEncoder encoder;
    //encoder.setCryptType(encrypt);
    encoder.setOutfile(wpfile.toStdString());

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

//out_bitmap = QString::fromStdString(sstr.str());
//reverseBitmap(out_bitmap_, reverse_bitmap_);


