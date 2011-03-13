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

#ifndef HTMLBUILDER_H
#define HTMLBUILDER_H

#include <QList>
#include <QString>

class HTMLTable;
class HTMLTableRow;
class HTMLTableCell;
class HTMLHeaderTableCell;
class HTMLBold;
class HTMLItalics;
class HTMLLink;
class HTMLHeading;
class HTMLSubHeading;
class HTMLSubSubHeading;
class HTMLParagraph;

class HTMLElement
{
public:
    virtual const QString html() = 0;
};

class HTMLDataElement : public HTMLElement
{
public:
    HTMLDataElement(const QString &);

    virtual const QString html();

protected:
    QString body;
};

class HTMLParentElement : public HTMLElement
{
public:
    HTMLParentElement(const QString & = QString());
    ~HTMLParentElement();

    virtual const QString html();
    inline const QString tagName() { return tag_name; }

    HTMLBold * bold();
    HTMLItalics * italics();
    HTMLLink * link(const QString &, const QString & = QString());
    HTMLTable * table(const QString & = QString());
    HTMLHeading * heading();
    HTMLSubHeading * subHeading();
    HTMLSubSubHeading * subSubHeading();
    HTMLParagraph * paragraph(const QString & = QString());

    void newLine() { children.append(new HTMLDataElement("<br>")); }

    HTMLParentElement & operator<<(const QString &);
    HTMLParentElement & operator<<(HTMLElement *);

protected:
    QList<HTMLElement *> children;
    QString args;
    QString tag_name;
};

class HTMLTable : public HTMLParentElement
{
public:
    HTMLTable(const QString & = QString());

    HTMLTableRow * addRow(const QString & = QString());

    const QString customHtml(int);
};

class HTMLTableRow : public HTMLParentElement
{
public:
    HTMLTableRow(const QString & = QString());

    HTMLTableCell * addCell(const QString & = QString());
    HTMLHeaderTableCell * addHeaderCell(const QString & = QString());

    const QString customHtml(int, int, bool &);
};

class HTMLTableCell : public HTMLParentElement
{
public:
    HTMLTableCell(const QString & = QString());
};

class HTMLHeaderTableCell : public HTMLTableCell
{
public:
    HTMLHeaderTableCell(const QString & = QString());
};

class HTMLDiv : public HTMLParentElement
{
public:
    HTMLDiv(const QString & = QString());
};

class HTMLBold : public HTMLParentElement
{
public:
    HTMLBold();
};

class HTMLItalics : public HTMLParentElement
{
public:
    HTMLItalics();
};

class HTMLLink : public HTMLParentElement
{
public:
    HTMLLink(const QString &, const QString & = QString());
};

class HTMLHeading : public HTMLParentElement
{
public:
    HTMLHeading();
};

class HTMLSubHeading : public HTMLParentElement
{
public:
    HTMLSubHeading();
};

class HTMLSubSubHeading : public HTMLParentElement
{
public:
    HTMLSubSubHeading();
};

class HTMLParagraph : public HTMLParentElement
{
public:
    HTMLParagraph(const QString & = QString());
};

#endif // HTMLBUILDER_H