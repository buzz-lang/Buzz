#ifndef BUZZ_QT_SYNTAX_HIGHLIGHTER_H
#define BUZZ_QT_SYNTAX_HIGHLIGHTER_H

class CBuzzQTSyntaxHighlighter;

#include <QHash>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class CBuzzQTSyntaxHighlighter : public QSyntaxHighlighter {

   Q_OBJECT

public:

   CBuzzQTSyntaxHighlighter(QTextDocument* pc_text);
   virtual ~CBuzzQTSyntaxHighlighter() {}

protected:

   virtual void highlightBlock(const QString& str_text);

   bool StringStart(const QString& str_text,
                    int* pn_start_idx,
                    QChar* pc_delim);

private:
      
   struct SHighlightingRule
   {
      QRegExp Pattern;
      QTextCharFormat Format;
   };
   QVector<SHighlightingRule> m_vecHighlightingRules;

   QTextCharFormat m_cKeywordFormat;
   QTextCharFormat m_cIdFormat;
   QTextCharFormat m_cCommentFormat;
   QTextCharFormat m_cStringFormat;
   QTextCharFormat m_cFunctionFormat;
   QTextCharFormat m_cTableFormat;
};

#endif
