// %Include Qt/Qtmod.sip

%Module expreditor2

%Import QtCore/QtCoremod.sip

%If (Qt_5_0_0 -)
%Import QtWidgets/QtWidgetsmod.sip
%End

%Import QtGui/QtGuimod.sip

%Feature SEEXPR_ENABLE_DEEPWATER
%Feature SEEXPR_ENABLE_OPENGL_DIALOGS

%MappedType std::string
{
%TypeHeaderCode
#include <iostream>
#include <string>
%End

%ConvertFromTypeCode
    // convert an std::string to a Python (unicode) string
    PyObject* newstring;
    newstring = PyUnicode_DecodeUTF8(sipCpp->c_str(), sipCpp->length(), NULL);
    if(newstring == NULL) {
        PyErr_Clear();

#if PY_MAJOR_VERSION < 3
        newstring = PyString_FromString(sipCpp->c_str());
#else
        newstring = PyUnicode_FromString(sipCpp->c_str());
#endif
    }
    return newstring;
%End

%ConvertToTypeCode
     // Allow a Python string (or a unicode string) whenever a string is
     // expected.
     // If argument is a Unicode string, just decode it to UTF-8
     // If argument is a Python string, assume it's UTF-8
     if (sipIsErr == NULL)
#if PY_MAJOR_VERSION < 3
                return (PyString_Check(sipPy) || PyUnicode_Check(sipPy));
#else
                return PyUnicode_Check(sipPy);
#endif
        if (sipPy == Py_None) {
                *sipCppPtr = new std::string;
                return 1;
        }
        if (PyUnicode_Check(sipPy))        {
        PyObject* s = PyUnicode_AsEncodedString(sipPy, "UTF-8", "");
#if PY_MAJOR_VERSION < 3
        *sipCppPtr = new std::string(PyString_AS_STRING(s));
#else
        *sipCppPtr = new std::string(PyUnicode_AS_DATA(s));
#endif
        Py_DECREF(s);
        return 1;
        }
#if PY_MAJOR_VERSION < 3
        if (PyString_Check(sipPy)) {
        *sipCppPtr = new std::string(PyString_AS_STRING(sipPy));
            return 1;
        }
#endif
    return 0;
%End
};

%If (SEEXPR_ENABLE_OPENGL_DIALOGS)
class ExprDialog:QDialog{
%TypeHeaderCode
#include "ExprDialog.h"
%End
public:
    ExprEditor* editor;


    ExprDialog(QWidget* parent /TransferThis/ = NULL);
    std::string getExpressionString();
    void setExpressionString(const QString&);
    void show();

Q_SIGNALS:
    void preview();
    void expressionApplied();

public Q_SLOTS:
    void applyExpression();
    void clearExpression();
    
};
%End

class ExprControlCollection:QWidget{
%TypeHeaderCode
#include "ExprControlCollection.h"
%End
public:
        ExprControlCollection(QWidget* parent /TransferThis/=NULL);
};

class ExprEditor:QWidget{
%TypeHeaderCode
#include "ExprEditor.h"
%End

public:
    ExprEditor(QWidget* parent /TransferThis/);
    virtual void setControlCollectionWidget(ExprControlCollection* widget);
    ExprControlCollection* controlCollectionWidget() const;

Q_SIGNALS:
    void applyExpression(); // This is when text or controls are manipulated
    //void needNewPreview();
    void clearExpression();

public Q_SLOTS:
    void exprChanged();
    void rebuildControls();
    void controlChanged(int id);

public:
    QString getExpr();
    void setExpr(const QString& expression,const bool apply=false);
    void addError(const int startPos,const int endPos,const QString& error);
    void clearErrors();
    void clearExtraCompleters();
    void registerExtraFunction(const QString& name, const QString& docString);
    void registerExtraVariable(const QString& name, const QString& docString);
    void updateCompleter();
};

/**

class ExprGrapher2d
{
%TypeHeaderCode
#include "ExprGrapher2d.h"
%End

public:
    ExprGrapher2d(QWidget* parent /TransferThis/, int width, int height);
    void update();
};


**/

class ExprBrowser : QWidget
{
%TypeHeaderCode
#include "ExprBrowser.h"
%End


public:
    ExprBrowser(QWidget* parent /TransferThis/, ExprEditor* editor /TransferThis/);

    void addPath(const std::string& name,const std::string& path);
    std::string getSelectedPath();
    void selectPath(const char * path);
    void setSearchPath(const QString& context, const QString& path);

public Q_SLOTS:
    void update();
    void clear();

    void saveExpression();
    void saveExpressionAs();
    void saveLocalExpressionAs();

};

%If(SEEXPR_ENABLE_OPENGL_DIALOGS)
class ExprShortTextEdit:QTextEdit
{
%TypeHeaderCode
#include "ExprShortEdit.h"
%End

public:
    ExprShortTextEdit(QWidget* parent);

Q_SIGNALS:
    void textChanged();
};


class ExprShortEdit:QWidget
{
%TypeHeaderCode
#include "ExprShortEdit.h"
%End

public:
    ExprShortEdit(QWidget* parent);
    std::string getExpressionString() const;
    void setExpressionString(const QString&);
    void clearExtraCompleters();
    void registerExtraFunction(const QString& name,const QString& docString);
    void registerExtraVariable(const QString& name,const QString& docString);
    void setSimple(bool enabled);
    void updateCompleter();
    void setDetailsMenu(QMenu *menu);
    void setSearchPath(const QString& context, const QString& path);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    virtual void hideErrors(bool hidden, const std::string &err);
    void showDetails(int idx);

    QToolButton* expandButton;
    ExprShortTextEdit* edit;

protected Q_SLOTS:
    virtual void detailPressed();
    virtual void expandPressed();
    virtual void textFinished();
    virtual void handleTextEdited();
    virtual void controlChanged(int id);
    virtual void rebuildControls();
    virtual void expressionApplied();
    virtual void dialogClosed();

Q_SIGNALS:
    void exprChanged();
};
%End

class ExprCurve:QWidget{
%TypeHeaderCode
#include "ExprCurve.h"
%End
public:
    ExprCurve(QWidget* parent = NULL, QString pLabel = "", QString vLabel = "", QString iLabel = "");
};

class ExprColorCurve:QWidget{
%TypeHeaderCode
#include "ExprColorCurve.h"
%End
public:
    ExprColorCurve(QWidget* parent = NULL, QString pLabel = "", QString vLabel = "", QString iLabel = "");
};


class ExprColorSwatchWidget:QWidget{
%TypeHeaderCode
#include "ExprColorSwatch.h"
%End
public:
    ExprColorSwatchWidget(bool indexLabel, QWidget* parent = NULL);
};

%If (SEEXPR_ENABLE_DEEPWATER)
class ExprDeepWater:QWidget{
%TypeHeaderCode
#include "ExprDeepWater.h"
%End
public:
    ExprDeepWater(QWidget* parent = NULL);
};
%End
