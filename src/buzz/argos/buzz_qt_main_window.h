#ifndef BUZZ_QT_MAIN_WINDOW_H
#define BUZZ_QT_MAIN_WINDOW_H

namespace argos {
   class CQTOpenGLMainWindow;
   class CComposableEntity;
}

class CBuzzQTMainWindow;
class CBuzzQTEditor;
class CBuzzQTFindDialog;
class CBuzzController;

class QAction;
class QStatusBar;
class QTableWidget;
class QTreeView;

#include <QMainWindow>

using namespace argos;

class CBuzzQTMainWindow : public QMainWindow {

   Q_OBJECT

public:

   CBuzzQTMainWindow(CQTOpenGLMainWindow* pc_parent);
   virtual ~CBuzzQTMainWindow();

public slots:

   void New();
   void Open();
   void OpenRecentFile();
   bool Save();
   bool SaveAs();
   void Execute();
   void Find();
   void CodeModified();
   void CheckBuzzStatus(int n_step);
   void HandleMsgTableSelection();
   void HandleEntitySelection(size_t un_index);
   void HandleEntityDeselection(size_t);
   void VariableTreeChanged();
   void FunctionTreeChanged();

private:

   bool MaybeSave();
   void PopulateBuzzControllers();
   void ReadSettings();
   void WriteSettings();
   void CreateCodeEditor();
   void CreateBuzzMessageTable();
   void CreateBuzzStateDocks();
   void CreateFileActions();
   void CreateEditActions();
   void CreateCodeActions();
   void OpenFile(const QString& str_path = QString());
   bool SaveFile(const QString& str_path = QString());
   void SetCurrentFile(const QString& str_path);
   void UpdateRecentFiles();
   void SetMessage(int n_row,
                   const QString& str_robot_id,
                   const QString& str_message);

   QString StrippedFileName(const QString& str_path);

   virtual void closeEvent(QCloseEvent* pc_event);

private:

   enum { MAX_RECENT_FILES = 5 };

   CQTOpenGLMainWindow* m_pcMainWindow;
   QStatusBar* m_pcStatusbar;
   CBuzzQTEditor* m_pcCodeEditor;
   CBuzzQTFindDialog* m_pcFindDialog;
   QDockWidget* m_pcBuzzMsgDock;
   QTableWidget* m_pcBuzzMessageTable;
   QDockWidget* m_pcBuzzVariableDock;
   QDockWidget* m_pcBuzzFunctionDock;
   QTreeView* m_pcBuzzVariableTree;
   QTreeView* m_pcBuzzFunctionTree;

   std::vector<CBuzzController*> m_vecControllers;
   std::vector<CComposableEntity*> m_vecRobots;
   size_t m_unSelectedRobot;
   QString m_strFileName;

   QAction* m_pcFileNewAction;
   QAction* m_pcFileOpenAction;
   QAction* m_pcFileOpenRecentAction[MAX_RECENT_FILES];
   QAction* m_pcFileSaveAction;
   QAction* m_pcFileSaveAsAction;
   QAction* m_pcFileSeparateRecentAction;
   QAction* m_pcEditUndoAction;
   QAction* m_pcEditRedoAction;
   QAction* m_pcEditCopyAction;
   QAction* m_pcEditCutAction;
   QAction* m_pcEditPasteAction;
   QAction* m_pcEditFindAction;
   QAction* m_pcCodeExecuteAction;

};

#endif
