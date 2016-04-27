#include "buzz_qt_syntax_highlighter.h"

/****************************************/
/****************************************/

CBuzzQTSyntaxHighlighter::CBuzzQTSyntaxHighlighter(QTextDocument* pc_text) :
   QSyntaxHighlighter(pc_text) {
   SHighlightingRule sRule;
   m_cKeywordFormat.setForeground(Qt::darkBlue);
   m_cKeywordFormat.setFontWeight(QFont::Bold);
   QStringList cKeywordPatterns;
   cKeywordPatterns << "\\band\\b"    << "\\bbreak\\b"  << "\\bdo\\b"   << "\\belse\\b"     << "\\belseif\\b"
                    << "\\bend\\b"    << "\\bfalse\\b"  << "\\bfor\\b"  << "\\bfunction\\b" << "\\bif\\b"
                    << "\\bin\\b"     << "\\blocal\\b"  << "\\bnil\\b"  << "\\bnot\\b"      << "\\bor\\b"
                    << "\\brepeat\\b" << "\\breturn\\b" << "\\bthen\\b" << "\\btrue\\b"     << "\\buntil\\b" << "\\bwhile\\b";
   foreach (const QString& cPattern, cKeywordPatterns) {
      sRule.Pattern = QRegExp(cPattern);
      sRule.Format = m_cKeywordFormat;
      m_vecHighlightingRules.append(sRule);
   }

   m_cSingleLineCommentFormat.setForeground(Qt::darkGray);
   m_cSingleLineCommentFormat.setFontItalic(true);
   sRule.Pattern = QRegExp("#[^[\n]*");
   sRule.Format = m_cSingleLineCommentFormat;
   m_vecHighlightingRules.append(sRule);

   m_cQuotationFormat.setForeground(Qt::darkGreen);
   sRule.Pattern = QRegExp("\".*\"");
   sRule.Format = m_cQuotationFormat;
   m_vecHighlightingRules.append(sRule);
}

/****************************************/
/****************************************/

void CBuzzQTSyntaxHighlighter::highlightBlock(const QString& str_text) {
   /*
    * Apply normal rules
    */
   foreach (const SHighlightingRule& sRule, m_vecHighlightingRules) {
      QRegExp cExpression(sRule.Pattern);
      int i = cExpression.indexIn(str_text);
      while(i >= 0) {
         int nLength = cExpression.matchedLength();
         setFormat(i, nLength, sRule.Format);
         i = cExpression.indexIn(str_text, i + nLength);
      }
   }
}

/****************************************/
/****************************************/
