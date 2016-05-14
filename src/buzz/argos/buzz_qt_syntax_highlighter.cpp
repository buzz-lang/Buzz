#include "buzz_qt_syntax_highlighter.h"

/****************************************/
/****************************************/

CBuzzQTSyntaxHighlighter::CBuzzQTSyntaxHighlighter(QTextDocument* pc_text) :
   QSyntaxHighlighter(pc_text) {
   /* Keyword formatting */
   SHighlightingRule sRule;
   m_cKeywordFormat.setForeground(Qt::darkBlue);
   m_cKeywordFormat.setFontWeight(QFont::Bold);
   QStringList cKeywordPatterns;
   cKeywordPatterns << "\\bvar\\b" << "\\bnil\\b" << "\\bif\\b" << "\\belse\\b" << "\\bfunction\\b"
                    << "\\breturn\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\band\\b" << "\\bor\\b"
                    << "\\bnot\\b" << "\\bsize\\b" << "\\bforeach\\b" << "\\binclude\\b";
   foreach (const QString& cPattern, cKeywordPatterns) {
      sRule.Pattern = QRegExp(cPattern);
      sRule.Format = m_cKeywordFormat;
      m_vecHighlightingRules.append(sRule);
   }
   /* Function formatting */
   m_cFunctionFormat.setForeground(Qt::darkRed);
   m_cFunctionFormat.setFontWeight(QFont::Bold);
   /* Table formatting */
   m_cTableFormat.setForeground(Qt::darkGreen);
   m_cTableFormat.setFontWeight(QFont::Bold);
   /* Single line comment formatting */
   m_cCommentFormat.setForeground(Qt::darkGray);
   m_cCommentFormat.setFontItalic(true);
   m_cCommentFormat.setFontWeight(QFont::Normal);
   /* String formatting */
   m_cStringFormat.setForeground(Qt::darkCyan);
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
      int nLength;
      while(i >= 0) {
         nLength = cExpression.matchedLength();
         setFormat(i, nLength, sRule.Format);
         i = cExpression.indexIn(str_text, i + nLength);
      }
   }
   /*
    * Function formatting
    */
   QRegExp cFunExpr("([A-Za-z_][A-Za-z0-9_]*)(?:\\s*)(\\()(?:.*)(\\))");
   int nFStart = cFunExpr.indexIn(str_text);
   while(nFStart >= 0) {
      setFormat(nFStart, cFunExpr.capturedTexts()[1].length(), m_cFunctionFormat);
      setFormat(cFunExpr.pos(2), 1, m_cFunctionFormat);
      setFormat(cFunExpr.pos(3), 1, m_cFunctionFormat);
      nFStart = cFunExpr.indexIn(str_text, nFStart + cFunExpr.matchedLength());
   }
   /*
    * Table formatting
    */
   QRegExp cTblExpr("([A-Za-z_][A-Za-z0-9_]*)(?:\\s*)(\\.)");
   int nTStart = cTblExpr.indexIn(str_text);
   while(nTStart >= 0) {
      setFormat(nTStart, cTblExpr.capturedTexts()[1].length(), m_cTableFormat);
      setFormat(cTblExpr.pos(2), 1, m_cTableFormat);
      nTStart = cTblExpr.indexIn(str_text, nTStart + cTblExpr.matchedLength());
   }
   /*
    * String formatting
    */
   /* Was a string start found? */
   int nStart = 0, nEnd;
   QChar cDelim;
   while(StringStart(str_text, &nStart, &cDelim)) {
      /* Look for the matching delimiter */
      nEnd = str_text.indexOf(cDelim, nStart + 1);
      if(nEnd == -1) break;
      /* Format the string */
      setFormat(nStart, nEnd - nStart + 1, m_cStringFormat);
      /* Update start index */
      nStart = nEnd + 1;
   }
   /*
    * Comment formatting
    */
   QRegExp cCommExpr("#[^[\n]*");
   int nCStart = cCommExpr.indexIn(str_text);
   if(nCStart >= 0)
      setFormat(nCStart, cCommExpr.matchedLength(), m_cCommentFormat);
}

/****************************************/
/****************************************/

bool CBuzzQTSyntaxHighlighter::StringStart(const QString& str_text,
                                           int* pn_start_idx,
                                           QChar* pc_delim) {
   if(*pn_start_idx >= str_text.length()) return false;
   int nSQuotePos = str_text.indexOf("'", *pn_start_idx);
   int nDQuotePos = str_text.indexOf("\"", *pn_start_idx);
   *pc_delim = '\0';
   if(nSQuotePos >= 0 && nDQuotePos >= 0) {
      /* The text contains both ' and ", consider only the first found */
      if(nSQuotePos < nDQuotePos) {
         /* Single quote found first */
         *pn_start_idx = nSQuotePos;
         *pc_delim = '\'';
      }
      else {
         /* Double quote found first */
         *pn_start_idx = nDQuotePos;
         *pc_delim = '"';
      }
   }
   else if(nSQuotePos >= 0) {
      /* Only single quote found */
      *pn_start_idx = nSQuotePos;
      *pc_delim = '\'';
   }
   else if(nDQuotePos >= 0) {
      /* Only double quote found */
      *pn_start_idx = nDQuotePos;
      *pc_delim = '"';
   }
   return *pn_start_idx != -1;
}

/****************************************/
/****************************************/
