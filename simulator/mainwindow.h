#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>

class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void timerDone();
    void sendNewMap();
    


private:

    void loadScript(const QString& fname);

    std::vector<std::string> wpfiles_;

    Ui::MainWindow *ui_;

    QList<QPoint> pointlist_;
    int line_wait_time_;
    int map_wait_time_;

    int map_seq_;
};

#endif // MAINWINDOW_H
