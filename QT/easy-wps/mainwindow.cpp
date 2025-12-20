#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "childwnd.h"
#include <QMdiSubWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QColorDialog>
#include <QActionGroup>
#include <QFontDatabase>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMainWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initMainWindow()
{
    //初始化字号列表项
    QFontDatabase fontdb;
    foreach(int fontsize,fontdb.standardSizes())
        ui->sizeComboBox->addItem(QString::number(fontsize));

    QFont defFont;      //当前应用程序默认的字体
    QString sFontSize;
    int defFontSize;    //当前应用程序默认字体的字号
    int defFontindex;   //当前字号在组合框中的索引号

    defFont = QApplication::font();
    defFontSize = defFont.pointSize();
    sFontSize = QString::number(defFontSize);
    defFontindex = ui->sizeComboBox->findText(sFontSize);

    ui->sizeComboBox->setCurrentIndex(defFontindex);

    //设置多文档区域滚动条
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 字体组合框信号连接
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged,
            [this](const QFont &font) {
                textFamily(font.family());
            });

    // 字号组合框信号连接
    connect(ui->sizeComboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::textSize);

    // 刷新菜单和工具栏状态
    refreshMenus();
    connect(ui->mdiArea,&QMdiArea::subWindowActivated,
            this,&MainWindow::refreshMenus);

    // 添加子窗口列表菜单
    addSubWndListMenu();
    connect(ui->menu_W,&QMenu::aboutToShow,
            this,&MainWindow::addSubWndListMenu);

    // 保证对齐操作的互斥性
    QActionGroup *alignGroup = new QActionGroup(this);
    alignGroup->addAction(ui->leftAlignAction);
    alignGroup->addAction(ui->rightAlignAction);
    alignGroup->addAction(ui->centerAction);
    alignGroup->addAction(ui->justifyAction);
}

void MainWindow::docNew()
{
    childwnd *child_wnd = new childwnd();
    ui->mdiArea->addSubWindow(child_wnd);

    // 连接复制可用信号到剪切和复制动作
    connect(child_wnd,SIGNAL(copyAvailable(bool)),
            ui->cutAction,SLOT(setEnabled(bool)));
    connect(child_wnd,SIGNAL(copyAvailable(bool)),
            ui->copyAction,SLOT(setEnabled(bool)));

    child_wnd->newDoc();
    child_wnd->show();
    formatEnabled();
}

void MainWindow::docOpen()
{
    QString docName = QFileDialog::getOpenFileName(this,
                                                   "打开文档",
                                                   "",
                                                   "文本文件(*.txt);;"
                                                   "HTML文件(*.html *.htm);;"
                                                   "所有文件(*.*)");

    if(!docName.isEmpty()){
        QMdiSubWindow *existWnd = findChildWnd(docName);
        if(existWnd)
        {
            ui->mdiArea->setActiveSubWindow(existWnd);
            return;
        }

        childwnd *childWnd = new childwnd;
        ui->mdiArea->addSubWindow(childWnd);

        connect(childWnd,SIGNAL(copyAvailable(bool)),
                ui->cutAction,SLOT(setEnabled(bool)));
        connect(childWnd,SIGNAL(copyAvailable(bool)),
                ui->copyAction,SLOT(setEnabled(bool)));

        if(childWnd->loadDoc(docName)){
            statusBar()->showMessage("文档已打开",3000);
            childWnd->show();
            formatEnabled();
        }else{
            childWnd->close();
        }
    }
}

void MainWindow::docSave()
{
    childwnd* child = activateChildWnd();
    if(child && child->saveDoc())
        statusBar()->showMessage("保存成功",3000);
}

void MainWindow::docSaveAs()
{
    childwnd* child = activateChildWnd();
    if(child && child->saveAsDoc())
        statusBar()->showMessage("保存成功",3000);
}

void MainWindow::docPrint()
{
    childwnd* childWnd = activateChildWnd();
    if(!childWnd) return;

    QPrinter pter(QPrinter::HighResolution);
    QPrintDialog *ddlg = new QPrintDialog(&pter,this);
    ddlg->setOption(QAbstractPrintDialog::PrintSelection,true);
    ddlg->setWindowTitle("打印文档");

    if(ddlg->exec() == QDialog::Accepted)
        childWnd->print(&pter);

    delete ddlg;
}

void MainWindow::docPrintPreview()
{
    childwnd* childWnd = activateChildWnd();
    if(!childWnd) return;

    QPrinter pter;
    QPrintPreviewDialog preview(&pter,this);
    connect(&preview,SIGNAL(paintRequested(QPrinter*)),
            this,SLOT(printPreview(QPrinter*)));
    preview.exec();
}

void MainWindow::docUndo()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->undo();
}

void MainWindow::docRedo()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->redo();
}

void MainWindow::docCut()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->cut();
}

void MainWindow::docPaste()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->paste();
}

void MainWindow::textBold()
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    QTextCharFormat fmt;
    bool res = ui->boldAction->isChecked();
    fmt.setFontWeight(res ? QFont::Bold : QFont::Normal);
    child->setFormatOnSelectedWord(fmt);
}

void MainWindow::textItalic()
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    QTextCharFormat fmt;
    fmt.setFontItalic(ui->italicAction->isChecked());
    child->setFormatOnSelectedWord(fmt);
}

void MainWindow::textUnderline()
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    QTextCharFormat fmt;
    fmt.setFontUnderline(ui->underlineAction->isChecked());
    child->setFormatOnSelectedWord(fmt);
}

void MainWindow::textFamily(const QString &fmly)
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    QTextCharFormat fmt;
    fmt.setFontFamily(fmly);
    child->setFormatOnSelectedWord(fmt);
}

void MainWindow::textSize(const QString &ps)
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    qreal pointSize = ps.toFloat();
    if(pointSize > 0)
    {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        child->setFormatOnSelectedWord(fmt);
    }
}

void MainWindow::docCopy()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->copy();
}

void MainWindow::textColor()
{
    childwnd* child = activateChildWnd();
    if(!child) return;

    //获取用户选择的颜色
    QColor color = QColorDialog::getColor(child->textColor(),this);
    if(!color.isValid()) return;

    QTextCharFormat fmt;
    fmt.setForeground(color);   //设置文本颜色
    child->setFormatOnSelectedWord(fmt);

    //更新工具栏颜色图标
    QPixmap pix(16,16);
    pix.fill(color);
    ui->colorAction->setIcon(pix);
}

void MainWindow::paraStyle(int nStyle)
{
    childwnd* child = activateChildWnd();
    if(child)
        child->setParaSyle(nStyle);
}

void MainWindow::formatEnabled()
{
    bool hasChild = (activateChildWnd() != nullptr);

    ui->boldAction->setEnabled(hasChild);
    ui->italicAction->setEnabled(hasChild);
    ui->underlineAction->setEnabled(hasChild);
    ui->leftAlignAction->setEnabled(hasChild);
    ui->centerAction->setEnabled(hasChild);
    ui->rightAlignAction->setEnabled(hasChild);
    ui->justifyAction->setEnabled(hasChild);
    ui->colorAction->setEnabled(hasChild);
}

childwnd *MainWindow::activateChildWnd()
{
    QMdiSubWindow* actWnd = ui->mdiArea->activeSubWindow();
    return actWnd ? qobject_cast<childwnd*>(actWnd->widget()) : nullptr;
}

QMdiSubWindow *MainWindow::findChildWnd(const QString &docName)
{
    QString strFile = QFileInfo(docName).canonicalFilePath();
    if(strFile.isEmpty()) return nullptr;

    //遍历当前所有的子窗口
    foreach (QMdiSubWindow* subWnd, ui->mdiArea->subWindowList()) {
        childwnd* chilWnd = qobject_cast<childwnd*>(subWnd->widget());
        if(chilWnd && chilWnd->m_CurDocPath == strFile)
            return subWnd;
    }
    return nullptr;
}

void MainWindow::refreshMenus()
{
    bool hasChild = (activateChildWnd() != nullptr);

    ui->saveAction->setEnabled(hasChild);
    ui->saveAsAction->setEnabled(hasChild);
    ui->printAction->setEnabled(hasChild);
    ui->printPreviewAction->setEnabled(hasChild);
    ui->pasteAction->setEnabled(hasChild);
    ui->closeAction->setEnabled(hasChild);
    ui->closeAllAction->setEnabled(hasChild);
    ui->titleAction->setEnabled(hasChild);
    ui->cascadeAction->setEnabled(hasChild);
    ui->nextAction->setEnabled(hasChild);
    ui->previousAction->setEnabled(hasChild);

    //文档打开且有内容选中
    bool hasSelect = (hasChild && activateChildWnd()->textCursor().hasSelection());
    ui->cutAction->setEnabled(hasSelect);
    ui->copyAction->setEnabled(hasSelect);
    ui->boldAction->setEnabled(hasSelect);
    ui->italicAction->setEnabled(hasSelect);
    ui->underlineAction->setEnabled(hasSelect);
    ui->leftAlignAction->setEnabled(hasSelect);
    ui->centerAction->setEnabled(hasSelect);
    ui->rightAlignAction->setEnabled(hasSelect);
    ui->justifyAction->setEnabled(hasSelect);
    ui->colorAction->setEnabled(hasSelect);
}

void MainWindow::addSubWndListMenu()
{
    ui->menu_W->clear();
    ui->menu_W->addAction(ui->closeAction);
    ui->menu_W->addAction(ui->closeAllAction);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->titleAction);
    ui->menu_W->addAction(ui->cascadeAction);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->nextAction);
    ui->menu_W->addAction(ui->previousAction);

    QList<QMdiSubWindow*> wnds = ui->mdiArea->subWindowList();
    if(!wnds.isEmpty()) ui->menu_W->addSeparator();

    for(int i = 0; i < wnds.size(); ++i)
    {
        childwnd* childWnd = qobject_cast<childwnd*>(wnds.at(i)->widget());

        // 添加安全检查，确保childWnd不为空
        if (!childWnd) continue;

        QString menuitem_text;
        menuitem_text = QString("%1 %2")
                            .arg(i+1)
                            .arg(childWnd->getCurDocName());

        QAction *menuitem_act = ui->menu_W->addAction(menuitem_text);
        menuitem_act->setCheckable(true);
        menuitem_act->setChecked(childWnd == activateChildWnd());

        // 使用lambda表达式连接菜单项点击事件
        connect(menuitem_act, &QAction::triggered, [this, subWindow = wnds.at(i)]() {
            if (subWindow) {
                ui->mdiArea->setActiveSubWindow(subWindow);
            }
        });
    }

    formatEnabled();
}

void MainWindow::on_newAction_triggered()
{
    docNew();
}

void MainWindow::on_closeAction_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_closeAllAction_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_titleAction_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_cascadeAction_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}

void MainWindow::on_nextAction_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_previousAction_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->mdiArea->closeAllSubWindows();
    if(ui->mdiArea->currentSubWindow())
        event->ignore();    //忽略此事件
    else
        event->accept();    //接受此事件
}

void MainWindow::on_openAction_triggered()
{
    docOpen();
}

void MainWindow::on_saveAction_triggered()
{
    docSave();
}

void MainWindow::on_saveAsAction_triggered()
{
    docSaveAs();
}

void MainWindow::on_undoAction_triggered()
{
    docUndo();
}

void MainWindow::on_redoAction_triggered()
{
    docRedo();
}

void MainWindow::on_cutAction_triggered()
{
    docCut();
}

void MainWindow::on_copyAction_triggered()
{
    docCopy();
}

void MainWindow::on_pasteAction_triggered()
{
    docPaste();
}

void MainWindow::on_boldAction_triggered()
{
    textBold();
}

void MainWindow::on_italicAction_triggered()
{
    textItalic();
}

void MainWindow::on_underlineAction_triggered()
{
    textUnderline();
}

void MainWindow::on_fontComboBox_activated(const QString &arg1)
{
    textFamily(arg1);
}

void MainWindow::on_sizeComboBox_activated(const QString &arg1)
{
    textSize(arg1);
}

void MainWindow::on_leftAlignAction_triggered()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->setAlignOfDocumentText(1);
}

void MainWindow::on_rightAlignAction_triggered()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->setAlignOfDocumentText(2);
}

void MainWindow::on_centerAction_triggered()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->setAlignOfDocumentText(3);
}

void MainWindow::on_justifyAction_triggered()
{
    childwnd* child = activateChildWnd();
    if(child)
        child->setAlignOfDocumentText(4);
}

void MainWindow::on_colorAction_triggered()
{
    textColor();
}

void MainWindow::on_comboBox_activated(int index)
{
    paraStyle(index);
}

void MainWindow::on_printAction_triggered()
{
    docPrint();
}

void MainWindow::printPreview(QPrinter *printer)
{
    childwnd* child = activateChildWnd();
    if(child)
        child->print(printer);
}

void MainWindow::on_printPreviewAction_triggered()
{
    docPrintPreview();
}
