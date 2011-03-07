/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2010 Matus & Michal Tomlein

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

#include "records.h"
#include "input_widgets.h"
#include "modify_dialogue.h"
#include "global.h"
#include "variables.h"
#include "warnings.h"

#include <QSqlRecord>
#include <QApplication>

using namespace Global;

DBRecord::DBRecord():
MTRecord()
{}

DBRecord::DBRecord(const QString & type, const QString & id_field, const QString & id, const MTDictionary & parents):
MTRecord(type, id_field, id, parents)
{}

Customer::Customer(const QString & id):
DBRecord("customers", "id", id, MTDictionary())
{}

void Customer::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Customer"));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, id(), 99999999));
    md->addInputWidget(new MDLineEdit("company", tr("Company:"), md, attributes.value("company").toString()));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md, attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md, attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM customers" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class CustomerAttributes
{
public:
    CustomerAttributes() {
        dict.insert("id", QApplication::translate("Customer", "ID"));
        dict.insert("company", QApplication::translate("Customer", "Company"));
        dict.insert("address", QApplication::translate("Customer", "Address"));
        dict.insert("mail", QApplication::translate("Customer", "E-mail"));
        dict.insert("phone", QApplication::translate("Customer", "Phone"));
    }

    MTDictionary dict;
};

const MTDictionary & Customer::attributes()
{
    static CustomerAttributes dict;
    return dict.dict;
}

Circuit::Circuit(const QString & parent, const QString & id):
DBRecord("circuits", "id", id, MTDictionary("parent", parent))
{}

void Circuit::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    QString customer = Customer(parent("parent")).stringValue("company");
    if (customer.isEmpty())
        customer = parent("parent").rightJustified(8, '0');
    md->setWindowTitle(tr("Customer: %1 %2 Cooling circuit").arg(customer).arg(rightTriangle()));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    } else {
        attributes.insert("year", QDate::currentDate().year());
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, id(), 9999));
    md->addInputWidget(new MDLineEdit("name", tr("Circuit name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("operation", tr("Place of operation:"), md, attributes.value("operation").toString()));
    md->addInputWidget(new MDLineEdit("building", tr("Building:"), md, attributes.value("building").toString()));
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md, attributes.value("device").toString()));
    md->addInputWidget(new MDCheckBox("hermetic", tr("Hermetically sealed"), md, attributes.value("hermetic").toInt()));
    md->addInputWidget(new MDPlainTextEdit("manufacturer", tr("Manufacturer:"), md, attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md, attributes.value("type").toString()));
    md->addInputWidget(new MDLineEdit("sn", tr("Serial number:"), md, attributes.value("sn").toString()));
    md->addInputWidget(new MDSpinBox("year", tr("Year of purchase:"), md, 1900, 2999, attributes.value("year").toInt()));
    md->addInputWidget(new MDDateEdit("commissioning", tr("Date of commissioning:"), md, attributes.value("commissioning").toString()));
    MDCheckBox * disused = new MDCheckBox("disused", tr("Disused"), md, attributes.value("disused").toInt());
    md->addInputWidget(disused);
    MDDateEdit * decommissioning = new MDDateEdit("decommissioning", tr("Date of decommissioning:"), md, attributes.value("decommissioning").toString());
    decommissioning->setEnabled(disused->isChecked());
    QObject::connect(disused, SIGNAL(toggled(bool)), decommissioning, SLOT(setEnabled(bool)));
    md->addInputWidget(decommissioning);
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md, attributes.value("field").toString(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md, 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md, attributes.value("oil").toString(), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md, 0.0, 999999.9, attributes.value("oil_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDCheckBox("leak_detector", tr("Fixed leakage detector installed"), md, attributes.value("leak_detector").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("runtime", tr("Run-time per day:"), md, 0.0, 24.0, attributes.value("runtime").toDouble(), QApplication::translate("Units", "hours")));
    md->addInputWidget(new MDDoubleSpinBox("utilisation", tr("Rate of utilisation:"), md, 0.0, 100.0, attributes.value("utilisation").toDouble(), QApplication::translate("Units", "%")));
    MDSpinBox * inspection_interval = new MDSpinBox("inspection_interval", tr("Inspection interval:"), md, 0, 999999, attributes.value("inspection_interval").toInt(), QApplication::translate("Units", "days"));
    inspection_interval->setSpecialValueText(tr("Automatic"));
    md->addInputWidget(inspection_interval);
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM circuits WHERE parent = :parent" + QString(id().isEmpty() ? "" : " AND id <> :id"));
    query_used_ids.bindValue(":parent", parent("parent"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class CircuitAttributes
{
public:
    CircuitAttributes() {
        dict.insert("id", QApplication::translate("Circuit", "ID"));
        dict.insert("name", QApplication::translate("Circuit", "Circuit name"));
        dict.insert("operation", QApplication::translate("Circuit", "Place of operation"));
        dict.insert("building", QApplication::translate("Circuit", "Building"));
        dict.insert("device", QApplication::translate("Circuit", "Device"));
        dict.insert("hermetic", QApplication::translate("Circuit", "Hermetically sealed"));
        dict.insert("manufacturer", QApplication::translate("Circuit", "Manufacturer"));
        dict.insert("type", QApplication::translate("Circuit", "Type"));
        dict.insert("sn", QApplication::translate("Circuit", "Serial number"));
        dict.insert("year", QApplication::translate("Circuit", "Year of purchase"));
        dict.insert("commissioning", QApplication::translate("Circuit", "Date of commissioning"));
        dict.insert("field", QApplication::translate("Circuit", "Field of application"));
        // numBasicAttributes: 12
        dict.insert("disused", QApplication::translate("Circuit", "Disused"));
        dict.insert("decommissioning", QApplication::translate("Circuit", "Date of decommissioning"));
        dict.insert("refrigerant", QApplication::translate("Circuit", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("Circuit", "Amount of refrigerant") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("oil", QApplication::translate("Circuit", "Oil"));
        dict.insert("oil_amount", QApplication::translate("Circuit", "Amount of oil") + "||" + QApplication::translate("Units", "kg"));
        dict.insert("leak_detector", QApplication::translate("Circuit", "Fixed leakage detector installed"));
        dict.insert("runtime", QApplication::translate("Circuit", "Run-time per day") + "||" + QApplication::translate("Units", "hours"));
        dict.insert("utilisation", QApplication::translate("Circuit", "Rate of utilisation") + "||%");
        dict.insert("inspection_interval", QApplication::translate("Circuit", "Inspection interval") + "||" + QApplication::translate("Units", "days"));
    }

    MTDictionary dict;
};

const MTDictionary & Circuit::attributes()
{
    static CircuitAttributes dict;
    return dict.dict;
}

int Circuit::numBasicAttributes()
{
    return 12;
}

Inspection::Inspection(const QString & customer, const QString & circuit, const QString & date):
DBRecord("inspections", "date", date, MTDictionary(QStringList() << "customer" << "circuit", QStringList() << customer << circuit))
{}

Inspection::Inspection(const QString & table, const QString & id_column, const QString & id, const MTDictionary & parents):
DBRecord(table, id_column, id, parents)
{
}

void Inspection::initModifyDialogue(ModifyDialogue * md)
{
    QString customer = Customer(parent("customer")).stringValue("company");
    if (customer.isEmpty())
        customer = parent("customer").rightJustified(8, '0');
    QString circuit = Circuit(parent("customer"), parent("circuit")).stringValue("name");
    if (circuit.isEmpty())
        circuit = parent("circuit").rightJustified(4, '0');
    md->setWindowTitle(tr("Customer: %1 %2 Cooling circuit: %3 > Inspection").arg(customer).arg(rightTriangle()).arg(circuit));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    }
    bool nominal_allowed = true;
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date, nominal FROM inspections WHERE customer = :customer AND circuit = :circuit" + QString(id().isEmpty() ? "" : " AND date <> :date"));
    query_used_ids.bindValue(":customer", parent("customer"));
    query_used_ids.bindValue(":circuit", parent("circuit"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
            if (nominal_allowed && query_used_ids.value(1).toInt()) { nominal_allowed = false; }
        }
    }
    md->setUsedIds(used_ids);
    MDDateTimeEdit * date = new MDDateTimeEdit("date", tr("Date:"), md, id());
    if (isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(lockDate(), "yyyy.MM.dd"));
    }
    md->addInputWidget(date);
    MTCheckBoxGroup * chbgrp_i_type = new MTCheckBoxGroup(md);
    MDCheckBox * chb_nominal = new MDCheckBox("nominal", tr("Nominal inspection"), md, attributes.value("nominal").toInt() && nominal_allowed, nominal_allowed);
    md->addInputWidget(chb_nominal);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_nominal->widget());
    MDCheckBox * chb_repair = new MDCheckBox("repair", tr("Repair"), md, attributes.value("repair").toInt(), true);
    md->addInputWidget(chb_repair);
    chbgrp_i_type->addCheckBox((MTCheckBox *)chb_repair->widget());
    md->addInputWidget(new MDCheckBox("outside_interval", tr("Outside the inspection interval"), md, attributes.value("outside_interval").toInt()));
    Variables query; QString var_id, var_name, var_type, subvar_id, subvar_name, subvar_type;
    MDAbstractInputWidget * iw = NULL;
    while (query.next()) {
        var_id = query.value("VAR_ID").toString();
        subvar_id = query.value("SUBVAR_ID").toString();
        if (subvar_id.isEmpty()) {
            if (!query.value("VAR_VALUE").toString().isEmpty()) { continue; }
            var_name = tr("%1:").arg(query.value("VAR_NAME").toString());
            var_type = query.value("VAR_TYPE").toString();
            if (var_id == "inspector") {
                iw = new MDComboBox(var_id, var_name, md,
                    attributes.value(var_id).toString(), listInspectors(), query.value("VAR_COL_BG").toString());
                iw->label()->setAlternativeText(tr("Repairman:"));
                iw->label()->toggleAlternativeText(chb_repair->isChecked());
                iw->label()->addConnection(chb_repair, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
                md->addInputWidget(iw);
            } else if (var_id == "rmds") {
                iw = new MDPlainTextEdit(var_id, var_name, md,
                                         attributes.value(var_id).toString(), query.value("VAR_COL_BG").toString());
                iw->setShowInForm(false);
                md->addInputWidget(iw);
            } else if (var_id == "operator") {
                iw = new MDComboBox(var_id, var_name, md,
                    attributes.value(var_id).toString(), listOperators(parent("customer")), query.value("VAR_COL_BG").toString());
                md->addInputWidget(iw);
            } else if (var_id == "ar_type") {
                iw = new MDComboBox(var_id, var_name, md,
                    attributes.value(var_id).toString(), listAssemblyRecordTypes(), query.value("VAR_COL_BG").toString());
                iw->setShowInForm(false);
                md->addInputWidget(iw);
            } else if (var_type == "int") {
                md->addInputWidget(new MDSpinBox(var_id, var_name, md, -999999999, 999999999,
                    attributes.value(var_id).toInt(), query.value("VAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "float") {
                iw = new MDNullableDoubleSpinBox(var_id, var_name, md, -999999999.9, 999999999.9,
                    attributes.value(var_id), query.value("VAR_UNIT").toString(), query.value("VAR_COL_BG").toString());
                if (var_id == "refr_add_am") {
                    iw->label()->setAlternativeText(tr("New charge:"));
                    iw->label()->toggleAlternativeText(chb_nominal->isChecked());
                    iw->label()->addConnection(chb_nominal, SIGNAL(toggled(bool)), SLOT(toggleAlternativeText(bool)));
                }
                md->addInputWidget(iw);
            } else if (var_type == "string") {
                iw = new MDLineEdit(var_id, var_name, md,
                                    attributes.value(var_id).toString(), 0, query.value("VAR_COL_BG").toString());
                if (var_id == "arno") {
                    if (id().isEmpty()) iw->setVariantValue(QString("%1-%2-%3").arg(parent("customer")).arg(parent("circuit")).arg(date->variantValue().toDateTime().toString("yyMMdd")));
                    iw->setShowInForm(false);
                }
                md->addInputWidget(iw);
            } else if (var_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(var_id, var_name, md,
                    attributes.value(var_id).toString(), query.value("VAR_COL_BG").toString()));
            } else if (var_type == "bool") {
                iw = new MDCheckBox(var_id, "", md, attributes.value(var_id).toInt());
                iw->label()->setLabelText(var_name);
                md->addInputWidget(iw);
            } else {
                md->addInputWidget(new MDLineEdit(var_id, var_name, md,
                    attributes.value(var_id).toString(), 0, query.value("VAR_COL_BG").toString()));
            }
        } else {
            if (!query.value("SUBVAR_VALUE").toString().isEmpty()) { continue; }
            subvar_name = tr("%1: %2:").arg(query.value("VAR_NAME").toString()).arg(query.value("SUBVAR_NAME").toString());
            subvar_type = query.value("SUBVAR_TYPE").toString();
            if (subvar_type == "int") {
                md->addInputWidget(new MDSpinBox(subvar_id, subvar_name, md, -999999999, 999999999,
                    attributes.value(subvar_id).toInt(), query.value("SUBVAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "float") {
                md->addInputWidget(new MDNullableDoubleSpinBox(subvar_id, subvar_name, md, -999999999.9, 999999999.9,
                    attributes.value(subvar_id), query.value("SUBVAR_UNIT").toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "string") {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), 0, query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "text") {
                md->addInputWidget(new MDPlainTextEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), query.value("VAR_COL_BG").toString()));
            } else if (subvar_type == "bool") {
                iw = new MDCheckBox(subvar_id, query.value("SUBVAR_NAME").toString(), md,
                    attributes.value(subvar_id).toInt());
                iw->label()->setLabelText(tr("%1:").arg(query.value("VAR_NAME").toString()));
                md->addInputWidget(iw);
            } else {
                md->addInputWidget(new MDLineEdit(subvar_id, subvar_name, md,
                    attributes.value(subvar_id).toString(), 0, query.value("VAR_COL_BG").toString()));
            }
        }
    }
}

InspectionByInspector::InspectionByInspector(const QString & inspector_id):
Inspection("inspections LEFT JOIN customers ON inspections.customer = customers.id"
           " LEFT JOIN circuits ON inspections.circuit = circuits.id AND circuits.parent = inspections.customer", "date", "", MTDictionary("inspector", inspector_id))
{
}

Repair::Repair(const QString & date):
DBRecord("repairs", "date", date, MTDictionary())
{}

void Repair::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Repair"));
    QVariantMap attributes;
    if (!id().isEmpty() || !values().isEmpty()) {
        attributes = list();
    } else {
        for (int i = 0; i < parents().count(); ++i) {
            attributes.insert(parents().key(i), parents().value(i));
        }
    }
    MDDateTimeEdit * date = new MDDateTimeEdit("date", tr("Date:"), md, id());
    if (isDatabaseLocked())
        date->setMinimumDate(QDate::fromString(lockDate(), "yyyy.MM.dd"));
    md->addInputWidget(date);
    MDLineEdit * customer = new MDLineEdit("customer", tr("Customer:"), md, attributes.value("customer").toString());
    if (!attributes.value("parent").toString().isEmpty())
        customer->setEnabled(false);
    md->addInputWidget(customer);
    md->addInputWidget(new MDLineEdit("device", tr("Device:"), md, attributes.value("device").toString()));
    md->addInputWidget(new MDComboBox("field", tr("Field of application:"), md, attributes.value("field").toString(), fieldsOfApplication()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    MDComboBox * repairman = new MDComboBox("repairman", tr("Repairman:"), md, attributes.value("repairman").toString(), listInspectors());
    repairman->setNullValue(QVariant(QVariant::Int));
    md->addInputWidget(repairman);
    md->addInputWidget(new MDLineEdit("arno", tr("Assembly record No.:"), md, attributes.value("arno").toString()));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md, 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_add_am", tr("Refrigerant addition:"), md, -999999999.9, 999999999.9, attributes.value("refr_add_am").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_reco", tr("Refrigerant recovery:"), md, -999999999.9, 999999999.9, attributes.value("refr_reco").toDouble(), QApplication::translate("Units", "kg")));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM repairs" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class RepairAttributes
{
public:
    RepairAttributes() {
        dict.insert("date", QApplication::translate("Repair", "Date"));
        dict.insert("customer", QApplication::translate("Repair", "Customer"));
        dict.insert("device", QApplication::translate("Repair", "Device"));
        dict.insert("field", QApplication::translate("Repair", "Field of application"));
        dict.insert("refrigerant", QApplication::translate("Repair", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("Repair", "Amount of refrigerant"));
        dict.insert("refr_add_am", QApplication::translate("Repair", "Refrigerant addition"));
        dict.insert("refr_reco", QApplication::translate("Repair", "Refrigerant recovery"));
        dict.insert("repairman", QApplication::translate("Repair", "Repairman"));
        dict.insert("arno", QApplication::translate("Repair", "Assembly record No."));
    }

    MTDictionary dict;
};

const MTDictionary & Repair::attributes()
{
    static RepairAttributes dict;
    return dict.dict;
}

VariableRecord::VariableRecord(Type type, const QString & var_id, const QString & subvar_id):
DBRecord(type ? "subvariables" : "variables", "id", type ? subvar_id : var_id, type ? MTDictionary("parent", var_id) : MTDictionary())
{ v_type = type; }

void VariableRecord::initModifyDialogue(ModifyDialogue * md)
{
    switch (v_type) {
        case SUBVARIABLE: md->setWindowTitle(tr("Subvariable")); break;
        default: md->setWindowTitle(tr("Variable")); break;
    }
    QVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        if (v_type == SUBVARIABLE) {
            Subvariable query(parent("parent"), id());
            if (query.next()) {
                attributes.insert("id", query.value("SUBVAR_ID"));
                attributes.insert("name", query.value("SUBVAR_NAME"));
                attributes.insert("type", query.value("SUBVAR_TYPE"));
                attributes.insert("unit", query.value("SUBVAR_UNIT"));
                attributes.insert("value", query.value("SUBVAR_VALUE"));
                attributes.insert("compare_nom", query.value("SUBVAR_COMPARE_NOM"));
                attributes.insert("tolerance", query.value("SUBVAR_TOLERANCE"));
            }
        } else {
            Variable query(id());
            if (query.next()) {
                attributes.insert("id", query.value("VAR_ID"));
                attributes.insert("name", query.value("VAR_NAME"));
                attributes.insert("type", query.value("VAR_TYPE"));
                attributes.insert("unit", query.value("VAR_UNIT"));
                attributes.insert("value", query.value("VAR_VALUE"));
                attributes.insert("compare_nom", query.value("VAR_COMPARE_NOM"));
                attributes.insert("tolerance", query.value("VAR_TOLERANCE"));
                attributes.insert("col_bg", query.value("VAR_COL_BG"));
            }
        }
        if (variableNames().contains(id())) { enable_all = false; }
    }
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds(true);
    if (!id().isEmpty()) { used_ids.removeAll(id()); }
    md->setUsedIds(used_ids);
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md, attributes.value("unit").toString(), 0, "", enable_all));
    md->addInputWidget(new MDComboBox("type", tr("Type:"), md, attributes.value("type").toString(), MTDictionary(variableTypes()).swapKeysAndValues(), "", enable_all));
    md->addInputWidget(new MDHighlightedPlainTextEdit("value", tr("Value:"), md, attributes.value("value").toString(), used_ids, enable_all));
    md->addInputWidget(new MDCheckBox("compare_nom", tr("Compare value with the nominal one"), md, attributes.value("compare_nom").toInt()));
    md->addInputWidget(new MDDoubleSpinBox("tolerance", tr("Tolerance:"), md, 0.0, 999999.9, attributes.value("tolerance").toDouble()));
    if (v_type == VARIABLE) {
        md->addInputWidget(new MDColourComboBox("col_bg", tr("Colour:"), md, attributes.value("col_bg").toString()));
    }
}

Table::Table(const QString & id, const QString & uid):
DBRecord("tables", uid.isEmpty() ? "id" : "uid", uid.isEmpty() ? id : uid, MTDictionary())
{}

void Table::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Table"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("Name:"), md, attributes.value("id").toString()));
    md->addInputWidget(new MDCheckBox("highlight_nominal", tr("Highlight the nominal inspection"), md, attributes.value("highlight_nominal").toInt()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM tables" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

Inspector::Inspector(const QString & id):
DBRecord("inspectors", "id", id, MTDictionary())
{}

void Inspector::initModifyDialogue(ModifyDialogue * md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Inspector"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 9999));
    md->addInputWidget(new MDLineEdit("person", tr("Certified person:"), md, attributes.value("person").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md, attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    MDInputWidget * iw = new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md, 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency);
    iw->setShowInForm(false);
    md->addInputWidget(iw);
    iw = new MDDoubleSpinBox("list_price", tr("List price:"), md, 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency);
    iw->setShowInForm(false);
    md->addInputWidget(iw);
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM inspectors" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class InspectorAttributes
{
public:
    InspectorAttributes() {
        dict.insert("id", QApplication::translate("Inspector", "ID"));
        dict.insert("person", QApplication::translate("Inspector", "Certified person"));
        dict.insert("mail", QApplication::translate("Inspector", "E-mail"));
        dict.insert("phone", QApplication::translate("Inspector", "Phone"));
    }

    MTDictionary dict;
};

const MTDictionary & Inspector::attributes()
{
    static InspectorAttributes dict;
    return dict.dict;
}

ServiceCompany::ServiceCompany(const QString & id):
DBRecord("service_companies", "id", id, MTDictionary())
{}

void ServiceCompany::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Service company"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 99999999));
    md->addInputWidget(new MDAddressEdit("address", tr("Address:"), md, attributes.value("address").toString()));
    md->addInputWidget(new MDLineEdit("phone", tr("Phone:"), md, attributes.value("phone").toString()));
    md->addInputWidget(new MDLineEdit("mail", tr("E-mail:"), md, attributes.value("mail").toString()));
    md->addInputWidget(new MDLineEdit("website", tr("Website:"), md, attributes.value("website").toString()));
    md->addInputWidget(new MDFileChooser("image", tr("Image:"), md, attributes.value("image").toInt()));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM service_companies" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class ServiceCompanyAttributes
{
public:
    ServiceCompanyAttributes() {
        dict.insert("name", QApplication::translate("ServiceCompany", "Name:"));
        dict.insert("id", QApplication::translate("ServiceCompany", "ID:"));
        dict.insert("address", QApplication::translate("ServiceCompany", "Address:"));
        dict.insert("phone", QApplication::translate("ServiceCompany", "Phone:"));
        dict.insert("mail", QApplication::translate("ServiceCompany", "E-mail:"));
        dict.insert("website", QApplication::translate("ServiceCompany", "Website:"));
    }

    MTDictionary dict;
};

const MTDictionary & ServiceCompany::attributes()
{
    static ServiceCompanyAttributes dict;
    return dict.dict;
}

RecordOfRefrigerantManagement::RecordOfRefrigerantManagement(const QString & date):
DBRecord("refrigerant_management", "date", date, MTDictionary())
{}

void RecordOfRefrigerantManagement::initModifyDialogue(ModifyDialogue * md)
{
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));

    md->setWindowTitle(tr("Record of refrigerant management"));
    QVariantMap attributes;
    if (!id().isEmpty()) {
        attributes = list();
    }
    MDDateTimeEdit * date = new MDDateTimeEdit("date", tr("Date:"), md, attributes.value("date").toString());
    if (isDatabaseLocked()) {
        date->setMinimumDate(QDate::fromString(lockDate(), "yyyy.MM.dd"));
    }
    md->addInputWidget(date);
    md->addInputWidget(new MDLineEdit("partner", tr("Business partner:"), md, attributes.value("partner").toString()));
    MDLineEdit * partner_id = new MDLineEdit("partner_id", tr("Business partner (ID):"), md, attributes.value("partner_id").toString(), 99999999);
    partner_id->setNullValue(QVariant(QVariant::Int));
    md->addInputWidget(partner_id);
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("purchased", tr("Purchased (new):"), md, 0.0, 999999999.9, attributes.value("purchased").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("purchased_reco", tr("Purchased (recovered):"), md, 0.0, 999999999.9, attributes.value("purchased_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold", tr("Sold (new):"), md, 0.0, 999999999.9, attributes.value("sold").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("sold_reco", tr("Sold (recovered):"), md, 0.0, 999999999.9, attributes.value("sold_reco").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_rege", tr("Reclaimed:"), md, 0.0, 999999999.9, attributes.value("refr_rege").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("refr_disp", tr("Disposed of:"), md, 0.0, 999999999.9, attributes.value("refr_disp").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked", tr("Leaked (new):"), md, 0.0, 999999999.9, attributes.value("leaked").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("leaked_reco", tr("Leaked (recovered):"), md, 0.0, 999999999.9, attributes.value("leaked_reco").toDouble(), QApplication::translate("Units", "kg")));
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT date FROM refrigerant_management" + QString(id().isEmpty() ? "" : " WHERE date <> :date"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":date", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class RecordOfRefrigerantManagementAttributes
{
public:
    RecordOfRefrigerantManagementAttributes() {
        dict.insert("date", QApplication::translate("RecordOfRefrigerantManagement", "Date"));
        dict.insert("partner", QApplication::translate("RecordOfRefrigerantManagement", "Business partner"));
        dict.insert("partner_id", QApplication::translate("RecordOfRefrigerantManagement", "Business partner (ID)"));
        dict.insert("refrigerant", QApplication::translate("RecordOfRefrigerantManagement", "Refrigerant"));
        dict.insert("purchased", QApplication::translate("RecordOfRefrigerantManagement", "Purchased (new)"));
        dict.insert("purchased_reco", QApplication::translate("RecordOfRefrigerantManagement", "Purchased (recovered)"));
        dict.insert("sold", QApplication::translate("RecordOfRefrigerantManagement", "Sold (new)"));
        dict.insert("sold_reco", QApplication::translate("RecordOfRefrigerantManagement", "Sold (recovered)"));
        dict.insert("refr_rege", QApplication::translate("RecordOfRefrigerantManagement", "Reclaimed"));
        dict.insert("refr_disp", QApplication::translate("RecordOfRefrigerantManagement", "Disposed of"));
        dict.insert("leaked", QApplication::translate("RecordOfRefrigerantManagement", "Leaked (new)"));
        dict.insert("leaked_reco", QApplication::translate("RecordOfRefrigerantManagement", "Leaked (recovered)"));
    }

    MTDictionary dict;
};

const MTDictionary & RecordOfRefrigerantManagement::attributes()
{
    static RecordOfRefrigerantManagementAttributes dict;
    return dict.dict;
}

WarningRecord::WarningRecord(const QString & id):
DBRecord("warnings", "id", id, MTDictionary())
{}

void WarningRecord::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Warning"));
    QVariantMap attributes; bool enable_all = true;
    if (!id().isEmpty()) {
        Warning query(id().toInt());
        if (query.next()) {
            for (int i = 0; i < query.record().count(); ++i) {
                attributes.insert(query.record().fieldName(i), query.value(query.record().fieldName(i)));
            }
        }
        if (!attributes.value("name").toString().isEmpty()) {
            md->setWindowTitle(tr("%1: %2").arg(tr("Warning")).arg(attributes.value("name").toString()));
        }
        enable_all = id().toInt() < 1000;
    }
    md->addInputWidget(new MDCheckBox("enabled", tr("Enabled"), md, attributes.value("enabled").toInt()));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString(), 0, "", enable_all));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md, attributes.value("description").toString(), 0, "", enable_all));
    md->addInputWidget(new MDSpinBox("delay", tr("Delay:"), md, 0, 999999, attributes.value("delay").toInt(), tr("days"), "", enable_all));
    QStringList used_ids;
    used_ids << "refrigerant_amount" << "oil_amount" << "sum" << "p_to_t";
    used_ids << listSupportedFunctions();
    used_ids << listVariableIds();
    md->setUsedIds(used_ids);
}

AssemblyRecordType::AssemblyRecordType(const QString & id):
DBRecord("assembly_record_types", "id", id, MTDictionary())
{}

void AssemblyRecordType::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Assembly record type"));

    QVariantMap attributes;
    if (!id().isEmpty() || !this->attributes().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 99999999));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("description", tr("Description:"), md, attributes.value("description").toString()));
    MDGroupedCheckBoxes * md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display options:"), md, attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordType::ShowServiceCompany, tr("Show service company"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomer, tr("Show customer"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCustomerContactPersons, tr("Show customer contact persons"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuit, tr("Show circuit"));
    md_display_options->addCheckBox(AssemblyRecordType::ShowCircuitUnits, tr("Show circuit units"));
    md->addInputWidget(md_display_options);
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class AssemblyRecordTypeAttributes
{
public:
    AssemblyRecordTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordType", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordType", "Name"));
        dict.insert("description", QApplication::translate("AssemblyRecordType", "Description"));
    }

    MTDictionary dict;
};

const MTDictionary & AssemblyRecordType::attributes()
{
    static AssemblyRecordTypeAttributes dict;
    return dict.dict;
}

bool AssemblyRecordType::remove()
{
    AssemblyRecordTypeCategory type_categories(id());
    return type_categories.remove() && MTRecord::remove();
}

AssemblyRecordItemType::AssemblyRecordItemType(const QString & id):
DBRecord("assembly_record_item_types", "id", id, MTDictionary())
{}

void AssemblyRecordItemType::initModifyDialogue(ModifyDialogue * md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Assembly record item type"));

    QVariantMap attributes;
    if (!id().isEmpty() || !this->attributes().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 99999999));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString()));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit:"), md, attributes.value("unit").toString()));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md, 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md, 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md, 0.0, 100.0, attributes.value("discount").toDouble(), "%"));
    md->addInputWidget(new MDSpinBox("ean", tr("EAN code:"), md, 0, 99999999, attributes.value("ean").toInt()));
    md->addInputWidget(new MDComboBox("category_id", tr("Category:"), md, attributes.value("category_id").toString(), listAssemblyRecordItemCategories()));
    md->addInputWidget(new MDComboBox("inspection_variable_id", tr("Get value from inspection:"), md, attributes.value("inspection_variable_id").toString(), listAllVariables()));
    md->addInputWidget(new MDComboBox("value_data_type", tr("Data type:"), md, attributes.value("value_data_type").toString(), listDataTypes()));

    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class AssemblyRecordItemTypeAttributes
{
public:
    AssemblyRecordItemTypeAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordItemType", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordItemType", "Name"));
        dict.insert("unit", QApplication::translate("AssemblyRecordItemType", "Unit"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItemType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItemType", "List price"));
        dict.insert("discount", QApplication::translate("AssemblyRecordItemType", "Discount"));
        dict.insert("ean", QApplication::translate("AssemblyRecordItemType", "EAN code"));
        dict.insert("category_id", QApplication::translate("AssemblyRecordItemType", "Category"));
    }

    MTDictionary dict;
};

const MTDictionary & AssemblyRecordItemType::attributes()
{
    static AssemblyRecordItemTypeAttributes dict;
    return dict.dict;
}

AssemblyRecordTypeCategory::AssemblyRecordTypeCategory(const QString & record_type_id):
MTRecord("assembly_record_type_categories", "", "", MTDictionary("record_type_id", record_type_id))
{}

class AssemblyRecordTypeCategoryAttributes
{
public:
    AssemblyRecordTypeCategoryAttributes() {
        dict.insert("record_type_id", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record type ID"));
        dict.insert("record_category_id", QApplication::translate("AssemblyRecordTypeCategory", "Assembly record category type ID"));
    }

    MTDictionary dict;
};

const MTDictionary & AssemblyRecordTypeCategory::attributes()
{
    static AssemblyRecordTypeCategoryAttributes dict;
    return dict.dict;
}

AssemblyRecordItemCategory::AssemblyRecordItemCategory(const QString & id)
    : DBRecord("assembly_record_item_categories", "id", id, MTDictionary())
{
}

class AssemblyRecordItemCategoryAttributes
{
public:
    AssemblyRecordItemCategoryAttributes() {
        dict.insert("id", QApplication::translate("AssemblyRecordItemCategory", "ID"));
        dict.insert("name", QApplication::translate("AssemblyRecordItemCategory", "Name"));
    }

    MTDictionary dict;
};

const MTDictionary & AssemblyRecordItemCategory::attributes()
{
    static AssemblyRecordItemCategoryAttributes dict;
    return dict.dict;
}

void AssemblyRecordItemCategory::initModifyDialogue(ModifyDialogue * md)
{
    md->setWindowTitle(tr("Assembly record item category"));

    QVariantMap attributes;
    if (!id().isEmpty() || !this->attributes().isEmpty()) {
        attributes = list();
    }

    if (attributes.value("id").toInt() >= 1000)
        md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 99999999, QString(), false));
    else
        md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 999));
    md->addInputWidget(new MDLineEdit("name", tr("Name:"), md, attributes.value("name").toString()));
    MDGroupedCheckBoxes * md_display_options = new MDGroupedCheckBoxes("display_options", tr("Display Options:"), md, attributes.value("display_options").toInt());
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowValue, tr("Show value"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowAcquisitionPrice, tr("Show acquisition price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowListPrice, tr("Show list price"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowDiscount, tr("Show discount"));
    md_display_options->addCheckBox(AssemblyRecordItemCategory::ShowTotal, tr("Calculate total"));
    md->addInputWidget(md_display_options);
    MDRadioButtonGroup * md_position = new MDRadioButtonGroup("display_position", tr("Display:"), md, QString::number(attributes.value("display_position").toInt()));
    md_position->addRadioButton(tr("In table"), QString::number(AssemblyRecordItemCategory::DisplayAtTop));
    md_position->addRadioButton(tr("Separately"), QString::number(AssemblyRecordItemCategory::DisplayAtBottom));
    md->addInputWidget(md_position);
    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM assembly_record_item_categories" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

AssemblyRecordItem::AssemblyRecordItem(const QString & record_id)
    : MTRecord("assembly_record_items", "", "", MTDictionary("arno", record_id))
{
}

AssemblyRecordItem::AssemblyRecordItem(const QString & table, const QString & id_column, const QString & id, const MTDictionary & parents)
    : MTRecord(table, id_column, id, parents)
{
}

class AssemblyRecordItemAttributes
{
public:
    AssemblyRecordItemAttributes() {
        dict.insert("arno", QApplication::translate("AssemblyRecordItem", "Assembly record number"));
        dict.insert("item_type_id", QApplication::translate("AssemblyRecordItem", "Record item type ID"));
        dict.insert("value", QApplication::translate("AssemblyRecordItem", "Value"));
        dict.insert("acquisition_price", QApplication::translate("AssemblyRecordItem", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("AssemblyRecordItem", "List price"));
    }

    MTDictionary dict;
};

const MTDictionary & AssemblyRecordItem::attributes()
{
    static AssemblyRecordItemAttributes dict;
    return dict.dict;
}

AssemblyRecordItemByInspector::AssemblyRecordItemByInspector(const QString & inspector_id)
    : AssemblyRecordItem("assembly_record_items LEFT JOIN inspections ON assembly_record_items.arno = inspections.arno", "", "",
               MTDictionary(QStringList() << "item_type_id" << "source", QStringList() << inspector_id << QString::number(AssemblyRecordItem::Inspectors)))
{
}

File::File(const QString & file_id):
MTRecord("files", "id", file_id, MTDictionary())
{}

Person::Person(const QString & person_id, const QString & customer_id):
MTRecord("persons", "id", person_id, MTDictionary("company_id", customer_id))
{}

CircuitUnitType::CircuitUnitType(const QString & id):
DBRecord("circuit_unit_types", "id", id, MTDictionary())
{}

void CircuitUnitType::initModifyDialogue(ModifyDialogue * md)
{
    QString currency = Global::DBInfoValueForKey("currency", "EUR");

    md->setWindowTitle(tr("Assembly record item type"));
    MTDictionary refrigerants(listRefrigerantsToString().split(';'));
    MTDictionary locations;
    locations.insert(tr("External"), QString::number(CircuitUnitType::External));
    locations.insert(tr("Internal"), QString::number(CircuitUnitType::Internal));
    MTDictionary output_units;
    output_units.insert("kW", "kW");
    output_units.insert("m3", "m3");

    QVariantMap attributes;
    if (!id().isEmpty() || !this->attributes().isEmpty()) {
        attributes = list();
    }

    md->addInputWidget(new MDLineEdit("id", tr("ID:"), md, attributes.value("id").toString(), 99999999));
    md->addInputWidget(new MDLineEdit("manufacturer", tr("Manufacturer:"), md, attributes.value("manufacturer").toString()));
    md->addInputWidget(new MDLineEdit("type", tr("Type:"), md, attributes.value("type").toString()));
    md->addInputWidget(new MDComboBox("refrigerant", tr("Refrigerant:"), md, attributes.value("refrigerant").toString(), refrigerants));
    md->addInputWidget(new MDDoubleSpinBox("refrigerant_amount", tr("Amount of refrigerant:"), md, 0.0, 999999.9, attributes.value("refrigerant_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDComboBox("oil", tr("Oil:"), md, attributes.value("oil").toString(), oils()));
    md->addInputWidget(new MDDoubleSpinBox("oil_amount", tr("Amount of oil:"), md, 0.0, 999999.9, attributes.value("oil_amount").toDouble(), QApplication::translate("Units", "kg")));
    md->addInputWidget(new MDDoubleSpinBox("acquisition_price", tr("Acquisition price:"), md, 0.0, 999999999.9, attributes.value("acquisition_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("list_price", tr("List price:"), md, 0.0, 999999999.9, attributes.value("list_price").toDouble(), currency));
    md->addInputWidget(new MDDoubleSpinBox("discount", tr("Discount:"), md, 0.0, 100.0, attributes.value("discount").toDouble(), "%"));
    md->addInputWidget(new MDComboBox("location", tr("Location:"), md, attributes.value("location").toString(), locations));
    md->addInputWidget(new MDLineEdit("unit", tr("Unit of measure:"), md, attributes.value("unit").toString()));
    QList<MDAbstractInputWidget *> gw_list;
    gw_list.append(new MDDoubleSpinBox("output", tr("Value:"), md, 0.0, 999999.9, attributes.value("output").toDouble()));
    gw_list.append(new MDComboBox("output_unit", tr("Unit:"), md, attributes.value("output_unit").toString(), output_units));
    gw_list.append(new MDDoubleSpinBox("output_t0_tc", tr("At t0/tc:"), md, 0.0, 999999.9, attributes.value("output_t0_tc").toDouble()));
    md->addGroupedInputWidgets(tr("Output"), gw_list);
    md->addInputWidget(new MDPlainTextEdit("notes", tr("Notes:"), md, attributes.value("notes").toString()));


    QStringList used_ids; QSqlQuery query_used_ids;
    query_used_ids.setForwardOnly(true);
    query_used_ids.prepare("SELECT id FROM circuit_unit_types" + QString(id().isEmpty() ? "" : " WHERE id <> :id"));
    if (!id().isEmpty()) { query_used_ids.bindValue(":id", id()); }
    if (query_used_ids.exec()) {
        while (query_used_ids.next()) {
            used_ids << query_used_ids.value(0).toString();
        }
    }
    md->setUsedIds(used_ids);
}

class CircuitUnitTypeAttributes
{
public:
    CircuitUnitTypeAttributes() {
        dict.insert("id", QApplication::translate("CircuitUnitType", "ID"));
        dict.insert("manufacturer", QApplication::translate("CircuitUnitType", "Manufacturer"));
        dict.insert("type", QApplication::translate("CircuitUnitType", "Type"));
        dict.insert("refrigerant", QApplication::translate("CircuitUnitType", "Refrigerant"));
        dict.insert("refrigerant_amount", QApplication::translate("CircuitUnitType", "Amount of refrigerant"));
        dict.insert("oil", QApplication::translate("CircuitUnitType", "Oil"));
        dict.insert("oil_amount", QApplication::translate("CircuitUnitType", "Amount of oil"));
        dict.insert("acquisition_price", QApplication::translate("CircuitUnitType", "Acquisition price"));
        dict.insert("list_price", QApplication::translate("CircuitUnitType", "List price"));
        dict.insert("discount", QApplication::translate("CircuitUnitType", "Discount"));
        dict.insert("location", QApplication::translate("CircuitUnitType", "Location"));
        dict.insert("unit", QApplication::translate("CircuitUnitType", "Unit of measure"));
        dict.insert("output", QApplication::translate("CircuitUnitType", "Output"));
        dict.insert("output_t0_tc", QApplication::translate("CircuitUnitType", "Output at t0/tc"));
        dict.insert("category_id", QApplication::translate("CircuitUnitType", "Category"));
        dict.insert("notes", QApplication::translate("CircuitUnitType", "Notes"));
    }

    MTDictionary dict;
};

const MTDictionary & CircuitUnitType::attributes()
{
    static CircuitUnitTypeAttributes dict;
    return dict.dict;
}

const QString CircuitUnitType::locationToString(int id)
{
    switch (id) {
    case CircuitUnitType::External:
        return tr("External");
    case CircuitUnitType::Internal:
        return tr("Internal");
    }
    return QString();
}

CircuitUnit::CircuitUnit(const MTDictionary & dict)
    : MTRecord("circuit_units", "", "", dict)
{
}

InspectionImage::InspectionImage(const QString & customer_id, const QString & circuit_id, const QString & inspection_id)
    : MTRecord("inspection_images", "", "", MTDictionary(QStringList() << "customer" << "circuit" << "date", QStringList() << customer_id << circuit_id << inspection_id))
{
}
