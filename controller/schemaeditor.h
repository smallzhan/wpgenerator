#ifndef MODEEDITOR_H
#define MODEEDITOR_H

#include <QDialog>

namespace Ui {
class SchemaEditor;
}

class SchemaEditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit SchemaEditor(const QString &conf_name, QWidget *parent = 0);
    ~SchemaEditor();

    QString getContents();
    
private:
    Ui::SchemaEditor *ui;
};

#endif // MODEEDITOR_H
