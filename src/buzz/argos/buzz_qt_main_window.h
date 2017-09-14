#ifndef BUZZ_QT_MAIN_WINDOW_H
#define BUZZ_QT_MAIN_WINDOW_H

/*
 * Forward declarations.
 */
namespace argos {
   class CQTOpenGLMainWindow;
   class CComposableEntity;
}
class CBuzzQTMainWindow;
class CBuzzController;
class CBuzzQTTabWidget;
class QTableWidget;
class QTextDocument;
class QTreeView;

#include <QMainWindow>

using namespace argos;

/****************************************/
/****************************************/

class CBuzzQTTabWidget : public QTabWidget {

   Q_OBJECT

public:

   CBuzzQTTabWidget(QWidget* pc_parent) : QTabWidget(pc_parent) {}
   virtual ~CBuzzQTTabWidget() {}

signals:

   void TabRemoved();

protected:

   virtual void tabRemoved(int) {
      emit TabRemoved();
   }

};

/****************************************/
/****************************************/

class CBuzzQTMainWindow : public QMainWindow {

   Q_OBJECT

public:

   /**
    * Class constructor.
    * @param pc_parent The ARGoS main window.
    */
   CBuzzQTMainWindow(CQTOpenGLMainWindow* pc_parent);

   /**
    * The class destructor.
    */
   virtual ~CBuzzQTMainWindow();

   /**
    * Handler for when the this widget is closed.
    * Stop the simulation and close everything.
    */
   virtual void closeEvent(QCloseEvent* pc_event);

   /**
    * Opens the file at the given path.
    * If an open tab already refers to the given path, switch to that tab.
    * Otherwise, open a new tab.
    * @param str_path The path to open
    * @return <tt>true</tt> if operation succeeded, <tt>false</tt> if not
    */
   bool OpenFile(const QString& str_path);

public slots:

   /**
    * Create tab for a new or existing file.
    * The file name is chosen through a dialog window.
    */
   void NewOpen();

   /**
    * Open recently used file.
    */
   void OpenRecentFile();

   /**
    * Save file in current tab.
    */
   void Save();

   /**
    * Save file in current tab with a new name.
    * A dialog to choose the name is shown. The name in the tab is updated.
    */
   void SaveAs();

   /**
    * Save all the open files.
    */
   void SaveAll();

   /**
    * Undoes the last action.
    */
   void Undo();

   /**
    * Redoes the last undone action.
    */
   void Redo();

   /**
    * Copies text from the current tab into the clipboard.
    */
   void Copy();

   /**
    * Cuts text from the current tab into the clipboard.
    */
   void Cut();

   /**
    * Pastes text from the clipboard into the current tab.
    */
   void Paste();

   /**
    * Find/replace dialog on the current tab.
    */
   void Find();

   /**
    * Sets the main script.
    * The main script is the one to compile.
    */
   void SetMain();

   /**
    * Compiles the main script.
    * @return <tt>true</tt> if compilation was successful, <tt>false</tt> otherwise
    */
   bool Compile();

   /**
    * Executes the main script.
    */
   void Execute();

   /**
    * Closes an editor.
    */
   void CloseEditor(int n_idx);

   /**
    * Sets whether the tab has been modified.
    * @param b_modified
    */
   void SetTabModified(bool b_modified);

   /**
    * Updates the enabled/disabled state of the actions.
    */
   void UpdateActions();

   /**
    * Updates the actions corresponding to the recent files.
    */
   void UpdateRecentFilesActions();

   /**
    * Updates the actions corresponding to setting the main script.
    */
   void UpdateSetMainScriptActions();

   /**
    * Checks the status of the Buzz virtual machines at every step.
    * @param n_step The step.
    */
   void CheckBuzzStatus(int n_step);

   /**
    * Handles the selection of a row in the run-time error table.
    */
   void HandleRunTimeErrorSelection();

   /**
    * Handles the selection of an entity in ARGoS.
    * @param un_index The index of the selected entity.
    */
   void HandleEntitySelection(size_t un_index);

   /**
    * Handles the deselection of an entity in ARGoS.
    */
   void HandleEntityDeselection(size_t);

   /**
    * Handles changes in the variable tree.
    */
   void VariableTreeChanged();

   /**
    * Handles changes in the function tree.
    */
   void FunctionTreeChanged();

   /**
    * Updates the status bar with the latest line and column number info
    */
    void ReceiveLineAndColumnNumbers(int line, int column);

    /**
     * Updates the window title to the name of the file currently in the editor
     */
     void HandleEditorFileChange(QString& filename);

private:

   /**
    * Goes through the ARGoS controllers and loads the script
    * pointed to by the robots.
    */
   void PopulateBuzzControllers();

   /**
    * The full path to the main script file.
    * The main script file is passed to bzzc.
    * Upon setting the main script file, some menu items are activated.
    * Upon setting an empty main script file, some menu items are deactivated.
    */
   void SetMainScript(const QString& str_path);

   /**
    * Creates the Qt actions of the "File" menu.
    */
   void CreateFileActions();

   /**
    * Creates the Qt actions of the "Edit" menu.
    */
   void CreateEditActions();

   /**
    * Creates the Qt actions of the "Script" menu.
    */
   void CreateScriptActions();

   /**
    * Creates the dockable widget that displays the compilation results.
    */
   void CreateCompilationDock();

   /**
    * Creates the dockable widget that displays the compilation results.
    */
   void CreateRunTimeErrorDock();

   /**
    * Creates the dockable widget that displays the robot state.
    */
   void CreateBuzzStateDocks();

   /**
    * Writes the application settings.
    */
   void WriteSettings();

   /**
    * Reads the application settings.
    */
   void ReadSettings();

   /**
    * Sets a run-time error in the error table.
    */
   void SetRunTimeError(int n_row,
                        const QString& str_robot_id,
                        const QString& str_message);

private:

   /** The main script is the one passed to bzzc. This contains the full path. */
   QString m_strMainScript;
   /** The bytecode corresponding to the main script. This contains the full path. */
   QString m_strMainBcode;
   /** The debugging information corresponding to the main script. This contains the full path. */
   QString m_strMainDbgInfo;

   /** Currently selected robot in ARGoS */
   size_t m_unSelectedRobot;

   /** Number of recent files stored in the "File" menu */
   enum { MAX_RECENT_FILES = 5 };

   /** The main window of ARGoS */
   CQTOpenGLMainWindow* m_pcMainWindow;
   /** The Buzz controllers */
   std::vector<CBuzzController*> m_vecControllers;
   /** The robots */
   std::vector<CComposableEntity*> m_vecRobots;

   /** The status bar */
   QStatusBar* m_pcStatusbar;
   /** The editor tabs */
   CBuzzQTTabWidget* m_pcEditors;
   /** The compilation dock widget */
   QDockWidget* m_pcCompilationDock;
   /** Buffer for compilation messages */
   QTextDocument* m_pcCompilationMsg;
   /** The run-time error dock widget */
   QDockWidget* m_pcRunTimeErrorDock;
   /** The variable dock widget */
   QDockWidget* m_pcBuzzVariableDock;
   /** The function dock widget */
   QDockWidget* m_pcBuzzFunctionDock;
   /** The variable tree widget */
   QTreeView* m_pcBuzzVariableTree;
   /** The function tree widget */
   QTreeView* m_pcBuzzFunctionTree;
   /** Table of run-time error messages */
   QTableWidget* m_pcRunTimeErrorTable;

   /** Qt Action: new or open file */
   QAction* m_pcFileNewOpenAction;
   /** Qt Action: open recent file */
   QAction* m_pcFileOpenRecentAction[MAX_RECENT_FILES];
   /** Qt Action: save file */
   QAction* m_pcFileSaveAction;
   /** Qt Action: save file as */
   QAction* m_pcFileSaveAsAction;
   /** Qt Action: save all files */
   QAction* m_pcFileSaveAllAction;
   /** Qt Action: separator for recent files, shown only when recent files are recorded */
   QAction* m_pcFileSeparateRecentAction;
   /** Qt Action: undo */
   QAction* m_pcEditUndoAction;
   /** Qt Action: redo */
   QAction* m_pcEditRedoAction;
   /** Qt Action: copy */
   QAction* m_pcEditCopyAction;
   /** Qt Action: cut */
   QAction* m_pcEditCutAction;
   /** Qt Action: paste */
   QAction* m_pcEditPasteAction;
   /** Qt Action: find/replace */
   QAction* m_pcEditFindAction;
   /** Qt Action: compile script */
   QAction* m_pcScriptCompileAction;
   /** Qt Action: execute script */
   QAction* m_pcScriptExecuteAction;
   /** Qt menu: set main script */
   QMenu* m_pcScriptSetMainMenu;

};

#endif
