#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

namespace Ui {
class EditDialog;
}

class QScrollArea;
class QHBoxLayout;

class ShowWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ShowWidget(QWidget *parent = 0);
    ~ShowWidget();

    void setString(const QString& bitmap);

    QString outputString() const
    {
        return edit_str_;
    }

    void showBitmap();

    enum EditState
    {
        kEDIT_ADD,
        kEDIT_REM
    };

    void setEditType(EditState state)
    {
        edit_state_ = state;
    }

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


    EditState edit_state_;

private:
    QString edit_str_;
    int col_size_;
    int row_size_;
    int dot_size_;

    QPoint select_point_;

    QPoint map2global(int i, int j) const;
    QPoint map2global(const QPoint& mpos) const;
    QPoint global2map(const QPoint& gpos) const;
    QPoint global2map(int i, int j) const;
};





class EditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditDialog(const QString& bitmap, QWidget *parent = 0);
    ~EditDialog();
    

    void showBitmap();
    QString outputString() const;

public slots:
    void markEditAdd();
    void markEditRemove();

private:
    Ui::EditDialog *ui;
    int width_;
    int height_;

    void adjustSize();
};

#endif // EDITDIALOG_H
