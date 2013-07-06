#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EditDialog(const QString& bitmap, QWidget *parent = 0);
    ~EditDialog();
    
    QString outputString();
    void showBitmap();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::EditDialog *ui;
    QString edit_str_;
    QPoint select_point_;

    QPoint begin_;
    QPoint center_;

    int col_size_;
    int row_size_;
    int dot_size_;

    QPoint map2global(int i, int j) const;
    QPoint map2global(const QPoint& mpos) const;
    QPoint global2map(const QPoint& gpos) const;
    QPoint global2map(int i, int j) const;
};

#endif // EDITDIALOG_H
