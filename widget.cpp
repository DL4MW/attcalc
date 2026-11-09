#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "widget.h"
#include "cableselect.h"
#include "dirmngr.h"

#include <vector>
#include <math.h>
#include <stdio.h>
#include "Simplex.h"
#include "lmcurve.h"

#define USE_FIT 1

using namespace std;

FILE *file=NULL;

extern DirMngr dirMngr;
extern QTranslator translator;

Band Widget::baender[]={
    {"160m",1.8},
    {"80m",3.6},
    {"40m",7.1},
    {"30m",10.1},
    {"20m",14.2},
    {"17m",18.1},
    {"15m",21.2},
    {"12m",24.9},
    {"11m CB",27.0},
    {"10m",28.5},
    {"6m",50.5},
    {"2m",145},
    {"70cm",435},
    {"23cm",1290},
    {"13cm",2400},
    {"9cm",3400},
    {"6cm",5660},
    {"3cm",10000},
    {"",0}
};

IndexName diameter[]={
    {"semi-flex/rigid",1},
    {"<= 3mm", 2},
    {"5mm",3},
    {"7mm", 4},
    {"10mm",5},
    {">=11mm",6},
    {"",0}
};

IndexName impedance[]={
    {"50 Ohm",1},
    {"60 Ohm", 2},
    {"75 Ohm",3},
    {"other", 100},
    {"",0}
};

double CableDialog::d[CBL_ROWS],CableDialog::f[CBL_ROWS];
int CableDialog::counter;

// ──────────────────────────────────────────────
// Widget
// ──────────────────────────────────────────────

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    Band *b = baender;
    while (b->f > 0.0) {
        comboBoxBand->addItem(b->band, b->f);
        b++;
    }
    warnLabel->setVisible(false);
    cableLabel->setText(tr("--- kein Kabel gewählt ---"));

    // Beim ersten Start (leeres Kabelverzeichnis) automatisch herunterladen
    QDir cableDir(dirMngr.GetDir(DIR_DATA1));
    bool noCables = cableDir.entryList(QStringList("*.cbl"), QDir::Files).isEmpty();
    if (noCables)
        QTimer::singleShot(0, this, &Widget::onFirstStart);
}

void Widget::onFirstStart()
{
    QMessageBox::information(this, tr("Kabelbibliothek"),
        tr("Beim ersten Start werden die Kabeldaten vom Server heruntergeladen.\n"
           "Bitte stellen Sie sicher, dass eine Internetverbindung besteht."));

    if (!m_cableSelectDialog)
        m_cableSelectDialog = new CableSelectDialog(this);

#ifdef Q_OS_ANDROID
    m_cableSelectDialog->showMaximized();
#endif
    m_cableSelectDialog->downloadCables();
    m_cableSelectDialog->exec();
}

void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::LanguageChange)
        retranslateUi(this);
}

void Widget::CalcDamping()
{
    double f = spinBoxF->value();
    double l = spinBoxL->value();

    if (!m_cableSelected || f < 0) {
        resultLabel->setText(" *** ");
        powerLabel->setText(" *** ");
        warnLabel->setVisible(false);
        return;
    }

    double result = l * (m_selectedCable.k1
                       + m_selectedCable.k2 * f
                       + m_selectedCable.k3 * sqrt(f));
    if (result < 0.0) result = 0.0;
    resultLabel->setText(QString("%1").arg(result, 0, 'f', 2));

    double power = double(spinBoxPower->value()) / pow(10.0, result / 10.0);
    powerLabel->setText(QString("%1").arg(power, 0, 'f', 1));

    if      (f < m_selectedCable.LowF()  / 5.0) warnLabel->setVisible(true);
    else if (f > m_selectedCable.HighF() * 5.0) warnLabel->setVisible(true);
    else                                          warnLabel->setVisible(false);
}

void Widget::on_selectCableButton_clicked()
{
    // Dialog wird als persistentes Member gehalten, damit Filter-Zustand erhalten bleibt
    if (!m_cableSelectDialog)
        m_cableSelectDialog = new CableSelectDialog(this);

    if (m_cableSelected)
        m_cableSelectDialog->setCurrentCable(m_selectedCable);

#ifdef Q_OS_ANDROID
    m_cableSelectDialog->showMaximized();
#endif
    if (m_cableSelectDialog->exec() == QDialog::Accepted) {
        m_selectedCable = m_cableSelectDialog->selectedCable();
        m_cableSelected = true;
        cableLabel->setText(m_selectedCable.name);
        CalcDamping();
    }
}

void Widget::on_comboBoxBand_currentIndexChanged(int index)
{
    spinBoxF->setValue(baender[index].f);
}

void Widget::on_comboBoxLang_currentIndexChanged(int index)
{
    if (index == 2) qApp->installTranslator(&translator);
    else if (index == 1) qApp->removeTranslator(&translator);
}

void Widget::on_spinBoxF_valueChanged(double)  { CalcDamping(); }
void Widget::on_spinBoxL_valueChanged(double)  { CalcDamping(); }
void Widget::on_spinBoxPower_valueChanged(int) { CalcDamping(); }

void Widget::on_webButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://schueler.ws/?page_id=237"));
}

// ──────────────────────────────────────────────
// Cable
// ──────────────────────────────────────────────

void Cable::Write()
{
    QSettings settings(filename,QSettings::IniFormat);
    settings.setValue("name",name);
    settings.setValue("remarks",remarks);
    settings.setValue("dealer",dealer);
    settings.setValue("datasheet",dataSheet);
    settings.setValue("manufacturer",manufacturer);
    settings.setValue("impedance",impedance);
    settings.setValue("diameter",diameter);
    settings.setValue("k1",k1);
    settings.setValue("k2",k2);
    settings.setValue("k3",k3);
    settings.setValue("error",residualError);
    for(int i=0;i<CBL_ROWS;i++){
        settings.setValue(QString("f%1").arg(i),f[i]);
        settings.setValue(QString("d%1").arg(i),d[i]);
    }
}

void Cable::Read(QString _filename)
{
    filename=_filename;
    QSettings settings(filename,QSettings::IniFormat);
    name=settings.value("name",name).toString();
    remarks=settings.value("remarks",remarks).toString();
    dealer=settings.value("dealer",dealer).toString();
    dataSheet=settings.value("datasheet",dataSheet).toString();
    manufacturer=settings.value("manufacturer",manufacturer).toString();
    impedance=settings.value("impedance",1).toInt();
    diameter=settings.value("diameter",5).toInt();
    k1=settings.value("k1",k1).toDouble();
    k2=settings.value("k2",k2).toDouble();
    k3=settings.value("k3",k3).toDouble();
    residualError=settings.value("error",0).toDouble();
    for(int i=0;i<CBL_ROWS;i++){
        f[i]=settings.value(QString("f%1").arg(i),f[i]).toDouble();
        d[i]=settings.value(QString("d%1").arg(i),d[i]).toDouble();
    }
}

Cable::Cable()
{
    name="";
    remarks="";
    dealer="";
    manufacturer="";
    impedance=1;
    diameter=5;
#ifdef PORTABLE
    filename=dirMngr.CreatePath(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss.cbl"),DIR_DATA1);
#else
    filename=dirMngr.CreatePath(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss.cbl"),DIR_DATA2);
#endif
    for(int i=0;i<CBL_ROWS;i++){
        f[i]=0.0;
        d[i]=0.0;
        used[i]=false;
    }
    k1=1.0;
    k2=1.0;
}

double Cable::HighF()
{
    double fx=0;
    for(int i=0;i<CBL_ROWS;i++)
        if(f[i]>0.0 && f[i]>fx) fx=f[i];
    return fx;
}

double Cable::LowF()
{
    double fx=0;
    for(int i=0;i<CBL_ROWS;i++)
        if(f[i]>0.0){ if(fx<=0.0) fx=f[i]; else if(f[i]<fx) fx=f[i]; }
    return fx;
}

// ──────────────────────────────────────────────
// CableDialog
// ──────────────────────────────────────────────

CableDialog::CableDialog(QWidget *parent):QDialog(parent)
{
    setupUi(this);
    tableView->setModel(&tableModel);

    IndexNames *n=diameter;
    while(n->index>0){ comboBoxDiameter->addItem(n->name,n->index); n++; }
    n=impedance;
    while(n->index>0){ comboBoxImpedance->addItem(n->name,n->index); n++; }
}

Cable CableDialog::GetCable()
{
    Cable cable;
    cable.dealer=lineEditDealer->text();
    cable.manufacturer=lineEditManufacturer->text();
    cable.name=lineEditName->text();
    cable.remarks=textEditRemarks->toPlainText();
    cable.dataSheet=lineEditDataSheet->text();
    cable.k1=k1; cable.k2=k2; cable.k3=k3;
    cable.impedance=comboBoxImpedance->currentData().toInt();
    cable.diameter=comboBoxDiameter->currentData().toInt();
    cable.residualError=err;
    if(!filename.isEmpty())cable.filename=filename;
    for(int i=0;i<CBL_ROWS;i++){
        cable.f[i]=tableModel.f[i];
        cable.d[i]=tableModel.d[i];
    }
    return cable;
}

void CableDialog::SetCable(Cable &cable)
{
    lineEditDealer->setText(cable.dealer);
    lineEditManufacturer->setText(cable.manufacturer);
    lineEditName->setText(cable.name);
    textEditRemarks->setText(cable.remarks);
    lineEditDataSheet->setText(cable.dataSheet);
    filename=cable.filename;
    k1=cable.k1; k2=cable.k2; k3=cable.k3;
    err=cable.residualError;
    for(int i=0;i<CBL_ROWS;i++){
        tableModel.setData(tableModel.index(i,0),cable.f[i]);
        tableModel.setData(tableModel.index(i,1),cable.d[i]);
    }
    tableModel.changed=false;
    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));
    labelQuality->setText(QString("%1").arg(err));
    int idx=0;
    for(int i=0;i<comboBoxDiameter->count();i++)
        if(comboBoxDiameter->itemData(i).toInt()==cable.diameter) idx=i;
    comboBoxDiameter->setCurrentIndex(idx);
    idx=0;
    for(int i=0;i<comboBoxImpedance->count();i++)
        if(comboBoxImpedance->itemData(i).toInt()==cable.impedance) idx=i;
    comboBoxImpedance->setCurrentIndex(idx);
}

double CableDialog::Damping(double f, double k1, double k2, double k3)
{
    return 100*(k1+k2*f+k3*sqrt(f));
}

#ifdef USE_FIT
double func( double t, const double *p )
{
    return CableDialog::Damping(t,p[0],p[1],p[2]);
}
#endif

double CableDialog::Error(vector<double> x)
{
    double err=0.0;
    for(int i=0;i<CBL_ROWS;i++)
        if(f[i]>0.1 && d[i]>0.001){
            double a=d[i]-Damping(f[i],x[0],x[1],x[2]);
            err+=a*a;
        }
    if(file)fprintf(file,"%d %.14f %.14f %.14f %.14f\n",counter,err,x[0],x[1],x[2]);
    counter++;
    return err;
}

#ifdef USE_FIT
void CableDialog::Calc()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    counter=0;
    double start[3];
    file=NULL;

    int i,m_dat=0;
    double att[CBL_ROWS],freq[CBL_ROWS];
    for(i=0;i<CBL_ROWS;i++){
        att[i]=tableModel.d[i];
        freq[i]=tableModel.f[i];
        d[i]=tableModel.d[i];
        f[i]=tableModel.f[i];
        if(att[i]>0 && freq[i]>0)m_dat++;
    }
    start[0]=0.005; start[1]=0.005; start[2]=0.005;

    lm_status_struct status;
    lm_control_struct control=lm_control_double;
    control.printflags=0;
    lmcurve_fit(3,start,m_dat,freq,att,func,&control,&status);

    k1=start[0]; k2=start[1]; k3=start[2];
    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));

    err=0.0;
    int count=0;
    for(i=0;i<CBL_ROWS;i++)
        if((freq[i]>0.1)&&(att[i]>0.01)){
            double a=att[i]-Damping(freq[i],k1,k2,k3);
            err+=a*a; count++;
        }
    if(count) err=err/count;
    labelQuality->setText(QString("%1").arg(err));
    tableModel.changed=false;

    QApplication::restoreOverrideCursor();

    if(k1<=-0.001 || k2<=-0.001 || k3<=0){
        QMessageBox::warning(this,tr("Achtung!"),
          tr("Das Funktionsfitting lieferte teilweise physikalisch unsinnige Koeffizienten.\n"
             "Das passiert beispielsweise, wenn die Dämpfungswerte zu ungenau sind.\n"
             "Oder es wird ein Low-Loss Kabel verwendet, dass bei einigen Frequenzen die Genauigkeit des Modells sprengt.\n"
             "Innerhalb der vorhandenen Frequenzintervalle kann die Interpolation dennoch meist hinreichend genau arbeiten.\n"
             "Bleiben Sie aber kritisch."));
    }
    if(err>1.0){
        QMessageBox::warning(this,tr("Achtung!"),
          tr("Der verbleibende Fehler ist relativ groß.\n"
             "Vermutlich ist der Messfehler der verwendeten Dämpfungswerte relativ groß.\n"
             "Die Genauigkeit der interpolierten Werte wird etwas darunter leiden."));
    }
}

#else
void CableDialog::Calc()
{
    const int MAX_ITER=200000;
    const double TOL=1e-8;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    counter=0;
    vector<double> start(3,0.0);
    file=fopen("trace.txt","wt");
    for(int i=0;i<CBL_ROWS;i++){ d[i]=tableModel.d[i]; f[i]=tableModel.f[i]; }
    start[0]=0.005; start[1]=0.005; start[2]=0.005;
    CSimplex NMS(3,TOL,MAX_ITER,&CableDialog::Error);
    err=NMS.amoeba(start);
    k1=start[0]; k2=start[1]; k3=start[2];
    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));
    labelQuality->setText(QString("%1").arg(err));
    tableModel.changed=false;
    if(file){ fclose(file); file=NULL; }
    QApplication::restoreOverrideCursor();
    if(k1<=0||k2<=0||k3<=0||err>1.0){
        QMessageBox::warning(this,"Achtung!",
          "Das Funktionsfitting lieferte keine zufriedenstellende Qualität.\n"
          "Berechnete Dämpfungswerte werden u.U. sehr ungenau ausfallen.\n"
          "Suchen Sie nach besseren Werten für dieses Kabel.");
    }
}
#endif

// ──────────────────────────────────────────────
// VTable
// ──────────────────────────────────────────────

int VTable::columnCount(const QModelIndex &) const { return 2; }
int VTable::rowCount(const QModelIndex &)    const { return CBL_ROWS; }

QVariant VTable::data(const QModelIndex &index, int role) const
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        if(index.column()==0) return f[index.row()];
        if(index.column()==1) return d[index.row()];
    }
    return QVariant();
}

Qt::ItemFlags VTable::flags(const QModelIndex &) const
{
    return Qt::ItemIsEditable|Qt::ItemIsEnabled;
}

QVariant VTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        if(orientation==Qt::Vertical) return QString("%1").arg(section);
        if(section==0) return tr("Frequenz/MHz");
        return tr("Dämpfung/db/100m");
    }
    return QVariant();
}

bool VTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        if(index.column()==0){ f[index.row()]=value.toDouble(); changed=true; }
        if(index.column()==1){ d[index.row()]=value.toDouble(); changed=true; }
    }
    return true;
}

VTable::VTable()
{
    for(int i=0;i<CBL_ROWS;i++){ f[i]=0.0; d[i]=0.0; }
    changed=false;
}

// ──────────────────────────────────────────────
// MyListItem
// ──────────────────────────────────────────────

QVariant MyListItem::data(int role) const
{
    if(role==INDEX_ROLE) return index;
    return QListWidgetItem::data(role);
}

void MyListItem::setData(int role, const QVariant &value)
{
    if(role==INDEX_ROLE) index=value.toInt();
    else QListWidgetItem::setData(role,value);
}
