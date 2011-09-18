/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2011 Matus & Michal Tomlein

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

#include "edit_circuit_dialogue_compressors_tab.h"
#include "edit_dialogue_table.h"
#include "records.h"
#include "global.h"

EditCircuitDialogueCompressorsTab::EditCircuitDialogueCompressorsTab(const QString & customer_id, const QString & circuit_id, QWidget * parent)
    : EditDialogueTab(parent),
      customer_id(customer_id)
{
    setName(tr("Compressors"));

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(9);

    QList<EditDialogueTableCell *> cells;
    EditDialogueTableCell * cell = new EditDialogueTableCell(tr("Name"), Global::String);
    cell->setId("name");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Manufacturer"), Global::String);
    cell->setId("manufacturer");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Type"), Global::String);
    cell->setId("type");
    cells.append(cell);
    cell = new EditDialogueTableCell(tr("Serial number"), Global::String);
    cell->setId("sn");
    cells.append(cell);
    compressors_table = new EditDialogueBasicTable(tr("Compressors"), cells, this);
    layout->addWidget(compressors_table);

    load(circuit_id);
}

void EditCircuitDialogueCompressorsTab::save(const QVariant & circuit_id)
{
    QList<MTDictionary> all_values = compressors_table->allValues();

    for (int i = 0; i < all_values.count(); ++i) {
        Compressor compressor_rec;
        if (all_values.at(i).contains("id")) {
            compressor_rec = Compressor(all_values.at(i).value("id"));
            if (former_ids.contains(all_values.at(i).value("id").toInt()))
                former_ids.removeAll(all_values.at(i).value("id").toInt());
        } else {
            compressor_rec = Compressor(QString());
        }

        QVariantMap map;
        map.insert("customer_id", customer_id);
        map.insert("circuit_id", circuit_id);
        map.insert("name", all_values.at(i).value("name"));
        map.insert("manufacturer", all_values.at(i).value("manufacturer"));
        map.insert("type", all_values.at(i).value("type"));
        map.insert("sn", all_values.at(i).value("sn"));
        compressor_rec.update(map);
    }
    for (int i = 0; i < former_ids.count(); ++i) {
        Compressor compressor_rec(QString::number(former_ids.at(i)));
        compressor_rec.remove();
    }
}

void EditCircuitDialogueCompressorsTab::load(const QString & circuit_id)
{
    if (!circuit_id.isEmpty()) {
        Compressor compressor_rec(QString(), MTDictionary(QStringList() << "customer_id" << "circuit_id",
                                                                       QStringList() << customer_id << circuit_id));

        EditDialogueTableCell * cell;
        QMap<QString, EditDialogueTableCell *> compressors_data;
        ListOfVariantMaps compressors = compressor_rec.listAll();
        for (int i = 0; i < compressors.count(); ++i) {
            former_ids.append(compressors.at(i).value("id").toInt());

            cell = new EditDialogueTableCell(compressors.at(i).value("name"), Global::String);
            cell->setId("name");
            compressors_data.insert("name", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("manufacturer"), Global::String);
            cell->setId("manufacturer");
            compressors_data.insert("manufacturer", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("type"), Global::String);
            cell->setId("type");
            compressors_data.insert("type", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("sn"), Global::String);
            cell->setId("sn");
            compressors_data.insert("sn", cell);
            cell = new EditDialogueTableCell(compressors.at(i).value("id"), "id");
            compressors_data.insert("id", cell);
            compressors_table->addRow(compressors_data);
        }
    }

    if (!compressors_table->rowsCount()) compressors_table->addNewRow();
}