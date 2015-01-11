/*******************************************************************
 This file is part of Leaklog
 Copyright (C) 2008-2015 Matus & Michal Tomlein

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

#ifndef PARTNER_WIDGETS_H
#define PARTNER_WIDGETS_H

class MDLineEdit;
class MDComboBox;
class QWidget;
class QString;

#include <QObject>

class PartnerWidgets : public QObject
{
    Q_OBJECT

public:
    PartnerWidgets(const QString &, const QString &, QWidget *);

    MDLineEdit *partnerIdWidget() { return partner_id_le; }
    MDLineEdit *partnerNameWidget() { return partner_name_le; }
    MDComboBox *partnersWidget() { return partners_cb; }

private slots:
    void partnerChanged(int);

private:
    MDLineEdit *partner_id_le;
    MDLineEdit *partner_name_le;
    MDComboBox *partners_cb;
};

#endif // PARTNER_WIDGETS_H
