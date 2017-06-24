/*
 * Buzz includes
 */
#include "../buzzparser.h"
#include "buzz_controller.h"
#include "buzz_qt_main_window.h"
#include "buzz_qt_editor.h"
#include "buzz_qt_statetree_model.h"

/*
 * ARGoS includes
 */
#include <argos3/core/config.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_main_window.h>
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>

/*
 * Qt includes
 */
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeView>

/****************************************/
/****************************************/

/*
 * Returns the stripped version of the file name
 */
static QString StrippedFileName(const QString& str_path) {
   return QFileInfo(str_path).fileName();
}

/****************************************/
/****************************************/

CBuzzQTMainWindow::CBuzzQTMainWindow(CQTOpenGLMainWindow* pc_parent) :
   QMainWindow(pc_parent),
   m_pcMainWindow(pc_parent) {
   /* Set window title */
   setWindowTitle(tr("Buzz Editor"));
   /* Add a status bar */
   m_pcStatusbar = new QStatusBar(this);
   setStatusBar(m_pcStatusbar);
   /* Add the tab container for the editors */
   m_pcEditors = new CBuzzQTTabWidget(this);
   m_pcEditors->setDocumentMode(true);
   m_pcEditors->setMovable(true);
   m_pcEditors->setTabsClosable(true);
   m_pcEditors->setTabBarAutoHide(false);
   setCentralWidget(m_pcEditors);
   connect(m_pcEditors, SIGNAL(tabCloseRequested(int)),
           this, SLOT(CloseEditor(int)));
   connect(m_pcEditors, SIGNAL(TabRemoved()),
           this, SLOT(UpdateActions()));
   connect(m_pcEditors, SIGNAL(TabRemoved()),
           this, SLOT(UpdateSetMainScriptActions()));
   /* Use in-window menubar */
   menuBar()->setNativeMenuBar(false);
   /* Create actions */
   CreateFileActions();
   CreateEditActions();
   CreateScriptActions();
   UpdateActions();
   /* Create dockable widgets */
   CreateCompilationDock();
   CreateRunTimeErrorDock();
   CreateBuzzStateDocks();
   /* Restore settings */
   ReadSettings();
   UpdateRecentFilesActions();
   /* Go through the robots */
   PopulateBuzzControllers();
   /* Hide docks */
   m_pcBuzzVariableDock->hide();
   m_pcBuzzFunctionDock->hide();
   /* Check buzz status every step */
   connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
           this, SLOT(CheckBuzzStatus(int)));
}

/****************************************/
/****************************************/

CBuzzQTMainWindow::~CBuzzQTMainWindow() {
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::closeEvent(QCloseEvent* pc_event) {
   /* Save all that must be saved */
   for(int i = 0; i < m_pcEditors->count(); ++i) {
      if(!m_pcEditors->widget(i)->close()) {
         /* One editor was not saved, yet was modified - no exit */
         pc_event->ignore();
         return;
      }
      m_pcEditors->removeTab(i);
   }
   /* Write settings */
   WriteSettings();
   /* Stop the simulation and close everything */
   QApplication::quit();
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::OpenFile(const QString& str_path) {
   /* Get canonical path */
   QString strCanPath =
      QFileInfo(str_path).exists() ?
      QFileInfo(str_path).canonicalFilePath():
      str_path;
   /* Search through tabs to make sure the file has not been found already */
   int i = 0;
   CBuzzQTEditor* pcEditor = NULL;
   bool bFound = false;
   for(; i < m_pcEditors->count() && !bFound; ++i) {
      pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->widget(i));
      if(!pcEditor) fprintf(stderr, "[BUG] tab %d is not a buzz editor??\n", i);
      else bFound = (pcEditor->GetScriptPath() == strCanPath);
   }
   /* If found, switch to that tab and return true */
   if(bFound) {
      m_pcEditors->setCurrentIndex(i-1);
      return true;
   }
   /* Otherwise, create a new editor */
   pcEditor = new CBuzzQTEditor(strCanPath);
   /* Handle its signals */
   connect(pcEditor, SIGNAL(RecentFilesChanged()),
           this, SLOT(UpdateRecentFilesActions()));
   /* Attempt to load the file */
   if(pcEditor->Open()) {
      /* Success */
      /* Add tab and focus on it */
      int nIdx = m_pcEditors->addTab(pcEditor, StrippedFileName(strCanPath));
      m_pcEditors->setCurrentIndex(nIdx);
      /* Handle its signals */
      connect(pcEditor, SIGNAL(modificationChanged(bool)),
              this, SLOT(SetTabModified(bool)));
      /* Update main script, if needed */
      if(m_strMainScript.isEmpty())
         SetMainScript(strCanPath);
      /* Update enabled state of the actions */
      UpdateActions();
      UpdateSetMainScriptActions();
      statusBar()->showMessage(tr("File opened"), 2000);
      return true;
   }
   else {
      /* File could not be loaded */
      /* Delete the editor */
      disconnect(pcEditor, SIGNAL(RecentFilesChanged()),
                 this, SLOT(UpdateRecentFilesActions()));
      delete pcEditor;
      return false;
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::NewOpen() {
   /* Choose file name using dialog */
   QFileDialog cFileDiag(this);
   cFileDiag.setFileMode(QFileDialog::AnyFile);
   cFileDiag.setNameFilter(tr("Buzz scripts (*.bzz)"));
   cFileDiag.setOptions(
      QFileDialog::DontConfirmOverwrite |
      QFileDialog::DontUseNativeDialog);
   cFileDiag.setDefaultSuffix("bzz");
   QString strFName;
   if(cFileDiag.exec()) {
      QStringList strFNames = cFileDiag.selectedFiles();
      if(strFNames.size() > 0) {
         strFName = strFNames[0];
      }
   }
   /* If no file was chosen, return */
   if(strFName.isEmpty()) return;
   /* Try to open the file */
   if(!OpenFile(strFName)) {
      /* The file was not open */
      /* Create editor */
      CBuzzQTEditor* pcEditor = new CBuzzQTEditor(strFName);
      /* Handle its signals */
      connect(pcEditor, SIGNAL(modificationChanged(bool)),
              this, SLOT(SetTabModified(bool)));
      connect(pcEditor, SIGNAL(RecentFilesChanged()),
              this, SLOT(UpdateRecentFilesActions()));
      /* Add tab and focus on it */
      int nIdx = m_pcEditors->addTab(pcEditor, StrippedFileName(strFName));
      m_pcEditors->setCurrentIndex(nIdx);
      /* Make empty editor */
      pcEditor->New();
      /* Update enabled state of the actions */
      UpdateActions();
      UpdateSetMainScriptActions();
      statusBar()->showMessage(tr("New file created"), 2000);
   }
   /* Update main script, if needed */
   if(m_strMainScript.isEmpty())
      SetMainScript(strFName);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::OpenRecentFile() {
   /* Get signal sender as cast it as QAction */
   QAction* pcRecFile = qobject_cast<QAction*>(sender());
   if(pcRecFile)
      /* Open file pointed to by the QAction */
      OpenFile(pcRecFile->data().toString());
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Save() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = qobject_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      if(pcEditor->Save()) {
         /* Update message */
         statusBar()->showMessage(tr("Script saved"), 2000);
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SaveAs() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      if(pcEditor->SaveAs()) {
         /* Update tab title */
         m_pcEditors->setTabText(
            m_pcEditors->currentIndex(),
            StrippedFileName(pcEditor->GetScriptPath()));
         /* Update message */
         statusBar()->showMessage(tr("Script saved"), 2000);
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SaveAll() {
   /* Save all that must be saved */
   for(int i = 0; i < m_pcEditors->count(); ++i)
      qobject_cast<CBuzzQTEditor*>(m_pcEditors->widget(i))->Save();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Undo() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      pcEditor->undo();
   }
   /* Update message */
   statusBar()->showMessage(tr("Undo!"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Redo() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      pcEditor->redo();
   }
   /* Update message */
   statusBar()->showMessage(tr("Redo!"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Copy() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      pcEditor->copy();
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Cut() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      pcEditor->cut();
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Paste() {
   /* Make sure a tab is currently focused */
   if(m_pcEditors->currentWidget()) {
      /* Get editor */
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->currentWidget());
      if(!pcEditor) {
         fprintf(stderr, "[BUG] pcEditor is NULL\n");
         return;
      }
      /* Perform operation */
      pcEditor->paste();
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Find() {
   statusBar()->showMessage(tr("Edit -> Find"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetMain() {
   /* Get signal sender as cast it as QAction */
   QAction* pcMainFile = qobject_cast<QAction*>(sender());
   if(pcMainFile)
      /* Make main the file pointed to by the QAction */
      SetMainScript(pcMainFile->data().toString());
   statusBar()->showMessage(tr("Set main script"), 2000);
}

/****************************************/
/****************************************/

bool CBuzzQTMainWindow::Compile() {
   /* Make a main script was set */
   if(m_strMainScript.isEmpty()) {
      QMessageBox::critical(
         this,
         "Main script not set",
         "The main script has not been set!");
      return false;
   }
   /* Save everything */
   SaveAll();
   /* Change cursor */
   QApplication::setOverrideCursor(Qt::WaitCursor);
   /* Stop simulation */
   m_pcMainWindow->SuspendExperiment();
   /*
    * Call buzz compiler
    */
   QProcess cBuzzCompiler;
   cBuzzCompiler.start("bzzc",
                       QStringList() <<
                       "-b" << m_strMainBcode <<
                       "-d" << m_strMainDbgInfo <<
                       m_strMainScript);
   if(! cBuzzCompiler.waitForFinished() || cBuzzCompiler.exitCode() != 0) {
      /* Compilation error, delete files */
      QFile cBcodeFile(m_strMainBcode);
      cBcodeFile.remove();
      QFile cDbgInfoFile(m_strMainDbgInfo);
      cDbgInfoFile.remove();
      QApplication::restoreOverrideCursor();
      /* Show compilation error */
      m_pcCompilationMsg->setPlainText(
         cBuzzCompiler.readAllStandardError());
      /* Jump to error line, if error message allows it */
      QRegExp cRE("^[a-zA-Z0-9_/.]+:[0-9]+:[0-9]+");
      QString strErrorMsg = m_pcCompilationMsg->toPlainText();
      if(cRE.indexIn(strErrorMsg) == 0) {
         /* Parse file, line, column */
         QStringList cFields = strErrorMsg.split(":");
         /* Open file */
         OpenFile(cFields[0]);
         /* Move to the position */
         qobject_cast<CBuzzQTEditor*>(
            m_pcEditors->currentWidget())->GoTo(cFields[1].toInt(),
                                                cFields[2].toInt());
      }
      statusBar()->showMessage(tr("Compilation failed"), 2000);
      return false;
   }
   /* Compilation successful */
   QApplication::restoreOverrideCursor();
   qobject_cast<QPlainTextEdit*>(m_pcCompilationDock->widget())->document()->setPlainText(cBuzzCompiler.readAllStandardOutput());
   statusBar()->showMessage(tr("Compilation successful"), 2000);
   /* Update Buzz state if visible */
   if(m_pcBuzzVariableDock->isVisible()) {
      static_cast<CBuzzQTStateTreeModel*>(
         m_pcBuzzVariableTree->model())->Refresh();
   }
   if(m_pcBuzzFunctionDock->isVisible()) {
      static_cast<CBuzzQTStateTreeModel*>(
         m_pcBuzzFunctionTree->model())->Refresh();
   }
   return true;
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::Execute() {
   /* Stop simulation */
   m_pcMainWindow->SuspendExperiment();
   /* Make sure the compiled files exist */
   QFile cBcodeFile(m_strMainBcode);
   QFile cDbgInfoFile(m_strMainDbgInfo);
   if(!cBcodeFile.exists() || !cDbgInfoFile.exists() ||
      QFileInfo(m_strMainScript).lastModified() >
      QFileInfo(cBcodeFile).lastModified())
      if(!Compile()) return;
   /* Set the script for all the robots */
   QApplication::setOverrideCursor(Qt::WaitCursor);
   for(size_t i = 0; i < m_vecControllers.size(); ++i) {
      m_vecControllers[i]->SetBytecode(m_strMainBcode.toStdString(),
                                       m_strMainDbgInfo.toStdString());
   }
   QApplication::restoreOverrideCursor();
   /* Clear the error table */
   m_pcRunTimeErrorTable->clearContents();
   m_pcRunTimeErrorTable->setRowCount(1);
   /* Resume simulation */
   m_pcMainWindow->ResumeExperiment();
   statusBar()->showMessage(tr("Script in execution"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CloseEditor(int n_idx) {
   /* Make sure there's an editor to close */
   if(!m_pcEditors->widget(n_idx)) return;
   /* Get editor */
   CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(m_pcEditors->widget(n_idx));
   if(!pcEditor) {
      fprintf(stderr, "[BUG] pcEditor is NULL\n");
      return;
   }
   /* Empty main script if this editor was the main script */
   if(pcEditor->GetScriptPath() == m_strMainScript) {
      SetMainScript("");
   }
   /* Close editor (this might require saving) */
   pcEditor->close();
   statusBar()->showMessage(tr("Closed"), 2000);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetTabModified(bool b_modified) {
   if(sender()) {
      CBuzzQTEditor* pcEditor = dynamic_cast<CBuzzQTEditor*>(sender());
      if(pcEditor) {
         int nIdx = m_pcEditors->indexOf(pcEditor);
         if(nIdx >= 0) {
            QString strLabel = StrippedFileName(pcEditor->GetScriptPath());
            if(b_modified) {
               strLabel += " [*]";
            }
            m_pcEditors->setTabText(nIdx, strLabel);
         }
         else {
            fprintf(stderr, "[BUG] Tab widget not found in SetTabModified()\n");
         }
      }
      else {
         fprintf(stderr, "[BUG] Not-editor sender for SetTabModified()\n");
      }
   }
   else {
      fprintf(stderr, "[BUG] NULL sender for SetTabModified()\n");
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::UpdateActions() {
   /* Is an editor open? */
   bool bEditor = m_pcEditors->currentWidget();
   /* Set the state of the actions */
   m_pcFileSaveAction->setEnabled(bEditor);
   m_pcFileSaveAsAction->setEnabled(bEditor);
   m_pcFileSaveAllAction->setEnabled(bEditor);
   m_pcEditUndoAction->setEnabled(bEditor);
   m_pcEditRedoAction->setEnabled(bEditor);
   m_pcEditCopyAction->setEnabled(bEditor);
   m_pcEditCutAction->setEnabled(bEditor);
   m_pcEditPasteAction->setEnabled(bEditor);
   // m_pcEditFindAction->setEnabled(bEditor);
   // m_pcScriptCompileAction->setEnabled(bEditor);
   m_pcScriptExecuteAction->setEnabled(bEditor);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::UpdateRecentFilesActions() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   /* Get list of recent files */
   QStringList cFiles = cSettings.value("recentFiles").toStringList();
   /* Get number of files to show */
   int nFileCount = qMin(cFiles.size(), (int)MAX_RECENT_FILES);
   /* Go through the files and make them visible in the menu */
   for(int i = 0; i < nFileCount; ++i) {
      QString strText = QString("&%1 %2")
         .arg(i + 1)
         .arg(StrippedFileName(cFiles[i]));
      m_pcFileOpenRecentAction[i]->setText(strText);
      m_pcFileOpenRecentAction[i]->setData(cFiles[i]);
      m_pcFileOpenRecentAction[i]->setVisible(true);
   }
   /* Make extra actions invisibile */
   for(int i = nFileCount; i < MAX_RECENT_FILES; ++i) {
      m_pcFileOpenRecentAction[i]->setVisible(false);
   }
   /* Show separator if necessary */
   m_pcFileSeparateRecentAction->setVisible(nFileCount > 0);
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::UpdateSetMainScriptActions() {
   /* Remove all actions */
   while(!m_pcScriptSetMainMenu->isEmpty()) {
      QAction* pcToDel = m_pcScriptSetMainMenu->actions().first();
      m_pcScriptSetMainMenu->removeAction(pcToDel);
      delete pcToDel;
   }
   /* Add all actions */
   for(int i = 0; i < m_pcEditors->count(); ++i) {
      CBuzzQTEditor* pcEditor = qobject_cast<CBuzzQTEditor*>(m_pcEditors->widget(i));
      QString strTxt = tr("&%1 %2")
         .arg(i+1)
         .arg(StrippedFileName(pcEditor->GetScriptPath()));
      QAction* pcAct = new QAction(strTxt, this);
      pcAct->setData(pcEditor->GetScriptPath());
      connect(pcAct, SIGNAL(triggered()),
              this, SLOT(SetMain()));
      m_pcScriptSetMainMenu->addAction(pcAct);
   }
   /* Enable/disable menu */
   m_pcScriptSetMainMenu->setEnabled(!m_pcScriptSetMainMenu->isEmpty());
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CheckBuzzStatus(int n_step) {
   int nRow = 0;
   m_pcRunTimeErrorTable->clearContents();
   m_pcRunTimeErrorTable->setRowCount(m_vecControllers.size());
   for(size_t i = 0; i < m_vecControllers.size(); ++i) {
      if(m_vecControllers[i]->GetBuzzVM()->state != BUZZVM_STATE_READY) {
         SetRunTimeError(nRow,
                         QString::fromStdString(m_vecControllers[i]->GetId()),
                         QString::fromStdString(m_vecControllers[i]->ErrorInfo()));
         ++nRow;
      }
   }
   m_pcRunTimeErrorTable->setRowCount(nRow);
   if(nRow > 0)
      m_pcMainWindow->SuspendExperiment();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleRunTimeErrorSelection() {
   QList<QTableWidgetItem*> cSel = m_pcRunTimeErrorTable->selectedItems();
   if(!cSel.empty()) {
      /* Select corresponding robot */
      m_pcMainWindow->GetOpenGLWidget().SelectEntity(
         CSimulator::GetInstance().GetSpace().GetEntity(
            cSel[0]->data(Qt::DisplayRole).toString().toStdString()));
      /* Get error position field */
      QString strPos = cSel[1]->data(Qt::DisplayRole).toString();
      QRegExp cRE("^[a-zA-Z0-9_/.]+:[0-9]+:[0-9]+");
      if(cRE.indexIn(strPos) == 0) {
         /* Parse file, line, column */
         QStringList cFields = strPos.split(":");
         /* Open file */
         OpenFile(cFields[0]);
         /* Move to the position */
         qobject_cast<CBuzzQTEditor*>(
            m_pcEditors->currentWidget())->GoTo(cFields[1].toInt(),
                                                cFields[2].toInt());
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleEntitySelection(size_t un_index) {
   /* Get selected entity */
   CComposableEntity* pcSelectedEntity =
      dynamic_cast<CComposableEntity*>(
         CSimulator::GetInstance().GetSpace().
         GetRootEntityVector()[un_index]);
   /* Make sure it's a composable entity */
   if(pcSelectedEntity != NULL) {
      /* Search for it in the list of entities managed by the editor */
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
      /* Was it found? */
      if(bFound &&
         m_vecControllers[m_unSelectedRobot]->GetBuzzVM() != NULL) {
         /* Create a new tree model */
         CBuzzQTStateTreeVariableModel* pcVarModel =
            new CBuzzQTStateTreeVariableModel(
               m_vecControllers[m_unSelectedRobot],
               true,
               m_pcBuzzVariableTree);
         /* Refresh the model */
         pcVarModel->Refresh();
         /* Handle signals and slots */
         connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
                 pcVarModel, SLOT(Refresh(int)));
         connect(m_pcMainWindow, SIGNAL(ExperimentReset()),
                 pcVarModel, SLOT(Refresh()));
         connect(pcVarModel, SIGNAL(modelReset()),
                 this, SLOT(VariableTreeChanged()),
                 Qt::QueuedConnection);
         /* Set the model and show it */
         m_pcBuzzVariableTree->setModel(pcVarModel);
         m_pcBuzzVariableTree->expandAll();
         m_pcBuzzVariableDock->show();
         /* Create a new function model */
         CBuzzQTStateTreeFunctionModel* pcFunModel =
            new CBuzzQTStateTreeFunctionModel(
               m_vecControllers[m_unSelectedRobot],
               true,
               m_pcBuzzFunctionTree);
         /* Refresh the model */
         pcFunModel->Refresh();
         /* Handle signals and slots */
         connect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
                 pcFunModel, SLOT(Refresh(int)));
         connect(m_pcMainWindow, SIGNAL(ExperimentReset()),
                 pcFunModel, SLOT(Refresh()));
         connect(pcFunModel, SIGNAL(modelReset()),
                 this, SLOT(FunctionTreeChanged()),
                 Qt::QueuedConnection);
         /* Set the model and show it */
         m_pcBuzzFunctionTree->setModel(pcFunModel);
         m_pcBuzzFunctionTree->expandAll();
         m_pcBuzzFunctionDock->show();
      }
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::HandleEntityDeselection(size_t) {
   /* Disconnect signals */
   disconnect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
              m_pcBuzzVariableTree->model(), SLOT(Refresh(int)));
   disconnect(m_pcMainWindow, SIGNAL(ExperimentReset()),
              m_pcBuzzVariableTree->model(), SLOT(Refresh()));
   disconnect(m_pcBuzzVariableTree->model(), SIGNAL(modelReset()),
              this, SLOT(VariableTreeChanged()));
   /* Hide the dock */
   m_pcBuzzVariableDock->hide();
   /* Get rid of model */
   delete m_pcBuzzVariableTree->model();
   m_pcBuzzVariableTree->setModel(NULL);
   /* Disconnect signals */
   disconnect(&(m_pcMainWindow->GetOpenGLWidget()), SIGNAL(StepDone(int)),
              m_pcBuzzFunctionTree->model(), SLOT(Refresh(int)));
   disconnect(m_pcMainWindow, SIGNAL(ExperimentReset()),
              m_pcBuzzFunctionTree->model(), SLOT(Refresh()));
   disconnect(m_pcBuzzFunctionTree->model(), SIGNAL(modelReset()),
              this, SLOT(FunctionTreeChanged()));
   /* Hide the dock */
   m_pcBuzzFunctionDock->hide();
   /* Get rid of model */
   delete m_pcBuzzFunctionTree->model();
   m_pcBuzzFunctionTree->setModel(NULL);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::VariableTreeChanged() {
   QAbstractItemModel* pcModel = m_pcBuzzVariableTree->model();
   m_pcBuzzVariableTree->setModel(NULL);
   m_pcBuzzVariableTree->setModel(pcModel);
//   m_pcBuzzVariableTree->setRootIndex(m_pcBuzzVariableTree->model()->index(0, 0));
   m_pcBuzzVariableTree->expandAll();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::FunctionTreeChanged() {
   QAbstractItemModel* pcModel = m_pcBuzzFunctionTree->model();
   m_pcBuzzFunctionTree->setModel(NULL);
   m_pcBuzzFunctionTree->setModel(pcModel);
//   m_pcBuzzFunctionTree->setRootIndex(m_pcBuzzFunctionTree->model()->index(0, 0));
   m_pcBuzzFunctionTree->expandAll();
}

/****************************************/
/****************************************/

void GetBuzzScriptFromDbgInfo(const void* key, void* data, void* params) {
   buzzdebug_entry_t tDbgEntry = *reinterpret_cast<const buzzdebug_entry_t*>(key);
   QString strFname = tDbgEntry->fname;
   QStringList& cFnames = *reinterpret_cast<QStringList*>(params);
   if(!cFnames.contains(strFname))
      cFnames.append(strFname);
}

void CBuzzQTMainWindow::PopulateBuzzControllers() {
   /* Get list of controllable entities */
   CSpace& cSpace = CSimulator::GetInstance().GetSpace();
   CSpace::TMapPerType& tControllables = cSpace.GetEntitiesByType("controller");
   /* Go through them and
    * - keep a pointer to each Buzz controller
    * - keep a pointer to each Buzz-controlled robot
    * - make sure only one script was associated
    */
   QString cDbgFile;
   buzzdebug_t tDbgInfo = NULL;
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
         /* Get file name */
         if(cDbgFile.isEmpty() && !pcBuzzController->GetDbgInfoFName().empty()) {
            cDbgFile = pcBuzzController->GetDbgInfoFName().c_str();
            tDbgInfo = pcBuzzController->GetBuzzDbgInfo();
         }
         /* Make sure there are no other scripts */
         if(cDbgFile != pcBuzzController->GetDbgInfoFName().c_str())
            LOGERR << "[WARNING] Ignoring debug file '"
                   << pcBuzzController->GetDbgInfoFName()
                   << "' because it conflicts with '"
                   << cDbgFile.toStdString()
                   << "'"
                   << std::endl;
      }
      else {
         LOGERR << "[WARNING] Entity \""
                << pcControllable->GetParent().GetId()
                << "\" does not have a Buzz controller associated"
                << std::endl;
      }
   }
   /* Now we have the files associated to the controllers */
   if(tDbgInfo) {
      /* Go through the debug file and load all the contained files */
      QStringList cScripts;
      buzzdict_foreach(tDbgInfo->script2off,
                       GetBuzzScriptFromDbgInfo,
                       &cScripts);
      for(int i = 0; i < cScripts.size(); ++i)
         OpenFile(cScripts[i]);
      /* If exactly one script has been opened, that is the main one */
      if(cScripts.size() != 1)
         SetMainScript("");
      else
         SetMainScript(cScripts[0]);
   }
   else {
      SetMainScript("");
   }
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetMainScript(const QString& str_path) {
   /* This flag is used to decide whether the script file is set or not. */
   bool isSet = !str_path.isEmpty();
   m_strMainScript = str_path;
   m_strMainBcode = m_strMainScript.left(m_strMainScript.lastIndexOf('.') + 1) + "bo";
   m_strMainDbgInfo = m_strMainScript.left(m_strMainScript.lastIndexOf('.') + 1) + "bdb";
   /* Activate/deactivate menu items according to isSet flag */
   m_pcScriptExecuteAction->setEnabled(isSet);
   /* Change window title */
   if(isSet)
      setWindowTitle(tr("Buzz Editor - ") + StrippedFileName(m_strMainScript));
   else
      setWindowTitle(tr("Buzz Editor"));
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateFileActions() {
   QIcon cFileNewOpenIcon;
   cFileNewOpenIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/open.png"));
   m_pcFileNewOpenAction = new QAction(cFileNewOpenIcon, tr("&New/Open..."), this);
   m_pcFileNewOpenAction->setToolTip(tr("Create a new editor for new or existing file"));
   m_pcFileNewOpenAction->setStatusTip(tr("Create a new editor for new or existing file"));
   m_pcFileNewOpenAction->setShortcut(QKeySequence::New);
   connect(m_pcFileNewOpenAction, SIGNAL(triggered()),
           this, SLOT(NewOpen()));
   QIcon cFileSaveIcon;
   cFileSaveIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/save.png"));
   m_pcFileSaveAction = new QAction(cFileSaveIcon, tr("&Save"), this);
   m_pcFileSaveAction->setToolTip(tr("Save the current file"));
   m_pcFileSaveAction->setStatusTip(tr("Save the current file"));
   m_pcFileSaveAction->setShortcut(QKeySequence::Save);
   connect(m_pcFileSaveAction, SIGNAL(triggered()),
           this, SLOT(Save()));
   QIcon cFileSaveAsIcon;
   cFileSaveAsIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/save.png"));
   m_pcFileSaveAsAction = new QAction(cFileSaveAsIcon, tr("S&ave as..."), this);
   m_pcFileSaveAsAction->setToolTip(tr("Save the current file under a new name"));
   m_pcFileSaveAsAction->setStatusTip(tr("Save the current file under a new name"));
   m_pcFileSaveAsAction->setShortcut(QKeySequence::SaveAs);
   connect(m_pcFileSaveAsAction, SIGNAL(triggered()),
           this, SLOT(SaveAs()));
   m_pcFileSaveAllAction = new QAction(tr("Sa&ve all"), this);
   m_pcFileSaveAllAction->setToolTip(tr("Save all the currently open files"));
   m_pcFileSaveAllAction->setStatusTip(tr("Save all the currently open files"));
   connect(m_pcFileSaveAllAction, SIGNAL(triggered()),
           this, SLOT(SaveAll()));
   QMenu* pcMenu = menuBar()->addMenu(tr("&File"));
   pcMenu->addAction(m_pcFileNewOpenAction);
   pcMenu->addSeparator();
   pcMenu->addAction(m_pcFileSaveAction);
   pcMenu->addAction(m_pcFileSaveAsAction);
   pcMenu->addAction(m_pcFileSaveAllAction);
   m_pcFileSeparateRecentAction = pcMenu->addSeparator();
   for (int i = 0; i < MAX_RECENT_FILES; ++i) {
      m_pcFileOpenRecentAction[i] = new QAction(this);
      m_pcFileOpenRecentAction[i]->setVisible(false);
      pcMenu->addAction(m_pcFileOpenRecentAction[i]);
      connect(m_pcFileOpenRecentAction[i], SIGNAL(triggered()),
              this, SLOT(OpenRecentFile()));
   }
   QToolBar* pcToolBar = addToolBar(tr("File"));
   pcToolBar->setObjectName("FileToolBar");
   pcToolBar->setIconSize(QSize(32,32));
   pcToolBar->addAction(m_pcFileNewOpenAction);
   pcToolBar->addAction(m_pcFileSaveAction);
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
           this, SLOT(Undo()));
   QIcon cEditRedoIcon;
   cEditRedoIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/redo.png"));
   m_pcEditRedoAction = new QAction(cEditRedoIcon, tr("&Redo"), this);
   m_pcEditRedoAction->setToolTip(tr("Redo last operation"));
   m_pcEditRedoAction->setStatusTip(tr("Redo last operation"));
   m_pcEditRedoAction->setShortcut(QKeySequence::Redo);
   connect(m_pcEditRedoAction, SIGNAL(triggered()),
           this, SLOT(Redo()));
   QIcon cEditCopyIcon;
   cEditCopyIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/copy.png"));
   m_pcEditCopyAction = new QAction(cEditCopyIcon, tr("&Copy"), this);
   m_pcEditCopyAction->setToolTip(tr("Copy selected text into clipboard"));
   m_pcEditCopyAction->setStatusTip(tr("Copy selected text into clipboard"));
   m_pcEditCopyAction->setShortcut(QKeySequence::Copy);
   connect(m_pcEditCopyAction, SIGNAL(triggered()),
           this, SLOT(Copy()));
   QIcon cEditCutIcon;
   cEditCutIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/cut.png"));
   m_pcEditCutAction = new QAction(cEditCutIcon, tr("&Cut"), this);
   m_pcEditCutAction->setToolTip(tr("Move selected text into clipboard"));
   m_pcEditCutAction->setStatusTip(tr("Move selected text into clipboard"));
   m_pcEditCutAction->setShortcut(QKeySequence::Cut);
   connect(m_pcEditCutAction, SIGNAL(triggered()),
           this, SLOT(Cut()));
   QIcon cEditPasteIcon;
   cEditPasteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/paste.png"));
   m_pcEditPasteAction = new QAction(cEditPasteIcon, tr("&Paste"), this);
   m_pcEditPasteAction->setToolTip(tr("Paste text from clipboard"));
   m_pcEditPasteAction->setStatusTip(tr("Paste text from clipboard"));
   m_pcEditPasteAction->setShortcut(QKeySequence::Paste);
   connect(m_pcEditPasteAction, SIGNAL(triggered()),
           this, SLOT(Paste()));
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

void CBuzzQTMainWindow::CreateScriptActions() {
   m_pcScriptCompileAction = new QAction(tr("&Compile"), this);
   m_pcScriptCompileAction->setToolTip(tr("Compile script"));
   m_pcScriptCompileAction->setStatusTip(tr("Compile script"));
   m_pcScriptCompileAction->setShortcut(tr("Ctrl+B"));
   connect(m_pcScriptCompileAction, SIGNAL(triggered()),
           this, SLOT(Compile()));
   QIcon cScriptExecuteIcon;
   cScriptExecuteIcon.addPixmap(QPixmap(m_pcMainWindow->GetIconDir() + "/execute.png"));
   m_pcScriptExecuteAction = new QAction(cScriptExecuteIcon, tr("&Execute"), this);
   m_pcScriptExecuteAction->setToolTip(tr("Execute script"));
   m_pcScriptExecuteAction->setStatusTip(tr("Execute script"));
   m_pcScriptExecuteAction->setShortcut(tr("Ctrl+E"));
   connect(m_pcScriptExecuteAction, SIGNAL(triggered()),
           this, SLOT(Execute()));
   m_pcScriptSetMainMenu = new QMenu(tr("&Set main script"), this);
   m_pcScriptSetMainMenu->setEnabled(false);
   QMenu* pcMenu = menuBar()->addMenu(tr("&Script"));
   pcMenu->addAction(m_pcScriptCompileAction);
   pcMenu->addAction(m_pcScriptExecuteAction);
   pcMenu->addMenu(m_pcScriptSetMainMenu);
   QToolBar* pcToolBar = addToolBar(tr("Script"));
   pcToolBar->setObjectName("ScriptToolBar");
   pcToolBar->setIconSize(QSize(32,32));
   pcToolBar->addAction(m_pcScriptExecuteAction);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateCompilationDock() {
   /* Create dock and make it movable, floatable, and not closable */
   m_pcCompilationDock = new QDockWidget(tr("Compilation"), this);
   m_pcCompilationDock->setObjectName("CompilationDock");
   m_pcCompilationDock->setFeatures(QDockWidget::DockWidgetMovable |
                                    QDockWidget::DockWidgetFloatable);
   /* Create plain text viewer */
   QPlainTextEdit* pcTextView = new QPlainTextEdit();
   pcTextView->setReadOnly(true);
   m_pcCompilationMsg = pcTextView->document();
   m_pcCompilationDock->setWidget(pcTextView);
   addDockWidget(Qt::BottomDockWidgetArea, m_pcCompilationDock);
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateRunTimeErrorDock() {
   /* Create dock and make it movable, floatable, and not closable */
   m_pcRunTimeErrorDock = new QDockWidget(tr("Run-time errors"), this);
   m_pcRunTimeErrorDock->setObjectName("RunTimeErrorDock");
   m_pcRunTimeErrorDock->setFeatures(QDockWidget::DockWidgetMovable |
                                    QDockWidget::DockWidgetFloatable);
   /* Create table */
   m_pcRunTimeErrorTable = new QTableWidget();
   m_pcRunTimeErrorTable->setColumnCount(3);
   QStringList cHeaders;
   cHeaders << tr("Robot")
            << tr("Location")
            << tr("Message");
   m_pcRunTimeErrorTable->setHorizontalHeaderLabels(cHeaders);
   m_pcRunTimeErrorTable->horizontalHeader()->setStretchLastSection(true);
   m_pcRunTimeErrorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_pcRunTimeErrorTable->setSelectionMode(QAbstractItemView::SingleSelection);
   m_pcRunTimeErrorDock->setWidget(m_pcRunTimeErrorTable);
   addDockWidget(Qt::BottomDockWidgetArea, m_pcRunTimeErrorDock);
   connect(m_pcRunTimeErrorTable, SIGNAL(itemSelectionChanged()),
           this, SLOT(HandleRunTimeErrorSelection()));
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::CreateBuzzStateDocks() {
   /* Variable tree dock */
   m_pcBuzzVariableDock = new QDockWidget(tr("Variables"), this);
   m_pcBuzzVariableDock->setObjectName("BuzzVariableDock");
   m_pcBuzzVariableDock->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);
   m_pcBuzzVariableTree = new QTreeView();
   m_pcBuzzVariableDock->setWidget(m_pcBuzzVariableTree);
   addDockWidget(Qt::LeftDockWidgetArea, m_pcBuzzVariableDock);
   m_pcBuzzVariableDock->hide();
   /* Function tree dock */
   m_pcBuzzFunctionDock = new QDockWidget(tr("Functions"), this);
   m_pcBuzzFunctionDock->setObjectName("BuzzFunctionDock");
   m_pcBuzzFunctionDock->setFeatures(QDockWidget::DockWidgetMovable |
                                     QDockWidget::DockWidgetFloatable);
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

void CBuzzQTMainWindow::WriteSettings() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   cSettings.setValue("size", size());
   cSettings.setValue("position", pos());
   cSettings.setValue("docks", saveState());
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::ReadSettings() {
   QSettings cSettings;
   cSettings.beginGroup("BuzzEditor");
   resize(cSettings.value("size", QSize(640,480)).toSize());
   move(cSettings.value("position", QPoint(0,0)).toPoint());
   restoreState(cSettings.value("docks").toByteArray());
   cSettings.endGroup();
}

/****************************************/
/****************************************/

void CBuzzQTMainWindow::SetRunTimeError(int n_row,
                                        const QString& str_robot_id,
                                        const QString& str_message) {
   /* Fill in the robot id */
   m_pcRunTimeErrorTable->setItem(
      n_row, 0,
      new QTableWidgetItem(str_robot_id));
   /* Was compilation successful? */
   if(str_message == "Compilation successful.") {
      m_pcRunTimeErrorTable->setItem(
         n_row, 2,
         new QTableWidgetItem(str_message));
      return;
   }
   /* Split the error message in its parts */
   QStringList listFields = str_message.split(
      ":",
      QString::KeepEmptyParts,
      Qt::CaseInsensitive);
   /* Set position and error message (byte offset or file/line/col */
   if(str_message == "Script not loaded!") {
      /* Error message */
      m_pcRunTimeErrorTable->setItem(
         n_row, 2,
         new QTableWidgetItem(str_message));
   }
   else if(str_message.startsWith("At bytecode offset", Qt::CaseInsensitive)) {
      /* Position */
      m_pcRunTimeErrorTable->setItem(
         n_row, 1,
         new QTableWidgetItem(listFields[0]));
      /* Error message */
      QString strErr = listFields[1].trimmed();
      for(int i = 2; i < listFields.size(); ++i)
         strErr += ": " + listFields[i].trimmed();
      m_pcRunTimeErrorTable->setItem(
         n_row, 2,
         new QTableWidgetItem(strErr));
   }
   else {
      /* Position */
      m_pcRunTimeErrorTable->setItem(
         n_row, 1,
         new QTableWidgetItem(listFields[0] + ":" + listFields[1] + ":" + listFields[2]));
      /* Error message */
      QString strErr = listFields[3].trimmed();
      for(int i = 4; i < listFields.size(); ++i)
         strErr += ": " + listFields[i].trimmed();
      m_pcRunTimeErrorTable->setItem(
         n_row, 2,
         new QTableWidgetItem(strErr));
   }
}

/****************************************/
/****************************************/
