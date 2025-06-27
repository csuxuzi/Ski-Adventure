#ifndef PLACEHOLDERDIALOG_H
#define PLACEHOLDERDIALOG_H

#include <QDialog>

class PlaceholderDialog : public QDialog
{
public:
    explicit PlaceholderDialog(const QString& title, QWidget* parent = nullptr);
};

#endif // PLACEHOLDERDIALOG_H
