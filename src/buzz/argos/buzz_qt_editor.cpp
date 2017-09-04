/*
 * Buzz includes
 */
#include "buzz_qt_editor.h"
#include "buzz_qt_syntax_highlighter.h"

/*
 * Qt includes
 */
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QTextBlock>
#include <QTextStream>

/****************************************/
/****************************************/

/*
 * Template code for a new Buzz script
 */
static QString SCRIPT_TEMPLATE =
   "# Use Shift + Click to select a robot\n"
   "# When a robot is selected, its variables appear in this editor\n\n"
   "# Use Ctrl + Click (Cmd + Click on Mac) to move a selected robot to a different location\n\n\n\n"
   "# Put your global variables here\n\n\n\n"
   "# This function is executed every time you press the 'execute' button\n"
   "function init() {\n"
   "   # put your code here\n"
   "}\n\n\n\n"
   "# This function is executed at each time step\n"
   "# It must contain the logic of your controller\n"
   "function step() {\n"
   "   # put your code here\n"
   "}\n\n\n\n"
   "# This function is executed every time you press the 'reset'\n"
   "# button in the GUI. It is supposed to restore the state\n"
   "# of the controller to whatever it was right after init() was\n"
   "# called. The state of sensors and actuators is reset\n"
   "# automatically by ARGoS.\n"
   "function reset() {\n"
   "   # put your code here\n"
   "}\n\n\n\n"
   "# This function is executed only once, when the robot is removed\n"
   "# from the simulation\n"
   "function destroy() {\n"
   "   # put your code here\n"
   "}\n";

/****************************************/
/****************************************/

CBuzzQTEditor::CBuzzQTEditor(const QString& str_path) :
   m_strScriptPath(
      QFileInfo(str_path).exists() ?
      QFileInfo(str_path).canonicalFilePath() :
      str_path) {
   /* This widget is deleted when the close event is accepted */
   setAttribute(Qt::WA_DeleteOnClose);
   /* Set font */
   QFont cFont;
   cFont.setFamily("Monospace");
   cFont.setStyleHint(QFont::Monospace);
   cFont.setFixedPitch(true);
   cFont.setWeight(QFont::Bold);
   setFont(cFont);
   /* Set tab width to 3 */
   QFontMetrics cFontMetrics(cFont);
   setTabStopWidth(2 * cFontMetrics.width(' '));
   /* Set syntax highlighting */
   new CBuzzQTSyntaxHighlighter(document());
   /* Set line numbering */
   m_pcLineNumberArea = new CLineNumberArea(this);
   /* Connect signals */
   connect(this, SIGNAL(blockCountChanged(int)),
           this, SLOT(UpdateLineNumberAreaWidth(int)));
   connect(this, SIGNAL(updateRequest(const QRect&, int)),
           this, SLOT(UpdateLineNumberArea(const QRect&, int)));
   connect(this, SIGNAL(cursorPositionChanged()),
           this, SLOT(HighlightCurrentLine()));
   /* Final touches */
   UpdateLineNumberAreaWidth(0);
   HighlightCurrentLine();
}

/****************************************/
/****************************************/

void CBuzzQTEditor::LineNumberAreaPaintEvent(QPaintEvent* pc_event) {
   QPainter cPainter(m_pcLineNumberArea);
   cPainter.fillRect(pc_event->rect(), Qt::lightGray);
   QTextBlock cBlock = firstVisibleBlock();
   int nBlockNumber = cBlock.blockNumber();
   int nTop = (int) blockBoundingGeometry(cBlock).translated(contentOffset()).top();
   int nBottom = nTop + (int) blockBoundingRect(cBlock).height();
   while (cBlock.isValid() && nTop <= pc_event->rect().bottom()) {
      if (cBlock.isVisible() && nBottom >= pc_event->rect().top()) {
         QString strNumber = QString::number(nBlockNumber + 1);
         cPainter.setPen(Qt::black);
         cPainter.drawText(0, nTop,
                           m_pcLineNumberArea->width(), fontMetrics().height(),
                           Qt::AlignRight, strNumber);
      }
      cBlock = cBlock.next();
      nTop = nBottom;
      nBottom = nTop + (int) blockBoundingRect(cBlock).height();
      ++nBlockNumber;
   }
}

/****************************************/
/****************************************/

int CBuzzQTEditor::LineNumberAreaWidth() {
   int nDigits = 1;
   int nMax = qMax(1, blockCount());
   while (nMax >= 10) {
      nMax /= 10;
      ++nDigits;
   }
   int nSpace = 3 + fontMetrics().width(QLatin1Char('9')) * nDigits;
   return nSpace;
}

/****************************************/
/****************************************/

void CBuzzQTEditor::New() {
   setPlainText(SCRIPT_TEMPLATE);
   /* Connect signal for modified content */
   connect(document(), SIGNAL(contentsChanged()),
           this, SLOT(CodeModified()));
   /* Set modified flag */
   document()->setModified(true);
   UpdateRecentFiles();
}

/****************************************/
/****************************************/

bool CBuzzQTEditor::Open() {
   /* Attempt to open file */
   QFile cFile(m_strScriptPath);
   if(!cFile.open(QFile::ReadOnly | QFile::Text)) {
      return false;
   }
   /* Read succeeded */
   /* Change cursor to hourglass */
   QApplication::setOverrideCursor(Qt::WaitCursor);
   /* Read text */
   QTextStream cTxt(&cFile);
   setPlainText(cTxt.readAll());
   /* Connect signal for modified content */
   connect(document(), SIGNAL(contentsChanged()),
           this, SLOT(CodeModified()));
   UpdateRecentFiles();
   /* Restore cursor */
   QApplication::restoreOverrideCursor();
   return true;
}

/****************************************/
/****************************************/

bool CBuzzQTEditor::Save() {
   /* Attempt to open file */
   QFile cFile(m_strScriptPath);
   if(!cFile.open(QFile::WriteOnly | QFile::Text)) {
      /* Read failed */
      QMessageBox::warning(this, tr("Buzz Editor"),
                           tr("Cannot write file %1:\n%2.")
                           .arg(m_strScriptPath)
                           .arg(cFile.errorString()));
      return false;
   }
   /* Change cursor to hourglass */
   QApplication::setOverrideCursor(Qt::WaitCursor);
   /* Save file */
   QTextStream cTxt(&cFile);
   cTxt << toPlainText();
   /* Handle 'modified' flag */
   document()->setModified(false);
   /* Restore cursor */
   QApplication::restoreOverrideCursor();
   return true;
}

/****************************************/
/****************************************/

bool CBuzzQTEditor::SaveAs() {
   /* Choose file name using dialog */
   const QString strFName = QFileDialog::getSaveFileName(
      this, // parent
      tr("Save file"),           // dialog title
      m_strScriptPath,           // by default the current file name is selected
      tr("Buzz scripts (*.bzz)") // file filter
      );
   /* Was the file chosen? */
   if(strFName.isEmpty())
      return false;
   /* Update the script file name */
   m_strScriptPath = QFileInfo(strFName).canonicalFilePath();
   UpdateRecentFiles();
   /* Save the file */
   return Save();
}

/****************************************/
/****************************************/

void CBuzzQTEditor::closeEvent(QCloseEvent* pc_event) {
   /* Whether or not to close the editor */
   bool bAccept = true;
   /* Save if document modified */
   if(document()->isModified()) {
      QMessageBox::StandardButton cChoice =
         QMessageBox::warning(
            this, // parent widget
            tr("Buzz editor"), // dialog title
            tr("'%1' has been modified. Do you want to save your changes?")
            .arg(m_strScriptPath),
            QMessageBox::Save |
            QMessageBox::Discard |
            QMessageBox::Cancel);
      if(cChoice == QMessageBox::Save)
         bAccept= Save();
      else if(cChoice == QMessageBox::Cancel)
         bAccept = false;
      else // discard
         bAccept = true;
   }
   /* Accept the close event? */
   if(bAccept)
      pc_event->accept();
   else
      pc_event->ignore();
}

/****************************************/
/****************************************/

void CBuzzQTEditor::resizeEvent(QResizeEvent* pc_event) {
   QPlainTextEdit::resizeEvent(pc_event);
   QRect cRect = contentsRect();
   m_pcLineNumberArea->setGeometry(QRect(cRect.left(), cRect.top(),
                                         LineNumberAreaWidth(), cRect.height()));
}

/****************************************/
/****************************************/

void CBuzzQTEditor::UpdateLineNumberAreaWidth(int) {
   setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

/****************************************/
/****************************************/

void CBuzzQTEditor::GoTo(int n_line,
                         int n_column) {
   QTextCursor cCursor = textCursor();
   cCursor.movePosition(QTextCursor::Start,
                        QTextCursor::MoveAnchor);
   cCursor.movePosition(QTextCursor::Down,
                        QTextCursor::MoveAnchor,
                        n_line);
   cCursor.movePosition(QTextCursor::NextCharacter,
                        QTextCursor::MoveAnchor,
                        n_column);
   setTextCursor(cCursor);
   setFocus(Qt::OtherFocusReason);
}

/****************************************/
/****************************************/

void CBuzzQTEditor::HighlightCurrentLine() {
   QList<QTextEdit::ExtraSelection> cListExtraSel;
   if (!isReadOnly()) {
      QTextEdit::ExtraSelection cSel;
      QColor cLineColor = QColor(Qt::yellow).lighter(160);
      cSel.format.setBackground(cLineColor);
      cSel.format.setProperty(QTextFormat::FullWidthSelection, true);
      cSel.cursor = textCursor();
      cSel.cursor.clearSelection();
      cListExtraSel.append(cSel);
   }
   setExtraSelections(cListExtraSel);
}

/****************************************/
/****************************************/

void CBuzzQTEditor::UpdateLineNumberArea(const QRect& c_rect,
                                         int n_dy) {
   if(n_dy) {
      m_pcLineNumberArea->scroll(0, n_dy);
   }
   else {
      m_pcLineNumberArea->update(0, c_rect.y(),
                                 m_pcLineNumberArea->width(), c_rect.height());
   }
   if(c_rect.contains(viewport()->rect())) {
      UpdateLineNumberAreaWidth(0);
   }
}
   
/****************************************/
/****************************************/

void CBuzzQTEditor::CodeModified() {
   setWindowModified(document()->isModified());
}

/****************************************/
/****************************************/

void CBuzzQTEditor::UpdateRecentFiles() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   /* Get list of recent files */
   QStringList cFiles = cSettings.value("recentFiles").toStringList();
   /* Remove current file name, if any */
   cFiles.removeAll(m_strScriptPath);
   /* Add current file name to the list */
   cFiles.prepend(m_strScriptPath);
   /* Remove extra file names */
   while(cFiles.size() > MAX_RECENT_FILES) {
      cFiles.removeLast();
   }
   /* Write data */
   cSettings.setValue("recentFiles", cFiles);
   cSettings.endGroup();
   emit RecentFilesChanged();
}

/****************************************/
/****************************************/
