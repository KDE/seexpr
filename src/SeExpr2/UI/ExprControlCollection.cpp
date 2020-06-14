/*
* Copyright Disney Enterprises, Inc.  All rights reserved.
* Copyright (C) 2020 L. E. Segovia <amy@amyspark.me>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License
* and the following modification to it: Section 6 Trademarks.
* deleted and replaced with:
*
* 6. Trademarks. This License does not grant permission to use the
* trade names, trademarks, service marks, or product names of the
* Licensor and its affiliates, except as required for reproducing
* the content of the NOTICE file.
*
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
*
* @file ExprControlCollection.cpp
* @brief Manages/creates a bunch of ExprControls by using expression text
* @author  aselle
*/
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QRadioButton>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QLabel>
#include "ExprEditor.h"
#include "ExprHighlighter.h"
#include "ExprCompletionModel.h"
#include "ExprCurve.h"
#include "ExprColorCurve.h"
#include "ExprControl.h"
#include "ExprControlCollection.h"
#include "EditableExpression.h"
#include "Editable.h"

ExprControlCollection::ExprControlCollection(QWidget* parent, bool showAddButton)
    : QWidget(parent), count(0), showAddButton(showAddButton), editableExpression(0) {
    controlLayout = new QVBoxLayout();
    controlLayout->setMargin(0);
    controlLayout->setSpacing(0);
    controlLayout->insertStretch(-1, 100);

    if (showAddButton) {
        QPushButton* button = new QPushButton(tr("Add Widget"));
        button->setFocusPolicy(Qt::NoFocus);
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->insertStretch(-1, 100);
        buttonLayout->addWidget(button, 0);
        controlLayout->addLayout(buttonLayout);
        connect(button, SIGNAL(clicked()), SLOT(addControlDialog()));
    }
    setLayout(controlLayout);
}

ExprControlCollection::~ExprControlCollection() { delete editableExpression; }

ExprAddDialog::ExprAddDialog(int& count, QWidget* parent) : QDialog(parent) {
    QVBoxLayout* verticalLayout;
    verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(3);
    verticalLayout->setMargin(3);
    setLayout(verticalLayout);
    QHBoxLayout* horizontalLayout = new QHBoxLayout();

    horizontalLayout->addWidget(new QLabel(tr("Variable")));
    // TODO would be nice to unique this over multiple sessions
    variableName = new QLineEdit(QString::fromLatin1("$var%1").arg(count++));

    horizontalLayout->addWidget(variableName);
    verticalLayout->addLayout(horizontalLayout);

    tabWidget = new QTabWidget();

    // Curve
    {
        QWidget* curveTab = new QWidget();
        QFormLayout* curveLayout = new QFormLayout(curveTab);
        curveLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Lookup")));
        curveLookup = new QLineEdit(tr("$u"));
        curveLayout->setWidget(0, QFormLayout::FieldRole, curveLookup);
        tabWidget->addTab(curveTab, QString(tr("Curve")));
    }

    // Color Curve
    {
        QWidget* colorCurveTab = new QWidget();
        QFormLayout* colorCurveLayout = new QFormLayout(colorCurveTab);
        colorCurveLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Lookup")));
        colorCurveLookup = new QLineEdit(tr("$u"));
        colorCurveLayout->setWidget(0, QFormLayout::FieldRole, colorCurveLookup);
        tabWidget->addTab(colorCurveTab, QString(tr("Color Curve")));
    }

    // Integer
    {
        QWidget* intTab = new QWidget();
        QFormLayout* intFormLayout = new QFormLayout(intTab);
        intFormLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Default")));
        intFormLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("Min")));
        intFormLayout->setWidget(2, QFormLayout::LabelRole, new QLabel(tr("Max")));
        intDefault = new QLineEdit(tr("0"));
        intFormLayout->setWidget(0, QFormLayout::FieldRole, intDefault);
        intMin = new QLineEdit(tr("0"));
        intFormLayout->setWidget(1, QFormLayout::FieldRole, intMin);
        intMax = new QLineEdit(tr("10"));
        intFormLayout->setWidget(2, QFormLayout::FieldRole, intMax);
        tabWidget->addTab(intTab, QString(tr("Int")));
    }

    // Float
    {
        QWidget* floatTab = new QWidget();
        QFormLayout* floatFormLayout = new QFormLayout(floatTab);
        floatFormLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Default")));
        floatFormLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("Min")));
        floatFormLayout->setWidget(2, QFormLayout::LabelRole, new QLabel(tr("Max")));
        floatDefault = new QLineEdit(tr("0"));
        floatFormLayout->setWidget(0, QFormLayout::FieldRole, floatDefault);
        floatMin = new QLineEdit(tr("0"));
        floatFormLayout->setWidget(1, QFormLayout::FieldRole, floatMin);
        floatMax = new QLineEdit(tr("1"));
        floatFormLayout->setWidget(2, QFormLayout::FieldRole, floatMax);

        tabWidget->addTab(floatTab, QString(tr("Float")));
    }

    // Vector
    {
        QWidget* vectorTab = new QWidget();
        QFormLayout* vectorFormLayout = new QFormLayout(vectorTab);
        vectorFormLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Default")));
        vectorFormLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("Min")));
        vectorFormLayout->setWidget(2, QFormLayout::LabelRole, new QLabel(tr("Max")));
        vectorDefault0 = new QLineEdit(tr("0"));
        vectorDefault1 = new QLineEdit(tr("0"));
        vectorDefault2 = new QLineEdit(tr("0"));
        QHBoxLayout* compLayout = new QHBoxLayout();
        compLayout->addWidget(vectorDefault0);
        compLayout->addWidget(vectorDefault1);
        compLayout->addWidget(vectorDefault2);
        vectorFormLayout->setLayout(0, QFormLayout::FieldRole, compLayout);
        vectorMin = new QLineEdit(tr("0"));
        vectorFormLayout->setWidget(1, QFormLayout::FieldRole, vectorMin);
        vectorMax = new QLineEdit(tr("1"));
        vectorFormLayout->setWidget(2, QFormLayout::FieldRole, vectorMax);

        tabWidget->addTab(vectorTab, QString(tr("Vector")));
    }

    // Color
    {
        QWidget* colorTab = new QWidget();
        QFormLayout* colorLayout = new QFormLayout(colorTab);
        colorWidget = new QPushButton();
        colorWidget->setFixedWidth(30);
        colorWidget->setFixedWidth(30);
        colorLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Color")));
        colorLayout->setWidget(0, QFormLayout::FieldRole, colorWidget);
        color = Qt::red;
        QPixmap colorPix(30, 30);
        colorPix.fill(color);
        colorWidget->setIcon(QIcon(colorPix));
        tabWidget->addTab(colorTab, QString(tr("Color")));

        connect(colorWidget, SIGNAL(clicked()), this, SLOT(colorChooseClicked()));
    }

    // Color Swatch
    {
        QWidget* swatchTab = new QWidget();
        QFormLayout* swatchLayout = new QFormLayout(swatchTab);
        swatchLookup = new QLineEdit(tr("$u"));
        swatchLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Lookup")));
        swatchLayout->setWidget(0, QFormLayout::FieldRole, swatchLookup);
        rainbowPaletteBtn = new QRadioButton(tr("Rainbow"));
        rainbowPaletteBtn->setChecked(true);
        grayPaletteBtn = new QRadioButton(tr("Shades of Gray"));
        swatchLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("Colors")));
        swatchLayout->setWidget(1, QFormLayout::FieldRole, rainbowPaletteBtn);
        swatchLayout->setWidget(2, QFormLayout::LabelRole, new QLabel(QString()));
        swatchLayout->setWidget(2, QFormLayout::FieldRole, grayPaletteBtn);
        tabWidget->addTab(swatchTab, QString(tr("Swatch")));
    }

    // String literal
    {
        QWidget* stringTab = new QWidget();
        QFormLayout* stringLayout = new QFormLayout(stringTab);
        stringTypeWidget = new QComboBox();
        stringTypeWidget->addItem(tr("string"));
        stringTypeWidget->addItem(tr("file"));
        stringTypeWidget->addItem(tr("directory"));
        stringDefaultWidget = new QLineEdit();
        stringNameWidget = new QLineEdit(QString::fromLatin1("str1"));

        stringLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("String Name")));
        stringLayout->setWidget(0, QFormLayout::FieldRole, stringNameWidget);
        stringLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("String Type")));
        stringLayout->setWidget(1, QFormLayout::FieldRole, stringTypeWidget);
        stringLayout->setWidget(2, QFormLayout::LabelRole, new QLabel(tr("String Default")));
        stringLayout->setWidget(3, QFormLayout::FieldRole, stringDefaultWidget);

        tabWidget->addTab(stringTab, QString(tr("String")));
    }
#ifdef SEEXPR_ENABLE_ANIMCURVE
    // Anim Curve
    {
        QWidget* curveTab = new QWidget();
        QFormLayout* curveLayout = new QFormLayout(curveTab);
        curveLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Lookup")));
        curveLayout->setWidget(1, QFormLayout::LabelRole, new QLabel(tr("Link")));
        animCurveLookup = new QLineEdit(QString::fromLatin1("$frame"));
        animCurveLink = new QLineEdit(QString());
        curveLayout->setWidget(0, QFormLayout::FieldRole, animCurveLookup);
        curveLayout->setWidget(1, QFormLayout::FieldRole, animCurveLink);
        tabWidget->addTab(curveTab, QString(tr("AnimCurve")));
    }
#endif
#ifdef SEEXPR_ENABLE_DEEPWATER
    // DeepWater
    {
        QWidget* deepWaterTab = new QWidget();
        QFormLayout* deepWaterLayout = new QFormLayout(deepWaterTab);
        deepWaterLayout->setWidget(0, QFormLayout::LabelRole, new QLabel(tr("Lookup")));
        deepWaterLookup = new QLineEdit(tr("$u"));
        deepWaterLayout->setWidget(0, QFormLayout::FieldRole, deepWaterLookup);
        tabWidget->addTab(deepWaterTab, QString(tr("Deep Water")));
    }
#endif

    verticalLayout->addWidget(tabWidget);

    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    verticalLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    tabWidget->setCurrentIndex(0);
}

void ExprAddDialog::colorChooseClicked() {
    color = QColorDialog::getColor(color);
    if (color.isValid()) {
        QPixmap colorPix(30, 30);
        colorPix.fill(color);
        ((QPushButton*)sender())->setIcon(QIcon(colorPix));
    }
}

const char* ExprAddDialog::initSwatch() {
    if (rainbowPaletteBtn->isChecked())
        return ("[1,0,0],[1,.6,0],[1,1,0],[0,1,0],[0,1,1],[0,0,1],[.6,.1,.6],[1,0,1],[1,1,1],[0,0,0]");
    else if (grayPaletteBtn->isChecked())
        return (
            "[1,1,1],[.9,.9,.9],[.8,.8,.8],[.7,.7,.7],[.6,.6,.6],[.5,.5,.5],[.4,.4,.4],[.3,.3,.3],[.2,.2,.2],[0,0,0]");
    else
        return ("[1,1,1],[.5,.5,.5],[0,0,0]");
}

void ExprControlCollection::addControlDialog() {
    ExprAddDialog* dialog = new ExprAddDialog(count, this);
    if (dialog->exec()) {
        QString s;
        switch (dialog->tabWidget->currentIndex()) {
            case 0:
                s = QString::fromLatin1("%1 = curve(%2,0,0,4,1,1,4);\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->curveLookup->text());
                break;
            case 1:
                s = QString::fromLatin1("%1 = ccurve(%2,0,[0,0,0],4,1,[1,1,1],4);\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->colorCurveLookup->text());
                break;
            case 2:
                s = QString::fromLatin1("%1 = %2; # %3,%4\n")
                    .arg(dialog->variableName->text())
                    .arg(dialog->intDefault->text())
                    .arg(dialog->intMin->text())
                    .arg(dialog->intMax->text());
                break;
            case 3:
                s = QString::fromLatin1("%1 = %2; # %3, %4\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->floatDefault->text())
                        .arg(crack_atof(dialog->floatMin->text().toStdString().c_str()), 0, 'f', 3)
                        .arg(crack_atof(dialog->floatMax->text().toStdString().c_str()), 0, 'f', 3);
                break;
            case 4:
                s = QString::fromLatin1("%1 = [%2,%3,%4]; # %5, %6\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->vectorDefault0->text())
                        .arg(dialog->vectorDefault1->text())
                        .arg(dialog->vectorDefault2->text())
                        .arg(crack_atof(dialog->vectorMin->text().toStdString().c_str()), 0, 'f', 3)
                        .arg(crack_atof(dialog->vectorMax->text().toStdString().c_str()), 0, 'f', 3);
                break;
            case 5:
                s = QString::fromLatin1("%1 = [%2,%3,%4];\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->color.redF())
                        .arg(dialog->color.greenF())
                        .arg(dialog->color.blueF());
                break;
            case 6:
                s = QString::fromLatin1("%1 = swatch(%2,%3);\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->swatchLookup->text())
                        .arg(QString::fromLatin1(dialog->initSwatch()));
                break;
            case 7:
                s = QString::fromLatin1("\"%1\" #%2 %3\n")
                        .arg(dialog->stringDefaultWidget->text())
                        .arg(dialog->stringTypeWidget->currentText())
                        .arg(dialog->stringNameWidget->text());
                break;
#ifdef SEEXPR_ENABLE_ANIMCURVE
            case 8:
                s = QString::fromLatin1("%1 = animCurve(%2,\"constant\",\"constant\",0,\"%3\");")
                        .arg(dialog->variableName->text())
                        .arg(dialog->animCurveLookup->text())
                        .arg(dialog->animCurveLink->text());
                break;
#endif
#ifdef SEEXPR_ENABLE_DEEPWATER
            case 9:
                s = QString::fromLatin1("%1 = deepWater(%2,9,30,0,1,0,5,0,0,[0,0,0],0,0,0);\n")
                        .arg(dialog->variableName->text())
                        .arg(dialog->deepWaterLookup->text());
                break;
#endif
        }
        emit insertString(s.toStdString());
    }
}

bool ExprControlCollection::rebuildControls(const QString& expressionText, std::vector<QString>& variables) {
    // parse a new editable expression so  we can check if we need to make new controls
    EditableExpression* newEditable = new EditableExpression;
    newEditable->setExpr(expressionText.toStdString());

    // check for new variables

    bool newVariables = true;
    if (editableExpression && editableExpression->getVariables() == newEditable->getVariables()) newVariables = false;
    if (newVariables) {
        const std::vector<std::string>& vars = newEditable->getVariables();
        variables.clear();
        for (size_t k = 0; k < vars.size(); k++) {
            variables.push_back(QString::fromLatin1("$%1").arg(QString::fromStdString(vars[k])));
        }
    }

    if (newEditable->size() == 0 && !editableExpression) return false;

    if (editableExpression && editableExpression->controlsMatch(*newEditable)) {
        // controls match so we only need to update positions (i.e. if the user typed and shifted some controls)
        editableExpression->updateString(*newEditable);
        delete newEditable;
    } else {
        // controls did not match

        // delete old controls
        for (unsigned int i = 0; i < _controls.size(); i++) {
            controlLayout->removeWidget(_controls[i]);
            delete _controls[i];
        }
        _linkedId = -1;
        _controls.clear();

        // swap to new editable expression
        delete editableExpression;
        editableExpression = newEditable;

        // build new controls
        for (size_t i = 0; i < editableExpression->size(); i++) {
            Editable* editable = (*editableExpression)[i];
            ExprControl* widget = 0;
            // Create control "factory" (but since its only used here...)
            if (NumberEditable* x = dynamic_cast<NumberEditable*>(editable))
                widget = new NumberControl(i, x);
            else if (VectorEditable* x = dynamic_cast<VectorEditable*>(editable))
                widget = new VectorControl(i, x);
            else if (StringEditable* x = dynamic_cast<StringEditable*>(editable))
                widget = new StringControl(i, x);
            else if (CurveEditable* x = dynamic_cast<CurveEditable*>(editable))
                widget = new CurveControl(i, x);
            else if (ColorCurveEditable* x = dynamic_cast<ColorCurveEditable*>(editable))
                widget = new CCurveControl(i, x);
#ifdef SEEXPR_ENABLE_ANIMCURVE
            else if (AnimCurveEditable* x = dynamic_cast<AnimCurveEditable*>(editable)) 
                widget = new AnimCurveControl(i, x);
#endif
            else if (ColorSwatchEditable* x = dynamic_cast<ColorSwatchEditable*>(editable))
                widget = new ColorSwatchControl(i, x);
#ifdef SEEXPR_ENABLE_DEEPWATER
            else if (DeepWaterEditable* x = dynamic_cast<DeepWaterEditable*>(editable))
                widget = new DeepWaterControl(i, x);
#endif
            else {
                std::cerr << "SeExpr editor logic error, cannot find a widget for the given editable" << std::endl;
            }
            if (widget) {
                // successfully made widget
                int insertPoint = controlLayout->count() - 1;
                if (showAddButton) insertPoint--;
                controlLayout->insertWidget(insertPoint, widget);
                _controls.push_back(widget);
                connect(widget, SIGNAL(controlChanged(int)), SLOT(singleControlChanged(int)));
                connect(widget, SIGNAL(linkColorEdited(int, QColor)), SLOT(linkColorEdited(int, QColor)));
                connect(widget, SIGNAL(linkColorLink(int)), SLOT(linkColorLink(int)));
            } else {
                std::cerr << "Expr Editor Logic ERROR did not make widget" << std::endl;
            }
        }
    }
    return newVariables;
}

void ExprControlCollection::showEditor(int idx) {
#ifdef SEEXPR_ENABLE_ANIMCURVE
    if (idx < 0 || idx >= (int)_controls.size()) return;

    /* Right now we only launch the anim curve editor.
     * It would be better to launch them generically. */
    AnimCurveControl* control = dynamic_cast<AnimCurveControl*>(_controls[idx]);
    if (!control) return;

    control->editGraphClicked();
#endif
}

void ExprControlCollection::linkColorLink(int id) {
    _linkedId = id;
    for (unsigned int i = 0; i < _controls.size(); i++) {
        _controls[i]->linkDisconnect(_linkedId);
    }
}

void ExprControlCollection::linkColorEdited(int id, QColor color) {
    if (id == _linkedId) emit linkColorOutput(color);
}

void ExprControlCollection::linkColorInput(QColor color) {
    // TODO: fix
    if (_linkedId < 0 || _linkedId >= (int)_controls.size()) return;
    _controls[_linkedId]->setColor(color);
}

void ExprControlCollection::updateText(const int id, QString& text) {
    Q_UNUSED(id);
    if (editableExpression) text = QString::fromStdString(editableExpression->getEditedExpr());
}

void ExprControlCollection::singleControlChanged(int id) { emit controlChanged(id); }
