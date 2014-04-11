#include "schemaeditor.h"
#include "ui_schemaeditor.h"

#include <QTextStream>

SchemaEditor::SchemaEditor(const QString &conf_name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SchemaEditor)
{
    ui->setupUi(this);

    QFile file(conf_name);
    if (file.open(QFile::ReadWrite))
    {
        QTextStream in(&file);
        QString contents = in.readAll();
        ui->plainTextEdit->setPlainText(contents);
    }
}

QString SchemaEditor::getContents()
{
    return ui->plainTextEdit->toPlainText();
}

SchemaEditor::~SchemaEditor()
{
    delete ui;
}
