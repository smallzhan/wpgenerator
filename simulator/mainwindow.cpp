#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wpmanager.h"
#include "wpencoder.h"

#include <QtGui/QPainter>
#include <QFileDialog>
#include <QTimer>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    line_wait_time_(0),
    map_wait_time_(0),
    map_seq_(0),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open water printer simulation files"),
                                "",
                                tr("wpsim files (*.wpsim)"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty())
    {
        loadScript(fileName);
    }

}

void MainWindow::on_actionEdit_triggered()
{

}

void MainWindow::loadScript(const QString& conf_file)
{
    WaterPrinter::WPManager manager(conf_file.toStdString().c_str());
    manager.loadConf();


    line_wait_time_ = manager.line_wait_time_;
    map_wait_time_ = manager.map_wait_time_;

    //printf("l_w_t = %d, m_w_t = %d\n", line_wait_time_, map_wait_time_);
    std::string conf_str(conf_file.toStdString());
    size_t last_slash;
    wpfiles_.clear();
    for (std::vector<std::string>::const_iterator it = manager.fnames_.begin();
         it != manager.fnames_.end(); ++it)
    {
        if ((last_slash = conf_str.find_last_of("/")) != std::string::npos)
        {
            std::string fname = conf_str.substr(0, last_slash + 1) + (*it) + ".3dwp";
            wpfiles_.push_back(fname);
            //printf("put the file %s in the vector wpfiles\n", fname.c_str());
        }
    }
    map_seq_ = 0;

    timerDone();

}

void MainWindow::sendNewMap()
{
     QTimer::singleShot(map_wait_time_ / 1000, this, SLOT(timerDone()));
}

void MainWindow::timerDone()
{
    WaterPrinter::WPEncoder encoder;
    encoder.setCryptType(false);
    encoder.setOutfile(wpfiles_[map_seq_]);
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

    ui_->showWidget->setWaterDrop(QString::fromStdString(sstr.str()));
    ++ map_seq_;
    if (map_seq_ == wpfiles_.size())
    {
        map_seq_ = 0;
    }
}
