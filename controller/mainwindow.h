#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace WaterPrinter
{
    class WPSocket;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    void connectServer();
    void disconnServer();
    void setPause();
    void setPlay();
    void setSendFile();
    void editSchema();

    void sendTimeTick();
    void sendSchema_1();
    void sendSchema_2();
    void sendSchema_3();

    void setSketch();

private:
    Ui::MainWindow *ui;

    bool is_connected_;


    QString work_dir_;
    WaterPrinter::WPSocket *socket_;

    QString conf_name_;

    void readSchema();
    void sendSchemaString(const QString &str);
    void sendFile(const QString &fname);
    void transformImage(const QString &image, const QString &wpfile);

    QString timetick_;
    QString sketch_;
    QString schema_1_;
    QString schema_2_;
    QString schema_3_;

    QString sketch_name_;

    QTimer * p_timer_;

 private slots:
    void timerDone();

};

#endif // MAINWINDOW_H
