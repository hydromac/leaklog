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

#ifndef VIEWTABSETTINGS_H
#define VIEWTABSETTINGS_H

#include "view.h"
#include "linkparser.h"

#include <QString>

class Navigation;
class MainWindowSettings;

class QObject;
class QWebView;

class ViewTabSettings
{
public:
    ViewTabSettings();
    virtual QObject * object() = 0;

    virtual void enableTools() = 0; // TODO: remove

    virtual MainWindowSettings & mainWindowSettings() = 0;

    virtual Navigation * navigation() const = 0;
    virtual QWebView * webView() const = 0;

    virtual void setView(View::ViewID view) = 0;
    virtual void refreshView() = 0;

    virtual View * view(View::ViewID view) = 0;
    virtual View::ViewID currentView() const = 0;
    virtual QString currentTable() const = 0;

    virtual bool isShowDateUpdatedChecked() const = 0;
    virtual bool isShowOwnerChecked() const = 0;
    virtual bool isCompareValuesChecked() const = 0;
    virtual bool isPrinterFriendlyVersionChecked() const = 0;

    virtual QString appendDefaultOrderToColumn(const QString &) const = 0;

    void restoreDefaults();

    inline bool isCustomerSelected() const { return m_customer >= 0; }
    inline QString selectedCustomer() const { return QString::number(m_customer); }
    void setSelectedCustomer(int customer, const QString & company = QString());
    void clearSelectedCustomer() { m_customer = -1; m_customer_company.clear(); clearSelectedCircuit(); }

    inline const QString & selectedCustomerCompany() const { return m_customer_company; }

    inline bool isCircuitSelected() const { return m_circuit >= 0; }
    inline QString selectedCircuit() const { return QString::number(m_circuit); }
    void setSelectedCircuit(int circuit) { clearSelectedCircuit(); m_circuit = circuit; }
    void clearSelectedCircuit() { m_circuit = -1; m_compressor = -1; clearSelectedInspection(); clearSelectedRepair(); }

    inline bool isCompressorSelected() const { return m_compressor >= 0; }
    inline QString selectedCompressor() const { return QString::number(m_compressor); }
    void setSelectedCompressor(int compressor) { m_compressor = compressor; }

    inline bool isInspectionSelected() const { return !m_inspection.isEmpty(); }
    inline QString selectedInspection() const { return m_inspection; }
    void setSelectedInspection(const QString & inspection, bool has_ar = false) { m_inspection = inspection; m_has_assembly_record = has_ar; }
    void clearSelectedInspection() { m_inspection.clear(); m_has_assembly_record = false; m_inspection_is_repair = false; }
    inline bool hasAssemblyRecord() const { return m_has_assembly_record; }

    inline bool selectedInspectionIsRepair() const { return m_inspection_is_repair; }
    void setSelectedInspectionIsRepair(bool inspection_is_repair) { m_inspection_is_repair = inspection_is_repair; }

    inline bool isRepairSelected() const { return !m_repair.isEmpty(); }
    inline QString selectedRepair() const { return m_repair; }
    void setSelectedRepair(const QString & repair) { m_repair = repair; }
    inline void clearSelectedRepair() { m_repair.clear(); }

    inline bool isInspectorSelected() const { return m_inspector >= 0; }
    inline QString selectedInspector() const { return QString::number(m_inspector); }
    void setSelectedInspector(int inspector, const QString & inspector_name = QString());

    inline const QString & selectedInspectorName() const { return m_inspector_name; }

    inline QString selectedAssemblyRecordType() const { return QString::number(m_assembly_record_type); }
    inline bool isAssemblyRecordTypeSelected() const { return m_assembly_record_type >= 0; }
    void setSelectedAssemblyRecordType(int assembly_record_type) { m_assembly_record_type = assembly_record_type; }

    inline bool isAssemblyRecordItemTypeSelected() const { return m_assembly_record_item_type >= 0; }
    inline QString selectedAssemblyRecordItemType() const { return QString::number(m_assembly_record_item_type); }
    void setSelectedAssemblyRecordItemType(int assembly_record_item_type) { m_assembly_record_item_type = assembly_record_item_type; }

    inline bool isAssemblyRecordItemCategorySelected() const { return m_assembly_record_item_category >= 0; }
    inline QString selectedAssemblyRecordItemCategory() const { return QString::number(m_assembly_record_item_category); }
    void setSelectedAssemblyRecordItemCategory(int assembly_record_item_category) { m_assembly_record_item_category = assembly_record_item_category; }

    inline bool isCircuitUnitTypeSelected() const { return m_circuit_unit_type >= 0; }
    inline QString selectedCircuitUnitType() const { return QString::number(m_circuit_unit_type); }
    void setSelectedCircuitUnitType(int circuit_unit_type) { m_circuit_unit_type = circuit_unit_type; }

    void loadCustomer(int, bool);
    void loadCircuit(int, bool);
    void loadInspection(const QString &, bool);
    void loadRepair(const QString &, bool);
    void loadInspector(int, bool);
    void loadInspectorReport(int, bool);
    void loadAssemblyRecordType(int, bool);
    void loadAssemblyRecordItemType(int, bool);
    void loadAssemblyRecordItemCategory(int, bool);
    void loadAssemblyRecord(const QString &, bool);
    void loadCircuitUnitType(int, bool);

    Link * lastLink() const { return m_last_link; }
    void setLastLink(Link *);

    Link * receivedLink() const { return m_received_link; }
    void setReceivedLink(Link * link);
    void loadReceivedLink();

    virtual void loadPreviousLink();
    virtual void loadNextLink();

    bool hasPreviousLinks() const { return m_previous_links.count() > 0; }
    bool hasNextLinks() const { return m_next_links.count() > 0; }

    LinkParser & linkParser() { return link_parser; }

    void enableBackAndForwardButtons();

protected:
    virtual void emitEnableBackAndForwardButtons(bool enableBack, bool enableForward) = 0;

private:
    void updateLastLink();

    void saveToPreviousLinks();
    void saveToNextLinks();

    int m_customer;
    QString m_customer_company;
    int m_circuit;
    int m_compressor;
    QString m_inspection;
    bool m_inspection_is_repair;
    QString m_repair;
    int m_inspector;
    QString m_inspector_name;
    int m_assembly_record_type;
    int m_assembly_record_item_type;
    int m_assembly_record_item_category;
    int m_circuit_unit_type;
    bool m_has_assembly_record;

    Link * m_last_link;
    Link * m_received_link;
    LinkParser link_parser;
    QList<Link *> m_previous_links;
    QList<Link *> m_next_links;
};

#endif // VIEWTABSETTINGS_H
