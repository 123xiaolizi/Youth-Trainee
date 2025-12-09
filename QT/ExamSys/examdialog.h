#ifndef EXAMDIALOG_H
#define EXAMDIALOG_H
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QRadioButton>
#include <QTextEdit>
#include <QPushButton>
#include <QString>
#include <QStringList>
class Examdialog : public QDialog
{
    Q_OBJECT;
public:
    Examdialog(QWidget* parent);
    void initTimer();       //初始化计时器
    bool initTextEdit();    //初始化文本编辑器
    void initButtons();     //初始化按钮及标签
    void initLayout();      //初始化布局管理器
    bool chackBtnClicked(); //检查答题卡是否都选好了
    void getScore();        //计算分数

public slots:
    void inTime();
    void commit();

private:
    int             m_timeNow;      //当前时间
    QTimer          *m_timer;        //计时器

    QTextEdit       *m_textEdit;     //用于显示题目的文本编辑器
    QStringList     m_answerList;   //存储答案


    QGridLayout     *m_layout;       //布局管理器
    QLabel          *m_titleLabels[10];          //题目标签
    QButtonGroup    *m_btnGroups[9];     //单选按钮分组
    QRadioButton    *m_radioBtns[32];    //单选题按钮
    QCheckBox       *m_checkBtns[4];      //多选题按钮
    QRadioButton    *m_radioA;         //判断题A选项
    QRadioButton    *m_radioB;         //判断题B选项
    QPushButton     *m_commitBtn;         //提交按钮

};

#endif // EXAMDIALOG_H
