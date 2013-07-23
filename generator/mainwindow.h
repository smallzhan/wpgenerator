#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class QPainter;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    void transformImage();

    
public slots:
    void updateStatus();
    void getFontSize(int sz);
    void getSpaceSize(int sz);
    void getThres(int thres);

    void textChanged();
    void fontChanged();

    void clearImage();
    void timerDone();

    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionSave_triggered();
    void on_actionEditMatrix_triggered();

    void showPreview(bool preview);

private:
    Ui::MainWindow *ui;
    QTimer* timer;
    QString out_bitmap_;
    QString reverse_bitmap_;
    int thres_;
    QString fontfamily_;
    QString text_;
    QString fname_;
    int size_;

    bool update_;

    void reverseBitmap(const QString& in, QString& out);
};

#endif // MAINWINDOW_H
