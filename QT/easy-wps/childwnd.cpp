#include "childwnd.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTextList>
childwnd::childwnd()
{
    //子窗口关闭时销毁该类的实例对象
    setAttribute(Qt::WA_DeleteOnClose);
    m_bSaved = false;
}

/*新建一个文档的时候先给个默认的名字 用静态变量记录当前打开文档数*/
void childwnd::newDoc()
{
    static int wndSeqNum = 1;
    m_CurDocPath = QString("WPS 文档 %1").arg(wndSeqNum++);
    //设置窗口标题
    setWindowTitle(m_CurDocPath + "[*]" + " - MyWPS");
    connect(document(),&QTextDocument::contentsChanged,this,&childwnd::docBeModified);
}

//获取当前文档名字
QString childwnd::getCurDocName()
{
    return QFileInfo(m_CurDocPath).fileName();
}

//打开文档
bool childwnd::loadDoc(const QString &docName)
{
    if(!docName.isEmpty())
    {
        QFile file(docName);
        // 修复bug：检查文件是否存在的逻辑错误
        // 原代码：if(file.exists()) return false; 这是错误的！
        // 应该改为：如果文件不存在则返回false
        if(!file.exists()) {
            return false;  // 文件不存在，返回false
        }

        if(!file.open(QFile::ReadOnly))
            return false;

        QByteArray text = file.readAll();
        file.close();  // 添加文件关闭

        if(Qt::mightBeRichText(text))
            setHtml(text);
        else
            setPlainText(text);

        setCurDoc(docName);
        connect(document(),&QTextDocument::contentsChanged,this,&childwnd::docBeModified);
        return true;
    }
    return false;
}

void childwnd::setCurDoc(const QString& docName)
{
    //canonicalFilePath()返回标准名称路径，可以过滤"." ".."
    m_CurDocPath = QFileInfo(docName).canonicalFilePath();
    m_bSaved = true;                //文档已被保存
    document()->setModified(false); //文档未改动
    setWindowModified(false);       //窗口不显示改动标识
    setWindowTitle(getCurDocName() + "[*]");//设置子窗口标题
}
bool childwnd::saveDoc()
{
    if(m_bSaved) return saveDocOpt(m_CurDocPath);
    else  return saveAsDoc();

}
bool childwnd::saveAsDoc()
{
    /*函数原型如下（简化）：
        QString QFileDialog::getSaveFileName(
            QWidget *parent = nullptr,
            const QString &caption = QString(),
            const QString &dir = QString(),
            const QString &filter = QString(),
            QString *selectedFilter = nullptr,
            QFileDialog::Options options = Options()
        )
        参数说明：
        parent：父窗口，对话框会显示在父窗口的上面，并共享其任务栏条目。
        caption：对话框的标题。
        dir：默认打开的目录。如果为空，将使用当前工作目录。
        filter：过滤器，用于只显示特定类型的文件。多个过滤器用两个分号隔开，例如："Text files (.txt);;Images (.png *.xpm *.jpg)"
        selectedFilter：指针，用于返回用户选择的过滤器。可以为nullptr，表示不关心。
        options：对话框的一些选项，例如是否使用本地文件对话框、是否显示详细信息等。
        返回值：用户选择的文件路径（字符串）。如果用户取消对话框，则返回空字符串*/
    QString docName = QFileDialog::getSaveFileName(this,
                                                   "另存为",
                                                   m_CurDocPath,
                                                   "HTML文档 (*.htm *.html);;"
                                                   "所有文件(*.*)");
    if(docName.isEmpty()) return false;
    else return saveDocOpt(docName);
}
bool childwnd::saveDocOpt(QString docName)
{
    // 检查文件名是否以 .htm 或 .html 结尾（不区分大小写）
    // 如果不是这两种扩展名，则自动添加 .html 后缀
    if( !(docName.endsWith(".htm", Qt::CaseInsensitive) ||
          docName.endsWith(".html", Qt::CaseInsensitive)) ) {
        // 文件名没有HTML扩展名，添加默认的.html扩展名
        docName += ".html";
    }

    // 创建QTextDocumentWriter对象，用于将QTextDocument内容写入文件
    // 参数为文件名，根据文件扩展名自动确定输出格式
    QTextDocumentWriter writer(docName);

    // 将当前窗口的document()写入文件
    // write()方法返回bool值表示写入是否成功
    bool isSuccess = writer.write(this->document());

    // 如果写入成功，更新当前文档路径
    if(isSuccess) {
        setCurDoc(docName);  // 保存当前文档的文件路径
    }

    // 返回保存操作是否成功
    return isSuccess;
}
/**
 * @brief 对选中的单词或光标所在单词应用字符格式
 * @param fmt 要应用的文本字符格式
 *
 * 功能说明：
 * 1. 如果没有选中文本，则选中光标所在的整个单词
 * 2. 对选中的文本应用指定的字符格式
 * 3. 同时将格式设置为后续输入的默认格式
 */
void childwnd::setFormatOnSelectedWord(const QTextCharFormat &fmt)
{
    // 获取当前文本光标
    QTextCursor tcursor = textCursor();

    // 如果没有选中文本，则选中光标所在的整个单词
    if(!tcursor.hasSelection())
        tcursor.select(QTextCursor::WordUnderCursor);

    // 合并字符格式到选中的文本（保留原有格式，只应用新格式的属性）
    tcursor.mergeCharFormat(fmt);

    // 设置当前光标位置的字符格式，影响后续输入的文字
    mergeCurrentCharFormat(fmt);
}

/**
 * @brief 设置文档文本的对齐方式
 * @param aligntype 对齐类型：
 *                  1 = 左对齐
 *                  2 = 右对齐
 *                  3 = 居中对齐
 *                  4 = 两端对齐
 *
 * 注意：Qt::AlignAbsolute 标志表示考虑布局方向（从左到右或从右到左）
 */
void childwnd::setAlignOfDocumentText(int aligntype)
{
    if(aligntype == 1)
        setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);    // 左对齐
    else if(aligntype == 2)
        setAlignment(Qt::AlignRight | Qt::AlignAbsolute);   // 右对齐
    else if(aligntype == 3)
        setAlignment(Qt::AlignCenter);                      // 居中对齐
    else if(aligntype == 4)
        setAlignment(Qt::AlignJustify);                     // 两端对齐
}

/**
 * @brief 设置段落样式（主要是项目符号和编号）
 * @param pStyle 段落样式编号：
 *               0  = 清除列表格式
 *               1  = 实心圆点
 *               2  = 空心圆
 *               3  = 实心方块
 *               4  = 数字编号 (1, 2, 3...)
 *               5  = 小写字母编号 (a, b, c...)
 *               6  = 大写字母编号 (A, B, C...)
 *               7  = 小写罗马数字 (i, ii, iii...)
 *               8  = 大写罗马数字 (I, II, III...)
 *
 * 注意：当pStyle为0时，清除当前段落的列表格式
 */
void childwnd::setParaSyle(int pStyle)
{
    QTextCursor tcursor = textCursor();
    QTextListFormat::Style sname;

    // 如果pStyle不为0，设置列表样式
    if(pStyle != 0)
    {
        // 根据pStyle参数选择对应的列表样式
        switch(pStyle){
        case 1:
            sname = QTextListFormat::ListDisc;        // 实心圆点 ●
            break;
        case 2:
            sname = QTextListFormat::ListCircle;      // 空心圆 ○
            break;
        case 3:
            sname = QTextListFormat::ListSquare;      // 实心方块 ■
            break;
        case 4:
            sname = QTextListFormat::ListDecimal;     // 数字编号 1, 2, 3...
            break;
        case 5:
            sname = QTextListFormat::ListLowerAlpha;  // 小写字母 a, b, c...
            break;
        case 6:
            sname = QTextListFormat::ListUpperAlpha;  // 大写字母 A, B, C...
            break;
        case 7:
            sname = QTextListFormat::ListLowerRoman;  // 小写罗马数字 i, ii, iii...
            break;
        case 8:
            sname = QTextListFormat::ListUpperRoman;  // 大写罗马数字 I, II, III...
            break;
        default:
            sname = QTextListFormat::ListDisc;        // 默认使用实心圆点
        }

        // 开始编辑块，用于将多个编辑操作合并为一个撤销/重做操作
        tcursor.beginEditBlock();

        // 获取当前块的格式
        QTextBlockFormat tBlockFmt = tcursor.blockFormat();

        // 创建列表格式对象
        QTextListFormat tListFmt;

        // 检查当前光标是否已经在列表中
        if(tcursor.currentList()){
            // 如果已经在列表中，获取现有列表格式
            tListFmt = tcursor.currentList()->format();
        }else{
            // 如果不在列表中，设置缩进
            // 新列表的缩进比当前块格式的缩进多一级
            tListFmt.setIndent(tBlockFmt.indent() + 1);

            // 将块的缩进重置为0，由列表格式控制缩进
            tBlockFmt.setIndent(0);
            tcursor.setBlockFormat(tBlockFmt);
        }

        // 设置列表样式
        tListFmt.setStyle(sname);

        // 创建列表
        tcursor.createList(tListFmt);

        // 结束编辑块
        tcursor.endEditBlock();
    }
    else
    {
        // pStyle为0，清除列表格式
        QTextBlockFormat tbfmt;
        tbfmt.setObjectIndex(-1);   // 设置为-1表示清除列表格式
        tcursor.mergeBlockFormat(tbfmt);
    }
}

/**
 * @brief 窗口关闭事件处理函数
 * @param event 关闭事件对象
 *
 * 功能说明：
 * 1. 在窗口关闭前检查文档是否需要保存
 * 2. 如果promptSave()返回true，接受关闭事件
 * 3. 否则忽略关闭事件，窗口不会关闭
 */
void childwnd::closeEvent(QCloseEvent *event)
{
    if( promptSave() )
        event->accept();    // 接受关闭事件，窗口将被关闭
    else
        event->ignore();    // 忽略关闭事件，窗口保持打开
}

/**
 * @brief 提示保存文档
 * @return 是否允许继续操作
 *         true  = 可以继续（已保存或用户放弃保存）
 *         false = 用户取消操作
 *
 * 注意：此函数在文档被修改且需要关闭或新建时调用
 */
bool childwnd::promptSave()
{
    // 如果文档没有被修改，直接返回true
    if(! document()->isModified())
        return true;

    // 显示警告消息框，询问用户如何处理
    QMessageBox::StandardButton result;
    result = QMessageBox::warning(this,
                                  QString("系统提示"),
                                  QString("文档%1已修改，是否保存？")
                                      .arg(getCurDocName()),
                                  QMessageBox::Yes |      // 保存按钮
                                      QMessageBox::Discard |  // 放弃保存按钮
                                      QMessageBox::No);       // 取消按钮

    // 根据用户选择进行处理
    if(result == QMessageBox::Yes)
        return saveDoc();           // 保存文档，返回保存结果
    else if(result == QMessageBox::No)
        return false;               // 用户取消操作
    return true;                    // 用户选择放弃保存，返回true
}

/**
 * @brief 文档修改状态变化的响应函数
 *
 * 功能说明：
 * 1. 当文档内容发生变化时，此函数被调用
 * 2. 设置窗口的修改状态标志，显示或隐藏窗口标题中的"*"
 * 3. 用于在QMainWindow标题栏中显示文档是否被修改
 */
void childwnd::docBeModified()
{
    // 设置窗口的修改状态，显示为"*"标记
    setWindowModified(document()->isModified());
}
