#ifndef BUZZ_QT_EDITOR_H
#define BUZZ_QT_EDITOR_H

class CBuzzQTEditor;

#include <QPlainTextEdit>

class CBuzzQTEditor : public QPlainTextEdit {

   Q_OBJECT

public:

   CBuzzQTEditor(QWidget* pc_parent);
   virtual ~CBuzzQTEditor() {}
      
   void LineNumberAreaPaintEvent(QPaintEvent* pc_event);
   int LineNumberAreaWidth();

protected:

   void resizeEvent(QResizeEvent* pc_event);

private slots:

   void UpdateLineNumberAreaWidth(int);
   void HighlightCurrentLine();
   void UpdateLineNumberArea(const QRect& c_rect, int n_dy);

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

   CLineNumberArea* m_pcLineNumberArea;

};

#endif
