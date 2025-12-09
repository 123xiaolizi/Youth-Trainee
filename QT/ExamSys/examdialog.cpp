#include "examdialog.h"
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
Examdialog::Examdialog(QWidget* parent):QDialog(parent)
{
    //设置字体
    QFont font;
    font.setPointSize(16);
    setFont(font);
    //设置背景颜色
    setPalette(QPalette(QColor(123,232, 123)));
    resize(400,900);
    setWindowTitle("考试已用时：0 分 0 秒");
    initTimer();
    initLayout();
    if(!initTextEdit()){
        QMessageBox::information(this,"提示","初始化题库数据文件失败！");
        QTimer::singleShot(0,qApp,SLOT(quit()));
    }
    initButtons();
    show();


}

void Examdialog::initTimer()
{
    m_timeNow = 0;
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);//间隔一秒
    m_timer->start();
    connect(m_timer, &QTimer::timeout, this, &Examdialog::inTime);

}

bool Examdialog::initTextEdit()
{
    //m_textEdit = new QTextEdit(this);
    QString fileName("../exam.txt");
    QFile file(fileName);
    QTextStream stream_in(&file);     //文本流
    stream_in.setEncoding(QStringConverter::Utf8);//设置编码格式
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_textEdit = new QTextEdit(this);
        m_textEdit->setReadOnly(true);
        QString dataText;
        int lineNum = 0;
        QString oneLine;        //读取单行
        QStringList stringList; //保存答案
        while(!stream_in.atEnd())
        {

            if(lineNum == 0)//首行跳过
            {
                stream_in.readLine();
                ++lineNum;
                continue;
            }
            //过滤答案行
            if((lineNum >= 6 && lineNum <= 6 * 9 && (lineNum % 6 == 0) )
                || (lineNum == 6 * 9 + 4) )
            {
                oneLine = stream_in.readLine();
                stringList = oneLine.split(" ");
                m_answerList.push_back(stringList.at(1));
                qDebug()<<stringList.at(1);
                dataText += "\n";
                ++lineNum;
                continue;
            }
            oneLine = stream_in.readLine();
            dataText += oneLine + "\n";
            ++lineNum;
        }
        file.close();
        m_textEdit->setText(dataText);
        m_layout->addWidget(m_textEdit, 0, 0, 1, 10);
        return true;
    }
    else
    {
        qDebug()<<"打开文件失败";
        return false;
    }

}

void Examdialog::initButtons()
{

    QString letter("ABCD");
    for(int i = 0; i < 10; ++i)
    {
        //题目标签
        m_titleLabels[i] = new QLabel(this);
        m_titleLabels[i]->setText("第" + QString::number(i+1) + "题");
        m_layout->addWidget(m_titleLabels[i],1,i);
        m_btnGroups[i] = new QButtonGroup(this);
        if(i < 8)//单选
        {
            for(int j = 0; j < 4; ++j)
            {
                m_radioBtns[i*4+j] = new QRadioButton(this);
                m_radioBtns[i*4+j]->setText(letter[j]);
                m_layout->addWidget(m_radioBtns[i*4+j], 2 + j, i);
                m_btnGroups[i]->addButton(m_radioBtns[i*4+j]);
            }

        }
        else if(i == 8)//多选
        {
            for(int j = 0; j < 4; ++j)
            {
                m_checkBtns[j] = new QCheckBox(this);
                m_checkBtns[j]->setText(letter[j]);
                m_layout->addWidget(m_checkBtns[j], 2 + j, i);
                //m_btnGroups[i]->addButton(m_checkBtns[j]);
            }
        }
        else//判断
        {
            m_radioA = new QRadioButton("正确");
            m_radioB = new QRadioButton("错误");
            m_layout->addWidget(m_radioA, 3, i);
            m_layout->addWidget(m_radioB, 4, i);
            m_btnGroups[8]->addButton(m_radioA);
            m_btnGroups[8]->addButton(m_radioB);

        }

    }
    m_commitBtn = new QPushButton(this);
    m_commitBtn->setText("提交");
    m_commitBtn->setEnabled(false);
    connect(m_commitBtn, &QPushButton::clicked, this, &Examdialog::commit);
    m_layout->addWidget(m_commitBtn, 5, 9);
}

void Examdialog::initLayout()
{
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(10);   //设置控件间的间距
    m_layout->setContentsMargins(10, 10, 10, 10); //设置窗体与控件间的间隙 左、上、右、下
}

bool Examdialog::chackBtnClicked()
{
    int selectCount = 0;
    for(int i = 0; i < 8; ++i)
    {
        if(m_btnGroups[i]->checkedButton())
        {
            ++selectCount;
        }
    }
    if(selectCount != 8) return false;
    selectCount = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(m_checkBtns[i]->isChecked())
        {
            ++selectCount;
        }
    }

    if(selectCount == 0 || selectCount == 1) return false;

    if(!m_radioA->isChecked() && !m_radioB->isChecked()) return false;

    return true;
}

void Examdialog::getScore()
{
    int scores = 0;
    for(int i = 0; i < 10; i++)
    {
        //单选题计分
        if( i < 8)
            if(m_btnGroups[i]->checkedButton()->text() == m_answerList.at(i))
                scores += 10;

        //多项选择题计分
        if(i == 8){
            QString answer = m_answerList.at(i);
            bool hasA = false;
            bool hasB = false;
            bool hasC = false;
            bool hasD = false;

            if( answer.contains("A") ) hasA = true;
            if( answer.contains("B") ) hasB = true;
            if( answer.contains("C") ) hasC = true;
            if( answer.contains("D") ) hasD = true;

            bool checkA = m_checkBtns[0]->checkState();
            bool checkB = m_checkBtns[1]->checkState();
            bool checkC = m_checkBtns[2]->checkState();
            bool checkD = m_checkBtns[3]->checkState();

            if( hasA != checkA) continue;
            if( hasB != checkB) continue;
            if( hasC != checkC) continue;
            if( hasD != checkD) continue;

            scores += 10;
        }

        //判断题计分
        if(i == 9){
            if(m_btnGroups[8]->checkedButton()->text() == m_answerList.at(i))
                scores += 10;
        }
    }

    QString str = "您的分数是：" + QString::number(scores) + "分，是否重新考试？";
    int res = QMessageBox::information(this,"提示",str,QMessageBox::Yes | QMessageBox::No);
    if(res == QMessageBox::Yes)
        return;
    else
        close();
}


void Examdialog::inTime()
{
    ++m_timeNow;
    QString min = QString::number(m_timeNow / 60);
    QString sec = QString::number(m_timeNow % 60);
    setWindowTitle("考试已用时：" + min + " 分 " + sec + " 秒");
    if(chackBtnClicked())
    {
        m_commitBtn->setEnabled(true);
    }
}

void Examdialog::commit()
{
    getScore();
}
