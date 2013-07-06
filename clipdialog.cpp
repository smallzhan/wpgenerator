#include "clipdialog.h"
#include "ui_clipdialog.h"

ClipDialog::ClipDialog(const QPixmap& pxmap, QWidget *parent) :
    pxmap_(pxmap),
    QDialog(parent),
    ui_(new Ui::ClipDialog)
{
    ui_->setupUi(this);
}

ClipDialog::~ClipDialog()
{
    delete ui_;
}

void ClipDialog::showClipped()
{
    ui_->label->setPixmap(pxmap_.scaled(ui_->label->size(), Qt::KeepAspectRatio));
}
