#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wpmanager.h"

#include <QtGui/QPainter>
#include <QFileDialog>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    col_(64),
    row_(32),
    line_wait_time_(0),
    map_wait_time_(0),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    p_timer_ = new QTimer(this);
    connect(p_timer_, SIGNAL(timeout()), this, SLOT(timerDone()));
    p_timer_->start(20);
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

}


void MainWindow::paintEvent(QPaintEvent *)
{
    //pointlist_.removeFirst();
}

void MainWindow::timerDone()
{
    update();
}
