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

#include "assemblyrecordtypesview.h"

#include "global.h"
#include "mttextstream.h"
#include "records.h"
#include "viewtabsettings.h"
#include "mainwindowsettings.h"
#include "toolbarstack.h"

using namespace Global;

AssemblyRecordTypesView::AssemblyRecordTypesView(ViewTabSettings *settings):
    View(settings)
{
}

QString AssemblyRecordTypesView::renderHTML()
{
    QString highlighted_id = settings->selectedAssemblyRecordType();

    QString html; MTTextStream out(&html);
    AssemblyRecordType all_items("");
    if (!settings->toolBarStack()->isFilterEmpty()) {
        all_items.addFilter(settings->toolBarStack()->filterColumn(), settings->toolBarStack()->filterKeyword());
    }
    ListOfVariantMaps items = all_items.listAll("*", settings->mainWindowSettings().orderByForView(LinkParser::AllAssemblyRecordTypes));

    out << "<table cellspacing=\"0\" cellpadding=\"4\" style=\"width:100%;\" class=\"highlight\">";
    QString thead = "<tr>"; int thead_colspan = 2;
    for (int n = 0; n < AssemblyRecordType::attributes().count(); ++n) {
        thead.append("<th><a href=\"assemblyrecordtype:/order_by:"
                     + AssemblyRecordType::attributes().key(n) + "\">"
                     + AssemblyRecordType::attributes().value(n) + "</a></th>");
        thead_colspan++;
    }
    thead.append("</tr>");
    out << "<tr><th colspan=\"" << thead_colspan << "\" style=\"font-size: medium;\">"
        << tr("List of Assembly Record Types") << "</th></tr>";
    out << thead;
    QString id;
    for (int i = 0; i < items.count(); ++i) {
        id = items.at(i).value("id").toString();
        out << QString("<tr id=\"%1\" onclick=\"executeLink(this, '%1');\"").arg("assemblyrecordtype:" + id);
        if (highlighted_id == id) {
            out << " class=\"selected\"";
        }
        out << " style=\"cursor: pointer;\"><td><a href=\"\">" << id << "</a></td>";
        for (int n = 1; n < AssemblyRecordType::attributes().count(); ++n) {
            out << "<td>" << escapeString(items.at(i).value(AssemblyRecordType::attributes().key(n)).toString()) << "</td>";
        }
        out << "</tr>";
    }
    out << "</table>";
    return viewTemplate("assembly_record_types").arg(html);
}

QString AssemblyRecordTypesView::title() const
{
    return QApplication::translate("ToolBarStack", "List of Assembly Record Types");
}