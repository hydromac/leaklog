/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2013 Matus & Michal Tomlein

 Leaklog is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence
 as published by the Free Software Foundation; either version 2
 of the Licence, or (at your option) any later version.

 Leaklog is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with Leaklog; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
********************************************************************/

#include "viewtab.h"
#include "ui_viewtab.h"

#include "mainwindow.h"
#include "storeview.h"
#include "refrigerantmanagementview.h"
#include "customersview.h"
#include "circuitsview.h"
#include "inspectionsview.h"
#include "inspectiondetailsview.h"
#include "tableview.h"
#include "repairsview.h"
#include "inspectorsview.h"
#include "inspectordetailsview.h"
#include "operatorreportview.h"
#include "leakagesbyapplicationview.h"
#include "agendaview.h"
#include "inspectionimagesview.h"
#include "assemblyrecordsview.h"
#include "assemblyrecorddetailsview.h"
#include "assemblyrecordtypesview.h"
#include "assemblyrecorditemcategoriesview.h"
#include "assemblyrecorditemtypesview.h"
#include "circuitunittypesview.h"
#include "mtwebpage.h"
#include "reportdatacontroller.h"

#include <QWebFrame>

ViewTab::ViewTab(QWidget * parent):
    MTWidget(parent),
    ui(new Ui::ViewTab)
{
    ui->setupUi(this);

    QObject::connect(parentWindow(), SIGNAL(tablesChanged(const QStringList &)), this, SLOT(reloadTables(const QStringList &)));
    QObject::connect(parentWindow(), SIGNAL(tableAdded(int, const QString &)), this, SLOT(addTable(int, const QString &)));
    QObject::connect(parentWindow(), SIGNAL(tableRemoved(const QString &)), this, SLOT(removeTable(const QString &)));
    QObject::connect(ui->trw_navigation, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
                     this, SLOT(viewChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QObject::connect(this, SIGNAL(viewChanged(View::ViewID)),
                     ui->toolbarstack, SLOT(viewChanged(View::ViewID)));

    QObject::connect(ui->wv_main, SIGNAL(linkClicked(const QUrl &)), this, SLOT(executeLink(const QUrl &)));

    setDefaultWebPage();

    ui->splitter->setStyleSheet("QSplitter { background-color: #B8B8B8; }");

    ui->trw_navigation->setIconSize(QSize(20, 20));
    ui->trw_navigation->setStyleSheet(
                "QTreeWidget::item { padding-top: 2px; padding-bottom: 2px; }"
#ifdef Q_OS_MAC
                "QTreeWidget { background-color: #E7EBF0; }"
                "QTreeWidget::item:selected { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #77BBE7, stop: 1 #3E8ACF);"
                                             "color: white; border-color: #62A6DC; border-style: solid; border-width: 1px 0px 1px 0px; }"
                "QTreeWidget::item:!selected { background-color: #E7EBF0; }"
                "QTreeWidget::item:selected:disabled { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C4CDDF, stop: 1 #94A1B8);"
                                                      "color: white; border-color: #BCC6D6; border-style: solid; border-width: 1px 0px 0px 0px; }"
                "QTreeWidget::item:has-children { padding-left: 3px; color: #717E8B; }"
#else
                "QTreeWidget { background-color: white; }"
                "QTreeWidget::item:selected { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #DAECFC, stop: 1 #C4E0FC);"
                                             "border-color: #569DE5; border-style: solid; border-width: 1px; }"
                "QTreeWidget::item:!selected { background-color: white; }"
                "QTreeWidget::item:selected:disabled { background-color: #F7F7F7;"
                                                      "color: #787878; border-color: #DEDEDE; border-style: solid; border-width: 1px; }"
                "QTreeWidget::item:has-children { padding-left: 3px; color: #1E3287; }"
#endif
                "QTreeWidget::item:!has-children { padding-left: 7px; }");

    createViewItems();

    for (int i = 0; i < ui->trw_navigation->topLevelItemCount(); ++i)
        ui->trw_navigation->topLevelItem(i)->setExpanded(true);

    ui->toolbarstack->setSettings(this);

    setView(View::Store);
}

ViewTab::~ViewTab()
{
    delete ui;
}

void ViewTab::createViewItems()
{
    views[View::Store] = new StoreView(this);
    views[View::RefrigerantManagement] = new RefrigerantManagementView(this);
    views[View::Customers] = new CustomersView(this);
    views[View::Circuits] = new CircuitsView(this);
    views[View::Inspections] = new InspectionsView(this);
    views[View::InspectionDetails] = new InspectionDetailsView(this);
    views[View::TableOfInspections] = new TableView(this);
    views[View::Repairs] = new RepairsView(this);
    views[View::Inspectors] = new InspectorsView(this);
    views[View::InspectorDetails] = new InspectorDetailsView(this);
    views[View::OperatorReport] = new OperatorReportView(this);
    views[View::LeakagesByApplication] = new LeakagesByApplicationView(this);
    views[View::Agenda] = new AgendaView(this);
    views[View::InspectionImages] = new InspectionImagesView(this);
    views[View::AssemblyRecords] = new AssemblyRecordsView(this);
    views[View::AssemblyRecordDetails] = new AssemblyRecordDetailsView(this);
    views[View::AssemblyRecordTypes] = new AssemblyRecordTypesView(this);
    views[View::AssemblyRecordItemCategories] = new AssemblyRecordItemCategoriesView(this);
    views[View::AssemblyRecordItemTypes] = new AssemblyRecordItemTypesView(this);
    views[View::CircuitUnitTypes] = new CircuitUnitTypesView(this);

    QTreeWidgetItem * group_service_company = new QTreeWidgetItem(ui->trw_navigation);
    group_service_company->setText(0, tr("Service Company"));
    formatGroupItem(group_service_company);

    QTreeWidgetItem * item_store = new QTreeWidgetItem(group_service_company);
    item_store->setText(0, tr("Store"));
    item_store->setData(0, Qt::UserRole, View::Store);
    item_store->setIcon(0, QIcon(":/images/images/store_view.png"));
    view_items[View::Store] = item_store;

    QTreeWidgetItem * item_refrigerant_management = new QTreeWidgetItem(group_service_company);
    item_refrigerant_management->setText(0, tr("Refrigerant Management"));
    item_refrigerant_management->setData(0, Qt::UserRole, View::RefrigerantManagement);
    item_refrigerant_management->setIcon(0, QIcon(":/images/images/management_view.png"));
    view_items[View::RefrigerantManagement] = item_refrigerant_management;

    QTreeWidgetItem * item_leakages = new QTreeWidgetItem(group_service_company);
    item_leakages->setText(0, tr("Leakages by Application"));
    item_leakages->setData(0, Qt::UserRole, View::LeakagesByApplication);
    item_leakages->setIcon(0, QIcon(":/images/images/leakages_view.png"));
    view_items[View::LeakagesByApplication] = item_leakages;

    QTreeWidgetItem * item_agenda = new QTreeWidgetItem(group_service_company);
    item_agenda->setText(0, tr("Agenda"));
    item_agenda->setData(0, Qt::UserRole, View::Agenda);
    item_agenda->setIcon(0, QIcon(":/images/images/agenda_view.png"));
    view_items[View::Agenda] = item_agenda;

    QTreeWidgetItem * item_inspectors = new QTreeWidgetItem(group_service_company);
    item_inspectors->setText(0, tr("Inspectors"));
    item_inspectors->setData(0, Qt::UserRole, View::Inspectors);
    item_inspectors->setIcon(0, QIcon(":/images/images/inspectors_view.png"));
    view_items[View::Inspectors] = item_inspectors;

    QTreeWidgetItem * item_inspector_details = new QTreeWidgetItem(group_service_company);
    item_inspector_details->setText(0, tr("Inspector Details"));
    item_inspector_details->setData(0, Qt::UserRole, View::InspectorDetails);
    item_inspector_details->setIcon(0, QIcon(":/images/images/inspector_view.png"));
    view_items[View::InspectorDetails] = item_inspector_details;

    QTreeWidgetItem * item_customers = new QTreeWidgetItem(group_service_company);
    item_customers->setText(0, tr("Customers"));
    item_customers->setData(0, Qt::UserRole, View::Customers);
    item_customers->setIcon(0, QIcon(":/images/images/customers_view.png"));
    view_items[View::Customers] = item_customers;

    QTreeWidgetItem * item_operator_report = new QTreeWidgetItem(group_service_company);
    item_operator_report->setText(0, tr("Operator Report"));
    item_operator_report->setData(0, Qt::UserRole, View::OperatorReport);
    item_operator_report->setIcon(0, QIcon(":/images/images/report_view.png"));
    view_items[View::OperatorReport] = item_operator_report;

    QTreeWidgetItem * group_basic_logbook = new QTreeWidgetItem(ui->trw_navigation);
    group_basic_logbook->setText(0, tr("Basic Logbook"));
    formatGroupItem(group_basic_logbook);

    QTreeWidgetItem * item_repairs = new QTreeWidgetItem(group_basic_logbook);
    item_repairs->setText(0, tr("Repairs"));
    item_repairs->setData(0, Qt::UserRole, View::Repairs);
    item_repairs->setIcon(0, QIcon(":/images/images/repair_view.png"));
    view_items[View::Repairs] = item_repairs;

    QTreeWidgetItem * group_detailed_logbook = new QTreeWidgetItem(ui->trw_navigation);
    group_detailed_logbook->setText(0, tr("Detailed Logbook"));
    formatGroupItem(group_detailed_logbook);

    QTreeWidgetItem * item_circuits = new QTreeWidgetItem(group_detailed_logbook);
    item_circuits->setText(0, tr("Circuits"));
    item_circuits->setData(0, Qt::UserRole, View::Circuits);
    item_circuits->setIcon(0, QIcon(":/images/images/circuit_view.png"));
    view_items[View::Circuits] = item_circuits;

    QTreeWidgetItem * item_inspections = new QTreeWidgetItem(group_detailed_logbook);
    item_inspections->setText(0, tr("Inspections"));
    item_inspections->setData(0, Qt::UserRole, View::Inspections);
    item_inspections->setIcon(0, QIcon(":/images/images/inspection_view.png"));
    view_items[View::Inspections] = item_inspections;

    QTreeWidgetItem * item_inspection_details = new QTreeWidgetItem(group_detailed_logbook);
    item_inspection_details->setText(0, tr("Inspection Details"));
    item_inspection_details->setData(0, Qt::UserRole, View::InspectionDetails);
    item_inspection_details->setIcon(0, QIcon(":/images/images/inspection_view.png"));
    view_items[View::InspectionDetails] = item_inspection_details;

    QTreeWidgetItem * item_inspection_images = new QTreeWidgetItem(group_detailed_logbook);
    item_inspection_images->setText(0, tr("Inspection Images"));
    item_inspection_images->setData(0, Qt::UserRole, View::InspectionImages);
    item_inspection_images->setIcon(0, QIcon(":/images/images/image.png"));
    view_items[View::InspectionImages] = item_inspection_images;

    QTreeWidgetItem * item_assembly_records = new QTreeWidgetItem(group_detailed_logbook);
    item_assembly_records->setText(0, tr("Assembly Records"));
    item_assembly_records->setData(0, Qt::UserRole, View::AssemblyRecords);
    item_assembly_records->setIcon(0, QIcon(":/images/images/assembly_records_view.png"));
    view_items[View::AssemblyRecords] = item_assembly_records;

    QTreeWidgetItem * item_assembly_record_details = new QTreeWidgetItem(group_detailed_logbook);
    item_assembly_record_details->setText(0, tr("Assembly Record"));
    item_assembly_record_details->setData(0, Qt::UserRole, View::AssemblyRecordDetails);
    item_assembly_record_details->setIcon(0, QIcon(":/images/images/assembly_record_view.png"));
    view_items[View::AssemblyRecordDetails] = item_assembly_record_details;

    group_tables = new QTreeWidgetItem(ui->trw_navigation);
    group_tables->setText(0, tr("Tables"));
    formatGroupItem(group_tables);
    view_items[View::TableOfInspections] = NULL;

    QTreeWidgetItem * group_assembly_records = new QTreeWidgetItem(ui->trw_navigation);
    group_assembly_records->setText(0, tr("Assembly Records"));
    formatGroupItem(group_assembly_records);

    QTreeWidgetItem * item_assembly_record_types = new QTreeWidgetItem(group_assembly_records);
    item_assembly_record_types->setText(0, tr("Types"));
    item_assembly_record_types->setData(0, Qt::UserRole, View::AssemblyRecordTypes);
    item_assembly_record_types->setIcon(0, QIcon(":/images/images/view.png"));
    view_items[View::AssemblyRecordTypes] = item_assembly_record_types;

    QTreeWidgetItem * item_assembly_record_item_categories = new QTreeWidgetItem(group_assembly_records);
    item_assembly_record_item_categories->setText(0, tr("Item Categories"));
    item_assembly_record_item_categories->setData(0, Qt::UserRole, View::AssemblyRecordItemCategories);
    item_assembly_record_item_categories->setIcon(0, QIcon(":/images/images/view.png"));
    view_items[View::AssemblyRecordItemCategories] = item_assembly_record_item_categories;

    QTreeWidgetItem * item_assembly_record_item_types = new QTreeWidgetItem(group_assembly_records);
    item_assembly_record_item_types->setText(0, tr("Item Types"));
    item_assembly_record_item_types->setData(0, Qt::UserRole, View::AssemblyRecordItemTypes);
    item_assembly_record_item_types->setIcon(0, QIcon(":/images/images/view.png"));
    view_items[View::AssemblyRecordItemTypes] = item_assembly_record_item_types;

    QTreeWidgetItem * item_circuit_unit_types = new QTreeWidgetItem(group_assembly_records);
    item_circuit_unit_types->setText(0, tr("Circuit Unit Types"));
    item_circuit_unit_types->setData(0, Qt::UserRole, View::CircuitUnitTypes);
    item_circuit_unit_types->setIcon(0, QIcon(":/images/images/view.png"));
    view_items[View::CircuitUnitTypes] = item_circuit_unit_types;
}

void ViewTab::formatGroupItem(QTreeWidgetItem * item)
{
    QFont font = item->font(0);
#ifdef Q_OS_MAC
    font.setBold(true);
    font.setCapitalization(QFont::AllUppercase);
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() + 3);
#endif
    item->setFont(0, font);
    item->setSizeHint(0, QSize(0, 24));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
}

void ViewTab::reloadTables(const QStringList & tables)
{
    foreach (QTreeWidgetItem * item, group_tables->takeChildren())
        delete item;

    foreach (const QString & table, tables)
        addTable(-1, table);
}

void ViewTab::addTable(int index, const QString & table)
{
    QTreeWidgetItem * item_table = new QTreeWidgetItem;
    item_table->setText(0, table);
    item_table->setData(0, Qt::UserRole, View::TableOfInspections);
    item_table->setIcon(0, QIcon(":/images/images/table_view.png"));
    if (index < 0)
        group_tables->addChild(item_table);
    else
        group_tables->insertChild(index, item_table);
}

void ViewTab::removeTable(const QString & table)
{
    for (int i = 0; i < group_tables->childCount(); ++i) {
        QTreeWidgetItem * item = group_tables->child(i);
        if (item->text(0) == table) {
            delete item;
            break;
        }
    }
}

void ViewTab::enableTools()
{
    view_items[View::Circuits]->setDisabled(!isCustomerSelected());
    view_items[View::Inspections]->setDisabled(!isCircuitSelected());
    view_items[View::InspectionDetails]->setDisabled(!isInspectionSelected());
    for (int i = 0; i < group_tables->childCount(); ++i)
        group_tables->child(i)->setDisabled(!isCustomerSelected());
    view_items[View::OperatorReport]->setDisabled(!isCustomerSelected());
    view_items[View::AssemblyRecordDetails]->setDisabled(!isInspectionSelected());
    view_items[View::AssemblyRecords]->setDisabled(!isCircuitSelected());
    view_items[View::InspectorDetails]->setDisabled(!isInspectorSelected());
    view_items[View::InspectionImages]->setDisabled(!isInspectionSelected());

    ui->toolbarstack->enableTools();
}

MainWindowSettings & ViewTab::mainWindowSettings()
{
    return parentWindow()->settings();
}

ToolBarStack * ViewTab::toolBarStack() const
{
    return ui->toolbarstack;
}

QWebView * ViewTab::webView() const
{
    return ui->wv_main;
}

void ViewTab::setView(View::ViewID view)
{
    if (view_items[view])
        ui->trw_navigation->setCurrentItem(view_items[view]);
    else if (view == View::TableOfInspections)
        ui->trw_navigation->setCurrentItem(group_tables->child(0));
    refreshView();
}

void ViewTab::refreshView()
{
    viewChanged(ui->trw_navigation->currentItem(), NULL);
}

View::ViewID ViewTab::currentView() const
{
    QTreeWidgetItem * item = ui->trw_navigation->currentItem();

    if (item)
        return (View::ViewID)item->data(0, Qt::UserRole).toInt();

    return View::ViewCount;
}

QString ViewTab::currentTable() const
{
    QTreeWidgetItem * item = ui->trw_navigation->currentItem();

    if (item && item->parent() == group_tables)
        return item->text(0);

    return QString();
}

bool ViewTab::isShowDateUpdatedChecked() const
{
    return parentWindow()->isShowDateUpdatedChecked();
}

bool ViewTab::isShowOwnerChecked() const
{
    return parentWindow()->isShowOwnerChecked();
}

bool ViewTab::isCompareValuesChecked() const
{
    return parentWindow()->isCompareValuesChecked();
}

bool ViewTab::isPrinterFriendlyVersionChecked() const
{
    return parentWindow()->isPrinterFriendlyVersionChecked();
}

QString ViewTab::appendDefaultOrderToColumn(const QString & column) const
{
    return parentWindow()->appendDefaultOrderToColumn(column);
}

void ViewTab::emitEnableBackAndForwardButtons(bool enableBack, bool enableForward)
{
    emit enableBackButton(enableBack);
    emit enableForwardButton(enableForward);
}

void ViewTab::viewChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
#ifdef Q_OS_MAC
    if (previous && previous->parent()) {
        QFont font = previous->font(0);
        font.setBold(false);
        previous->setFont(0, font);
    }
#endif

    if (!current || !current->parent())
        return;

#ifdef Q_OS_MAC
    QFont font = current->font(0);
    font.setBold(true);
    current->setFont(0, font);
#endif

    View::ViewID view = (View::ViewID)current->data(0, Qt::UserRole).toInt();

    if (receivedLink()) {
        loadReceivedLink();
    } else {
        saveLink(view);
    }

    emit viewChanged(view);

    ui->wv_main->setHtml(views[view]->renderHTML(), QUrl("qrc:/html/"));
}

void ViewTab::loadPreviousLink()
{
    ViewTabSettings::loadPreviousLink();
    executeLink(receivedLink());
}

void ViewTab::loadNextLink()
{
    ViewTabSettings::loadNextLink();
    executeLink(receivedLink());
}

void ViewTab::executeLink(const QUrl & url)
{
    Link * link = linkParser().parse(url.toString());
    if (link) {
        setReceivedLink(link);
        executeLink(link);
    }
}

void ViewTab::executeLink(Link * link)
{
    QString id;
    bool ok = false;

    bool select_with_javascript = false;
    bool view_changed = true;
    Link * last_link = lastLink();
    if (last_link != NULL && link->compareViews(*last_link) < Link::MinViewDifference)
        view_changed = false;

    switch (link->viewAt(0)) {
    case LinkParser::ToggleDetailsVisible:
        id = link->idValue("toggledetailsvisible");
        if (id == "customer")
            mainWindowSettings().toggleCustomerDetailsVisible();
        else if (id == "circuit")
            mainWindowSettings().toggleCircuitDetailsVisible();
        refreshView();
        break;

    case LinkParser::Customer:
        select_with_javascript = !view_changed;
        id = link->idValue("customer");
        if (id != selectedCustomer()) {
            loadCustomer(id.toInt(), view_changed && link->countViews() <= 1 && link->action() == Link::View);
        } else if (link->countViews() <= 1 && link->action() == Link::View) {
            setView(View::Circuits);
        }

        if (link->countViews() <= 1 && link->action() == Link::Edit)
            parentWindow()->editCustomer();
        break;

    case LinkParser::Repair:
        select_with_javascript = !view_changed;
        loadRepair(link->idValue("repair"), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editRepair();
        break;

    case LinkParser::Store:
        if (link->action() == Link::Edit)
            parentWindow()->editServiceCompany();
        else
            setView(View::Store);
        break;

    case LinkParser::Inspector:
        select_with_javascript = !view_changed;
        loadInspector(link->idValue("inspector").toInt(), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editInspector();
        break;

    case LinkParser::InspectorReport:
        loadInspectorReport(link->idValue("inspectorreport").toInt(), link->action() == Link::View);
        break;

    case LinkParser::AllCustomers:
        setView(View::Customers);
        break;

    case LinkParser::ToggleDetailedView:
        id = link->idValue("toggledetailedview");
        ((StoreView *)views[View::Store])->toggleYear(id.toInt());
        refreshView();
        ui->wv_main->page()->mainFrame()->evaluateJavaScript(QString("document.getElementById('%1').scrollIntoView(true);").arg(id));
        break;

    case LinkParser::RefrigerantManagement:
        setView(View::RefrigerantManagement);
        break;

    case LinkParser::RecordOfRefrigerantManagement:
        if (link->action() == Link::Edit)
            parentWindow()->editRecordOfRefrigerantManagement(link->idValue("recordofrefrigerantmanagement"));
        break;

    case LinkParser::LeakagesByApplication:
        setView(View::LeakagesByApplication);
        break;

    case LinkParser::Agenda:
        setView(View::Agenda);
        break;

    case LinkParser::AssemblyRecordType:
        select_with_javascript = !view_changed;
        loadAssemblyRecordType(link->idValue("assemblyrecordtype").toInt(), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordType();
        break;

    case LinkParser::AssemblyRecordItemType:
        select_with_javascript = !view_changed;
        loadAssemblyRecordItemType(link->idValue("assemblyrecorditemtype").toInt(), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordItemType();
        break;

    case LinkParser::AssemblyRecordCategory:
        select_with_javascript = !view_changed;
        loadAssemblyRecordItemCategory(link->idValue("assemblyrecorditemcategory").toInt(), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editAssemblyRecordItemCategory();
        break;

    case LinkParser::CircuitUnitType:
        select_with_javascript = !view_changed;
        loadCircuitUnitType(link->idValue("circuitunittype").toInt(), view_changed && link->action() == Link::View);
        if (link->action() == Link::Edit)
            parentWindow()->editCircuitUnitType();
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;

    case LinkParser::AllInspectors:
        setView(View::Inspectors);
        break;

    case LinkParser::AllAssemblyRecordTypes:
        setView(View::AssemblyRecordTypes);
        break;

    case LinkParser::AllAssemblyRecordItemTypes:
        setView(View::AssemblyRecordItemTypes);
        break;

    case LinkParser::AllAssemblyRecordItemCategories:
        setView(View::AssemblyRecordItemCategories);
        break;

    case LinkParser::AllCircuitUnitTypes:
        setView(View::CircuitUnitTypes);
        break;

    case LinkParser::AllRepairs:
        setView(View::Repairs);
        break;
    }

    switch (link->viewAt(1)) {
    case LinkParser::Circuit:
        if (link->idValue("circuit") != selectedCircuit())
            loadCircuit(link->idValue("circuit").toInt(), link->countViews() <= 2 && link->action() == Link::View);
        else if (link->countViews() <= 2 && link->action() == Link::View)
            setView(View::Inspections);

        if (link->countViews() <= 2 && link->action() == Link::Edit)
            parentWindow()->editCircuit();
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;

    case LinkParser::OperatorReport:
        setView(View::OperatorReport);
        break;

    case LinkParser::AllRepairs:
        setView(View::Repairs);
        break;
    }

    switch (link->viewAt(2)) {
    case LinkParser::Inspection:
        if (link->idValue("inspection") != selectedInspection())
            loadInspection(link->idValue("inspection"), link->countViews() <= 3 && link->action() == Link::View);
        else if (link->countViews() <= 3 && link->action() == Link::View)
            setView(View::InspectionDetails);

        if (link->action() == Link::Edit)
            parentWindow()->editInspection();
        break;

    case LinkParser::AssemblyRecord:
        id = link->lastIdValue();
        id.remove(0, id.indexOf(":") + 1);
        if (id != selectedInspection())
            loadAssemblyRecord(id, link->action() == Link::View);
        else if (link->action() == Link::View)
            setView(View::AssemblyRecordDetails);
        break;

    case LinkParser::TableOfInspections:
        setView(View::TableOfInspections);
        break;

    case LinkParser::Compressor:
        ok = false;
        setSelectedCompressor(link->idValue("compressor").toInt(&ok));
        if (!ok) setSelectedCompressor(-1);
        break;

    case LinkParser::AllAssemblyRecords:
        setView(View::AssemblyRecords);
        break;
    }

    switch (link->viewAt(3)) {
    case LinkParser::AssemblyRecord:
        setView(View::AssemblyRecordDetails);
        break;

    case LinkParser::TableOfInspections:
        setView(View::TableOfInspections);
        break;

    case LinkParser::InspectionImages:
        setView(View::InspectionImages);
        break;
    }

    if (!link->countIds())
        select_with_javascript = false;
    if (select_with_javascript) {
        loadReceivedLink();
        ui->wv_main->page()->mainFrame()->evaluateJavaScript(QString("select('%1:%2');").arg(link->lastIdKey()).arg(link->lastIdValue()));
    }
}

void ViewTab::saveLink(int view)
{
    UrlEntity * url_entity = NULL, * e = NULL;

    switch (view) {
    case View::Store:
        url_entity = new UrlEntity("servicecompany");
        break;

    case View::RefrigerantManagement:
        url_entity = new UrlEntity("refrigerantmanagement");
        break;

    case View::Customers:
        url_entity = new UrlEntity("allcustomers");
        break;

    case View::Circuits:
        url_entity = new UrlEntity("customer", selectedCustomer());
        break;

    case View::Inspections:
        url_entity = new UrlEntity("customer", selectedCustomer());
        url_entity->addNext("circuit", selectedCircuit());
        break;

    case View::Repairs:
        url_entity = new UrlEntity;
        if (isCustomerSelected())
            url_entity->addNext("customer", selectedCustomer());
        url_entity->addNext("allrepairs");
        break;

    case View::Inspectors:
        url_entity = new UrlEntity("allinspectors");
        break;

    case View::InspectorDetails:
        url_entity = new UrlEntity("inspector", selectedInspector());
        break;

    case View::TableOfInspections:
        url_entity = new UrlEntity("customer", selectedCustomer());
        e = url_entity->addNext("circuit", selectedCircuit());
        if (isCompressorSelected())
            e = e->addNext("compressor", selectedCompressor());
        e->addNext("table");
        break;

    case View::InspectionDetails:
        url_entity = new UrlEntity("customer", selectedCustomer());
        url_entity->addNext("circuit", selectedCircuit())
                ->addNext("inspection", selectedInspection());
        break;

    case View::Agenda:
        url_entity = new UrlEntity("agenda");
        break;

    case View::OperatorReport:
        url_entity = new UrlEntity("customer", selectedCustomer());
        url_entity->addNext("operatorreport");
        break;

    case View::AssemblyRecordTypes:
        url_entity = new UrlEntity("allassemblyrecordtypes");
        break;

    case View::AssemblyRecordItemTypes:
        url_entity = new UrlEntity("allassemblyrecorditemtypes");
        break;

    case View::AssemblyRecordItemCategories:
        url_entity = new UrlEntity("allassemblyrecorditemcategories");
        break;

    case View::AssemblyRecordDetails:
        url_entity = new UrlEntity("customer", selectedCustomer());
        url_entity->addNext("circuit", selectedCircuit())
                ->addNext("assemblyrecord", selectedInspection());
        break;

    case View::CircuitUnitTypes:
        url_entity = new UrlEntity("allcircuitunittypes");
        break;

    case View::AssemblyRecords:
        e = url_entity = new UrlEntity;
        if (isCustomerSelected())
            e = e->addNext("customer", selectedCustomer());
        if (isCircuitSelected())
            e = e->addNext("circuit", selectedCircuit());
        e->addNext("allassemblyrecords");
        break;

    case View::InspectionImages:
        url_entity = new UrlEntity("customer", selectedCustomer());
        url_entity->addNext("circuit", selectedCircuit())
                ->addNext("inspection", selectedInspection())
                ->addNext("images");
        break;

    case View::LeakagesByApplication:
        url_entity = new UrlEntity("leakagesbyapplication");
        break;

    default:
        break;
    }
    if (url_entity)
        setLastLink(linkParser().parse(url_entity));
}

void ViewTab::setDefaultWebPage()
{
    MTWebPage * page = new MTWebPage(ui->wv_main);
    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->wv_main->setPage(page);
}

void ViewTab::reportData()
{
    ui->trw_navigation->setVisible(false);

    ReportDataController * controller = new ReportDataController(ui->wv_main, ui->toolbarstack);
    QObject::connect(controller, SIGNAL(processing(bool)), this, SLOT(setDisabled(bool)));
    QObject::connect(controller, SIGNAL(destroyed()), this, SLOT(reportDataFinished()));
}

void ViewTab::reportDataFinished()
{
    ui->trw_navigation->setVisible(true);

    parentWindow()->reportDataFinished();
    setDefaultWebPage();
    refreshView();
}