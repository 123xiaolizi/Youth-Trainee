#include "logindialog.h"
#include "examdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog w;
    int res = w.exec();
    if(res == QDialog::Accepted)
    {
        Examdialog *examDialog;
        examDialog = new Examdialog(NULL);
    }
    else
    {
        return 0;
    }

    return a.exec();
}
