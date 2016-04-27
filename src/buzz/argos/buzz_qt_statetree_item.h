#ifndef BUZZ_QT_STATETREE_ITEM_H
#define BUZZ_QT_STATETREE_ITEM_H

class CBuzzQTStateTreeItem;

#include <QList>
#include <QVariant>

class CBuzzQTStateTreeItem {

public:

   CBuzzQTStateTreeItem(CBuzzQTStateTreeItem* pc_parent = 0);
   CBuzzQTStateTreeItem(QList<QVariant>& list_data,
                        CBuzzQTStateTreeItem* pc_parent = 0);
   ~CBuzzQTStateTreeItem();

   CBuzzQTStateTreeItem* GetParent();

   CBuzzQTStateTreeItem* GetChild(size_t un_idx);

   void AddChild(CBuzzQTStateTreeItem* pc_child);

   void RemoveChild(CBuzzQTStateTreeItem* pc_child);

   size_t GetNumChildren() const;

   void SortChildren();

   QVariant GetData(int n_col) const;

   int GetRow();

private:

   QList<QVariant> m_listData;
   CBuzzQTStateTreeItem* m_pcParent;
   QList<CBuzzQTStateTreeItem*> m_listChildren;

};

#endif
