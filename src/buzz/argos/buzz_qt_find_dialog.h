#ifndef BUZZ_QT_FIND_DIALOG_H
#define BUZZ_QT_FIND_DIALOG_H

class CBuzzQTFindDialog;

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class CBuzzQTFindDialog : public QDialog {

   Q_OBJECT

public:

   CBuzzQTFindDialog(QWidget *parent = 0);

private:

   QLabel* m_pcLabel;
   QLineEdit* m_pcLineEdit;
   QCheckBox* m_pcCaseCheckBox;
   QCheckBox* m_pcFromStartCheckBox;
   QCheckBox* m_pcWholeWordsCheckBox;
   QCheckBox* m_pcSearchSelectionCheckBox;
   QCheckBox* m_pcBackwardCheckBox;
   QDialogButtonBox* m_pcButtonBox;
   QPushButton* m_pcFindButton;
   QPushButton* m_pcMoreButton;
   QWidget* m_pcExtension;

};

#endif
