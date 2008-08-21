/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008 Matus & Michal Tomlein

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

void MainWindow::viewChanged(const QString & view)
{
    if (!db.isOpen()) { wv_main->setHtml(QString()); return; }

    bool table_view = cb_view->currentText() == tr("Table of inspections");
    lbl_table->setEnabled(table_view);
    cb_table->setEnabled(table_view);
    lbl_since->setEnabled(table_view);
    spb_since->setEnabled(table_view);

    if (view == tr("All customers")) {
        viewAllCustomers();
    } else if (view == tr("Customer information") && selectedCustomer() >= 0) {
        viewCustomer(toString(selectedCustomer()));
    } else if (view == tr("Circuit information") && selectedCustomer() >= 0 && selectedCircuit() >= 0) {
        viewCircuit(toString(selectedCustomer()), toString(selectedCircuit()));
    } else if (view == tr("Inspection information") && selectedCustomer() >= 0 && selectedCircuit() >= 0 && !selectedInspection().isNull()) {
        viewInspection(toString(selectedCustomer()), toString(selectedCircuit()), selectedInspection());
    } else if (table_view && selectedCustomer() >= 0 && selectedCircuit() >= 0 && cb_table->currentIndex() >= 0) {
        viewTable(toString(selectedCustomer()), toString(selectedCircuit()), cb_table->currentText(), spb_since->value() == 1999 ? 0 : spb_since->value());
    } else {
        wv_main->setHtml(QString());
    }
}

void MainWindow::viewAllCustomers()
{
    QString html; QTextStream out(&html);
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, company, contact_person, address, mail, phone FROM customers ORDER BY id");
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    while (query.next()) {
        out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: large; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << query.value(0).toString() << "\">" << query.value(1).toString() << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td style=\"text-align: right; width:50%;\">" << tr("ID:") << "&nbsp;</td>";
        out << "<td>" << query.value(0).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Contact person:") << "&nbsp;</b></td>";
        out << "<td><b>" << query.value(2).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Address:") << "&nbsp;</td>";
        out << "<td>" << query.value(3).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("E-mail:") << "&nbsp;</td>";
        out << "<td>" << query.value(4).toString() << "</td></tr>";
        out << "</table></td><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Phone:") << "&nbsp;</td>";
        out << "<td>" << query.value(5).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
        out << "<td>";
        QSqlQuery circuits;
        circuits.setForwardOnly(true);
        circuits.prepare("SELECT id FROM circuits WHERE parent = :parent");
        circuits.bindValue(":parent", query.value(0).toInt());
        circuits.exec();
        int num_circuits = 0, num_inspections = 0;
        while (circuits.next()) {
            num_circuits++;
            MTDictionary inspection_parents("circuit", circuits.value(0).toString());
            inspection_parents.insert("customer", query.value(0).toString());
            MTRecord inspection_record("inspection", "", inspection_parents);
            num_inspections += inspection_record.list("COUNT(date)").value("COUNT(date)").toInt();
        }
        out << num_circuits;
        out << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
        out << "<td>" << num_inspections << "</td></tr>";
        out << "</table></td></tr>";
    }
    wv_main->setHtml(dict_html.value(tr("All customers")).arg(html));
}

void MainWindow::viewCustomer(const QString & customer_id)
{
    QString html; QTextStream out(&html);
    MTRecord customer("customer", customer_id, MTDictionary());
    QSqlQuery query = customer.select("company, contact_person, address, mail, phone");
    query.setForwardOnly(true);
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    while (query.next()) {
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "/modify\">" << query.value(0).toString() << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("ID:") << "&nbsp;</td>";
        out << "<td style=\"width:50%;\">" << customer_id << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\"><b>" << tr("Contact person:") << "&nbsp;</b></td>";
        out << "<td><b>" << query.value(1).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Address:") << "&nbsp;</td>";
        out << "<td>" << query.value(2).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("E-mail:") << "&nbsp;</td>";
        out << "<td>" << query.value(3).toString() << "</td></tr>";
        out << "</table></td>";
        out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Phone:") << "&nbsp;</td>";
        out << "<td style=\"width:50%;\">" << query.value(4).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Number of circuits:") << "&nbsp;</td>";
        out << "<td>";
        QSqlQuery circuits;
        //circuits.setForwardOnly(true);
        circuits.prepare("SELECT id, manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation FROM circuits WHERE parent = :parent ORDER BY id");
        circuits.bindValue(":parent", customer_id.toInt());
        circuits.exec();
        int num_circuits = 0, num_inspections = 0;
        while (circuits.next()) {
            num_circuits++;
            MTDictionary inspection_parents("circuit", circuits.value(0).toString());
            inspection_parents.insert("customer", query.value(0).toString());
            MTRecord inspection_record("inspection", "", inspection_parents);
            num_inspections += inspection_record.list("COUNT(date)").value("COUNT(date)").toInt();
        }
        out << num_circuits;
        out << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Total number of inspections:") << "&nbsp;</td>";
        out << "<td>" << num_inspections << "</td></tr>";
        out << "</table></td></tr>";
        out << "</table>";
        if (circuits.first()) {
            do {
                out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\"><tr><td rowspan=\"8\" style=\"width:10%;\"/>";
                out << "<td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:80%;\"><b>" << tr("Circuit:") << "&nbsp;";
                out << "<a href=\"customer:" << customer_id << "/circuit:" << circuits.value(0).toString() << "\">" << circuits.value(0).toString() << "</a></b></td>";
                out << "<td rowspan=\"8\" style=\"width:10%;\"/></tr>";
                out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Manufacturer:") << "&nbsp;</b></td>";
                out << "<td style=\"width:50%;\"><b>" << circuits.value(1).toString() << "</b></td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Type:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(2).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Serial number:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(3).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Year of purchase:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(4).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Date of commissioning:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(5).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Field of application:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(6).toString() << "</td></tr>";
                out << "</table></td>";
                out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Refrigerant:") << "&nbsp;</td>";
                out << "<td style=\"width:50%;\">" << circuits.value(7).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Amount of refrigerant:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(8).toString() << "&nbsp;kg</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Oil:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(9).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Amount of oil:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(10).toString() << "&nbsp;kg</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Service life:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(11).toString() << "&nbsp;" << tr("years") << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Run-time per day:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(12).toString() << "</td></tr>";
                out << "<tr><td style=\"text-align: right;\">" << tr("Rate of utilisation:") << "&nbsp;</td>";
                out << "<td>" << circuits.value(13).toString() << "&nbsp;%</td></tr>";
                out << "</table></td></tr>";
                out << "</table>";
            } while (circuits.next());
        }
    }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewCircuit(const QString & customer_id, const QString & circuit_id)
{
    QString html; QTextStream out(&html);
    MTRecord circuit("circuit", circuit_id, MTDictionary("parent", customer_id));
    QSqlQuery query = circuit.select("manufacturer, type, sn, year, commissioning, field, refrigerant, refrigerant_amount, oil, oil_amount, life, runtime, utilisation");
    query.setForwardOnly(true);
    query.exec();
    //query.setForwardOnly();
    //query.prepare("");
    //query.bindVariable();
    //query.exec();
    if (!query.next()) { return; }
        out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Company:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "\">";
        MTRecord customer("customer", customer_id, MTDictionary());
        out << customer.list("company").value("company").toString();
        out << "</a></b></td></tr>";
        out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: large; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
        out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/modify\">" << circuit_id << "</a></b></td></tr>";
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\"><b>" << tr("Manufacturer:") << "&nbsp;</b></td><td style=\"width:50%;\"><b>" << query.value(0).toString() << "</b></td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Type:") << "&nbsp;</td><td>" << query.value(1).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Serial number:") << "&nbsp;</td><td>" << query.value(2).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Year of purchase:") << "&nbsp;</td><td>" << query.value(3).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Date of commissioning:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(4).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Field of application:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(5).toString() << "</td></tr>";
        out << "</table></td>";
        out << "<td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td style=\"text-align: right; width:50%;\">" << tr("Refrigerant:") << "&nbsp;</td><td style=\"width:50%;\">" << query.value(6).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Amount of refrigerant:") << "&nbsp;</td><td>" << query.value(7).toString() << "&nbsp;kg</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Oil:") << "&nbsp;</td><td>" << query.value(8).toString() << "</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Amount of oil:") << "&nbsp;</td><td>" << query.value(9).toString() << "&nbsp;kg</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Service life:") << "&nbsp;</td><td>" << query.value(10).toString() << "&nbsp;years</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Run-time per day:") << "&nbsp;</td><td>" << query.value(11).toString() << "&nbsp;hours</td></tr>";
        out << "<tr><td style=\"text-align: right;\">" << tr("Rate of utilisation:") << "&nbsp;</td><td>" << query.value(12).toString() << "&nbsp;%</td></tr>";
        out << "</table></td></tr>";
        out << "</table>";
        MTDictionary inspection_parents("circuit", circuit_id);
        inspection_parents.insert("customer", customer_id);
        MTRecord inspection_record("inspection", "", inspection_parents);
        QSqlQuery inspections = inspection_record.select("date, nominal");
        inspections.exec();
        int num_inspections = 0;
        while (inspections.next()) {
            num_inspections++;
        }
        if (num_inspections > 0) {
            out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
            out << "<tr><td rowspan=\"";
            int row_span;
            if (num_inspections % 2 == 0) row_span = num_inspections / 2 + 1;
            else row_span = num_inspections / 2 + 2;
            out << row_span;
            out << "\" style=\"width:10%;\"/>";
            out << "<td colspan=\"2\" style=\"background-color: #eee; font-size: medium; text-align: center; width:80%;\"><b>";
            out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/table\">" << tr("Inspections") << "</a></b></td>";
            out << "<td rowspan=\"";
            out << row_span;
            out << "\" style=\"width:10%;\"/></tr>";
            out << "<tr><td style=\"width:40%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
            if (inspections.first()) {
                if (num_inspections % 2 != 0) num_inspections++;
                do {
                    if (int(num_inspections / 2) == inspections.at()) {
                        out << "</table></td>";
                        out << "<td style=\"width:40%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
                    }
                    out << "<tr><td style=\"text-align: center\">";
                    if (inspections.value(1).toInt() == 1) {
                        out << tr("Nominal:") << "&nbsp;";
                    }
                    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspections.value(0).toString() << "\">" << inspections.value(0).toString() << "</a>";
                    out << "</td></tr>";
                } while (inspections.next());
            }
            out << "</table></td></tr>";
            out << "</table>";
        }
    wv_main->setHtml(dict_html.value(tr("Customer information")).arg(html));
}

void MainWindow::viewInspection(const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    QString html; QTextStream out(&html);

    QSqlQuery vars("SELECT variables.id, variables.name, variables.type, variables.unit, variables.value, variables.compare_nom, subvariables.id, subvariables.name, subvariables.type, subvariables.unit, subvariables.value, subvariables.compare_nom FROM variables LEFT JOIN subvariables ON variables.id = subvariables.parent");
    const int VAR_ID = 0; const int VAR_NAME = 1; const int VAR_TYPE = 2; const int VAR_UNIT = 3; const int VAR_VALUE = 4; const int VAR_COMPARE_NOM = 5;
    const int SUBVAR_ID = 6; const int SUBVAR_NAME = 7; const int SUBVAR_TYPE = 8; const int SUBVAR_UNIT = 9; const int SUBVAR_VALUE = 10; const int SUBVAR_COMPARE_NOM = 11;

    MTDictionary inspection_parents("circuit", circuit_id);
    inspection_parents.insert("customer", customer_id);
    MTRecord inspection_record("inspection", inspection_date, inspection_parents);
    QSqlQuery inspection = inspection_record.select();
    inspection.setForwardOnly(true);
    inspection.exec();
    if (!inspection.next()) return;
    QSqlRecord ins_rec = inspection.record();
    int nominal = inspection.value(ins_rec.indexOf("nominal")).toInt();
    QSqlQuery nominal_ins;
    if (nominal == 0) {
        nominal_ins.prepare("SELECT * FROM inspections WHERE nominal = 1 AND customer = :customer_id AND circuit = :circuit_id");
        nominal_ins.bindValue(":customer_id", customer_id);
        nominal_ins.bindValue(":circuit_id", circuit_id);
        nominal_ins.exec();
        nominal_ins.next();
    }
    QSqlQuery warnings;
    warnings.prepare("SELECT id, name FROM warnings");
    warnings.exec();
    QSqlQuery warnings_conditions;
    warnings_conditions.prepare("SELECT parent, value_ins, function, value_nom FROM warnings_conditions");
    warnings_conditions.exec();
    QSqlQuery warnings_filters;
    warnings_filters.prepare("SELECT parent, circuit_attribute, function, value FROM warnings_filters");
    warnings_filters.exec();

    out << "<table cellspacing=\"0\" style=\"width:100%;\">";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
    out << "<tbody>";
    out << "<tr style=\"background-color: #eee;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>" << tr("Circuit:") << "&nbsp;";
    out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "\">" << circuit_id << "</a></b></td></tr>";
    out << "<tr style=\"background-color: #DFDFDF;\"><td colspan=\"2\" style=\"font-size: larger; width:100%; text-align: center;\"><b>";
    if (nominal == 1) out << tr("Nominal inspection:") << "&nbsp;";
    else out << tr("Inspection:") << "&nbsp;";
	out << "<a href=\"customer:" << customer_id << "/circuit:" << circuit_id << "/inspection:" << inspection_date << "/modify\">";
	out << inspection_date << "</a></b></td></tr>";
    out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";

    int num_shown_vars = 0;
    QStringList used_ids = listVariableIds(); // all = false
    while (vars.next()) {
        QString value; QString var_id; bool compare_nom = false;
        MTDictionary expression;
        if (vars.value(SUBVAR_ID).toString().isEmpty()) {
            var_id = vars.value(VAR_ID).toString();
        } else {
            var_id = vars.value(SUBVAR_ID).toString();
        }
        if (nominal == 0) {
            if (vars.value(VAR_COMPARE_NOM).toInt() == 1) {
                compare_nom = true;
            } else if (vars.value(SUBVAR_COMPARE_NOM).toInt() == 1) {
                compare_nom = true;
            }
        }
        if (!vars.value(VAR_VALUE).toString().isEmpty()) {
            expression = parseExpression(vars.value(VAR_VALUE).toString(), &used_ids);
        } else if (!vars.value(SUBVAR_VALUE).toString().isEmpty()) {
            expression = parseExpression(vars.value(SUBVAR_VALUE).toString(), &used_ids);
        }
        if (expression.count() != 0) {
            if (compare_nom) {
                value.append(expressionToHtml(nominal_ins, expression, customer_id, circuit_id, nominal_ins.value(nominal_ins.record().indexOf("date")).toString()));
                value.append("?");
            }
            value.append(expressionToHtml(inspection, expression, customer_id, circuit_id, inspection_date));
            //QMessageBox::information(this, "", value);
        } else {
            value.append(inspection.value(ins_rec.indexOf(var_id)).toString());
            if (value.isEmpty() && compare_nom) {
                value.append(nominal_ins.value(nominal_ins.record().indexOf(var_id)).toString());
                value.append("?");
            }
        }
        if (value.isEmpty()) continue;
        value.prepend("<expression>");
        value.append("</expression>");
        out << "<num_var>" << num_shown_vars << "</num_var>";
        out << "<tr><td style=\"text-align: right; width:50%;\">";
        if (vars.value(SUBVAR_ID).toString().isEmpty()) {
            out << vars.value(VAR_NAME).toString() << ":&nbsp;";
        } else {
            out << vars.value(VAR_NAME).toString() << ":&nbsp;" << vars.value(SUBVAR_NAME).toString() << ":&nbsp;";
        }
        out << "</td><td><table cellpadding=\"0\" cellspacing=\"0\"><tr><td align=\"right\" valign=\"center\">";
        out << value;
        out << "</td>";
        if (!vars.value(VAR_UNIT).toString().isEmpty()) {
            out << "<td valign=\"center\">&nbsp;";
            out << vars.value(VAR_UNIT).toString();
            out << "</td>";
        } else if (!vars.value(SUBVAR_UNIT).toString().isEmpty()) {
            out << "<td valign=\"center\">&nbsp;";
            out << vars.value(SUBVAR_UNIT).toString();
            out << "</td>";
        }
        out << "</tr></table></td>";
        //QMessageBox::information(this, "", vars.value(SUBVAR_ID).toString());
        num_shown_vars++;
    }
    if (num_shown_vars != 0) {
        html.replace(QString("<num_var>%1</num_var>").arg(int(num_shown_vars / 2 + num_shown_vars % 2)), "</table></td><td style=\"width:50%;\"><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">");
    }
    for (int i = 0; i < num_shown_vars; ++i) {
        html.remove(QString("<num_var>%1</num_var>").arg(i));
    }
    out << "</table></td></tbody>";
    out << "</table></td></tr>";
    QStringList warnings_list;
    if (warnings_list.count() > 0) {
        out << "<tr><td><table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\">";
        out << "<tr><td colspan=\"2\" style=\"font-size: larger; width:100%;\">";
        out << "<b><i18n>Warnings</i18n></b></td></tr>";
        out << "<tr><td colspan=\"2\">";
        out << warnings_list.join(", ");
        out << "</td></tr>";
        out << "</table></td></tr>";
    }
    out << "</table>";
    /*QPlainTextEdit * te = new QPlainTextEdit (dict_html.value(tr("Inspection information")).arg(html));
    te->show();*/
    //QMessageBox::information(this, "d", dict_html.value(tr("Inspection information")).arg(html));
    wv_main->setHtml(dict_html.value(tr("Inspection information")).arg(html));
}

void MainWindow::viewTable(const QString &, const QString &, const QString &, int)
{
    /*
    QStringList used_ids = listVariableIds(); // all = false
    for (;;) {
        MTDictionary expression(parseExpression(exp, &used_ids));
    }
    */
}

QString MainWindow::expressionToHtml(QSqlQuery & inspection, const MTDictionary & expression, const QString & customer_id, const QString & circuit_id, const QString & inspection_date)
{
    const QString sum_query("SELECT SUM(%1) FROM inspections WHERE date LIKE :year AND customer = :customer_id AND circuit = :circuit_id");
    QString value;
    for (int i = 0; i < expression.count(); ++i) {
        if (expression.value(i) == "id") {
            value.append(inspection.value(inspection.record().indexOf(expression.key(i))).toString());
        } else if (expression.value(i) == "sum") {
            QSqlQuery sum_ins;
            sum_ins.prepare(sum_query.arg(expression.key(i)));
            sum_ins.bindValue(":customer_id", customer_id);
            sum_ins.bindValue(":circuit_id", circuit_id);
            sum_ins.bindValue(":year", QString("%1%").arg(inspection_date.left(4)));
            if (sum_ins.exec() && sum_ins.next()) {
                value.append(sum_ins.value(0).toString());
            }
        } else if (expression.value(i) == "circuit_attribute") {
            QSqlQuery circuit;
            circuit.prepare("SELECT :circuit_attribute FROM circuits WHERE parent = :customer_id AND id = :circuit_id");
            circuit.bindValue(":circuit_attribute", expression.key(i));
            circuit.bindValue(":customer_id", customer_id);
            circuit.bindValue(":circuit_id", circuit_id);
            if (circuit.exec() && circuit.next()) {
                value.append(circuit.value(0).toString());
            }
        } else {
            value.append(expression.key(i));
        }
    }
    return value;
}