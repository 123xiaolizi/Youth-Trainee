#include "logindialog.h"
#include "ui_logindialog.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setFixedSize(600, 300);
    QLabel* m_lblBg = new QLabel(this);
    m_lblBg->setPixmap(QPixmap(":/login_img.jpg"));
    m_lblBg->setScaledContents(true);//填充
    m_lblBg->resize(this->size());
    //将背景Label置于最底层
    m_lblBg->lower();
    ui->codeEdit->setEchoMode(QLineEdit::Password);
    this->setWindowTitle("科目一模拟考试-登陆");
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginBtn_clicked()
{
    //使用正则表达式判断账号邮箱是否合法
    //元字符解释:^表示规则字符串开始， $表示规则字符串的结束
    //+表示匹配次数>=1次 *表示匹配次数任意， {n,m}表示匹配次数至少n次之多m次
    QRegularExpression re("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if(!re.match(ui->accountEdit->text()).hasMatch())
    {
        QMessageBox::information(this, "提示", "邮箱格式不合法!");
        ui->accountEdit->clear();
        ui->accountEdit->setFocus();//聚焦

    }
    else
    {
        //QMessageBox::information(this, "提示", "欢迎登陆科目一模拟考试系统!");
        QString filename;
        QString accinput;
        QString code;
        QString strLine;
        QStringList strList;
        filename = "../account.txt";
        accinput = ui->accountEdit->text();
        code = ui->codeEdit->text();

        QFile file(filename);
        QTextStream stream(&file);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while(!stream.atEnd())
            {
                strLine = stream.readLine();
                strList =  strLine.split(",");
                if(accinput == strList.at(0))
                {
                    if(code == strList.at(1))
                    {
                        QMessageBox::information(this, "提示", "登陆成功!");
                        return;
                    }
                    else
                    {
                        QMessageBox::information(this, "提示", "密码不正确!");
                        ui->codeEdit->clear();
                        ui->codeEdit->setFocus();
                        return;
                    }
                }
            }
            QMessageBox::information(this, "提示", "账号不正确!");
            ui->accountEdit->clear();
            ui->codeEdit->clear();
            ui->accountEdit->setFocus();
        }
        else
        {
            QMessageBox::information(this, "提示", "打开文件失败!");
        }
    }
}

