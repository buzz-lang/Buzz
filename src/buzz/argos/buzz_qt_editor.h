#ifndef BUZZ_QT_EDITOR_H
#define BUZZ_QT_EDITOR_H

class CBuzzQTEditor;

#include <QPlainTextEdit>

class CBuzzQTEditor : public QPlainTextEdit {

   Q_OBJECT

public:

   /**
    * Class constructor.
    * @parent str_path The path of the script file.
    */
   CBuzzQTEditor(const QString& str_path);

   /**
    * Class destructor.
    */
   virtual ~CBuzzQTEditor() {}

   /**
    * Returns the canonical path to the script file.
    */
   QString GetScriptPath() const {
      return m_strScriptPath;
   }

   /**
    * Paints the line number on the left of the editor.
    * @param pc_event The paint event.
    */
   void LineNumberAreaPaintEvent(QPaintEvent* pc_event);

   /**
    * Returns the width of the line number area.
    */
   int LineNumberAreaWidth();

   /**
    * Sets the cursor at the given line and column.
    * @param n_line The line.
    * @param n_column The column.
    */
   void GoTo(int n_line, int n_column);

signals:

   /**
    * This signal is emitted when the list of recent files has changed.
    */
   void RecentFilesChanged();

public slots:

   /**
    * Populates the editor with a template.
    */
   void New();

   /**
    * Attempts to open the script file.
    * @return <tt>true</tt> for success, <tt>false</tt> for failure.
    */
   bool Open();

   /**
    * Attempts to save the script file.
    * @return <tt>true</tt> for success, <tt>false</tt> for failure.
    */
   bool Save();
      
   /**
    * Attempts to save the script file with a new name.
    * @return <tt>true</tt> for success, <tt>false</tt> for failure.
    */
   bool SaveAs();

protected:

   /**
    * Event handler for when the editor is closed.
    * @param pc_event The close event.
    */
   void closeEvent(QCloseEvent* pc_event);

   /**
    * Event handler for when the editor is resized.
    * @param pc_event The resize event.
    */
   void resizeEvent(QResizeEvent* pc_event);

private slots:

   /**
    * Slot for when the text is scrolled down or up.
    */
   void UpdateLineNumberAreaWidth(int);

   /**
    * Slot to highlight the current line.
    */
   void HighlightCurrentLine();

   /**
    * Slot for when the text is scrolled down or up.
    */
   void UpdateLineNumberArea(const QRect& c_rect, int n_dy);

   /**
    * Slot to mark the editor content as modified.
    */
   void CodeModified();

   /**
    * Updates the list of recent files.
    */
   void UpdateRecentFiles();

private:

   /********************/
   /********************/

   class CLineNumberArea : public QWidget {

   public:
      CLineNumberArea(CBuzzQTEditor* pc_editor) :
         QWidget(pc_editor) {
         m_pcEditor = pc_editor;
      }

      QSize sizeHint() const {
         return QSize(m_pcEditor->LineNumberAreaWidth(), 0);
      }

   protected:

      void paintEvent(QPaintEvent* pc_event) {
         m_pcEditor->LineNumberAreaPaintEvent(pc_event);
      }

   private:

      CBuzzQTEditor* m_pcEditor;

   };

   /********************/
   /********************/

   /** Number of recent files stored in the "File" menu */
   enum { MAX_RECENT_FILES = 5 };

   /** The canonical path to the script file */
   QString m_strScriptPath;

   /** The number line area */
   CLineNumberArea* m_pcLineNumberArea;

};

#endif
