#include "ui/PlaceholderDialog.h"
#include <QLabel>
#include <QVBoxLayout>

PlaceholderDialog::PlaceholderDialog(const QString& title, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(title);
    setFixedSize(400, 200);

    QLabel* label = new QLabel(QString("'%1' 的内容将在这里显示。").arg(title), this);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
}
