#include "tabbed_modify_dialogue.h"

#include "records.h"
#include "input_widgets.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QLabel>
#include <QVariantMap>

TabbedModifyDialogue::TabbedModifyDialogue(DBRecord * record, QWidget * parent)
    : ModifyDialogue(parent)
{
    main_tabw = new QTabWidget;

    init(record);

    md_record->initModifyDialogue(this);

    ModifyDialogueColumnLayout(&md_inputwidgets, md_grid_main).layout();
}

TabbedModifyDialogue::~TabbedModifyDialogue()
{
    delete main_tabw;
}

void TabbedModifyDialogue::addMainGridLayout(QVBoxLayout * md_vlayout_main)
{
    QWidget * main_form_tab = new QWidget(main_tabw);
    main_form_tab->setLayout(md_grid_main);

    main_tabw->addTab(main_form_tab, tr("Basic information"));
    md_vlayout_main->addWidget(main_tabw);
}

void TabbedModifyDialogue::addTab(ModifyDialogueTab * tab)
{
    main_tabw->addTab(tab, tab->name());
}

void TabbedModifyDialogue::save()
{
    for (int i = 1; i < main_tabw->count(); ++i) {
        ((ModifyDialogueTab *) main_tabw->widget(i))->save(idFieldValue().toInt());
    }
    ModifyDialogue::save();
}

ModifyDialogueTab::ModifyDialogueTab(QWidget * parent)
    : QWidget(parent)
{
}

AssemblyRecordModifyDialogue::AssemblyRecordModifyDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new AssemblyRecordModifyDialogueTab(idFieldValue().toInt()));
}

AssemblyRecordModifyDialogueTab::AssemblyRecordModifyDialogueTab(int record_id, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    this->record_id = record_id;
    setName(tr("Assembly record items"));

    init();
}

void AssemblyRecordModifyDialogueTab::save(int record_id)
{
    this->record_id = record_id;

    AssemblyRecordTypeItem used_item_records(QString("%1").arg(record_id));
    used_item_records.remove();

    QVariantMap map;
    map.insert("record_type_id", record_id);

    QTreeWidgetItem * item;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        item = tree->topLevelItem(i);

        if (item->checkState(0) == Qt::Checked) {
            map.insert("record_item_id", item->data(0, Qt::UserRole).toInt());
            used_item_records.update(map);
        }
    }
}

void AssemblyRecordModifyDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel(tr("Assembly record items:")));

    tree = new QTreeWidget;
    tree->header()->hide();
    layout->addWidget(tree);

    AssemblyRecordItemType all_items("");
    ListOfVariantMaps items(all_items.listAll());

    AssemblyRecordTypeItem used_item_records(QString("%1").arg(record_id));
    ListOfVariantMaps used_items(used_item_records.listAll());

    QTreeWidgetItem * item;
    for (int i = 0; i < items.count(); ++i) {
        item = new QTreeWidgetItem;
        item->setText(0, items.at(i).value("name").toString());
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, Qt::UserRole, items.at(i).value("id"));

        for (int n = 0; n < used_items.count(); ++n) {
            if (used_items.at(n).value("record_item_id").toInt() == items.at(i).value("id").toInt()) {
                item->setCheckState(0, Qt::Checked);
                break;
            }
        }
        tree->addTopLevelItem(item);
    }
}

ModifyInspectionDialogue::ModifyInspectionDialogue(DBRecord * record, QWidget * parent)
    : TabbedModifyDialogue(record, parent)
{
    addTab(new ModifyInspectionDialogueTab(0));
}

ModifyInspectionDialogueTab::ModifyInspectionDialogueTab(int, QWidget * parent)
    : ModifyDialogueTab(parent)
{
    setName(tr("Assembly record"));

    init();
}

void ModifyInspectionDialogueTab::init()
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);

    QGridLayout * form_grid = new QGridLayout;

    QList<MDInputWidget *> md_inputwidgets;

    md_inputwidgets << new MDLineEdit("id", tr("Assembly record ID:"), this, "");
    md_inputwidgets << new MDComboBox("type", tr("Assembly record type:"), this, "", listAssemblyRecordItemTypes());

    ModifyDialogueColumnLayout(&md_inputwidgets, form_grid, 1).layout();

    layout->addLayout(form_grid);
}

void ModifyInspectionDialogueTab::save(int)
{

}

MTDictionary ModifyInspectionDialogueTab::listAssemblyRecordItemTypes()
{
    AssemblyRecordType types("");
    ListOfVariantMaps types_list = types.listAll();
    MTDictionary dict(true);

    for (int i = 0; i < types_list.count(); ++i) {
        dict.insert(types_list.at(i).value("name").toString(), types_list.at(i).value("id").toString());
    }
    return dict;
}
