#include "buzz_qt_statetree_model.h"
#include "buzz_qt_statetree_item.h"

#include <argos3/core/utility/logging/argos_log.h>

using namespace argos;

/****************************************/
/****************************************/

CBuzzQTStateTreeModel::CBuzzQTStateTreeModel(buzzvm_t pt_state,
                                             bool b_remove_empty_tables,
                                             QObject* pc_parent) :
   QAbstractItemModel(pc_parent),
   m_bRemoveEmptyTables(b_remove_empty_tables),
   m_ptState(pt_state) {
   m_pcDataRoot = new CBuzzQTStateTreeItem();
}

/****************************************/
/****************************************/

CBuzzQTStateTreeModel::~CBuzzQTStateTreeModel() {
   delete m_pcDataRoot;
}

/****************************************/
/****************************************/

QVariant CBuzzQTStateTreeModel::data(const QModelIndex& c_index,
                                     int n_role) const {
   if(!c_index.isValid()) return QVariant();
   if(n_role != Qt::DisplayRole) return QVariant();
   CBuzzQTStateTreeItem* pcItem = static_cast<CBuzzQTStateTreeItem*>(c_index.internalPointer());
   return pcItem->GetData(c_index.column());
}

/****************************************/
/****************************************/

Qt::ItemFlags CBuzzQTStateTreeModel::flags(const QModelIndex& c_index) const {
   if (!c_index.isValid()) return 0;
   else return Qt::ItemIsEnabled;
}

/****************************************/
/****************************************/

QModelIndex CBuzzQTStateTreeModel::index(int n_row,
                                         int n_column,
                                         const QModelIndex& c_parent) const {
   if(!hasIndex(n_row, n_column, c_parent)) return QModelIndex();
   CBuzzQTStateTreeItem* pcParentItem;
   if(!c_parent.isValid()) pcParentItem = m_pcDataRoot;
   else pcParentItem = static_cast<CBuzzQTStateTreeItem*>(c_parent.internalPointer());
   CBuzzQTStateTreeItem* pcChildItem = pcParentItem->GetChild(n_row);
   if(pcChildItem) return createIndex(n_row, n_column, pcChildItem);
   else return QModelIndex();
}

/****************************************/
/****************************************/

QModelIndex CBuzzQTStateTreeModel::parent(const QModelIndex& c_index) const {
   if (!c_index.isValid()) return QModelIndex();
   CBuzzQTStateTreeItem* pcChildItem = static_cast<CBuzzQTStateTreeItem*>(c_index.internalPointer());
   CBuzzQTStateTreeItem* pcParentItem = pcChildItem->GetParent();
   if (pcParentItem == m_pcDataRoot) return QModelIndex();
   else return createIndex(pcParentItem->GetRow(), 0, pcParentItem);
}

/****************************************/
/****************************************/

int CBuzzQTStateTreeModel::rowCount(const QModelIndex& c_parent) const {
   CBuzzQTStateTreeItem* pcParentItem;
   if(c_parent.column() > 0) return 0;
   if(!c_parent.isValid()) pcParentItem = m_pcDataRoot;
   else pcParentItem = static_cast<CBuzzQTStateTreeItem*>(c_parent.internalPointer());
   return pcParentItem->GetNumChildren();
}

/****************************************/
/****************************************/

void CBuzzQTStateTreeModel::SetBuzzState(buzzvm_t pt_state) {
   m_ptState = pt_state;
   Refresh();
}

/****************************************/
/****************************************/

struct SBuzzProcessStateData {
   buzzvm_t VM;
   CBuzzQTStateTreeItem* Item;
   bool NoEmptyTables;
};

void CBuzzQTStateTreeModel::Refresh() {
   beginResetModel();
   delete m_pcDataRoot;
   m_pcDataRoot = new CBuzzQTStateTreeItem();
   ProcessBuzzState(m_ptState, m_pcDataRoot);
   m_pcDataRoot->SortChildren();
   endResetModel();
}

void CBuzzQTStateTreeModel::Refresh(int) {
   Refresh();
}

/****************************************/
/****************************************/

CBuzzQTStateTreeVariableModel::CBuzzQTStateTreeVariableModel(buzzvm_t pt_state,
                                                             bool b_remove_empty_tables,
                                                             QObject* pc_parent) :
   CBuzzQTStateTreeModel(pt_state, b_remove_empty_tables, pc_parent) {}

/****************************************/
/****************************************/

QVariant CBuzzQTStateTreeVariableModel::headerData(int n_section,
                                                   Qt::Orientation e_orientation,
                                                   int n_role) const {
   if(e_orientation != Qt::Horizontal ||
      n_role != Qt::DisplayRole ||
      n_section > 1) {
      return QVariant();
   }
   else {
      return n_section == 0 ? tr("Variable") : tr("Value");
   }
}

/****************************************/
/****************************************/

int CBuzzQTStateTreeVariableModel::columnCount(const QModelIndex&) const {
   return 2;
}

/****************************************/
/****************************************/

void ProcessBuzzObjectsInTable(const void* pt_key,
                               void* pt_data,
                               void* pt_params) {
   /* Get the key */
   const buzzobj_t tKey = *reinterpret_cast<const buzzobj_t*>(pt_key);
   /* Get the data */
   buzzobj_t tData = *reinterpret_cast<buzzobj_t*>(pt_data);
   /* Get the parameters */
   SBuzzProcessStateData* psParams = reinterpret_cast<SBuzzProcessStateData*>(pt_params);
   /* Buffer for element */
   QList<QVariant> cData;
   /* Insert key data */
   switch(tKey->o.type) {
      case BUZZTYPE_INT:
         cData << tKey->i.value;
         break;
      case BUZZTYPE_FLOAT:
         cData << tKey->f.value;
         break;
      case BUZZTYPE_STRING:
         cData << tKey->s.value.str;
         break;
      default:
         return;
   }
   /* Insert value data */
   if(tData->o.type != BUZZTYPE_TABLE) {
      /* Non-table data type */
      /* Insert key data */
      switch(tData->o.type) {
         case BUZZTYPE_INT:
            cData << tData->i.value;
            break;
         case BUZZTYPE_FLOAT:
            cData << tData->f.value;
            break;
         case BUZZTYPE_STRING:
            cData << QString("\"%1\"").arg(tData->s.value.str);
            break;
         case BUZZTYPE_USERDATA:
            cData << "[userdata]";
            break;
         default:
            return;
      }
      psParams->Item->AddChild(new CBuzzQTStateTreeItem(cData, psParams->Item));
   }
   else {
      /* Table data type */
      /* Add new child to tree */
      CBuzzQTStateTreeItem* pcChild = new CBuzzQTStateTreeItem(cData, psParams->Item);
      psParams->Item->AddChild(pcChild);
      /* Process the element of the child */
      SBuzzProcessStateData sParams2 = {
         .VM = psParams->VM,
         .Item = pcChild,
         .NoEmptyTables = psParams->NoEmptyTables
      };
      buzzdict_foreach(tData->t.value, ProcessBuzzObjectsInTable, &sParams2);
      /* If no elements were added, remove the child */
      if(psParams->NoEmptyTables) {
         if(pcChild->GetNumChildren() == 0) {
            psParams->Item->RemoveChild(pcChild);
         }
      }
   }
}

void ProcessBuzzObjects(const void* pt_key,
                        void* pt_data,
                        void* pt_params) {
   /* Get the key as a string */
   uint16_t unKeyId = *reinterpret_cast<const uint16_t*>(pt_key);
   /* Get the data */
   buzzobj_t tData = *reinterpret_cast<buzzobj_t*>(pt_data);
   /* Get the parameters */
   SBuzzProcessStateData* psParams = reinterpret_cast<SBuzzProcessStateData*>(pt_params);
   /* Buffer for element */
   QList<QVariant> cData;
   /* Insert key data */
   cData << buzzvm_string_get(psParams->VM, unKeyId);
   /* Insert value data */
   if(tData->o.type != BUZZTYPE_TABLE) {
      /* Non-table data type */
      /* Insert value data */
      switch(tData->o.type) {
         case BUZZTYPE_INT:
            cData << tData->i.value;
            break;
         case BUZZTYPE_FLOAT:
            cData << tData->f.value;
            break;
         case BUZZTYPE_STRING:
            cData << QString("\"%1\"").arg(tData->s.value.str);
            break;
         case BUZZTYPE_USERDATA:
            cData << "[userdata]";
            break;
         default:
            return;
      }
      psParams->Item->AddChild(new CBuzzQTStateTreeItem(cData, psParams->Item));
   }
   else {
      /* Table data type */
      /* Add new child to tree */
      CBuzzQTStateTreeItem* pcChild = new CBuzzQTStateTreeItem(cData, psParams->Item);
      psParams->Item->AddChild(pcChild);
      /* Process the element of the child */
      SBuzzProcessStateData sParams2 = {
         .VM = psParams->VM,
         .Item = pcChild,
         .NoEmptyTables = psParams->NoEmptyTables
      };
      buzzdict_foreach(tData->t.value, ProcessBuzzObjectsInTable, &sParams2);
      /* If no elements were added, remove the child */
      if(psParams->NoEmptyTables) {
         if(pcChild->GetNumChildren() == 0) {
            psParams->Item->RemoveChild(pcChild);
         }
      }
   }
}

void CBuzzQTStateTreeVariableModel::ProcessBuzzState(buzzvm_t pt_state,
                                                     CBuzzQTStateTreeItem* pc_item) {
   SBuzzProcessStateData sParams = {
      .VM = pt_state,
      .Item = pc_item,
      .NoEmptyTables = m_bRemoveEmptyTables
   };
   buzzdict_foreach(pt_state->gsyms, ProcessBuzzObjects, &sParams);
}

/****************************************/
/****************************************/

CBuzzQTStateTreeFunctionModel::CBuzzQTStateTreeFunctionModel(buzzvm_t pt_state,
                                                             bool b_remove_empty_tables,
                                                             QObject* pc_parent) :
   CBuzzQTStateTreeModel(pt_state, b_remove_empty_tables, pc_parent) {}

/****************************************/
/****************************************/

QVariant CBuzzQTStateTreeFunctionModel::headerData(int n_section,
                                                   Qt::Orientation e_orientation,
                                                   int n_role) const {
   return QVariant();
}

/****************************************/
/****************************************/

int CBuzzQTStateTreeFunctionModel::columnCount(const QModelIndex&) const {
   return 1;
}

/****************************************/
/****************************************/

void ProcessBuzzFunctionsInTable(const void* pt_key,
                                 void* pt_data,
                                 void* pt_params) {
   /* Get the key */
   buzzobj_t tKey = *reinterpret_cast<const buzzobj_t*>(pt_key);
   /* Get the data */
   buzzobj_t tData = *reinterpret_cast<buzzobj_t*>(pt_data);
   /* Get the parameters */
   SBuzzProcessStateData* psParams = reinterpret_cast<SBuzzProcessStateData*>(pt_params);
   /* Buffer for element */
   QList<QVariant> cData;
   /* Insert key data */
   if(tKey->o.type == BUZZTYPE_STRING) {
      cData << tKey->s.value.str;
   }
   else {
      return;
   }
   /* Insert value data */
   if(tData->o.type != BUZZTYPE_TABLE) {
      /* Non-table data type */
      if(tData->o.type == BUZZTYPE_CLOSURE) {
         cData[0] = cData[0].toString() + "()";
         psParams->Item->AddChild(new CBuzzQTStateTreeItem(cData, psParams->Item));
      }
   }
   else {
      /* Table data type */
      /* Add new child to tree */
      CBuzzQTStateTreeItem* pcChild = new CBuzzQTStateTreeItem(cData, psParams->Item);
      psParams->Item->AddChild(pcChild);
      /* Process the element of the child */
      SBuzzProcessStateData sParams2 = {
         .VM = psParams->VM,
         .Item = pcChild,
         .NoEmptyTables = psParams->NoEmptyTables
      };
      buzzdict_foreach(tData->t.value, ProcessBuzzFunctionsInTable, &sParams2);
      /* If no elements were added, remove the child */
      if(psParams->NoEmptyTables) {
         if(pcChild->GetNumChildren() == 0)
            psParams->Item->RemoveChild(pcChild);
      }
   }
}

void ProcessBuzzFunctions(const void* pt_key,
                          void* pt_data,
                          void* pt_params) {
   /* Get the key */
   uint16_t unKeyId = *reinterpret_cast<const uint16_t*>(pt_key);
   /* Get the data */
   buzzobj_t tData = *reinterpret_cast<buzzobj_t*>(pt_data);
   /* Get the parameters */
   SBuzzProcessStateData* psParams = reinterpret_cast<SBuzzProcessStateData*>(pt_params);
   /* Buffer for element */
   QList<QVariant> cData;
   /* Insert key data */
   cData << buzzvm_string_get(psParams->VM, unKeyId);
   /* Insert value data */
   if(tData->o.type != BUZZTYPE_TABLE) {
      /* Non-table data type */
      if(tData->o.type == BUZZTYPE_CLOSURE) {
         cData[0] = cData[0].toString() + "()";
         psParams->Item->AddChild(new CBuzzQTStateTreeItem(cData, psParams->Item));
      }
   }
   else {
      /* Table data type */
      /* Add new child to tree */
      CBuzzQTStateTreeItem* pcChild = new CBuzzQTStateTreeItem(cData, psParams->Item);
      psParams->Item->AddChild(pcChild);
      /* Process the element of the child */
      SBuzzProcessStateData sParams2 = {
         .VM = psParams->VM,
         .Item = pcChild,
         .NoEmptyTables = psParams->NoEmptyTables
      };
      buzzdict_foreach(tData->t.value, ProcessBuzzFunctionsInTable, &sParams2);
      /* If no elements were added, remove the child */
      if(psParams->NoEmptyTables) {
         if(pcChild->GetNumChildren() == 0)
            psParams->Item->RemoveChild(pcChild);
      }
   }
}

void CBuzzQTStateTreeFunctionModel::ProcessBuzzState(buzzvm_t pt_state,
                                                     CBuzzQTStateTreeItem* pc_item) {
   SBuzzProcessStateData sParams = {
      .VM = pt_state,
      .Item = pc_item,
      .NoEmptyTables = m_bRemoveEmptyTables
   };
   buzzdict_foreach(pt_state->gsyms, ProcessBuzzFunctions, &sParams);
}

/****************************************/
/****************************************/
