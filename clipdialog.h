#ifndef CLIPDIALOG_H
#define CLIPDIALOG_H

#include <QDialog>

namespace Ui {
class ClipDialog;
}

class ClipDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ClipDialog(const QPixmap &pxmap, QWidget *parent = 0);
    ~ClipDialog();

    void showClipped();

    QPixmap getClipped()
    {
        return pxmap_;
    }

signals:
    void getStatus(bool state);

    
private:
    Ui::ClipDialog *ui_;
    QPixmap pxmap_;
};

#endif // CLIPDIALOG_H
