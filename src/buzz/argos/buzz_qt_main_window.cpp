#include "buzz_controller.h"
#include "buzz_qt_main_window.h"
#include "buzz_qt_editor.h"
#include "buzz_qt_find_dialog.h"
#include "buzz_qt_statetree_model.h"
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_main_window.h>
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>

#include <argos3/core/config.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTextStream>
#include <QToolBar>
#include <QTableWidget>
#include <QTreeView>

/****************************************/
/****************************************/

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

CBuzzQTMainWindow::CBuzzQTMainWindow(CQTOpenGLMainWindow* pc_parent) :
   QMainWindow(pc_parent),
   m_pcMainWindow(pc_parent),
   m_pcStatusbar(NULL),
   m_pcCodeEditor(NULL),
   m_pcFindDialog(NULL),
   m_pcBuzzMessageTable(NULL) {
   /* Add a status bar */
   m_pcStatusbar = new QStatusBar(this);
   setStatusBar(m_pcStatusbar);
   /* Create the Buzz message table */
   CreateBuzzMessageTable();
   /* Populate list of Buzz controllers */
   PopulateBuzzControllers();
   /* Create the Buzz state docks */
   CreateBuzzStateDocks();
   /* Create editor */
   CreateCodeEditor();
   /* Create actions */
   CreateFileActions();
   CreateEditActions();
   CreateCodeActions();
   /* Set empty file */
   SetCurrentFile("");
   /* Read settings */
   ReadSettings();
}

/****************************************/
/****************************************/

CBuzzQTMainWindow::~CBuzzQTMainWindow() {
   WriteSettings();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::New() {
   if(MaybeSave()) {
      m_pcCodeEditor->setPlainText(SCRIPT_TEMPLATE);
      SetCurrentFile("");
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Open() {
   if(MaybeSave()) {
      QString strNewFileName =
         QFileDialog::getOpenFileName(this,
                                      tr("Open File"),
                                      "",
                                      "Buzz Scripts (*.bzz)");
      if (!strNewFileName.isEmpty()) {
         OpenFile(strNewFileName);
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::OpenRecentFile() {
   QAction* pcAction = qobject_cast<QAction*>(sender());
   if (pcAction) {
      OpenFile(pcAction->data().toString());
   }
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::Save() {
   if (m_strFileName.isEmpty()) {
      return SaveAs();
   } else {
      return SaveFile(m_strFileName);
   }
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::SaveAs() {
   QString strNewFileName =
      QFileDialog::getSaveFileName(this,
                                   tr("Save File"),
                                   "",
                                   "Buzz Files (*.buzz)");
   if (strNewFileName.isEmpty())
      return false;
   return SaveFile(strNewFileName);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Execute() {
   /* Save script */
   Save();
   /* Change cursor */
   QApplication::setOverrideCursor(Qt::WaitCursor);
   /* Stop simulation */
   m_pcMainWindow->SuspendExperiment();
   /* Clear the message table */
   m_pcBuzzMessageTable->clearContents();
   m_pcBuzzMessageTable->setRowCount(1);
   /* Create temporary file to contain the bytecode */
   QTemporaryFile cByteCode, cDbgInfo;
   if(! cByteCode.open()) {
      SetMessage(0, "ALL", "Can't create bytecode file.");
      QApplication::restoreOverrideCursor();
      return;
   }
   if(! cDbgInfo.open()) {
      SetMessage(0, "ALL", "Can't create bytecode file.");
      QApplication::restoreOverrideCursor();
      return;
   }
   /*
    * Compile script
    */
   QProcess cBuzzCompiler;
   cBuzzCompiler.start("bzzc",
                       QStringList() <<
                       "-b" << cByteCode.fileName() <<
                       "-d" << cDbgInfo.fileName() <<
                       m_strFileName);
   if(! cBuzzCompiler.waitForFinished()) {
      SetMessage(0, "ALL", QString(cBuzzCompiler.readAllStandardError()));
      QApplication::restoreOverrideCursor();
      return;
   }
   if(cBuzzCompiler.exitCode() != 0) {
      SetMessage(0, "ALL", QString(cBuzzCompiler.readAllStandardError()));
      QApplication::restoreOverrideCursor();
      return;
   }
   SetMessage(0, "ALL", "Compilation successful.");
   /* Set the script for all the robots */
   for(size_t i = 0; i < m_vecControllers.size(); ++i) {
      m_vecControllers[i]->SetBytecode(cByteCode.fileName().toStdString(),
                                       cDbgInfo.fileName().toStdString());
   }
   /* Update Buzz state if visible */
   if(m_pcBuzzVariableDock->isVisible()) {
      static_cast<CBuzzQTStateTreeModel*>(m_pcBuzzVariableTree->model())->SetBuzzState(
         m_vecControllers[m_unSelectedRobot]->GetBuzzVM());
   }
   if(m_pcBuzzFunctionDock->isVisible()) {
      static_cast<CBuzzQTStateTreeModel*>(m_pcBuzzFunctionTree->model())->SetBuzzState(
         m_vecControllers[m_unSelectedRobot]->GetBuzzVM());
   }
   /* Resume simulation */
   m_pcMainWindow->ResumeExperiment();
   QApplication::restoreOverrideCursor();
   statusBar()->showMessage(tr("Execution started"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Find() {
   if(! m_pcFindDialog) {
      m_pcFindDialog = new CBuzzQTFindDialog(this);
   }
   m_pcFindDialog->show();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CodeModified() {
   setWindowModified(m_pcCodeEditor->document()->isModified());
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::MaybeSave() {
   if(m_pcCodeEditor->document()->isModified()) {
      QMessageBox::StandardButton tReply;
      tReply = QMessageBox::warning(this, tr("ARGoS v" ARGOS_VERSION "-" ARGOS_RELEASE " - Buzz Editor"),
                                    tr("The document has been modified.\n"
                                       "Do you want to save your changes?"),
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      if (tReply == QMessageBox::Save)
         return Save();
      else if (tReply == QMessageBox::Cancel)
         return false;
   }
   return true;
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::PopulateBuzzControllers() {
   /* Get list of controllable entities */
   CSpace& cSpace = CSimulator::GetInstance().GetSpace();
   CSpace::TMapPerType& tControllables = cSpace.GetEntitiesByType("controller");
   /* Go through them and keep a pointer to each Buzz controller */
   for(CSpace::TMapPerType::iterator it = tControllables.begin();
       it != tControllables.end();
       ++it) {
      /* Try to convert the controller into a Buzz controller */
      CControllableEntity* pcControllable = any_cast<CControllableEntity*>(it->second);
      CBuzzController* pcBuzzController = dynamic_cast<CBuzzController*>(&(pcControllable->GetController()));
      if(pcBuzzController) {
         /* Conversion succeeded, add to indices */
         m_vecControllers.push_back(pcBuzzController);
         m_vecRobots.push_back(&pcControllable->GetParent());
      }
      else {
         LOGERR << "[WARNING] Entity \""
                << pcControllable->GetParent().GetId()
                << "\" does not have a Buzz controller associated"
                << std::endl;
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::ReadSettings() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   resize(cSettings.value("size", QSize(640,480)).toSize());
   move(cSettings.value("position", QPoint(0,0)).toPoint());
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::WriteSettings() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   cSettings.setValue("size", size());
   cSettings.setValue("position", pos());
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateCodeEditor() {
   /* Create code editor */
   m_pcCodeEditor = new CBuzzQTEditor(this);
   setCentralWidget(m_pcCodeEditor);
   m_pcCodeEditor->setPlainText(SCRIPT_TEMPLATE);
   /* Connect stuff */
   connect(m_pcCodeEditor->document(), SIGNAL(contentsChanged()),
           this, SLOT(CodeModified()));
   connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
           this, SLOT(CheckBuzzStatus(int)));
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateBuzzMessageTable() {
   m_pcBuzzMsgDock = new QDockWidget(tr("Messages"), this);
   m_pcBuzzMsgDock->setObjectName("BuzzMessageDock");
   m_pcBuzzMsgDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
   m_pcBuzzMsgDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
   m_pcBuzzMessageTable = new QTableWidget();
   m_pcBuzzMessageTable->setColumnCount(3);
   QStringList listHeaders;
   listHeaders << tr("Robot")
               << tr("Line")
               << tr("Message");
   m_pcBuzzMessageTable->setHorizontalHeaderLabels(listHeaders);
   m_pcBuzzMessageTable->horizontalHeader()->setStretchLastSection(true);
   m_pcBuzzMessageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_pcBuzzMessageTable->setSelectionMode(QAbstractItemView::SingleSelection);
   m_pcBuzzMsgDock->setWidget(m_pcBuzzMessageTable);
   addDockWidget(Qt::BottomDockWidgetArea, m_pcBuzzMsgDock);
   connect(m_pcBuzzMessageTable, SIGNAL(itemSelectionChanged()),
           this, SLOT(HandleMsgTableSelection()));
   m_pcBuzzMsgDock->hide();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateBuzzStateDocks() {
   /* Variable tree dock */
   m_pcBuzzVariableDock = new QDockWidget(tr("Variables"), this);
   m_pcBuzzVariableDock->setObjectName("BuzzVariableDock");
   m_pcBuzzVariableDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
   m_pcBuzzVariableDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
   m_pcBuzzVariableTree = new QTreeView();
   m_pcBuzzVariableDock->setWidget(m_pcBuzzVariableTree);
   addDockWidget(Qt::LeftDockWidgetArea, m_pcBuzzVariableDock);
   m_pcBuzzVariableDock->hide();
   /* Function tree dock */
   m_pcBuzzFunctionDock = new QDockWidget(tr("Functions"), this);
   m_pcBuzzFunctionDock->setObjectName("BuzzFunctionDock");
   m_pcBuzzFunctionDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
   m_pcBuzzFunctionDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
   m_pcBuzzFunctionTree = new QTreeView();
   m_pcBuzzFunctionDock->setWidget(m_pcBuzzFunctionTree);
   addDockWidget(Qt::LeftDockWidgetArea, m_pcBuzzFunctionDock);
   m_pcBuzzFunctionDock->hide();
   /* Connect stuff */
   connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(EntitySelected(size_t)),
           this, SLOT(HandleEntitySelection(size_t)));
   connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(EntityDeselected(size_t)),
           this, SLOT(HandleEntityDeselection(size_t)));
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateFileActions() {
   QIcon cFileNewIcon;
   cFileNewIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/new.png"));
   m_pcFileNewAction = new QAction(cFileNewIcon, tr("&New"), this);
   m_pcFileNewAction->setToolTip(tr("Create a new file"));
   m_pcFileNewAction->setStatusTip(tr("Create a new file"));
   m_pcFileNewAction->setShortcut(QKeySequence::New);
   connect(m_pcFileNewAction, SIGNAL(triggered()),
           this, SLOT(New()));
   QIcon cFileOpenIcon;
   cFileOpenIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/open.png"));
   m_pcFileOpenAction = new QAction(cFileOpenIcon, tr("&Open..."), this);
   m_pcFileOpenAction->setToolTip(tr("Open a file"));
   m_pcFileOpenAction->setStatusTip(tr("Open a file"));
   m_pcFileOpenAction->setShortcut(QKeySequence::Open);
   connect(m_pcFileOpenAction, SIGNAL(triggered()),
           this, SLOT(Open()));
   for (int i = 0; i < MAX_RECENT_FILES; ++i) {
      m_pcFileOpenRecentAction[i] = new QAction(this);
      m_pcFileOpenRecentAction[i]->setVisible(false);
      connect(m_pcFileOpenRecentAction[i], SIGNAL(triggered()),
              this, SLOT(OpenRecentFile()));
   }
   QIcon cFileSaveIcon;
   cFileSaveIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/save.png"));
   m_pcFileSaveAction = new QAction(cFileSaveIcon, tr("&Save"), this);
   m_pcFileSaveAction->setToolTip(tr("Save the current file"));
   m_pcFileSaveAction->setStatusTip(tr("Save the current file"));
   m_pcFileSaveAction->setShortcut(QKeySequence::Save);
   connect(m_pcFileSaveAction, SIGNAL(triggered()),
           this, SLOT(Save()));
   QIcon cFileSaveAsIcon;
   cFileSaveAsIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/saveas.png"));
   m_pcFileSaveAsAction = new QAction(cFileSaveAsIcon, tr("S&ave as..."), this);
   m_pcFileSaveAsAction->setToolTip(tr("Save the current file under a new name"));
   m_pcFileSaveAsAction->setStatusTip(tr("Save the current file under a new name"));
   m_pcFileSaveAsAction->setShortcut(QKeySequence::SaveAs);
   connect(m_pcFileSaveAsAction, SIGNAL(triggered()),
           this, SLOT(SaveAs()));
   QMenu* pcMenu = menuBar()->addMenu(tr("&File"));
   pcMenu->addAction(m_pcFileNewAction);
   pcMenu->addSeparator();
   pcMenu->addAction(m_pcFileOpenAction);
   pcMenu->addSeparator();
   pcMenu->addAction(m_pcFileSaveAction);
   pcMenu->addAction(m_pcFileSaveAsAction);
   m_pcFileSeparateRecentAction = pcMenu->addSeparator();
   for (int i = 0; i < MAX_RECENT_FILES; ++i) {
      pcMenu->addAction(m_pcFileOpenRecentAction[i]);
   }
   QToolBar* pcToolBar = addToolBar(tr("File"));
   pcToolBar->setObjectName("FileToolBar");
   pcToolBar->setIconSize(QSize(32,32));
   pcToolBar->addAction(m_pcFileNewAction);
   pcToolBar->addAction(m_pcFileOpenAction);
   pcToolBar->addAction(m_pcFileSaveAction);
   UpdateRecentFiles();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateEditActions() {
   QIcon cEditUndoIcon;
   cEditUndoIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/undo.png"));
   m_pcEditUndoAction = new QAction(cEditUndoIcon, tr("&Undo"), this);
   m_pcEditUndoAction->setToolTip(tr("Undo last operation"));
   m_pcEditUndoAction->setStatusTip(tr("Undo last operation"));
   m_pcEditUndoAction->setShortcut(QKeySequence::Undo);
   connect(m_pcEditUndoAction, SIGNAL(triggered()),
           m_pcCodeEditor, SLOT(undo()));
   QIcon cEditRedoIcon;
   cEditRedoIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/redo.png"));
   m_pcEditRedoAction = new QAction(cEditRedoIcon, tr("&Redo"), this);
   m_pcEditRedoAction->setToolTip(tr("Redo last operation"));
   m_pcEditRedoAction->setStatusTip(tr("Redo last operation"));
   m_pcEditRedoAction->setShortcut(QKeySequence::Redo);
   connect(m_pcEditRedoAction, SIGNAL(triggered()),
           m_pcCodeEditor, SLOT(redo()));
   QIcon cEditCopyIcon;
   cEditCopyIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/copy.png"));
   m_pcEditCopyAction = new QAction(cEditCopyIcon, tr("&Copy"), this);
   m_pcEditCopyAction->setToolTip(tr("Copy selected text into clipboard"));
   m_pcEditCopyAction->setStatusTip(tr("Copy selected text into clipboard"));
   m_pcEditCopyAction->setShortcut(QKeySequence::Copy);
   connect(m_pcEditCopyAction, SIGNAL(triggered()),
           m_pcCodeEditor, SLOT(copy()));
   QIcon cEditCutIcon;
   cEditCutIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/cut.png"));
   m_pcEditCutAction = new QAction(cEditCutIcon, tr("&Cut"), this);
   m_pcEditCutAction->setToolTip(tr("Move selected text into clipboard"));
   m_pcEditCutAction->setStatusTip(tr("Move selected text into clipboard"));
   m_pcEditCutAction->setShortcut(QKeySequence::Cut);
   connect(m_pcEditCutAction, SIGNAL(triggered()),
           m_pcCodeEditor, SLOT(cut()));
   QIcon cEditPasteIcon;
   cEditPasteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/paste.png"));
   m_pcEditPasteAction = new QAction(cEditPasteIcon, tr("&Paste"), this);
   m_pcEditPasteAction->setToolTip(tr("Paste text from clipboard"));
   m_pcEditPasteAction->setStatusTip(tr("Paste text from clipboard"));
   m_pcEditPasteAction->setShortcut(QKeySequence::Paste);
   connect(m_pcEditPasteAction, SIGNAL(triggered()),
           m_pcCodeEditor, SLOT(paste()));
   // QIcon cEditFindIcon;
   // cEditFindIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/find.png"));
   // m_pcEditFindAction = new QAction(cEditFindIcon, tr("&Find/Replace"), this);
   // m_pcEditFindAction->setToolTip(tr("Find/replace text"));
   // m_pcEditFindAction->setStatusTip(tr("Find/replace text"));
   // m_pcEditFindAction->setShortcut(QKeySequence::Find);
   // connect(m_pcEditFindAction, SIGNAL(triggered()),
   //         this, SLOT(Find()));
   QMenu* pcMenu = menuBar()->addMenu(tr("&Edit"));
   pcMenu->addAction(m_pcEditUndoAction);
   pcMenu->addAction(m_pcEditRedoAction);
   pcMenu->addSeparator();
   pcMenu->addAction(m_pcEditCopyAction);
   pcMenu->addAction(m_pcEditCutAction);
   pcMenu->addAction(m_pcEditPasteAction);
   // pcMenu->addSeparator();
   // pcMenu->addAction(m_pcEditFindAction);
   QToolBar* pcToolBar = addToolBar(tr("Edit"));
   pcToolBar->setObjectName("EditToolBar");
   pcToolBar->setIconSize(QSize(32,32));
   pcToolBar->addAction(m_pcEditUndoAction);
   pcToolBar->addAction(m_pcEditRedoAction);
   pcToolBar->addSeparator();
   pcToolBar->addAction(m_pcEditCopyAction);
   pcToolBar->addAction(m_pcEditCutAction);
   pcToolBar->addAction(m_pcEditPasteAction);
   // pcToolBar->addAction(m_pcEditFindAction);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateCodeActions() {
   QIcon cCodeExecuteIcon;
   cCodeExecuteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/execute.png"));
   m_pcCodeExecuteAction = new QAction(cCodeExecuteIcon, tr("&Execute"), this);
   m_pcCodeExecuteAction->setToolTip(tr("Execute code"));
   m_pcCodeExecuteAction->setStatusTip(tr("Execute code"));
   m_pcCodeExecuteAction->setShortcut(tr("Ctrl+E"));
   connect(m_pcCodeExecuteAction, SIGNAL(triggered()),
           this, SLOT(Execute()));
   QMenu* pcMenu = menuBar()->addMenu(tr("&Code"));
   pcMenu->addAction(m_pcCodeExecuteAction);
   QToolBar* pcToolBar = addToolBar(tr("Code"));
   pcToolBar->setObjectName("CodeToolBar");
   pcToolBar->setIconSize(QSize(32,32));
   pcToolBar->addAction(m_pcCodeExecuteAction);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::OpenFile(const QString& str_path) {
   QFile cFile(str_path);
   if(! cFile.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("ARGoS v" ARGOS_VERSION "-" ARGOS_RELEASE " - Buzz Editor"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(str_path)
                           .arg(cFile.errorString()));
      return;
   }
   QApplication::setOverrideCursor(Qt::WaitCursor);
   m_pcCodeEditor->setPlainText(cFile.readAll());
   QApplication::restoreOverrideCursor();
   SetCurrentFile(str_path);
   statusBar()->showMessage(tr("File loaded"), 2000);
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::SaveFile(const QString& str_path) {
   QFile cFile(str_path);
   if(! cFile.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("ARGoS v" ARGOS_VERSION "-" ARGOS_RELEASE " - Buzz Editor"),
                           tr("Cannot write file %1:\n%2.")
                           .arg(str_path)
                           .arg(cFile.errorString()));
      return false;
   }
   QTextStream cOut(&cFile);
   QApplication::setOverrideCursor(Qt::WaitCursor);
   cOut << m_pcCodeEditor->toPlainText();
   QApplication::restoreOverrideCursor();
   SetCurrentFile(str_path);
   statusBar()->showMessage(tr("File saved"), 2000);
   return true;
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetCurrentFile(const QString& str_path) {
   m_strFileName = str_path;
   QString strShownName;
   if(m_strFileName.isEmpty()) {
      strShownName = "untitled";
   }
   else {
      strShownName = StrippedFileName(m_strFileName);
   }
   setWindowTitle(tr("%1[*] - ARGoS v" ARGOS_VERSION "-" ARGOS_RELEASE " - Buzz Editor").arg(strShownName));
   if(!m_strFileName.isEmpty()) {
      m_pcCodeEditor->document()->setModified(false);
      setWindowModified(false);
      QSettings cSettings;
      cSettings.beginGroup("BuzzEditor");
      QStringList listFiles = cSettings.value("recent_files").toStringList();
      listFiles.removeAll(m_strFileName);
      listFiles.prepend(m_strFileName);
      while(listFiles.size() > MAX_RECENT_FILES) {
         listFiles.removeLast();
      }
      cSettings.setValue("recent_files", listFiles);
      cSettings.endGroup();
      UpdateRecentFiles();
   }
   else {
      m_pcCodeEditor->document()->setModified(true);
      setWindowModified(true);
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CheckBuzzStatus(int n_step) {
   int nRow = 0;
   m_pcBuzzMessageTable->clearContents();
   m_pcBuzzMessageTable->setRowCount(m_vecControllers.size());
   for(size_t i = 0; i < m_vecControllers.size(); ++i) {
      if(m_vecControllers[i]->GetBuzzVM()->state != BUZZVM_STATE_READY) {
         SetMessage(nRow,
                    QString::fromStdString(m_vecControllers[i]->GetId()),
                    QString::fromStdString(m_vecControllers[i]->ErrorInfo()));
         ++nRow;
      }
   }
   m_pcBuzzMessageTable->setRowCount(nRow);
   if(nRow > 0) {
      m_pcMainWindow->SuspendExperiment();
   }
   else {
      m_pcBuzzMsgDock->hide();
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleMsgTableSelection() {
   QList<QTableWidgetItem*> listSel = m_pcBuzzMessageTable->selectedItems();
   if(! listSel.empty()) {
      /* Get error position field */
      QString pos = listSel[1]->data(Qt::DisplayRole).toString();
      QStringList posFields = pos.split(":");
      /* Get line and column */
      int nLine = posFields[1].toInt();
      int nCol = posFields[2].toInt();
      /* Move to the position */
      QTextCursor cCursor = m_pcCodeEditor->textCursor();
      cCursor.setPosition(0, QTextCursor::MoveAnchor);
      cCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, nLine-1);
      cCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, nCol);
      m_pcCodeEditor->setTextCursor(cCursor);
      m_pcCodeEditor->setFocus();
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleEntitySelection(size_t un_index) {
   CComposableEntity* pcSelectedEntity = dynamic_cast<CComposableEntity*>(CSimulator::GetInstance().GetSpace().GetRootEntityVector()[un_index]);
   if(pcSelectedEntity != NULL) {
      bool bFound = false;
      m_unSelectedRobot = 0;
      while(!bFound && m_unSelectedRobot < m_vecRobots.size()) {
         if(m_vecRobots[m_unSelectedRobot] == pcSelectedEntity) {
            bFound = true;
         }
         else {
            ++m_unSelectedRobot;
         }
      }
      if(bFound &&
         m_vecControllers[m_unSelectedRobot]->GetBuzzVM() != NULL) {
         CBuzzQTStateTreeVariableModel* pcVarModel =
            new CBuzzQTStateTreeVariableModel(m_vecControllers[m_unSelectedRobot]->GetBuzzVM(),
                                              true,
                                              m_pcBuzzVariableTree);
         pcVarModel->Refresh();
         connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
                 pcVarModel, SLOT(Refresh(int)));
         connect(m_pcMainWindow, SIGNAL(ExperimentReset()),
                 pcVarModel, SLOT(Refresh()));
         connect(pcVarModel, SIGNAL(modelReset()),
                 this, SLOT(VariableTreeChanged()),
                 Qt::QueuedConnection);
         m_pcBuzzVariableTree->setModel(pcVarModel);
         m_pcBuzzVariableTree->expandAll();
         m_pcBuzzVariableDock->show();
         CBuzzQTStateTreeFunctionModel* pcFunModel =
            new CBuzzQTStateTreeFunctionModel(m_vecControllers[m_unSelectedRobot]->GetBuzzVM(),
                                              true,
                                              m_pcBuzzFunctionTree);
         pcFunModel->Refresh();
         connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
                 pcFunModel, SLOT(Refresh(int)));
         connect(m_pcMainWindow, SIGNAL(ExperimentReset()),
                 pcFunModel, SLOT(Refresh()));
         connect(pcFunModel, SIGNAL(modelReset()),
                 this, SLOT(FunctionTreeChanged()),
                 Qt::QueuedConnection);
         m_pcBuzzFunctionTree->setModel(pcFunModel);
         m_pcBuzzFunctionTree->expandAll();
         m_pcBuzzFunctionDock->show();
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleEntityDeselection(size_t) {
   disconnect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
              m_pcBuzzVariableTree->model(), SLOT(Refresh(int)));
   disconnect(m_pcMainWindow, SIGNAL(ExperimentReset()),
              m_pcBuzzVariableTree->model(), SLOT(Refresh()));
   disconnect(m_pcBuzzVariableTree->model(), SIGNAL(modelReset()),
              this, SLOT(VariableTreeChanged()));
   m_pcBuzzVariableDock->hide();
   delete m_pcBuzzVariableTree->model();
   m_pcBuzzVariableTree->setModel(NULL);
   disconnect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
              m_pcBuzzFunctionTree->model(), SLOT(Refresh(int)));
   disconnect(m_pcMainWindow, SIGNAL(ExperimentReset()),
              m_pcBuzzFunctionTree->model(), SLOT(Refresh()));
   disconnect(m_pcBuzzFunctionTree->model(), SIGNAL(modelReset()),
              this, SLOT(FunctionTreeChanged()));
   m_pcBuzzFunctionDock->hide();
   delete m_pcBuzzFunctionTree->model();
   m_pcBuzzFunctionTree->setModel(NULL);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::VariableTreeChanged() {
   m_pcBuzzVariableTree->setRootIndex(m_pcBuzzVariableTree->model()->index(0, 0));
   m_pcBuzzVariableTree->expandAll();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::FunctionTreeChanged() {
   m_pcBuzzFunctionTree->setRootIndex(m_pcBuzzFunctionTree->model()->index(0, 0));
   m_pcBuzzFunctionTree->expandAll();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::UpdateRecentFiles() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   QStringList listFiles = cSettings.value("recent_files").toStringList();
   int nRecentFiles = qMin(listFiles.size(), (int)MAX_RECENT_FILES);
   for(int i = 0; i < nRecentFiles; ++i) {
      m_pcFileOpenRecentAction[i]->setText(tr("&%1 %2").arg(i+1).arg(StrippedFileName(listFiles[i])));
      m_pcFileOpenRecentAction[i]->setData(listFiles[i]);
      m_pcFileOpenRecentAction[i]->setVisible(true);
   }
   for(int i = nRecentFiles; i < MAX_RECENT_FILES; ++i) {
      m_pcFileOpenRecentAction[i]->setVisible(false);
   }
   m_pcFileSeparateRecentAction->setVisible(nRecentFiles > 0);
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetMessage(int n_row,
                                   const QString& str_robot_id,
                                   const QString& str_message) {
   /* Fill in the robot id */
   m_pcBuzzMessageTable->setItem(
      n_row, 0,
      new QTableWidgetItem(str_robot_id));
   /* Was compilation successful? */
   if(str_message == "Compilation successful.") {
      m_pcBuzzMessageTable->setItem(
         n_row, 2,
         new QTableWidgetItem(str_message));
      m_pcBuzzMsgDock->show();
      return;
   }
   /* Split the error message in its parts */
   QStringList listFields = str_message.split(
      ":",
      QString::KeepEmptyParts,
      Qt::CaseInsensitive);
   /* Set position and error message (byte offset or file/line/col */
   if(str_message.startsWith("At bytecode offset", Qt::CaseInsensitive)) {
      /* Position */
      m_pcBuzzMessageTable->setItem(
         n_row, 1,
         new QTableWidgetItem(listFields[0]));
      /* Error message */
      QString strErr = listFields[1].trimmed();
      for(int i = 2; i < listFields.size(); ++i)
         strErr += ": " + listFields[i].trimmed();
      m_pcBuzzMessageTable->setItem(
         n_row, 2,
         new QTableWidgetItem(strErr));
   }
   else {
      /* Position */
      m_pcBuzzMessageTable->setItem(
         n_row, 1,
         new QTableWidgetItem(listFields[0] + ":" + listFields[1] + ":" + listFields[2]));
      /* Error message */
      QString strErr = listFields[3].trimmed();
      for(int i = 4; i < listFields.size(); ++i)
         strErr += ": " + listFields[i].trimmed();
      m_pcBuzzMessageTable->setItem(
         n_row, 2,
         new QTableWidgetItem(strErr));
   }
   m_pcBuzzMsgDock->show();
}

/****************************************/
/****************************************/

QString CBuzzQTMainWindow::StrippedFileName(const QString& str_path) {
   return QFileInfo(str_path).fileName();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::closeEvent(QCloseEvent* pc_event) {
   pc_event->ignore();
}

/****************************************/
/****************************************/
