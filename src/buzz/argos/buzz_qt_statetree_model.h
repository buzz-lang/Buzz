#ifndef BUZZ_QT_STATETREE_MODEL_H
#define BUZZ_QT_STATETREE_MODEL_H

class CBuzzQTStateTreeModel;
class CBuzzQTStateTreeVariableModel;
class CBuzzQTStateTreeFunctionModel;
class CBuzzQTStateTreeItem;   
class CBuzzController;   

#include <buzz/buzzvm.h>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

/****************************************/
/****************************************/

class CBuzzQTStateTreeModel : public QAbstractItemModel {

   Q_OBJECT

public:

   CBuzzQTStateTreeModel(CBuzzController* pc_controller,
                         bool b_remove_empty_tables,
                         QObject* pc_parent = 0);

   virtual ~CBuzzQTStateTreeModel();

   virtual QVariant data(const QModelIndex& c_index,
                         int n_role) const;

   virtual Qt::ItemFlags flags(const QModelIndex& c_index) const;

   virtual QModelIndex index(int n_row,
                             int n_column,
                             const QModelIndex& c_parent = QModelIndex()) const;
      
   virtual QModelIndex parent(const QModelIndex& c_index) const;

   virtual int rowCount(const QModelIndex& c_parent = QModelIndex()) const;

public slots:

   void Refresh();
   void Refresh(int);

protected:

   virtual void ProcessBuzzState(buzzvm_t pt_state,
                                 CBuzzQTStateTreeItem* pc_item) = 0;

protected:

   bool m_bRemoveEmptyTables;

private:

   CBuzzController* m_pcController;
   buzzvm_t m_ptState;
   CBuzzQTStateTreeItem* m_pcDataRoot;

};

/****************************************/
/****************************************/

class CBuzzQTStateTreeVariableModel : public CBuzzQTStateTreeModel {

   Q_OBJECT

public:

   CBuzzQTStateTreeVariableModel(CBuzzController* pc_controller,
                                 bool b_remove_empty_tables,
                                 QObject* pc_parent = 0);

   virtual ~CBuzzQTStateTreeVariableModel() {}

   virtual QVariant headerData(int n_section,
                               Qt::Orientation e_orientation,
                               int n_role = Qt::DisplayRole) const;

   virtual int columnCount(const QModelIndex& c_parent = QModelIndex()) const;

   virtual void ProcessBuzzState(buzzvm_t pt_state,
                                 CBuzzQTStateTreeItem* pc_item);
};

/****************************************/
/****************************************/

class CBuzzQTStateTreeFunctionModel : public CBuzzQTStateTreeModel {

   Q_OBJECT

public:

   CBuzzQTStateTreeFunctionModel(CBuzzController* pc_controller,
                                 bool b_remove_empty_tables,
                                 QObject* pc_parent = 0);

   virtual ~CBuzzQTStateTreeFunctionModel() {}

   virtual QVariant headerData(int n_section,
                               Qt::Orientation e_orientation,
                               int n_role = Qt::DisplayRole) const;

   virtual int columnCount(const QModelIndex& c_parent = QModelIndex()) const;

   virtual void ProcessBuzzState(buzzvm_t pt_state,
                                 CBuzzQTStateTreeItem* pc_item);
};

/****************************************/
/****************************************/

#endif
