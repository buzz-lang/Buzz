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

   void highlightBlock(const QString& str_text);

private:
      
   struct SHighlightingRule
   {
      QRegExp Pattern;
      QTextCharFormat Format;
   };
   QVector<SHighlightingRule> m_vecHighlightingRules;

   QRegExp m_cCommentStartExpression;
   QRegExp m_cCommentEndExpression;

   QTextCharFormat m_cKeywordFormat;
   QTextCharFormat m_cSingleLineCommentFormat;
   QTextCharFormat m_cQuotationFormat;
   QTextCharFormat m_cFunctionFormat;
};

#endif
