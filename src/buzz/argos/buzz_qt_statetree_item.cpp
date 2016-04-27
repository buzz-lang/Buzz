#include "buzz_qt_statetree_item.h"
#include <cstdio>

/****************************************/
/****************************************/

CBuzzQTStateTreeItem::CBuzzQTStateTreeItem(CBuzzQTStateTreeItem* pc_parent) :
   m_pcParent(pc_parent) {}

/****************************************/
/****************************************/

CBuzzQTStateTreeItem::CBuzzQTStateTreeItem(QList<QVariant>& list_data,
                                           CBuzzQTStateTreeItem* pc_parent) :
   m_listData(list_data),
   m_pcParent(pc_parent) {}

/****************************************/
/****************************************/

CBuzzQTStateTreeItem::~CBuzzQTStateTreeItem() {
   qDeleteAll(m_listChildren);
}

/****************************************/
/****************************************/

CBuzzQTStateTreeItem* CBuzzQTStateTreeItem::GetParent() {
   return m_pcParent;
}

/****************************************/
/****************************************/

CBuzzQTStateTreeItem* CBuzzQTStateTreeItem::GetChild(size_t un_idx) {
   return m_listChildren.value(un_idx);
}

/****************************************/
/****************************************/

void CBuzzQTStateTreeItem::AddChild(CBuzzQTStateTreeItem* pc_child) {
   m_listChildren.append(pc_child);
}

/****************************************/
/****************************************/

void CBuzzQTStateTreeItem::RemoveChild(CBuzzQTStateTreeItem* pc_child) {
   m_listChildren.removeOne(pc_child);
}

/****************************************/
/****************************************/

size_t CBuzzQTStateTreeItem::GetNumChildren() const {
   return m_listChildren.count();
}

/****************************************/
/****************************************/

bool ItemLessThan(const CBuzzQTStateTreeItem* pc_i1,
                  const CBuzzQTStateTreeItem* pc_i2) {
   if(pc_i1->GetData(0).type() == QVariant::Double &&
      pc_i2->GetData(0).type() == QVariant::Double) {
      return pc_i1->GetData(0).toDouble() < pc_i2->GetData(0).toDouble();
   }
   else {
      return pc_i1->GetData(0).toString().toLower() < pc_i2->GetData(0).toString().toLower();
   }
}

void CBuzzQTStateTreeItem::SortChildren() {
   qSort(m_listChildren.begin(), m_listChildren.end(), ItemLessThan);
   foreach(CBuzzQTStateTreeItem* pcItem, m_listChildren) {
      pcItem->SortChildren();
   }
}

/****************************************/
/****************************************/

QVariant CBuzzQTStateTreeItem::GetData(int n_col) const {
   return m_listData.value(n_col);
}

/****************************************/
/****************************************/

int CBuzzQTStateTreeItem::GetRow() {
   if(m_pcParent != NULL) {
      return m_pcParent->m_listChildren.indexOf(const_cast<CBuzzQTStateTreeItem*>(this));
   }
   else {
      return 0;
   }
}

/****************************************/
/****************************************/
