#ifndef WIDGET_H
#define WIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#define PORTABLE 1

#include <vector>

using namespace std;

#include "ui_widget.h"
#include "ui_cabledialog.h"

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

class Widget : public QWidget, private Ui::Widget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);

protected:
    QList<Cable> cables;
    static Band baender[];
    void changeEvent(QEvent *e);
    void CalcDamping();
    void ReadCables();
    void AppendCableWithFilter(Cable cable);
    bool IsLink(QString s){if(s.isEmpty()||!s.startsWith("http"))return false;else return true;}
public slots:
    void on_newButton_clicked();
    void on_changeButton_clicked();
    void on_shopButton_clicked();
    void on_comboBoxBand_currentIndexChanged(int index);
    void on_comboBoxLang_currentIndexChanged(int index);
    //void on_comboBoxCable_currentIndexChanged(int index);
    void on_spinBoxF_valueChanged(double newVal);
    void on_spinBoxL_valueChanged(double newVal);
    void on_spinBoxPower_valueChanged(int newVal);
    void on_webButton_clicked();
    void on_dataSheetButton_clicked();
    void on_cableView_currentRowChanged(int row);
    void on_groupBoxFilter_toggled(){ReadCables();}
    void on_checkBoxSemiRigid_toggled(){ReadCables();}
    void on_checkBox3mm_toggled(){ReadCables();}
    void on_checkBox5mm_toggled(){ReadCables();}
    void on_checkBox7mm_toggled(){ReadCables();}
    void on_checkBox10mm_toggled(){ReadCables();}
    void on_checkBox15mm_toggled(){ReadCables();}
    void on_checkBox50_toggled(){ReadCables();}
    void on_checkBox60_toggled(){ReadCables();}
    void on_checkBox75_toggled(){ReadCables();}
    void on_checkBoxImp_toggled(){ReadCables();}
};

class VTable:public QAbstractTableModel{
Q_OBJECT        
protected:
        public:
        double f[CBL_ROWS],d[CBL_ROWS];
        bool changed;
        VTable();
        // die Funktionen für das Modell
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
    Q_OBJECT

public:
    explicit CableDialog(QWidget *parent = 0);
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
