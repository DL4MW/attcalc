#ifndef WIDGET_H
#define WIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>

#define PORTABLE 1

#include <vector>

using namespace std;

#include "ui_widget.h"
#ifdef Q_OS_ANDROID
#include "ui_cabledialog_android.h"
#else
#include "ui_cabledialog.h"
#endif

#define CBL_ROWS 15

struct Cable{
    QString name, remarks, dealer, manufacturer, filename, dataSheet;
    int diameter,impedance;
    double f[CBL_ROWS],d[CBL_ROWS];
    bool used[CBL_ROWS];
    double k1,k2,k3,residualError;
    void Write();
    void Read(QString filename);
    double HighF();
    double LowF();
    Cable();
};

typedef struct Band{
    QString band;
    double f;
} Band;

typedef struct IndexNames{
    QString name;
    int index;
}IndexName;

#define INDEX_ROLE Qt::UserRole+1

class MyListItem:public QListWidgetItem{
protected:
    int index;
public:
    virtual QVariant data(int role) const override;
    virtual void setData(int role, const QVariant &value) override;
};

// Vorwärtsdeklaration – vollständige Definition in cableselect.h
class CableSelectDialog;

class Widget : public QWidget, private Ui::Widget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

protected:
    static Band baender[];
    Cable m_selectedCable;
    bool  m_cableSelected = false;
    CableSelectDialog *m_cableSelectDialog = nullptr;

    void changeEvent(QEvent *e);
    void CalcDamping();

public slots:
    void on_selectCableButton_clicked();
    void on_comboBoxBand_currentIndexChanged(int index);
    void on_comboBoxLang_currentIndexChanged(int index);
    void on_spinBoxF_valueChanged(double newVal);
    void on_spinBoxL_valueChanged(double newVal);
    void on_spinBoxPower_valueChanged(int newVal);
    void on_webButton_clicked();
    void onFirstStart();
};

class VTable:public QAbstractTableModel{
Q_OBJECT
protected:
    public:
    double f[CBL_ROWS],d[CBL_ROWS];
    bool changed;
    VTable();
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole )const;
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole )const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
};

class CableDialog : public QDialog, private Ui::CableDialog
{
   friend class Widget;
   friend class CableSelectDialog;
    Q_OBJECT

public:
    explicit CableDialog(QWidget *parent = nullptr);
    Cable GetCable();
    void SetCable(Cable &cable);
    static double Damping(double f,double k1,double k2, double k3);
    static double Error(vector<double> x);
protected:
    void Calc();
    double k1,k2,k3,err;
    static int counter;
    static double f[CBL_ROWS],d[CBL_ROWS];
    VTable tableModel;
    QString filename;
public slots:
    void on_calcButton_clicked(){Calc();}
};

#endif // WIDGET_H
