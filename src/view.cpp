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

#include "main_window.h"
#include "global.h"
#include "variables.h"
#include "warnings.h"
#include "dbfile.h"
#include "report_data.h"
#include "mttextstream.h"
#include "mtaddress.h"
#include "htmlbuilder.h"
#include "variable_evaluation.h"

#include <QDate>
#include <QSqlRecord>
#include <QSqlError>

using namespace Global;

QString MainWindow::viewChanged(int view)
{
    QString html;
    if (!db.isOpen()) { wv_main->setHtml(html); return html; }

    wv_main->setHtml(tr("Loading..."));
    qApp->processEvents();
    bool ok = true;
    do {
        navigation->setView(view, false);
        ok = true;
        switch (view) {
            case Navigation::ServiceCompany:
                html = viewServiceCompany(navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                break;
            case Navigation::RefrigerantManagement:
                html = viewRefrigerantManagement(navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                break;
            case Navigation::ListOfCustomers:
                html = viewAllCustomers();
                break;
            case Navigation::ListOfCircuits:
                if (isCustomerSelected()) {
                    html = viewCustomer(selectedCustomer());
                } else {
                    view = Navigation::ListOfCustomers; ok = false;
                }
                break;
            case Navigation::ListOfInspections:
                if (isCustomerSelected() && isCircuitSelected()) {
                    html = viewCircuit(selectedCustomer(), selectedCircuit(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                } else {
                    view = Navigation::ListOfCircuits; ok = false;
                }
                break;
            case Navigation::Inspection:
                if (isCustomerSelected() && isCircuitSelected() && isInspectionSelected()) {
                    html = viewInspection(selectedCustomer(), selectedCircuit(), selectedInspection());
                } else {
                    view = Navigation::ListOfInspections; ok = false;
                }
                break;
            case Navigation::TableOfInspections:
                cb_table_edit->setCurrentIndex(navigation->tableComboBox()->currentIndex());
                if (isCustomerSelected() && isCircuitSelected() && navigation->tableComboBox()->currentIndex() >= 0) {
                    html = viewTable(selectedCustomer(), selectedCircuit(), navigation->tableComboBox()->currentText(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                } else {
                    view = Navigation::ListOfCircuits; ok = false;
                }
                break;
            case Navigation::ListOfRepairs:
                html = viewRepairs(selectedRepair(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue(), isCustomerSelected() ? selectedCustomer() : "");
                break;
            case Navigation::ListOfInspectors:
                html = viewAllInspectors(selectedInspector());
                break;
            case Navigation::OperatorReport:
                if (isCustomerSelected()) {
                    html = viewOperatorReport(selectedCustomer(), navigation->filterSinceValue() == 1999 ? 0 : navigation->filterSinceValue());
                } else {
                    view = Navigation::ListOfCustomers; ok = false;
                }
                break;
            case Navigation::LeakagesByApplication:
                html = viewLeakagesByApplication();
                break;
            case Navigation::Agenda:
                html = viewAgenda();
                break;
            case Navigation::ListOfAssemblyRecordTypes:
                html = viewAllAssemblyRecordTypes(selectedAssemblyRecordType());
                break;
            case Navigation::ListOfAssemblyRecordItemTypes:
                html = viewAllAssemblyRecordItemTypes(selectedAssemblyRecordItemType());
                break;
            case Navigation::ListOfAssemblyRecordItemCategories:
                html = viewAllAssemblyRecordItemCategories(selectedAssemblyRecordItemCategory());
                break;
            case Navigation::AssemblyRecord:
                if (isCustomerSelected() && isCircuitSelected() && isInspectionSelected()) {
                    html = viewAssemblyRecord(selectedCustomer(), selectedCircuit(), selectedInspection());
                } else {
                    view = Navigation::ListOfInspections; ok = false;
                }
                break;
            case Navigation::ListOfCircuitUnitTypes:
                html = viewAllCircuitUnitTypes(selectedCircuitUnitType());
                break;
            case Navigation::ListOfAssemblyRecords:
                html = viewAllAssemblyRecords(selectedCustomer(), selectedCircuit());
                break;
            default:
                view = Navigation::ServiceCompany;
                break;
        }
    } while (!ok);
    wv_main->setHtml(html, QUrl("qrc:/html/")); return html;
}

QString MainWindow::currentView()
{
    QString view = tr("Service company");
    switch (navigation->view()) {
        case Navigation::ServiceCompany: view = QApplication::translate("Navigation", "Service company"); break;
        case Navigation::RefrigerantManagement: view = QApplication::translate("Navigation", "Refrigerant management"); break;
        case Navigation::ListOfCustomers: view = QApplication::translate("Navigation", "List of customers"); break;
        case Navigation::ListOfCircuits:
            view = Customer(selectedCustomer()).stringValue("company");
            break;
        case Navigation::ListOfInspections:
            view = Circuit(selectedCustomer(), selectedCircuit()).stringValue("name");
            view = Customer(selectedCustomer()).stringValue("company")
                   + " - " + QString(view.isEmpty() ? selectedCircuit() : view)
                   + " - " + QApplication::translate("Navigation", "List of inspections");
            break;
        case Navigation::AssemblyRecord:
        case Navigation::Inspection:
            view = Circuit(selectedCustomer(), selectedCircuit()).stringValue("name");
            view = Customer(selectedCustomer()).stringValue("company")
                   + " - " + QString(view.isEmpty() ? selectedCircuit() : view)
                   + " - " + selectedInspection();
            break;
        case Navigation::TableOfInspections:
            view = Circuit(selectedCustomer(), selectedCircuit()).stringValue("name");
            view = Customer(selectedCustomer()).stringValue("company")
                   + " - " + QString(view.isEmpty() ? selectedCircuit() : view)
                   + " - " + QApplication::translate("Navigation", "Table of inspections");
            break;
        case Navigation::ListOfRepairs: view = QApplication::translate("Navigation", "List of repairs"); break;
        case Navigation::ListOfInspectors: view = QApplication::translate("Navigation", "List of inspectors"); break;
        case Navigation::OperatorReport:
            view = QApplication::translate("Navigation", "Operator report")
                   + " - " + Customer(selectedCustomer()).stringValue("company");
            break;
        case Navigation::LeakagesByApplication: view = QApplication::translate("Navigation", "Leakages by application"); break;
        case Navigation::Agenda: view = QApplication::translate("Navigation", "Agenda"); break;
        case Navigation::ListOfAssemblyRecordTypes: view = QApplication::translate("Navigation", "List of assembly record types"); break;
        case Navigation::ListOfAssemblyRecordItemTypes: view = QApplication::translate("Navigation", "List of assembly record item types"); break;
        case Navigation::ListOfAssemblyRecordItemCategories: view = QApplication::translate("Navigation", "List of assembly record item categories"); break;
        case Navigation::ListOfCircuitUnitTypes: view = QApplication::translate("Navigation", "List of circuit unit types"); break;
        case Navigation::ListOfAssemblyRecords: break;
    }
    return view;
}

QString MainWindow::viewServiceCompany(int since)
{
    QString html; MTTextStream out(&html);
    ServiceCompany serv_company_record(DBInfoValueForKey("default_service_company"));
    QVariantMap serv_company = serv_company_record.list();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: large; width:100%; text-align: center;\"><b>";
    out << "<a href=\"servicecompany:" << serv_company.value("id").toString() << "/modify\">";
    out << tr("Service company") << "</a></b></td></tr>";
    out << "<tr><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    int num_valid = 0; QString attr_value;
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        attr_value = ServiceCompany::attributes().key(n);
        if (serv_company.value(attr_value).toString().isEmpty()) continue;
        out << "<num_attr>" << num_valid << "</num_attr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << ServiceCompany::attributes().value(n) << "&nbsp;</td>";
        out << "<td>" << MTVariant(serv_company.value(attr_value), (MTVariant::Type)dict_fieldtypes.value(attr_value)) << "</td></tr>";
        num_valid++;
    }
    if (num_valid != 0) {
        html.replace(QString("<num_attr>%1</num_attr>").arg(int(num_valid / 2 + num_valid % 2)), "</table></td><td width=\"50%\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int k = 0; k < num_valid; ++k) {
        html.remove(QString("<num_attr>%1</num_attr>").arg(k));
    }
    out << "</td></tr></table>";
    out << "</table>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><td style=\"background-color: #eee; font-size: medium; text-align: center;\"><b>";
    out << tr("Store") << "</b></td></tr>";
    out << "<tr><td align=\"center\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:60%;\" class=\"centred_with_borders\">";
    out << "<tr><th>" << tr("Year") << "</th>";
    out << "<th>" << tr("Refrigerant") << "</th>";
    out << "<th>" << tr("New in store") << "</th>";
    out << "<th>" << tr("Recovered in store") << "</th>";
    out << "<th>" << tr("Leaked in store") << "</th></tr>";
    out << "<store />";
    out << "</table></td></tr>";
    out << "<tr><td style=\"background-color: #eee; font-size: medium; text-align: center;\"><b>";
    out << tr("Refrigerant management") << "</b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"centred_with_borders\">";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    out << "<th colspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th colspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("VariableNames", "New charge") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Added") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Recovered") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Reclaimed") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th>";
    out << "<th colspan=\"2\">" << tr("Leaked in store") << "</th>";
    out << "</tr><tr style=\"background-color: #FBFBFB;\">";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "New") << "</td>";
    out << "<td>" << QApplication::translate("VariableNames", "Recovered") << "</td>";
    out << "</tr>";
    ReportData data(since);
    QString store_html; MTTextStream store_out(&store_html);
    QStringList list_refrigerants = listRefrigerantsToString().split(";");
    list_refrigerants.insert(0, "");
    QString refrigerant;
    QMap<QString, double> store_map;
    QMap<QString, double> store_recovered_map;
    QMap<QString, double> store_leaked_map;
    QList<int>::const_iterator y = data.store_years.constEnd();
    while (y != data.store_years.constBegin()) {
        y--;
        if (*y < since) break;
        store_map = data.store.value(*y);
        store_recovered_map = data.store_recovered.value(*y);
        store_leaked_map = data.store_leaked.value(*y);
        store_out << "<tr><th rowspan=\"" << store_map.size() << "\">" << *y << "</th>";
        for (int i = 0, n = 0; i < list_refrigerants.count(); ++i) {
            refrigerant = list_refrigerants.at(i);
            if (store_map.contains(refrigerant) || store_recovered_map.contains(refrigerant) || store_leaked_map.contains(refrigerant)) {
                if (n) { store_out << "<tr>"; }
                store_out << "<td>" << refrigerant << "</td>";
                store_out << "<td>" << store_map.value(refrigerant) << "</td>";
                store_out << "<td>" << store_recovered_map.value(refrigerant) << "</td>";
                store_out << "<td>" << store_leaked_map.value(refrigerant) << "</td></tr>";
                n++;
            }
        }
    }
    html.replace("<store />", store_html);
    int year, last_year = 0;
    bool it = false, bf = false;
    QString link;
    QMap<QString, QVector<double> *>::const_iterator sums_iterator;
    QVector<double> * sum_list = NULL;
    QMapIterator<QString, QVector<QString> > i(data.entries_map);
    i.toBack();
    while (i.hasPrevious()) { i.previous();
        year = i.key().left(4).toInt();
        if (year < last_year) { last_year = 0; }
        if (!last_year) {
            last_year = year;
            out << "<tr><th rowspan=\"<rowspan />\"><a href=\"toggledetailedview:" << year << "\">" << year << "</a></th>";
            int row_count = 0;
            sums_iterator = data.sums_map.constFind(QString::number(year));
            if (++sums_iterator != data.sums_map.constEnd()) {
                while (sums_iterator != data.sums_map.constEnd() && (sum_list = sums_iterator.value())) {
                    if (row_count) { out << "</tr><tr>"; }
                    out << "<th>" << sums_iterator.key().split("::").last() << "</th>";
                    for (int n = 0; n < sum_list->count(); ++n) {
                        out << "<th>";
                        if (sum_list->at(n)) out << sum_list->at(n);
                        out << "</th>";
                    }
                    row_count++;
                    ++sums_iterator;
                }
            }
            out << "</tr>";
            html.replace("<rowspan />", QString::number(row_count));
        }
        if (years_expanded_in_service_company_view.contains(year)) {
            link = i.value().at(0);
            bf = link.contains("nominal");
            it = link.startsWith("repair:");
            if (bf) link.remove("nominal");
            out << "<tr><td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            out << "><a href=\"" << link << "\">";
            out << i.key();
            out << "</a></td><td";
            if (bf) out << " style=\"font-weight: bold;\"";
            else if (it) out << " style=\"font-style: italic;\"";
            out << ">" << i.value().at(1) << "</td>";
            for (int n = 2; n < i.value().count(); ++n) {
                out << "<td";
                if (bf) out << " style=\"font-weight: bold;\"";
                else if (it) out << " style=\"font-style: italic;\"";
                out << ">";
                if (i.value().at(n).toDouble()) out << i.value().at(n).toDouble();
                out << "</td>";
            }
            out << "</tr>";
        }
    }
    out << "</table></td></tr>";
    out << "</table>";
    return dict_html.value(Navigation::ServiceCompany).arg(html);
}

QString MainWindow::viewRefrigerantManagement(int since)
{
    QString html; MTTextStream out(&html);
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"12\" style=\"font-size: medium;\">";
    out << tr("Refrigerant management") << "</th></tr>";
    out << "<tr><th rowspan=\"2\">" << tr("Date") << "</th>";
    out << "<th colspan=\"2\">" << QApplication::translate("RecordOfRefrigerantManagement", "Business partner") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Refrigerant") << "</th>";
    out << "<th colspan=\"2\">" << tr("Purchased") << "</th>";
    out << "<th colspan=\"2\">" << tr("Sold") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Reclaimed") << "</th>";
    out << "<th rowspan=\"2\">" << tr("Disposed of") << "</th>";
    out << "<th colspan=\"2\">" << tr("Leaked in store") << "</th>";
    out << "</tr><tr>";
    out << "<th>" << QApplication::translate("Customer", "Company") << "</th>";
    out << "<th>" << QApplication::translate("Customer", "ID") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "New") << "</th>";
    out << "<th>" << QApplication::translate("VariableNames", "Recovered") << "</th>";
    out << "</tr>";
    RecordOfRefrigerantManagement records("");
    if (!navigation->isFilterEmpty()) {
        records.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    QSqlQuery query = records.select("*", Qt::DescendingOrder);
    query.setForwardOnly(true);
    query.exec();
    QString date;
    while (query.next()) {
        date = QUERY("date").toString();
        if (since && date.left(4).toInt() < since) continue;
        out << "<tr onclick=\"window.location = 'recordofrefrigerantmanagement:" << date << "/modify'\" style=\"cursor: pointer;\">";
        out << "<td>" << date << "</td>";
        for (int n = 1; n < RecordOfRefrigerantManagement::attributes().count(); ++n) {
            out << "<td>" << MTVariant(QUERY(RecordOfRefrigerantManagement::attributes().key(n))) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::RefrigerantManagement).arg(html);
}

void MainWindow::writeCustomersTable(MTTextStream & out, const QString & customer_id)
{
    Customer all_customers(customer_id);
    if (customer_id.isEmpty() && !navigation->isFilterEmpty()) {
        all_customers.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps list(all_customers.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < Customer::attributes().count(); ++n) {
        thead.append("<th>" + Customer::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("<th>" + tr("Number of circuits") + "</th>");
    thead.append("<th>" + tr("Total number of inspections") + "</th>");
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium; background-color: floralwhite;\">";
    if (customer_id.isEmpty()) { out << tr("List of customers"); }
    else { out << "<a href=\"customer:" << customer_id << "/modify\">" << tr("Customer") << "</a>"; }
    out << "</th></tr>";
    out << thead;
    QString id; QString highlighted_id = selectedCustomer();
    for (int i = 0; i < list.count(); ++i) {
        id = list.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'customer:" << id << "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            out << " background-color: rgb(242, 248, 255);\">";
        } else { out << "\">"; }
        out << "<td>" << toolTipLink("customer", id.rightJustified(8, '0'), id) << "</td>";
        for (int n = 1; n < Customer::attributes().count(); ++n) {
            out << "<td>" << MTVariant(list.at(i).value(Customer::attributes().key(n)),
                                       (MTVariant::Type)dict_fieldtypes.value(Customer::attributes().key(n))) << "</td>";
        }
        out << "<td>" << Circuit(id, "").listAll("id").count() << "</td>";
        out << "<td>" << MTRecord("inspections", "date", "", MTDictionary("customer", id)).listAll("date").count() << "</td>";
        out << "</tr>";
    }
    out << "</table>";
}

void MainWindow::writeCircuitsTable(MTTextStream & out, const QString & customer_id, const QString & circuit_id, int cols_in_row)
{
    Circuit circuits_record(customer_id, circuit_id);
    if (circuit_id.isEmpty() && !navigation->isFilterEmpty()) {
        circuits_record.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps circuits(circuits_record.listAll());
    HTMLDiv div;
    HTMLTable * table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    HTMLTableRow * thead = new HTMLTableRow();
    int thead_colspan = 2;
    for (int n = 0; n < Circuit::numBasicAttributes(); ++n) {
        *(thead->addHeaderCell()) << Circuit::attributes().value(n).split("||").first();
        thead_colspan++;
    }
    *(thead->addHeaderCell()) << Circuit::attributes().value("refrigerant");
    *(thead->addHeaderCell()) << Circuit::attributes().value("oil") ;
    HTMLTableRow * _tr = table->addRow();
    HTMLTableCell * _td = _tr->addHeaderCell("colspan=\"" + QString::number(thead_colspan) + "\" style=\"font-size: medium; background-color: aliceblue;\"");
    if (circuit_id.isEmpty()) { *_td << tr("List of circuits"); }
    else { *_td << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << tr("Circuit") << "</a>"; }
    *table << thead;
    QString attr_value; QStringList dict_value; QString id;
    QString highlighted_id = selectedCircuit();
    bool show_disused = false;
    for (int i = 0; i < circuits.count(); ++i) {
        if (circuit_id.isEmpty() && circuits.at(i).value("disused").toInt()) { show_disused = true; continue; }
        id = circuits.at(i).value("id").toString();
        QString tr_attr = "onclick=\"window.location = 'customer:" + customer_id + "/circuit:" + id + "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            tr_attr.append(" background-color: rgb(242, 248, 255);");
        }
        tr_attr.append("\"");
        _tr = table->addRow(tr_attr);
        *(_tr->addCell()) << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id);
        for (int n = 1; n < Circuit::numBasicAttributes(); ++n) {
            dict_value = Circuit::attributes().value(n).split("||");
            attr_value = circuits.at(i).value(Circuit::attributes().key(n)).toString();
            if (Circuit::attributes().key(n) == "field") {
                if (attributeValues().contains("field::" + attr_value)) {
                    attr_value = attributeValues().value("field::" + attr_value);
                }
            } else if (Circuit::attributes().key(n) == "hermetic") {
                attr_value = attr_value.toInt() ? tr("Yes") : tr("No");
            }
            _td = _tr->addCell();
            *_td << escapeString(attr_value);
            if (dict_value.count() > 1) { *_td << "&nbsp;" << dict_value.last(); }
        }
        _td = _tr->addCell();
        *_td << QString::number(getCircuitRefrigerantAmount(customer_id, id, circuits.at(i).value("refrigerant_amount").toDouble())) << "&nbsp;" << QApplication::translate("Units", "kg");
        *_td << " " << circuits.at(i).value("refrigerant").toString();
        _td = _tr->addCell();
        *_td << circuits.at(i).value("oil_amount").toString() << "&nbsp;" << QApplication::translate("Units", "kg");
        *_td << " " << circuits.at(i).value("oil").toString().toUpper();
    }
    if (cols_in_row < 0) div << table;
    else div << table->customHtml(cols_in_row);
    if (show_disused) {
        div << "<br>";
        table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
        _tr = table->addRow();
        *(_tr->addHeaderCell("colspan=\"6\" style=\"font-size: medium;\"")) << tr("Disused circuits");
        _tr = table->addRow();
        *(_tr->addHeaderCell()) << Circuit::attributes().value("id");
        *(_tr->addHeaderCell()) << Circuit::attributes().value("manufacturer");
        *(_tr->addHeaderCell()) << Circuit::attributes().value("type");
        *(_tr->addHeaderCell()) << Circuit::attributes().value("sn");
        *(_tr->addHeaderCell()) << Circuit::attributes().value("commissioning");
        *(_tr->addHeaderCell()) << Circuit::attributes().value("decommissioning");
        for (int i = 0; i < circuits.count(); ++i) {
            if (!circuits.at(i).value("disused").toInt()) continue;
            id = circuits.at(i).value("id").toString();
            QString tr_attr = "onclick=\"window.location = 'customer:" + customer_id + "/circuit:" + id + "'\" style=\"cursor: pointer;";
            if (id == highlighted_id) {
                tr_attr.append(" background-color: rgb(242, 248, 255);");
            }
            tr_attr.append("\"");
            _tr = table->addRow(tr_attr);
            *(_tr->addCell()) << toolTipLink("customer/circuit", id.rightJustified(4, '0'), customer_id, id);
            *(_tr->addCell()) << circuits.at(i).value("manufacturer").toString();
            *(_tr->addCell()) << circuits.at(i).value("type").toString();
            *(_tr->addCell()) << circuits.at(i).value("sn").toString();
            *(_tr->addCell()) << circuits.at(i).value("commissioning").toString();
            *(_tr->addCell()) << circuits.at(i).value("decommissioning").toString();
        }
        div << table;
    }
    out << div.html();
}

QString MainWindow::viewAllCustomers()
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out);
    return dict_html.value(Navigation::ListOfCustomers).arg(html);
}

QString MainWindow::viewCustomer(const QString & customer_id)
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id);
    return dict_html.value(Navigation::ListOfCircuits).arg(html);
}

QString MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id, int year)
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    Inspection inspection_record(customer_id, circuit_id, "");
    if (!navigation->isFilterEmpty()) {
        inspection_record.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps inspections(inspection_record.listAll("date, nominal, repair, outside_interval, rmds, arno, inspector, operator, refr_add_am, refr_reco"));
    if (year) {
        for (int i = 0; i < inspections.count();) {
            if (inspections.at(i).value("date").toString().split(".").first().toInt() < year) {
                inspections.removeAt(i);
            } else { ++i; }
        }
    }
    Inspector inspectors_record("");
    MultiMapOfVariantMaps inspectors(inspectors_record.mapAll("id", "person"));
    Person operators_record(QString(), customer_id);
    MultiMapOfVariantMaps operators(operators_record.mapAll("id", "name"));
    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"9\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\">";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">";
    out << tr("Inspections and repairs") << "</a></th></tr>";
    out << "<tr><th>" << tr("Date") << "</th>";
    out << "<th>" << variableNames().value("refr_add_am") << "</th>";
    out << "<th>" << variableNames().value("refr_reco") << "</th>";
    out << "<th>" << variableNames().value("inspector") << "</th>";
    out << "<th>" << variableNames().value("operator") << "</th>";
    out << "<th>" << variableNames().value("rmds") << "</th>";
    out << "<th>" << variableNames().value("arno") << "</th></tr>";
    bool is_nominal, is_repair, is_outside_interval;
    QString id; QString highlighted_id = selectedInspection();
    for (int i = 0; i < inspections.count(); ++i) {
        id = inspections.at(i).value("date").toString();
        is_nominal = inspections.at(i).value("nominal").toInt();
        is_repair = inspections.at(i).value("repair").toInt();
        is_outside_interval = inspections.at(i).value("outside_interval").toInt();
        out << "<tr onclick=\"window.location = 'customer:" << customer_id << "/circuit:" << circuit_id;
        out << (is_repair ? "/repair:" : "/inspection:") << id << "'\" style=\"cursor: pointer;";
        if (id == highlighted_id) {
            out << " background-color: rgb(242, 248, 255);\">";
        } else { out << "\">"; }
        out << "<td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", id, customer_id, circuit_id, id);
        if (is_outside_interval) { out << "*"; }
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << "</td>";
        out << "<td>" << inspections.at(i).value("refr_add_am").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << inspections.at(i).value("refr_reco").toDouble() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
        out << "<td>" << escapeString(inspectors.value(inspections.at(i).value("inspector").toString()).value("person", inspections.at(i).value("inspector")).toString()) << "</td>";
        out << "<td>" << escapeString(operators.value(inspections.at(i).value("operator").toString()).value("name", inspections.at(i).value("operator")).toString()) << "</td>";
        if (!inspections.at(i).value("rmds").toString().isEmpty()) {
            out << "<td onmouseover=\"Tip('" << escapeString(escapeString(inspections.at(i).value("rmds").toString()), true, true) << "')\" onmouseout=\"UnTip()\">...</td>";
        } else {
            out << "<td></td>";
        }
        out << "<td>" << MTVariant(inspections.at(i).value("arno")) << "</td>";
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfInspections).arg(html);
}

QString MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; MTTextStream out(&html);
    writeCustomersTable(out, customer_id);
    out << "<br>";
    writeCircuitsTable(out, customer_id, circuit_id);
    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    QVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    bool repair = inspection.value("repair").toInt();
    Inspection nom_inspection_record(customer_id, circuit_id, "");
    nom_inspection_record.parents().insert("nominal", "1");
    QVariantMap nominal_ins = nom_inspection_record.list();

    out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\">";
    out << "<tr><th colspan=\"4\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\">";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id;
    out << (repair ? "/repair:" : "/inspection:") << inspection_date << "/modify\">";
    if (nominal) out << tr("Nominal inspection:"); else if (repair) out << tr("Repair:"); else out << tr("Inspection:");
    out << "&nbsp;" << inspection_date;
    out << "</a>";
    out << "</th></tr>";

    QStringList used_ids = listVariableIds(); // all = false
    Variables vars;
    int half = vars.count() % 2 ? (vars.count() / 2 + 1) : (vars.count() / 2);
    QVector<QString> rows(half); int row = 0;
    QString var_id, var_type; QString ins_value, nom_value;
    bool compare_nom = false, ok_eval = true, is_subvar = false;
    MTDictionary expression; double tolerance = 0.0;
    while (vars.next()) {
        compare_nom = false; ok_eval = true; is_subvar = false;
        if (vars.value("SUBVAR_ID").toString().isEmpty()) {
            var_id = vars.value("VAR_ID").toString();
            var_type = vars.value("VAR_TYPE").toString();
            tolerance = vars.value("VAR_TOLERANCE").toDouble();
        } else {
            var_id = vars.value("SUBVAR_ID").toString();
            var_type = vars.value("SUBVAR_TYPE").toString();
            tolerance = vars.value("SUBVAR_TOLERANCE").toDouble();
            is_subvar = true;
        }
        if (!nominal) {
            if (vars.value("VAR_COMPARE_NOM").toInt()) {
                compare_nom = true;
            } else if (vars.value("SUBVAR_COMPARE_NOM").toInt()) {
                compare_nom = true;
            }
        }
//*** Expressions and values ***
        expression.clear();
        if (!vars.value("VAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("VAR_VALUE").toString(), used_ids);
        } else if (!vars.value("SUBVAR_VALUE").toString().isEmpty()) {
            expression = parseExpression(vars.value("SUBVAR_VALUE").toString(), used_ids);
        }
        if (expression.count()) {
            ins_value = QString::number(evaluateExpression(inspection, expression, customer_id, circuit_id, &ok_eval));
            if (!ok_eval) continue;
            if (compare_nom) {
                nom_value = QString::number(evaluateExpression(nominal_ins, expression, customer_id, circuit_id, &ok_eval));
                if (!ok_eval) compare_nom = false;
            }
        } else {
            ins_value = inspection.value(var_id).toString();
            if (compare_nom) {
                nom_value = nominal_ins.value(var_id).toString();
                if (nom_value.isEmpty()) compare_nom = false;
            }
        }
        //if (ins_value.isEmpty()) continue;
        if (var_id == "inspector" && !ins_value.isEmpty()) {
            Inspector inspector(ins_value);
            ins_value = inspector.stringValue("person", ins_value);
        } else if (var_id == "operator" && !ins_value.isEmpty()) {
            if (ins_value.toInt())
                ins_value = Person(ins_value, customer_id).stringValue("name", ins_value);
        } else if (var_type == "bool") {
            ins_value = ins_value.toInt() ? tr("Yes") : tr("No");
        }
        if (row == half) { row = 0; }
        rows[row].append("<td style=\"text-align: right; width: 25%; border-style: none;\">");
        if (!is_subvar) {
            rows[row].append(vars.value("VAR_NAME").toString() + ":");
        } else {
            rows[row].append(vars.value("VAR_NAME").toString() + ": " + vars.value("SUBVAR_NAME").toString() + ":");
        }
        rows[row].append("</td><td style=\"width: 25%; border-style: none;\"><table cellpadding=\"0\" cellspacing=\"0\" class=\"no_border\" style=\"width: 0%\"><tr><td ");
        if ((is_subvar && vars.value("SUBVAR_TYPE").toString() != "text") || (!is_subvar && vars.value("VAR_TYPE").toString() != "text"))
            rows[row].append("align=\"right\"");
        rows[row].append(" valign=\"center\" style=\"border-style: none;\">");
        if (compare_nom) {
            rows[row].append(compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance).arg(ins_value));
        } else {
            rows[row].append(escapeString(ins_value));
        }
        rows[row].append("</td><td valign=\"center\" style=\"border-style: none;\">&nbsp;");
        if (!vars.value("VAR_UNIT").toString().isEmpty()) {
            rows[row].append(vars.value("VAR_UNIT").toString());
        } else if (!vars.value("SUBVAR_UNIT").toString().isEmpty()) {
            rows[row].append(vars.value("SUBVAR_UNIT").toString());
        }
        rows[row].append("</td></tr></table></td>");
        row++;
    }
    for (int i = 0; i < rows.count(); ++i) {
        out << "<tr>" << rows.at(i) << "</tr>";
    }
    out << "</table>";
//*** Warnings ***
    Warnings warnings(db, true, customer_id, circuit_id);
    QStringList warnings_list = listWarnings(warnings, customer_id, circuit_id, nominal_ins, inspection);
    if (warnings_list.count()) {
        out << "<br><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><th style=\"font-size: medium;\">" << tr("Warnings") << "</th></tr>";
        out << "<tr><td>" << warnings_list.join(", ") << "</td></tr></table>";
    }
    return dict_html.value(Navigation::Inspection).arg(html);
}

QString MainWindow::viewTable(const QString & customer_id, const QString & circuit_id, const QString & table_id, int year)
{
    QString html; MTTextStream out(&html);

    VariableEvaluation::EvaluationContext var_evaluation(customer_id, circuit_id);
    VariableEvaluation::Variable * variable = NULL, * subvariable = NULL;

    Table table_record(table_id);
    QVariantMap table = table_record.list();
    QStringList table_vars = table.value("variables").toString().split(";", QString::SkipEmptyParts);

    Inspection inspection_record(customer_id, circuit_id, "");
    ListOfVariantMaps inspections(inspection_record.listAll());
    QString last_inspection_date, last_entry_date, date;
    for (int i = 0; i < inspections.count(); ++i) {
        date = inspections.at(i).value("date").toString();
        if (date > last_entry_date) {
            last_entry_date = date;
            if (!inspections.at(i).value("repair").toInt()) {
                last_inspection_date = date;
            }
        }
        if ((!table.value("highlight_nominal").toInt() || !inspections.at(i).value("nominal").toInt())
            && date.split(".").first().toInt() < year) {
            inspections.removeAt(i);
            i--;
        }
    }

    for (int i = 0; i < inspections.count(); ++i) {
        if (inspections.at(i).value("nominal").toInt()) {
            var_evaluation.setNominalInspection(inspections[i]);
            break;
        }
    }

//*** Top tables ***
    Customer customer(customer_id);
    QVariantMap customer_info = customer.list("company, contact_person, address, mail, phone");
    Circuit circuit(customer_id, circuit_id);
    QVariantMap circuit_info = circuit.list("name, manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, runtime, utilisation");
    out << "<table><tr><th>" << QApplication::translate("Customer", "ID");
    out << "</th><th>" << QApplication::translate("Customer", "Company");
    out << "</th><th>" << QApplication::translate("Customer", "Contact person");
    out << "</th><th>" << QApplication::translate("Customer", "Address");
    out << "</th><th>" << QApplication::translate("Customer", "E-mail");
    out << "</th><th>" << QApplication::translate("Customer", "Phone");
    out << "</th></tr><tr>";
    out << "<td>" << toolTipLink("customer", customer_id.rightJustified(8, '0'), customer_id) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("company")) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("contact_person")) << "</td>";
    out << "<td>" << MTAddress(customer_info.value("address").toString()).toHtml() << "</td>";
    out << "<td>" << MTVariant(customer_info.value("mail")) << "</td>";
    out << "<td>" << MTVariant(customer_info.value("phone")) << "</td>";
    out << "</tr></table><br />";
    out << "<table><tr><th>" << QApplication::translate("Circuit", "ID");
    out << "</th><th>" << QApplication::translate("Circuit", "Name");
    out << "</th><th>" << QApplication::translate("Circuit", "Manufacturer");
    out << "</th><th>" << QApplication::translate("Circuit", "Type");
    out << "</th><th>" << QApplication::translate("Circuit", "Year of purchase");
    out << "</th><th>" << QApplication::translate("Circuit", "Date of commissioning");
    out << "</th><th>" << QApplication::translate("Circuit", "Refrigerant");
    out << "</th><th>" << QApplication::translate("Circuit", "Amount of refrigerant");
    out << "</th><th>" << QApplication::translate("Circuit", "Oil");
    out << "</th><th>" << QApplication::translate("Circuit", "Amount of oil");
    out << "</th></tr><tr>";
    out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(4, '0'), customer_id, circuit_id) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("name")) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("manufacturer")) << "</td>";
    out << "<td>" << MTVariant(circuit_info.value("type")) << "</td>";
    out << "<td>" << circuit_info.value("year").toString() << "</td>";
    out << "<td>" << circuit_info.value("commissioning").toString() << "</td>";
    out << "<td>" << circuit_info.value("refrigerant").toString() << "</td>";
    out << "<td>" << getCircuitRefrigerantAmount(customer_id, circuit_id, circuit_info.value("refrigerant_amount").toDouble()) << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
    out << "<td>";
    if (attributeValues().contains("oil::" + circuit_info.value("oil").toString())) {
        out << attributeValues().value("oil::" + circuit_info.value("oil").toString());
    }
    out << "</td>";
    out << "<td>" << circuit_info.value("oil_amount").toString() << "&nbsp;" << QApplication::translate("Units", "kg") << "</td>";
    out << "</td></tr></table><br />";

    out << "<table>";

//*** Head ***
    out << "<thead>";
    out << "<tr class=\"border_top\">";
    out << "<th rowspan=\"3\">" << tr("Date") << "</th>";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        int subvar_count = variable->countSubvariables();
        out << "<th colspan=\"" << subvar_count << "\" rowspan=\"";
        if (subvar_count > 0) out << 1;
        else if (variable->unit() != "") out << 2;
        else out << 3;
        out << "\" class=\"" << variable->colBg();
        out << "\">" << variable->name() << "</th>";
    }
    out << "</tr><tr>";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        if (variable->countSubvariables() > 0) {
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            for (int n = 0; n < subvariables.count(); ++n) {
                subvariable = subvariables.at(n);
                out << "<th rowspan=\"";
                if (subvariable->unit().isEmpty()) {
                    out << 2;
                } else out << 1;
                out << "\" class=\"" << variable->colBg();
                out << "\">" << subvariable->name() << "</th>";
            }
        }
    }
    out << "</tr><tr class=\"border_bottom\">";
    for (int i = 0; i < table_vars.count(); ++i) {
        variable = var_evaluation.variable(table_vars.at(i));
        if (!variable) continue;
        if (variable->countSubvariables() > 0) {
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            QString unit;
            for (int n = 0; n < subvariables.count(); ++n) {
                unit = subvariables.at(n)->unit();
                if (!unit.isEmpty()) {
                    out << "<th class=\"" << variable->colBg();
                    out << "\">" << unit << "</th>";
                }
            }
        } else {
            if (!variable->unit().isEmpty()) {
                out << "<th class=\"" << variable->colBg();
                out << "\">" << variable->unit() << "</th>";
            }
        }
    }
    out << "</tr></thead>";

//*** Body ***
    bool is_nominal, is_repair, is_outside_interval; QString inspection_date;
    out << "<tbody>";
    for (int i = 0; i < inspections.count(); ++i) {
        is_nominal = inspections.at(i).value("nominal").toInt();
        is_repair = inspections.at(i).value("repair").toInt();
        is_outside_interval = inspections.at(i).value("outside_interval").toInt();
        inspection_date = inspections.at(i).value("date").toString();
        out << "<tr class=\"";
        if (is_nominal && table.value("highlight_nominal").toInt()) {
            out << "nominal";
        }
        out << "\"><td>";
        if (is_nominal) { out << "<b>"; }
        else if (is_repair) { out << "<i>"; }
        out << toolTipLink(is_repair ? "customer/circuit/repair" : "customer/circuit/inspection", inspection_date, customer_id, circuit_id, inspection_date);
        if (is_outside_interval) { out << "*"; }
        if (is_nominal) { out << "</b>"; }
        else if (is_repair) { out << "</i>"; }
        out << "</td>";
        for (int n = 0; n < table_vars.count(); ++n) {
            variable = var_evaluation.variable(table_vars.at(n));
            if (!variable) continue;
            bool compare_nom = false; int rowspan = 1; QString ins_value; QString nom_value;
            QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
            if (subvariables.count() > 0) {
                for (int s = 0; s < subvariables.count(); ++s) {
                    subvariable = subvariables.at(s);
                    compare_nom = subvariable->compareNom() > 0;
                    if (subvariable->value().contains("sum")) {
                        QString i_year = inspection_date.split(".").first();
                        if (is_nominal) rowspan = 1;
                        else if (i > 0 && !inspections.at(i-1).value("nominal").toInt() && inspections.at(i-1).value("date").toString().split(".").first() == i_year) continue;
                        else {
                            int in = i;
                            for (; in < inspections.count(); ++in) {
                                if (inspections.at(in).value("nominal").toInt()) in--;
                                if (inspections.at(in).value("date").toString().split(".").first() != i_year) {
                                    break;
                                }
                            }
                            rowspan = in - i;
                        }
                    } else rowspan = 1;
                    ins_value = var_evaluation.evaluate(subvariable, inspections[i], nom_value);
                    compare_nom = !nom_value.isEmpty();
                    writeTableVarCell(out, subvariable->type(), ins_value, nom_value, variable->colBg(), compare_nom, rowspan, subvariable->tolerance());
                }
            } else {
                compare_nom = variable->compareNom() > 0;
                if (variable->value().contains("sum")) {
                    QString i_year = inspection_date.split(".").first();
                    if (is_nominal) rowspan = 1;
                    else if (i > 0 && !inspections.at(i-1).value("nominal").toInt() && inspections.at(i-1).value("date").toString().split(".").first() == i_year) continue;
                    else {
                        int in = i;
                        for (; in < inspections.count(); ++in) {
                            if (inspections.at(in).value("nominal").toInt()) in--;
                            if (inspections.at(in).value("date").toString().split(".").first() != i_year) {
                                break;
                            }
                        }
                        rowspan = in - i;
                    }
                } else rowspan = 1;
                ins_value = var_evaluation.evaluate(variable, inspections[i], nom_value);
                compare_nom = !nom_value.isEmpty();
                writeTableVarCell(out, variable->type(), ins_value, nom_value, variable->colBg(), compare_nom, rowspan, variable->tolerance());
            }
        }
        out << "</tr>";
    }
    out << "</tbody>";

//*** Foot ***
    out << "<tfoot>";
    MTDictionary foot_functions;
    foot_functions.insert("sum", tr("Sum"));
    foot_functions.insert("avg", tr("Average"));
    for (int f = 0; f < foot_functions.count(); ++f) {
        if (!table.value(foot_functions.key(f)).toString().isEmpty()) {
            out << "<tr class=\"border_top border_bottom\">";
            out << "<th>" << foot_functions.value(f) << "</th>";
            QStringList f_vars = table.value(foot_functions.key(f)).toString().split(";", QString::SkipEmptyParts);
            for (int i = 0; i < table_vars.count(); ++i) {
                variable = var_evaluation.variable(table_vars.at(i));
                if (!variable) continue;
                bool is_in_foot = f_vars.contains(table_vars.at(i));
                if (variable->countSubvariables() > 0) {
                    QList<VariableEvaluation::Variable *> subvariables = variable->subvariables();
                    for (int s = 0; s < subvariables.count(); ++s) {
                        subvariable = subvariables.at(s);
                        is_in_foot = f_vars.contains(table_vars.at(i));
                        if (subvariable->type() != "float" && subvariable->type() != "int") is_in_foot = false;
                        out << "<td class=\"" << variable->colBg() << "\">";
                        bool value_contains_sum = subvariable->value().contains(QRegExp("\\bsum\\b"));
                        bool skip_nominal = subvariable->value().startsWith("(1-nominal)*") &&
                                            inspections.count() && inspections.first().value("nominal").toInt();
                        if (is_in_foot) {
                            double value = 0.0; int num_ins = 0;
                            if (subvariable->value().isEmpty()) {
                                num_ins = inspections.count();
                                for (int ins = 0; ins < inspections.count(); ++ins) {
                                    value += inspections.at(ins).value(subvariable->id()).toDouble();
                                }
                            } else {
                                MTDictionary expression = parseExpression(subvariable->value(), var_evaluation.usedIds());
                                for (int ins = skip_nominal; ins < inspections.count(); ++ins) {
                                    if (value_contains_sum && ins > 0 && !inspections.at(ins-1).value("nominal").toInt() &&
                                        inspections.at(ins - 1).value("date").toString().split(".").first() == inspections.at(ins).value("date").toString().split(".").first())
                                        continue;
                                    num_ins++;
                                    value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                                }
                            }
                            if (num_ins && (foot_functions.key(f) == "avg" || subvariable->unit() == "%"))
                                { value /= (double)num_ins; }
                            out << value;
                        }
                        out << "</td>";
                    }
                } else {
                    if (variable->type() != "float" && variable->type() != "int") is_in_foot = false;
                    out << "<td class=\"" << variable->colBg() << "\">";
                    bool value_contains_sum = variable->value().contains(QRegExp("\\bsum\\b"));
                    bool skip_nominal = variable->value().startsWith("(1-nominal)*") &&
                                        inspections.count() && inspections.first().value("nominal").toInt();
                    if (is_in_foot) {
                        double value = 0.0; int num_ins = 0;
                        if (variable->value().isEmpty()) {
                            num_ins = inspections.count();
                            for (int ins = 0; ins < inspections.count(); ++ins) {
                                value += inspections.at(ins).value(table_vars.at(i)).toDouble();
                            }
                        } else {
                            MTDictionary expression = parseExpression(variable->value(), var_evaluation.usedIds());
                            for (int ins = skip_nominal; ins < inspections.count(); ++ins) {
                                if (value_contains_sum && ins > 0 && !inspections.at(ins - 1).value("nominal").toInt() &&
                                    inspections.at(ins - 1).value("date").toString().split(".").first() == inspections.at(ins).value("date").toString().split(".").first())
                                    continue;
                                num_ins++;
                                value += evaluateExpression(inspections[ins], expression, customer_id, circuit_id);
                            }
                        }
                        if (num_ins && (foot_functions.key(f) == "avg" || variable->unit() == "%"))
                            { value /= (double)num_ins; }
                        out << value;
                    }
                    out << "</td>";
                }
            }
            out << "</tr>";
        }
    }
    out << "</tfoot>";
    out << "</table>";

//*** Warnings ***
    Warnings warnings(db, true, customer_id, circuit_id);
    QString warnings_html;
    QStringList last_warnings_list, warnings_list, backup_warnings;
    for (int i = 0; i < inspections.count(); ++i) {
        inspection_date = inspections.at(i).value("date").toString();
        warnings_list = listWarnings(warnings, customer_id, circuit_id, var_evaluation.nominalInspection(), inspections[i]);
        backup_warnings = warnings_list;
        for (int n = 0; n < warnings_list.count(); ++n) {
            if (last_warnings_list.contains(warnings_list.at(n))) {
                warnings_list[n].prepend("<span style=\"color: red;\"><b>");
                warnings_list[n].append("</b></span>");
            }
        }
        if (warnings_list.count()) {
            warnings_html.append("<tr><td><a href=\"customer:" + customer_id + "/circuit:" + circuit_id);
            warnings_html.append(inspections.at(i).value("repair").toInt() ? "/repair:" : "/inspection:");
            warnings_html.append(inspection_date + "\">");
            warnings_html.append(inspection_date + "</a>");
            warnings_html.append("</td><td>");
            warnings_html.append(warnings_list.join(", "));
            warnings_html.append("</td></tr>");
        }
        if (!inspections.at(i).value("outside_interval").toInt())
            last_warnings_list.clear();
        last_warnings_list << backup_warnings;
    }
    QStringList delayed_warnings = listDelayedWarnings(warnings, customer_id, circuit_id, var_evaluation.nominalInspection(), last_entry_date, last_inspection_date);
    if (delayed_warnings.count()) {
        warnings_html.append("<tr><td colspan=\"2\"><b>");
        warnings_html.append(delayed_warnings.join(", "));
        warnings_html.append("</b></td></tr>");
    }
    if (!warnings_html.isEmpty()) {
        out << "<br /><table>";
        out << "<tr><th width=\"20%\">" << tr("Date") << "</th><th>" << tr("Warnings") << "</th></tr>";
        out << warnings_html;
        out << "</table>";
    }
    QString colours = !actionPrinter_friendly_version->isChecked() ? "<link href=\"colours.css\" rel=\"stylesheet\" type=\"text/css\" />" : "";
    return dict_html.value(Navigation::TableOfInspections).arg(colours).arg(html);
}

void MainWindow::writeTableVarCell(MTTextStream & out, const QString & var_type, const QString & ins_value, const QString & nom_value, const QString & bg_class, bool compare_nom, int rowspan, double tolerance)
{
    out << "<td class=\"" << bg_class << "\" rowspan=\"" << rowspan << "\"";
    if (var_type == "text" && !ins_value.isEmpty()) {
        out << "onmouseover=\"Tip('" << escapeString(escapeString(ins_value), true, true) << "')\" onmouseout=\"UnTip()\"";
    }
    out << ">";
    if (var_type == "text") {
        if (!ins_value.isEmpty()) out << "...";
    } else if (var_type == "string") {
        out << escapeString(ins_value);
    } else if (var_type == "bool") {
        out << (ins_value.toInt() ? tr("Yes") : tr("No"));
    } else if (compare_nom && !nom_value.isEmpty()) {
        out << compareValues(nom_value.toDouble(), ins_value.toDouble(), tolerance, bg_class).arg(ins_value);
    } else {
        out << ins_value;
    }
    out << "</td>";
}

#define checkWarningConditions(inspection) \
        num_conditions = warnings.warningConditionFunctionCount(id);\
        for (int i = 0; i < num_conditions; ++i) {\
            ok = true;\
            ins_value = evaluateExpression(inspection, warnings.warningConditionValueIns(id, i), customer_id, circuit_id, &ok);\
            if (!ok) { show_warning = false; break; }\
            nom_value = evaluateExpression(nominal_ins, warnings.warningConditionValueNom(id, i), customer_id, circuit_id, &ok);\
            if (!ok) { show_warning = false; break; }\
            function = warnings.warningConditionFunction(id, i);\
            if (function == "=" && ins_value == nom_value) {}\
            else if (function == "!=" && ins_value != nom_value) {}\
            else if (function == ">" && ins_value > nom_value) {}\
            else if (function == ">=" && ins_value >= nom_value) {}\
            else if (function == "<" && ins_value < nom_value) {}\
            else if (function == "<=" && ins_value <= nom_value) {}\
            else { show_warning = false; break; }\
        }

QStringList MainWindow::listWarnings(Warnings & warnings, const QString & customer_id, const QString & circuit_id, QVariantMap & nominal_ins, QVariantMap & inspection)
{
    QStringList warnings_list;
    Circuit circuit(customer_id, circuit_id);
    QVariantMap circuit_attributes = circuit.list();
    bool show_warning, ok; QString function;
    int id, num_conditions; double ins_value, nom_value;
    while (warnings.next()) {
        show_warning = true;
        if (warnings.value("delay").toInt()) { continue; }
        id = warnings.value("id").toInt();
        checkWarningConditions(inspection)
        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

QStringList MainWindow::listDelayedWarnings(Warnings & warnings, const QString & customer_id, const QString & circuit_id, QVariantMap & nominal_ins, const QString & last_entry_date, const QString & last_inspection_date, int * delay_out)
{
    QStringList warnings_list;
    Circuit circuit(customer_id, circuit_id);
    QVariantMap circuit_attributes = circuit.list();
    Inspection last_entry_record(customer_id, circuit_id, last_entry_date);
    QVariantMap last_entry = last_entry_record.list();
    QVariantMap last_inspection;
    if (last_inspection_date == last_entry_date) {
        last_inspection = last_entry;
    } else {
        Inspection last_inspection_record(customer_id, circuit_id, last_inspection_date);
        last_inspection = last_inspection_record.list();
    }
    QVariantMap * entry;
    bool show_warning, ok; QString function;
    int id, delay, interval, num_conditions; double ins_value, nom_value;
    while (warnings.next()) {
        show_warning = true;
        delay = warnings.value("delay").toInt();
        if (!delay) { continue; }
        id = warnings.value("id").toInt();
        if (id >= 1200 && id < 1300) {
            entry = &last_inspection;
            interval = circuit_attributes.value("inspection_interval").toInt();
            if (interval) { delay = interval; }
            if (delay_out) { *delay_out = delay; }
        } else { entry = &last_entry; }
        if (QDate::fromString(entry->value("date").toString().split("-").first(), "yyyy.MM.dd").daysTo(QDate::currentDate()) < delay) {
            continue;
        }
        if (!nominal_ins.isEmpty()) {
            checkWarningConditions(*entry)
        }
        if (show_warning) {
            warnings_list << warnings.value("name").toString();
        }
    }
    return warnings_list;
}

QString MainWindow::viewRepairs(const QString & highlighted_id, int year, const QString & customer_id)
{
    QString html; MTTextStream out(&html);
    MTDictionary parent;
    if (!customer_id.isEmpty()) {
        parent.insert("parent", customer_id);
        writeCustomersTable(out, customer_id);
        out << "<br>";
    }
    MTRecord repairs_record("repairs", "date", "", parent);
    if (!navigation->isFilterEmpty()) {
        repairs_record.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    QSqlQuery repairs = repairs_record.select();
    repairs.setForwardOnly(true);
    repairs.exec();
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"12\" style=\"font-size: medium;\">" << tr("List of repairs") << "</th></tr><tr>";
    for (int n = 0; n < Repair::attributes().count(); ++n) {
        out << "<th>" << Repair::attributes().value(n) << "</th>";
    }
    out << "</tr>";
    MultiMapOfVariantMaps inspectors(Inspector("").mapAll("id", "person"));
    QString id, attr_value;
    while (repairs.next()) {
        id = QUERY_VALUE(repairs, "date").toString();
        if (id.split(".").first().toInt() < year) continue;
        out << "<tr onclick=\"window.location = 'repair:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td>" << id << "</td>";
        for (int n = 1; n < Repair::attributes().count(); ++n) {
            attr_value = QUERY_VALUE(repairs, Repair::attributes().key(n)).toString();
            out << "<td>";
            if (Repair::attributes().key(n) == "field") {
                if (attributeValues().contains("field::" + attr_value)) {
                    attr_value = attributeValues().value("field::" + attr_value);
                }
            } else if (Repair::attributes().key(n) == "repairman") {
                attr_value = inspectors.value(attr_value).value("person", attr_value).toString();
            }
            out << escapeString(attr_value) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfRepairs).arg(html);
}

QString MainWindow::viewAllInspectors(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    Inspector inspectors_record("");
    if (!navigation->isFilterEmpty()) {
        inspectors_record.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps inspectors(inspectors_record.listAll());
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < Inspector::attributes().count(); ++n) {
        thead.append("<th>" + Inspector::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("<th>" + tr("Number of inspections") + "</th>");
    thead.append("<th>" + tr("Number of repairs") + "</th>");
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("List of inspectors") << "</th></tr>";
    out << thead;
    QString id;
    for (int i = 0; i < inspectors.count(); ++i) {
        id = inspectors.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'inspector:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id.rightJustified(4, '0') << "</a></td>";
        for (int n = 1; n < Inspector::attributes().count(); ++n) {
            out << "<td>" << escapeString(inspectors.at(i).value(Inspector::attributes().key(n)).toString()) << "</td>";
        }
        out << "<td>" << MTRecord("inspections", "date", "", MTDictionary("inspector", id)).listAll("date").count() << "</td>";
        out << "<td>" << MTRecord("repairs", "date", "", MTDictionary("repairman", id)).listAll("date").count() << "</td>";
        out << "</tr>";
    }
    return dict_html.value(Navigation::ListOfInspectors).arg(html);
}

QString MainWindow::viewOperatorReport(const QString & customer_id, int year)
{
    if (year == 0)
        year = QDate::currentDate().year() - 1;
    QString html; MTTextStream out(&html);
    Customer customer(customer_id);
    QVariantMap attributes = customer.list("company, address");
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><th style=\"font-size: medium; background-color: floralwhite;\">";
    out << tr("Operator report: %1").arg(year) << "</th></tr></table><br>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tr><th colspan=\"3\">" << tr("Owner information") << "</th></tr><tr>";
    out << "<th>" << Customer::attributes().value("id") << "</th>";
    out << "<th>" << Customer::attributes().value("company") << "</th>";
    out << "<th>" << Customer::attributes().value("address") << "</th>";
    out << "</tr><tr>";
    out << "<td>" << toolTipLink("customer", customer_id.rightJustified(8, '0'), customer_id) << "</td>";
    out << "<td>" << MTVariant(attributes.value("company")) << "</td>";
    out << "<td>" << MTVariant(attributes.value("address"), MTVariant::Address) << "</td>";
    out << "</tr><tr><th colspan=\"3\">" << tr("Operator information") << "</th></tr><tr>";
    out << "<th>" << Customer::attributes().value("id") << "</th>";
    out << "<th>" << Customer::attributes().value("company") << "</th>";
    out << "<th>" << Customer::attributes().value("address") << "</th>";
    out << "</tr><tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr></table><br>";
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    out << "<tr><th colspan=\"8\" style=\"font-size: medium; background-color: aliceblue;\">";
    out << tr("Circuit information", "Operator report") << "</th></tr><tr>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "ID") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Field of application") << "</th>";
    out << "<th colspan=\"4\">" << QApplication::translate("Circuit", "Amount of refrigerant") << "</th>";
    out << "<th rowspan=\"2\">" << QApplication::translate("Circuit", "Place of operation") << "</th>";
    out << "</tr><tr>";
    out << "<th>" << tr("At the beginning of this year") << "</th>";
    out << "<th>" << tr("Added") << "</th>";
    out << "<th>" << tr("Recovered") << "</th>";
    out << "<th>" << tr("At the end of this year") << "</th>";
    out << "</tr>";
    MTRecord inspections("inspections", "date", "", MTDictionary("customer", customer_id));
    inspections.parents().insert("nominal", "0");
    inspections.addFilter("date", QString("%1%").arg(year));
    MTDictionary nominal_inpection_parents("customer", customer_id);
    nominal_inpection_parents.insert("nominal", "1");
    QVariantMap sums; QVariantMap nominal_inspection;
    int nominal_inspection_year, commissioning_year, decommissioning_year;
    double refrigerant_amount, refrigerant_amount_begin, refrigerant_amount_end;
    QString circuit_id;
    QSqlQuery circuits = Circuit(customer_id, "").select("id, refrigerant, refrigerant_amount, field, operation, disused, commissioning, decommissioning");
    circuits.setForwardOnly(true);
    circuits.exec();
    while (circuits.next()) {
        circuit_id = QUERY_VALUE(circuits, "id").toString();

        inspections.parents().insert("circuit", circuit_id);
        sums = inspections.sumAll("refr_add_am, refr_reco");

        commissioning_year = QUERY_VALUE(circuits, "commissioning").toString().left(4).toInt();
        if (commissioning_year > year)
            continue;
        decommissioning_year = QUERY_VALUE(circuits, "decommissioning").toString().left(4).toInt();
        if (QUERY_VALUE(circuits, "disused").toInt() == 0)
            decommissioning_year = 9999;
        else if (decommissioning_year == 0)
            decommissioning_year = QDate::currentDate().year();
        if (decommissioning_year < year)
            continue;
        refrigerant_amount = QUERY_VALUE(circuits, "refrigerant_amount").toDouble();
        nominal_inpection_parents.insert("circuit", circuit_id);
        nominal_inspection = MTRecord("inspections", "date", "", nominal_inpection_parents).list("date, refr_add_am");
        nominal_inspection_year = nominal_inspection.value("date", "9999").toString().left(4).toInt();
        refrigerant_amount_begin = 0.0;
        refrigerant_amount_end = refrigerant_amount;
        if (commissioning_year < year)
            refrigerant_amount_begin += refrigerant_amount;
        if (nominal_inspection_year < year)
            refrigerant_amount_begin += nominal_inspection.value("refr_add_am", 0.0).toDouble();
        if (nominal_inspection_year <= year)
            refrigerant_amount_end += nominal_inspection.value("refr_add_am", 0.0).toDouble();
        if (decommissioning_year == year)
            refrigerant_amount_end = 0.0;

        out << "<tr onclick=\"window.location = 'customer:" << customer_id << "/circuit:" << circuit_id << "'\" style=\"cursor: pointer;\">";
        out << "<td>" << toolTipLink("customer/circuit", circuit_id.rightJustified(4, '0'), customer_id, circuit_id) << "</td>";
        out << "<td>" << QUERY_VALUE(circuits, "refrigerant").toString() << "</td>";
        out << "<td>" << fieldsOfApplication().firstKey(QUERY_VALUE(circuits, "field").toString()) << "</td>";
        out << "<td>" << refrigerant_amount_begin << "</td>";
        out << "<td>" << sums.value("refr_add_am").toDouble() << "</td>";
        out << "<td>" << sums.value("refr_reco").toDouble() << "</td>";
        out << "<td>" << refrigerant_amount_end << "</td>";
        out << "<td>" << MTVariant(QUERY_VALUE(circuits, "operation")) << "</td>";
        out << "</tr>";
    }
    out << "</table>";
    if (isInspectorSelected()) {
        attributes = Inspector(selectedInspector()).list("person, mail, phone");
        out << "<br><table><tr><td>";
        out << tr("Person responsible:", "Operator report") << " " << MTVariant(attributes.value("person"));
        out << "<br>" << tr("Phone:") << " " << MTVariant(attributes.value("phone"));
        out << "<br>" << tr("E-mail:") << " " << MTVariant(attributes.value("mail"));
        out << "</td></tr></table>";
    }
    return dict_html.value(Navigation::OperatorReport).arg(html);
}

QString MainWindow::viewLeakagesByApplication()
{
    QString html; MTTextStream out(&html);
    QMap<QString, QVector<double> > map;
    QStringList keys;
    const int VECTOR_SIZE = 3;
    map["All::All"].resize(VECTOR_SIZE);
    QSqlQuery inspections("SELECT circuits.refrigerant, circuits.field, inspections.refr_add_am FROM inspections LEFT JOIN circuits ON inspections.circuit = circuits.id AND inspections.customer = circuits.parent WHERE (inspections.nominal <> 1 OR inspections.nominal IS NULL)");
    while (inspections.next()) {
        keys.clear();
        keys << inspections.value(0).toString() + "::" + attributeValues().value(attributeValues().indexOfKey("field::" + inspections.value(1).toString()));
        keys << inspections.value(0).toString() + "::All";
        keys << "All::" + attributeValues().value(attributeValues().indexOfKey("field::" + inspections.value(1).toString()));
        for (int i = 0; i < keys.count(); ++i) {
            if (!map[keys.at(i)].size()) { map[keys.at(i)].resize(VECTOR_SIZE); }
            map[keys.at(i)][0] += inspections.value(2).toDouble() + inspections.value(3).toDouble();
        }
        map["All::All"][0] += inspections.value(2).toDouble() + inspections.value(3).toDouble();
    }
    QSqlQuery circuits("SELECT parent, id, refrigerant, field, refrigerant_amount FROM circuits");
    double refrigerant_amount;
    while (circuits.next()) {
        refrigerant_amount = getCircuitRefrigerantAmount(circuits.value(0).toString(), circuits.value(1).toString(), circuits.value(4).toDouble());
        keys.clear();
        keys << circuits.value(2).toString() + "::" + attributeValues().value(attributeValues().indexOfKey("field::" + circuits.value(3).toString()));
        keys << circuits.value(2).toString() + "::All";
        keys << "All::" + attributeValues().value(attributeValues().indexOfKey("field::" + circuits.value(3).toString()));
        for (int i = 0; i < keys.count(); ++i) {
            if (!map[keys.at(i)].size()) { map[keys.at(i)].resize(VECTOR_SIZE); }
            map[keys.at(i)][1] += refrigerant_amount;
        }
        map["All::All"][1] += refrigerant_amount;
    }
    QMutableMapIterator<QString, QVector<double> > iterator(map);
    MTDictionary used_refrigerants;
    while (iterator.hasNext()) { iterator.next();
        if (iterator.value()[1] != 0.0) {
            iterator.value()[2] = 100.0 * iterator.value()[0] / iterator.value()[1];
        }
        QString refrigerant = iterator.key().split("::").first();
        QString field = iterator.key().split("::").last();
        if (refrigerant == field) continue;
        if (refrigerant != "" && refrigerant != "All") {
            used_refrigerants.insert(refrigerant, attributeValues().value(attributeValues().indexOfKey("refrigerant::" + refrigerant)));
        }
    }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th style=\"font-size: medium;\">" << tr("Leakages by application") << "</th></tr></table><br>";
    QStringList tables;
    tables << variableNames().value("refr_add_am") << tr("Amount of refrigerant in circuits") << tr("Percentage of leakage by application");
    for (int t = 0; t < tables.count(); ++t) {
        out << "<table><thead><tr><th rowspan=\"2\" width=\"15%\">" << tables.at(t) << "</th>";
        out << "<th colspan=\"5\">" << tr("Fields") << "</th></tr>";
        out << "<tr><th>" << tr("All") << "</th>";
        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            out << "<th>" << attributeValues().value(n) << "</th>";
        }
        out << "</tr></thead>";
        out << "<tr><th>" << tr("All") << "</th>";
        out << "<td>" << map["All::All"].at(t) << "</td>";
        for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
            out << "<td>" << map.value("All::" + attributeValues().value(n), QVector<double>(VECTOR_SIZE)).at(t) << "</td>";
        }
        out << "</tr>";
        for (int i = 0; i < used_refrigerants.count(); ++i) {
            out << "<tr><th>" << used_refrigerants.value(i) << "</th>";
            out << "<td>" << map[used_refrigerants.key(i) + "::All"].at(t) << "</td>";
            for (int n = attributeValues().indexOfKey("field") + 1; n < attributeValues().count() && attributeValues().key(n).startsWith("field::"); ++n) {
                out << "<td>" << map.value(used_refrigerants.key(i) + "::" + attributeValues().value(n), QVector<double>(VECTOR_SIZE)).at(t) << "</td>";
            }
            out << "</tr>";
        }
        out << "<tr></tr></table><br>";
    }
    return dict_html.value(Navigation::LeakagesByApplication).arg(html);
}

QString MainWindow::viewAgenda()
{
    QString html; MTTextStream out(&html);

    QMultiMap<QString, QStringList> next_inspections_map;
    QString last_inspection_date, circuit, customer;
    int inspection_interval;

    MultiMapOfVariantMaps customers(Customer("").mapAll("id", "company"));

    MTRecord circuits_record("circuits", "id", "", MTDictionary("disused", "0"));
    if (!navigation->isFilterEmpty()) {
        circuits_record.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    QSqlQuery circuits = circuits_record.select("parent, id, name, operation, commissioning, refrigerant_amount, hermetic, leak_detector, inspection_interval");
    circuits.setForwardOnly(true);
    circuits.exec();
    while (circuits.next()) {
        inspection_interval = Warnings::circuitInspectionInterval(QUERY_VALUE(circuits, "refrigerant_amount").toDouble(),
                                                                  QUERY_VALUE(circuits, "hermetic").toInt(),
                                                                  QUERY_VALUE(circuits, "leak_detector").toInt(),
                                                                  QUERY_VALUE(circuits, "inspection_interval").toInt());
        if (inspection_interval) {
            customer = QUERY_VALUE(circuits, "parent").toString();
            circuit = QUERY_VALUE(circuits, "id").toString();
            last_inspection_date.clear();
            MTDictionary inspections_parents("customer", customer);
            inspections_parents.insert("circuit", circuit);
            inspections_parents.insert("outside_interval", "0");
            QSqlQuery inspections = MTRecord("inspections", "date", "", inspections_parents).select("date", Qt::DescendingOrder);
            inspections.setForwardOnly(true);
            if (inspections.exec() && inspections.next()) {
                last_inspection_date = inspections.value(0).toString();
            }
            if (last_inspection_date.isEmpty()) {
                last_inspection_date = QUERY_VALUE(circuits, "commissioning").toString();
                if (last_inspection_date.isEmpty()) continue;
            }
            next_inspections_map.insert(QDate::fromString(last_inspection_date.split("-").first(), "yyyy.MM.dd")
                                            .addDays(inspection_interval).toString("yyyy.MM.dd"),
                                        QStringList() << customer << circuit
                                            << QUERY_VALUE(circuits, "name").toString()
                                            << QUERY_VALUE(circuits, "operation").toString()
                                            << last_inspection_date);
        }
    }

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr>";
    out << "<th colspan=\"5\" style=\"font-size: medium;\">" << tr("Agenda") << "</th></tr>";
    out << "<tr><th>" << tr("Next inspection") << "</th><th>" << tr("Customer") << "</th>";
    out << "<th>" << tr("Circuit") << "</th><th>" << QApplication::translate("Circuit", "Place of operation") << "</th>";
    out << "<th>" << tr("Last inspection") << "</th></tr>";

    QString next_inspection, colour, circuit_name, operation;
    QMapIterator<QString, QStringList> i(next_inspections_map);
    while (i.hasNext()) { i.next();
        customer = i.value().value(0);
        circuit = i.value().value(1);
        circuit_name = i.value().value(2);
        operation = i.value().value(3);
        last_inspection_date = i.value().value(4);
        int days_to = QDate::currentDate().daysTo(QDate::fromString(i.key(), "yyyy.MM.dd"));
        switch (days_to) {
            case -1: next_inspection = tr("Yesterday"); break;
            case 0: next_inspection = tr("Today"); break;
            case 1: next_inspection = tr("Tomorrow"); break;
            default: next_inspection = i.key(); break;
        }
        if (days_to < 0) colour = "tomato";
        else if (days_to < 31) colour = "yellow";
        else colour = "";
        out << "<tr><td class=\"" << colour << "\">" << next_inspection << "</td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "\">";
        out << customer.rightJustified(8, '0') << " (" << escapeString(customers.value(customer).value("company").toString()) << ")</a></td>";
        out << "<td class=\"" << colour << "\"><a href=\"customer:" << customer << "/circuit:" << circuit << "\">";
        out << circuit.rightJustified(4, '0');
        if (!circuit_name.isEmpty()) { out << " (" << escapeString(circuit_name) << ")"; }
        out << "</a></td>";
        out << "<td class=\"" << colour << "\">" << escapeString(operation) << "</td>";
        out << "<td class=\"" << colour << "\">";
        if (last_inspection_date.contains("-"))
            out << "<a href=\"customer:" << customer << "/circuit:" << circuit << "/inspection:"
                << last_inspection_date << "\">" << last_inspection_date << "</a>";
        else
            out << last_inspection_date;
        out << "</td></tr>";
    }
    out << "</table>";

    return dict_html.value(Navigation::Agenda)
            .arg(actionPrinter_friendly_version->isChecked() ? "/*" : "")
            .arg(actionPrinter_friendly_version->isChecked() ? "*/" : "")
            .arg(html);
}

QString MainWindow::viewAllAssemblyRecordTypes(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    AssemblyRecordType all_items("");
    if (!navigation->isFilterEmpty()) {
        all_items.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps items(all_items.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < AssemblyRecordType::attributes().count(); ++n) {
        thead.append("<th>" + AssemblyRecordType::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("List of assembly record types") << "</th></tr>";
    out << thead;
    QString id;
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'assemblyrecordtype:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
        for (int n = 1; n < AssemblyRecordType::attributes().count(); ++n) {
            out << "<td>" << escapeString(items.at(i).value(AssemblyRecordType::attributes().key(n)).toString()) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfAssemblyRecordTypes).arg(html);
}

QString MainWindow::viewAllAssemblyRecordItemTypes(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    AssemblyRecordItemType all_items("");
    if (!navigation->isFilterEmpty()) {
        all_items.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps items(all_items.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < AssemblyRecordItemType::attributes().count(); ++n) {
        thead.append("<th>" + AssemblyRecordItemType::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("List of assembly record item types") << "</th></tr>";
    out << thead;
    QString id;
    MTDictionary categories(listAssemblyRecordItemCategories());
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'assemblyrecorditemtype:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
        for (int n = 1; n < AssemblyRecordItemType::attributes().count(); ++n) {
            out << "<td>";
            if (AssemblyRecordItemType::attributes().key(n) == "category_id")
                out << escapeString(categories.key(categories.indexOfValue(items.at(i).value(AssemblyRecordItemType::attributes().key(n)).toString())));
            else
                out << escapeString(items.at(i).value(AssemblyRecordItemType::attributes().key(n)).toString());
            out << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfAssemblyRecordItemTypes).arg(html);
}

QString MainWindow::viewAllAssemblyRecordItemCategories(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    AssemblyRecordItemCategory all_items("");
    if (!navigation->isFilterEmpty()) {
        all_items.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps items(all_items.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
        thead.append("<th>" + AssemblyRecordItemCategory::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("List of assembly record item categories") << "</th></tr>";
    out << thead;
    QString id;
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'assemblyrecorditemcategory:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
        for (int n = 1; n < AssemblyRecordItemCategory::attributes().count(); ++n) {
            out << "<td>" << escapeString(items.at(i).value(AssemblyRecordItemCategory::attributes().key(n)).toString()) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfAssemblyRecordItemCategories).arg(html);
}

QString MainWindow::viewAssemblyRecord(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    Inspection inspection_record(customer_id, circuit_id, inspection_date);
    QVariantMap inspection = inspection_record.list();
    bool nominal = inspection.value("nominal").toInt();
    bool repair = inspection.value("repair").toInt();
    bool locked = isRecordLocked(inspection_date);

    VariableEvaluation::EvaluationContext var_evaluation(customer_id, circuit_id);
    QString nom_value;

    HTMLDiv div;
    HTMLTable * table, * top_table;
    HTMLTableRow * _tr;
    HTMLTableCell * _td;
    QList<HTMLTable *> bottom_tables;

    AssemblyRecordType ar_type_record(inspection.value("ar_type").toString());
    QVariantMap ar_type = ar_type_record.list();
    int type_display_options = ar_type.value("display_options").toInt();

    if (type_display_options & AssemblyRecordType::ShowServiceCompany) {
        div << writeServiceCompany();
        div << "<br>";
    }

    table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
    *(table->addRow()->addHeaderCell()) << tr("Assembly record No. %1").arg(inspection.value("arno").toString());
    *(table->addRow()->addCell()) << "<h2>" << ar_type.value("name").toString() << "</h2>";
    *(table->addRow()->addCell()) << "<h3>" << ar_type.value("description").toString() << "</h3>";
    div << table;
    div << "<br>";

    QString html; MTTextStream out(&html);

    if (type_display_options & AssemblyRecordType::ShowCustomer) {
        writeCustomersTable(out, customer_id);
        out << "<br>";
    }
    if (type_display_options & AssemblyRecordType::ShowCustomerContactPersons) {
        out << customerContactPersons(customer_id)->html();
        out << "<br>";
    }

    if (type_display_options & AssemblyRecordType::ShowCircuit) {
        writeCircuitsTable(out, customer_id, circuit_id, 7);
        out << "<br>";
    }
    div << html;
    if (type_display_options & AssemblyRecordType::ShowCircuitUnits) {
        div << circuitUnitsTable(customer_id, circuit_id);
        div << "<br>";
    }

    top_table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
    _td = top_table->addRow()->addHeaderCell("colspan=\"6\" style=\"font-size: medium; background-color: lightgoldenrodyellow;\"");
    if (!locked) {
        *_td << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id;
        *_td << (repair ? "/repair:" : "/inspection:") << inspection_date << "/modify\">";
    }
    if (nominal) *_td << tr("Nominal inspection:"); else if (repair) *_td << tr("Repair:"); else *_td << tr("Inspection:");
    *_td << "&nbsp;" << inspection_date;
    if (!locked) *_td << "</a>";

    enum QUERY_RESULTS
    {
        VALUE = 0,
        NAME = 1,
        CATEGORY_ID = 2,
        CATEGORY_NAME = 3,
        DISPLAY_OPTIONS = 4,
        LIST_PRICE = 5,
        ACQUISITION_PRICE = 6,
        UNIT = 7,
        VARIABLE_ID = 8,
        VALUE_DATA_TYPE = 9,
        CATEGORY_POSITION = 10,
        DISCOUNT = 11
               };

    QSqlQuery categories_query(QString("SELECT assembly_record_items.value, assembly_record_items.name, assembly_record_item_categories.id, assembly_record_item_categories.name,"
                                       " assembly_record_item_categories.display_options, assembly_record_items.list_price, assembly_record_items.acquisition_price, assembly_record_items.unit,"
                                       " assembly_record_item_types.inspection_variable_id, assembly_record_item_types.value_data_type, assembly_record_item_categories.display_position, assembly_record_items.discount"
                                       " FROM assembly_record_items"
                                       " LEFT JOIN assembly_record_item_types ON assembly_record_items.item_type_id = assembly_record_item_types.id AND assembly_record_items.source = %1"
                                       " LEFT JOIN assembly_record_item_categories ON assembly_record_items.category_id = assembly_record_item_categories.id"
                                       " WHERE arno = '%2' ORDER BY assembly_record_item_types.category_id, assembly_record_item_types.name")
                               .arg(AssemblyRecordItem::AssemblyRecordItemTypes)
                               .arg(inspection.value("arno").toString()));
    int last_category = -1;
    int num_columns = 6, i, n;
    int colspans[num_columns];
    double absolute_total = 0.0, total;
    QString colspan = "colspan=\"%1\"";
    QString item_value;
    while (categories_query.next()) {
        if (last_category != categories_query.value(CATEGORY_ID).toInt()) {
            if (categories_query.value(CATEGORY_POSITION).toInt() == AssemblyRecordItemCategory::DisplayAtTop) {
                table = top_table;
            } else {
                table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"no_border\"");
                bottom_tables.append(table);
            }

            int cat_display_options = categories_query.value(DISPLAY_OPTIONS).toInt();

            for (i = 1; i < num_columns; ++i) colspans[i] = 0;
            i = n = 0; colspans[0] = 1;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowValue) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowAcquisitionPrice) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowListPrice) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowDiscount) { i = n; colspans[i] = 1; }
            else colspans[i]++;
            if (++n && cat_display_options & AssemblyRecordItemCategory::ShowTotal) { i = n; colspans[i] = 1; }
            else colspans[i]++;

            if (categories_query.value(CATEGORY_POSITION).toInt() == AssemblyRecordItemCategory::DisplayAtTop) {
                i = 0;
                _tr = table->addRow();
                *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << categories_query.value(CATEGORY_NAME).toString();
                if (colspans[++i])
                    *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << tr("Value");
                if (colspans[++i])
                    *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << tr("Acquisition price");
                if (colspans[++i])
                    *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << tr("List price");
                if (colspans[++i])
                    *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << tr("Discount");
                if (colspans[++i])
                    *(_tr->addHeaderCell(colspan.arg(colspans[i]))) << tr("Total");
            }
            last_category = categories_query.value(CATEGORY_ID).toInt();
        }
        i = 0; total = 0.0;
        _tr = table->addRow();
        *(_tr->addCell(colspan.arg(colspans[i]))) << categories_query.value(NAME).toString();
        if (colspans[++i]) {
            if (categories_query.value(VARIABLE_ID).toString().isEmpty()) {
                if (categories_query.value(VALUE_DATA_TYPE).toInt() == Global::Boolean)
                    item_value = categories_query.value(VALUE).toInt() ? tr("Yes") : tr("No");
                else
                    item_value = categories_query.value(VALUE).toString();
            } else {
                item_value = var_evaluation.evaluate(categories_query.value(VARIABLE_ID).toString(), inspection, nom_value);
            }
            *(_tr->addCell(colspan.arg(colspans[i]))) << item_value << " " << categories_query.value(UNIT).toString();
            total = item_value.toDouble();
        }
        if (colspans[++i])
            *(_tr->addCell(colspan.arg(colspans[i]))) << categories_query.value(ACQUISITION_PRICE).toString();
        if (colspans[++i]) {
            *(_tr->addCell(colspan.arg(colspans[i]))) << categories_query.value(LIST_PRICE).toString();
            total *= categories_query.value(LIST_PRICE).toDouble();
        }
        if (colspans[++i]) {
            double total_discount = categories_query.value(DISCOUNT).toDouble();
            total *= 1 - total_discount / 100;
            *(_tr->addCell(colspan.arg(colspans[i]))) << QString::number(total_discount) << " %";
        }
        if (colspans[++i]) {
            absolute_total += total;
            *(_tr->addCell(colspan.arg(colspans[i]))) << QString::number(total);
        }
    }
    _tr = top_table->addRow();
    *(_tr->addHeaderCell(QString("colspan=\"%1\"").arg(num_columns - 1))) << tr("Total");
    *(_tr->addCell()) << QString::number(absolute_total);

    div << top_table;
    for (int i = 0; i < bottom_tables.count(); ++i) {
        div << "<br>";
        div << bottom_tables.at(i);
    }

    return dict_html.value(Navigation::AssemblyRecord).arg(div.html());
}

HTMLTable * MainWindow::writeServiceCompany()
{
    ServiceCompany serv_company_record(DBInfoValueForKey("default_service_company"));
    QVariantMap serv_company = serv_company_record.list();
    HTMLTable * table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"");
    HTMLTableRow * _tr = table->addRow();
    HTMLTableCell * _td;
    if (serv_company.value("image").toInt()) {
        QByteArray byte_array = DBFile(serv_company.value("image").toInt()).data();
        if (!byte_array.isNull()) {
            _td = _tr->addCell("rowspan=\"3\" width=\"5%\"");
            *_td << QString("<img src=\"data:image/png;base64," + byte_array.toBase64() + "\">");
        }
    }
    _td = _tr->addHeaderCell("colspan=\"5\" style=\"background-color: #DFDFDF; font-size: medium; width:100%; text-align: center;\"");
    *_td << "<a href=\"servicecompany:" << serv_company.value("id").toString() << "/modify\">";
    *_td << tr("Service company") << "</a>";
    _tr = table->addRow();
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        if (serv_company.value(ServiceCompany::attributes().key(n)).toString().isEmpty()) continue;
        _td = _tr->addHeaderCell();
        QString attr = ServiceCompany::attributes().value(n);
        attr.chop(1);
        *_td << attr;
    }
    QString attr_value;
    _tr = table->addRow();
    for (int n = 0; n < ServiceCompany::attributes().count(); ++n) {
        attr_value = ServiceCompany::attributes().key(n);
        if (serv_company.value(attr_value).toString().isEmpty()) continue;
        _td = _tr->addCell();
        *_td << MTVariant(serv_company.value(attr_value), (MTVariant::Type)dict_fieldtypes.value(attr_value)).toHtml();
    }
    return table;
}

QString MainWindow::viewAllCircuitUnitTypes(const QString & highlighted_id)
{
    QString html; MTTextStream out(&html);
    CircuitUnitType all_items("");
    if (!navigation->isFilterEmpty()) {
        all_items.addFilter(navigation->filterColumn(), navigation->filterKeyword());
    }
    ListOfVariantMaps items(all_items.listAll());
    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < CircuitUnitType::attributes().count(); ++n) {
        thead.append("<th>" + CircuitUnitType::attributes().value(n) + "</th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">" << tr("List of circuit unit types") << "</th></tr>";
    out << thead;
    QString id;
    MTDictionary categories(listAssemblyRecordItemCategories());
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        out << "<tr onclick=\"window.location = 'circuitunittype:" << id << "";
        if (highlighted_id == id) {
            out << "/modify'\" style=\"background-color: rgb(242, 248, 255); font-weight: bold;";
        } else {
            out << "'\" style=\"";
        }
        out << " cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
        for (int n = 1; n < CircuitUnitType::attributes().count(); ++n) {
            out << "<td>";
            if (CircuitUnitType::attributes().key(n) == "location")
                out << CircuitUnitType::locationToString(items.at(i).value("location").toInt());
            else if (CircuitUnitType::attributes().key(n) == "oil")
                out << items.at(i).value(CircuitUnitType::attributes().key(n)).toString().toUpper();
            else if (CircuitUnitType::attributes().key(n) == "category_id")
                out << escapeString(categories.key(categories.indexOfValue(items.at(i).value(CircuitUnitType::attributes().key(n)).toString())));
            else if (CircuitUnitType::attributes().key(n) == "output")
                out << escapeString(QString("%1 %2").arg(items.at(i).value("output").toString()).arg(items.at(i).value("output_unit").toString()));
            else
                out << escapeString(items.at(i).value(CircuitUnitType::attributes().key(n)).toString());
            out << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return dict_html.value(Navigation::ListOfCircuitUnitTypes).arg(html);
}

HTMLTable * MainWindow::circuitUnitsTable(const QString & customer_id, const QString & circuit_id)
{
    HTMLTable * table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    HTMLTableRow * _tr;

    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Circuit units");
    *(_tr->addHeaderCell()) << tr("Manufacturer");
    *(_tr->addHeaderCell()) << tr("Type");
    *(_tr->addHeaderCell()) << tr("Serial number");

    enum QUERY_RESULTS
    {
        SN = 0,
        MANUFACTURER = 1,
        TYPE = 2,
        LOCATION = 3,
        UNIT_TYPE_ID = 4
    };
    QSqlQuery query(QString("SELECT circuit_units.sn, circuit_unit_types.manufacturer, circuit_unit_types.type, circuit_unit_types.location, circuit_unit_types.id"
                            " FROM circuit_units"
                            " LEFT JOIN circuit_unit_types ON circuit_units.unit_type_id = circuit_unit_types.id"
                            " WHERE circuit_units.company_id = %1 AND circuit_units.circuit_id = %2")
                    .arg(customer_id.toInt()).arg(circuit_id.toInt()));
    while (query.next()) {
        _tr = table->addRow();
        *(_tr->addCell()) << CircuitUnitType::locationToString(query.value(LOCATION).toInt());
        *(_tr->addCell()) << query.value(MANUFACTURER).toString();
        *(_tr->addCell()) << query.value(TYPE).toString();
        *(_tr->addCell()) << query.value(SN).toString();
    }

    return table;
}

HTMLTable * MainWindow::customerContactPersons(const QString & customer_id)
{
    HTMLTable * table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    HTMLTableRow * _tr;

    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Contact persons");
    *(_tr->addHeaderCell()) << tr("E-mail");
    *(_tr->addHeaderCell()) << tr("Phone");

    Person persons_record(QString(), customer_id);
    ListOfVariantMaps persons = persons_record.listAll();
    for (int i = 0; i < persons.count(); ++i) {
        _tr = table->addRow();
        *(_tr->addCell()) << persons.at(i).value("name").toString();
        *(_tr->addCell()) << persons.at(i).value("mail").toString();
        *(_tr->addCell()) << persons.at(i).value("phone").toString();
    }

    return table;
}

QString MainWindow::viewAllAssemblyRecords(const QString & customer_id, const QString & circuit_id)
{
    HTMLDiv div;
    HTMLTable * table;
    HTMLTableRow * _tr;
    HTMLTableCell * _td;

    MTDictionary inspectors = Global::listInspectors();

    QString html; MTTextStream out(&html);
    if (customer_id.toInt() >= 0) {
        writeCustomersTable(out, customer_id);
        out << "<br>";
    }
    if (circuit_id.toInt() >= 0) {
        writeCircuitsTable(out, customer_id, circuit_id);
        out << "<br>";
    }

    div << html;

    table = new HTMLTable("cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\"");
    _tr = table->addRow();
    _td = _tr->addHeaderCell("colspan=\"5\" style=\"background-color: #DFDFDF; font-size: medium; width:100%; text-align: center;\"");
    *_td << tr("Assembly records");
    _tr = table->addRow();
    *(_tr->addHeaderCell()) << tr("Date");
    *(_tr->addHeaderCell()) << tr("Assembly record number");
    *(_tr->addHeaderCell()) << tr("Assembly record name");
    *(_tr->addHeaderCell()) << tr("Inspector");

    enum QUERY_RESULTS
    {
        CUSTOMER_ID = 0,
        CIRCUIT_ID = 1,
        DATE = 2,
        ARNO = 3,
        AR_NAME = 4,
        INSPECTOR = 5
    };

    QSqlQuery query(QString("SELECT inspections.customer, inspections.circuit, inspections.date, inspections.arno, assembly_record_types.name, inspections.inspector"
                            " FROM inspections"
                            " LEFT JOIN assembly_record_types ON inspections.ar_type = assembly_record_types.id"
                            " WHERE inspections.arno != ''%1%2 ORDER BY inspections.date DESC")
                    .arg(customer_id.toInt() < 0 ? "" : QString(" AND inspections.customer = " + customer_id))
                    .arg(circuit_id.toInt() < 0 ? "" : QString(" AND inspections.circuit = " + circuit_id)));
    while (query.next()) {
        _tr = table->addRow(QString("onclick=\"window.location = 'customer:%1/circuit:%2/inspection:%3/assemblyrecord'\" style=\"cursor: pointer;\"")
                            .arg(query.value(CUSTOMER_ID).toString())
                            .arg(query.value(CIRCUIT_ID).toString())
                            .arg(query.value(DATE).toString()));
        *(_tr->addCell()) << query.value(DATE).toString();
        *(_tr->addCell()) << query.value(ARNO).toString();
        *(_tr->addCell()) << query.value(AR_NAME).toString();
        *(_tr->addCell()) << inspectors.key(inspectors.indexOfValue(query.value(INSPECTOR).toString()));
    }
    div << table;

    return dict_html.value(Navigation::ListOfAssemblyRecords).arg(div.html());
}
