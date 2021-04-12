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
      sRule.Pattern = QRegularExpression(cPattern);
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
   QRegularExpressionMatchIterator cMatchIt;
   /*
    * Apply normal rules
    */
   foreach (const SHighlightingRule& sRule, m_vecHighlightingRules) {
      QRegularExpression cExpression(sRule.Pattern);
      cMatchIt = cExpression.globalMatch(str_text);
      while(cMatchIt.hasNext()) {
         QRegularExpressionMatch cMatch = cMatchIt.next();
         setFormat(cMatch.capturedStart(), cMatch.capturedLength(), sRule.Format);
      }
   }
   /*
    * Function formatting
    */
   QRegularExpression cFunExpr("([A-Za-z_][A-Za-z0-9_]*)(?:\\s*)(\\()(?:.*)(\\))");
   cMatchIt = cFunExpr.globalMatch(str_text);
   while(cMatchIt.hasNext()) {
      QRegularExpressionMatch cMatch = cMatchIt.next();
      setFormat(cMatch.capturedStart(1), cMatch.capturedLength(1), m_cFunctionFormat);
      setFormat(cMatch.capturedStart(2), cMatch.capturedLength(2), m_cFunctionFormat);
      setFormat(cMatch.capturedStart(3), cMatch.capturedLength(3), m_cFunctionFormat);
   }
   /*
    * Table formatting
    */
   QRegularExpression cTblExpr("([A-Za-z_][A-Za-z0-9_]*)(?:\\s*)(\\.)");
   cMatchIt = cTblExpr.globalMatch(str_text);
   while(cMatchIt.hasNext()) {
      QRegularExpressionMatch cMatch = cMatchIt.next();
      setFormat(cMatch.capturedStart(1), cMatch.capturedLength(1), m_cTableFormat);
      setFormat(cMatch.capturedStart(2), cMatch.capturedLength(2), m_cTableFormat);
      setFormat(cMatch.capturedStart(3), cMatch.capturedLength(3), m_cTableFormat);
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
   QRegularExpression cCommExpr("#.*");
   QRegularExpressionMatch cMatch = cCommExpr.match(str_text);
   if(cMatch.hasMatch()) {
      setFormat(cMatch.capturedStart(), cMatch.capturedLength(), m_cCommentFormat);
   }
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
