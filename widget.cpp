#include "widget.h"
#include "dirmngr.h"

#include <vector>
#include <math.h>
#include <stdio.h>
#include "Simplex.h"
#include "lmcurve.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

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
   // {"other",100},
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

Widget::Widget(QWidget *parent) :
    QWidget(parent){
    setupUi(this);
    ReadCables();
    Band *b=baender;
    while(b->f>0.0){
        comboBoxBand->addItem(b->band,b->f);
        b++;
    }
    warnLabel->setVisible(false);

}

void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}


void Widget::CalcDamping()
{
    double f=spinBoxF->value();
    double l=spinBoxL->value();
    MyListItem *item=static_cast<MyListItem*>(cableView->currentItem());

    //int kIdx=cableView->currentIndex().row();
    int kIdx=-1;
    if(item)kIdx=item->data(INDEX_ROLE).toInt();
    //int kIdx=comboBoxCable->currentIndex();
    if(kIdx<0 || cables.count()<1 || f<0 ){
        resultLabel->setText(" *** ");
        powerLabel->setText(" *** ");
        cableLabel->setText(" *** ");
        warnLabel->setVisible(false);
        shopButton->setEnabled(false);
        dataSheetButton->setEnabled(false);
        return;
    }

    dataSheetButton->setEnabled(!cables[kIdx].dataSheet.isEmpty());
    shopButton->setEnabled(IsLink(cables[kIdx].dealer));

    cableLabel->setText(cables[kIdx].name);
    double result=l*(cables[kIdx].k1+cables[kIdx].k2*f+cables[kIdx].k3*sqrt(f));
    if(result<0.0)result=0.0;
    resultLabel->setText(QString("%1").arg(result,0,'f',2));

    double power=double(spinBoxPower->value())/pow(10,result/10.0);
    powerLabel->setText(QString("%1").arg(power,0,'f',1));

    if(f<cables[kIdx].LowF()/5)warnLabel->setVisible(true);
    else if(f>cables[kIdx].HighF()*5)warnLabel->setVisible(true);
    else warnLabel->setVisible(false);
}

void Widget::AppendCableWithFilter(Cable cable)
{
    bool doAppend=true;
    if(groupBoxFilter->isChecked()){

        switch(cable.diameter){
            case 1: if(!checkBoxSemiRigid->isChecked())doAppend=0;break;
            case 2: if(!checkBox3mm->isChecked())doAppend=0;break;
            case 3: if(!checkBox5mm->isChecked())doAppend=0;break;
            case 4: if(!checkBox7mm->isChecked())doAppend=0;break;
            case 5: if(!checkBox10mm->isChecked())doAppend=0;break;
            case 6: if(!checkBox15mm->isChecked())doAppend=0;break;
        }
        switch(cable.impedance){
            case 100: if(!checkBoxImp->isChecked())doAppend=0;break;
            case 1: if(!checkBox50->isChecked())doAppend=0;break;
            case 2: if(!checkBox60->isChecked())doAppend=0;break;
            case 3: if(!checkBox75->isChecked())doAppend=0;break;
        }
        if(doAppend)cables.append(cable);

    }else cables.append(cable);



}

void Widget::ReadCables()
{
    QDir dir(dirMngr.GetDir(DIR_DATA1));
    QStringList strlist=dir.entryList(QStringList("*.cbl"),QDir::Files);
    int count=strlist.count();
    cables.clear();
    if(count){
        int i;
        for(i=0;i<count;i++){
            Cable cable;
            cable.Read(dirMngr.CreatePath(strlist[i],DIR_DATA1));
            //cables.append(cable);
            AppendCableWithFilter(cable);
        }
    }

    dir.setPath(dirMngr.GetDir(DIR_DATA2));
    strlist=dir.entryList(QStringList("*.cbl"),QDir::Files);
    count=strlist.count();
    if(count){
        int i;
        for(i=0;i<count;i++){
            Cable cable;
            cable.Read(dirMngr.CreatePath(strlist[i],DIR_DATA2));
            //cables.append(cable);
            AppendCableWithFilter(cable);
        }
    }



/*
    int idx=comboBoxCable->currentIndex();
    comboBoxCable->clear();
    foreach(Cable cable,cables){
        QFileInfo fileInfo(cable.filename);
        comboBoxCable->addItem(QString("%1 (%2)").arg(cable.name).arg(fileInfo.fileName()));
    }
    comboBoxCable->setCurrentIndex(idx);
    */
    QModelIndex idx=cableView->currentIndex();
    cableView->clear();
    for(int i=0;i<cables.count();i++){
        Cable cable=cables[i];
        QFileInfo fileInfo(cable.filename);
        MyListItem *item=new MyListItem;
        item->setText(QString("%1 (%2)").arg(cable.name).arg(fileInfo.fileName()));
        item->setData(INDEX_ROLE,i);
        cableView->addItem(item);
    }
    cableView->setCurrentIndex(idx);

}

void Widget::on_newButton_clicked()
{
    CableDialog win(this);
    if(win.exec()==QDialog::Accepted){
        if(win.tableModel.changed)win.Calc();
        Cable cable=win.GetCable();
        cable.Write();
        ReadCables();
    }

}

void Widget::on_changeButton_clicked()
{
    CableDialog win(this);
    //int index=comboBoxCable->currentIndex();
//    int index=cableView->currentIndex().row();
    MyListItem *item=static_cast<MyListItem*>(cableView->currentItem());

    //int kIdx=cableView->currentIndex().row();
    int kIdx=-1;
    if(item)kIdx=item->data(INDEX_ROLE).toInt();


    if(kIdx>=0)win.SetCable(cables[kIdx]);
    if(win.exec()==QDialog::Accepted){
        Cable cable=win.GetCable();
        cable.Write();
        ReadCables();
    }
}

void Widget::on_comboBoxBand_currentIndexChanged(int index)
{
    spinBoxF->setValue(baender[index].f);
}

void Widget::on_comboBoxLang_currentIndexChanged(int index)
{
    //if(enableLangChange){
        if(index==2)qApp->installTranslator(&translator);
        else if(index==1)qApp->removeTranslator(&translator);
        //translator.load(comboBoxLang->itemData(index).toString());
    //}
}

/*
void Widget::on_comboBoxCable_currentIndexChanged(int index)
{
    CalcDamping();
}
*/

void Widget::on_cableView_currentRowChanged(int index)
{
    CalcDamping();
}

void Widget::on_spinBoxF_valueChanged(double newVal)
{
    CalcDamping();

}

void Widget::on_spinBoxL_valueChanged(double newVal)
{
    CalcDamping();

}


void Widget::on_spinBoxPower_valueChanged(int newVal)
{
    CalcDamping();

}


void Widget::on_webButton_clicked()
{
    QString link = "http://schueler.ws/?page_id=237";
    QDesktopServices::openUrl(QUrl(link));

}

void Widget::on_shopButton_clicked()
{
    //QString link = "https://www.wimo.com/de/kabel-stecker/kabel/koaxialkabel-meterware";
    MyListItem *item=dynamic_cast<MyListItem*>(cableView->currentItem());
    //int kIdx=cableView->currentIndex().row();
    int kIdx=item->data(INDEX_ROLE).toInt();
    if(kIdx<0)return;
    QString link=cables[kIdx].dealer;
    if(IsLink(link)){
        QDesktopServices::openUrl(QUrl(link));
    }

}

void Widget::on_dataSheetButton_clicked()
{

    MyListItem *item=dynamic_cast<MyListItem*>(cableView->currentItem());
    //int kIdx=cableView->currentIndex().row();
    int kIdx=item->data(INDEX_ROLE).toInt();
    if(kIdx<0)return;
    QString link=cables[kIdx].dataSheet;
    if(link.isEmpty())return;
    if(IsLink(link)){
        QDesktopServices::openUrl(QUrl(link));
    }else{ //lokale Datei
        QString fn=dirMngr.CreatePath(link,DIR_DATA1);
        if(QFile::exists(fn))QDesktopServices::openUrl(fn);
        else{
            fn=dirMngr.CreatePath(link,DIR_DATA2);
            dirMngr.CreatePath(link,DIR_DATA2);
        }

    }

}



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
        QString s=QString("f%1").arg(i);
        settings.setValue(s,f[i]);
        s=s=QString("d%1").arg(i);
        settings.setValue(s,d[i]);
        //s=s=QString("used%1").arg(i);
        //settings.setValue(s,used[i]);
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
        QString s=QString("f%1").arg(i);
        f[i]=settings.value(s,f[i]).toDouble();
        s=s=QString("d%1").arg(i);
        d[i]=settings.value(s,d[i]).toDouble();
        s=s=QString("used%1").arg(i);
        //used[i]=settings.value(s,used[i]).toBool();
    }


}

Cable::Cable()
{
    int i;
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
    for(i=0;i<CBL_ROWS;i++){
        f[i]=0.0;
        d[i]=0.0;
        used[i]=false;
    }
    k1=1.0;
    k2=1.0;

}

double Cable::HighF()
{
    int i;
    double fx=0;
    for(i=0;i<CBL_ROWS;i++)if(f[i]>0.0){
        if(fx<f[i])fx=f[i];
    }
    return fx;
}

double Cable::LowF()
{
    int i;
    double fx=0;
    for(i=0;i<CBL_ROWS;i++)if(f[i]>0.0){
        if(fx<=0.0)fx=f[i];
        if(fx>f[i])fx=f[i];
    }
    return fx;

}



CableDialog::CableDialog(QWidget *parent):QDialog(parent)
{
    setupUi(this);
    tableView->setModel(&tableModel);

    IndexNames *n=diameter;
    while(n->index>0){
        comboBoxDiameter->addItem(n->name,n->index);
        n++;
    }

    n=impedance;
    while(n->index>0){
        comboBoxImpedance->addItem(n->name,n->index);
        n++;
    }

}

Cable CableDialog::GetCable()
{
    Cable cable;
    cable.dealer=lineEditDealer->text();
    cable.manufacturer=lineEditManufacturer->text();
    cable.name=lineEditName->text();
    cable.remarks=textEditRemarks->toPlainText();
    cable.dataSheet=lineEditDataSheet->text();
    cable.k1=k1;
    cable.k2=k2;
    cable.k3=k3;
    cable.impedance=comboBoxImpedance->currentData().toInt();
    cable.diameter=comboBoxDiameter->currentData().toInt();
    cable.residualError=err;
    if(!filename.isEmpty())cable.filename=filename;
    int i;
    for(i=0;i<CBL_ROWS;i++){
        cable.f[i]=tableModel.f[i];
        cable.d[i]=tableModel.d[i];
        //used[i]=false;
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
    k1=cable.k1;
    k2=cable.k2;
    k3=cable.k3;
    err=cable.residualError;
    int i;
    for(i=0;i<CBL_ROWS;i++){
        tableModel.setData(tableModel.index(i,0),cable.f[i]);
        tableModel.setData(tableModel.index(i,1),cable.d[i]);
    }
    tableModel.changed=false;
    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));
    labelQuality->setText(QString("%1").arg(err));
    int idx=0;
    for(i=0;i<comboBoxDiameter->count();i++)
        if(comboBoxDiameter->itemData(i).toInt()==cable.diameter)idx=i;
    comboBoxDiameter->setCurrentIndex(idx);
    for(i=0;i<comboBoxImpedance->count();i++)
        if(comboBoxImpedance->itemData(i).toInt()==cable.impedance)idx=i;
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
    int i;
    for(i=0;i<CBL_ROWS;i++)if(f[i]>0.1 && d[i]>0.001){
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
    const int MAX_ITER = 200000;
    const double TOL = 1e-8;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    counter=0;
    double start[3];
    file=NULL;
    //file=fopen("trace.txt","wt");

    int i,m_dat=0;
    double att[CBL_ROWS],freq[CBL_ROWS];
    for(i=0;i<CBL_ROWS;i++){
        att[i]=tableModel.d[i];
        freq[i]=tableModel.f[i];
        d[i]=tableModel.d[i];
        f[i]=tableModel.f[i];
        if(att[i]>0 && freq[i]>0)m_dat++;

    }
    start[0] = 0.005; start[1] = 0.005; start[2]=0.005;

    lm_status_struct status;
    lm_control_struct control = lm_control_double;
    control.printflags = 0; // monitor status (+1) and parameters (+2)

    lmcurve_fit( 3, start, m_dat, freq, att, func, &control, &status );


    k1=start[0];
    k2=start[1];
    k3=start[2];

    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));


    err=0.0;

    int count=0;
    for(i=0;i<CBL_ROWS;i++)if((freq[i]>0.1) && (att[i]>0.01)){
                double a=att[i]-Damping(freq[i],k1,k2,k3);
                err+=a*a;
                if(file)fprintf(file,"%f %f %f %f\n",a,err,freq[i],att[i]);
                count++;
            }

    if(count)err=err/count;

    labelQuality->setText(QString("%1").arg(err));

    tableModel.changed=false;

    if(file){
        //fprintf(file," --> %f",result);
        fclose(file);
        file=NULL;
    }

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
        QMessageBox::warning(this,tr("Achtung!"),tr("Der verbleibende Fehler ist relativ groß.\n"
                             "Vermutlich ist der Messfehler der verwendeten Dämpfungswerte relativ groß.\n"
                             "Die Genauigkeit der interpolierten Werte wird etwas darunter leiden."));
    }

}


#else
void CableDialog::Calc()
{
    const int MAX_ITER = 200000;
    const double TOL = 1e-8;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    counter=0;
    vector<double> start(3,0.0);
    file=fopen("trace.txt","wt");
    int i;
    for(i=0;i<CBL_ROWS;i++){
        d[i]=tableModel.d[i];
        f[i]=tableModel.f[i];
    }

    start[0] = 0.005; start[1] = 0.005; start[2]=0.005;
    CSimplex NMS(3,TOL,MAX_ITER,&CableDialog::Error);

    err = NMS.amoeba(start);

    k1=start[0];
    k2=start[1];
    k3=start[2];

    labelK1->setText(QString("%1").arg(k1));
    labelK2->setText(QString("%1").arg(k2));
    labelK3->setText(QString("%1").arg(k3));

    labelQuality->setText(QString("%1").arg(err));


    tableModel.changed=false;

    if(file){
        fprintf(file," --> %f",result);
        fclose(file);
        file=NULL;
    }

    QApplication::restoreOverrideCursor();

    if(k1<=0 || k2<=0 || k3<=0 || result>1.0){
        QMessageBox::warning(this,"Achtung!","Das Funktionsfitting lieferte keine zufriedenstellende Qualität.\nBerechnete Dämpfungswerte werden u.U. sehr ungenau ausfallen.\nSuchen sie nach besseren Werten für dieses Kabel.");
    }

}
#endif

int VTable::columnCount ( const QModelIndex & parent) const
{
    return 2;
}

int VTable::rowCount ( const QModelIndex & parent ) const
{
    return CBL_ROWS;
}

QVariant VTable::data ( const QModelIndex & index, int role ) const
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        if(index.column()==0){
            return f[index.row()];
        }
        else if(index.column()==1){
            return d[index.row()];
        }
    }
    return QVariant();
}

Qt::ItemFlags VTable::flags ( const QModelIndex & index ) const
{
    return Qt::ItemIsEditable|Qt::ItemIsEnabled;
}

QVariant VTable::headerData ( int section, Qt::Orientation orientation, int role)const
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
    if(orientation==Qt::Vertical)return QString("%1").arg(section);
    else {
        if(section==0)return tr("Frequenz/MHz");
        else return tr("Dämpfung/db/100m");
    }
    }
    return QVariant();
}

bool VTable::setData ( const QModelIndex & index, const QVariant & value, int role)
{
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        if(index.column()==0){
            f[index.row()]=value.toDouble();
            changed=true;
        }
        else if(index.column()==1){
            d[index.row()]=value.toDouble();
            changed=true;
        }
    }
    return true;
}

VTable::VTable()
{
    int i;
    for(i=0;i<CBL_ROWS;i++){
        f[i]=0.0;
        d[i]=0.0;
    }
    changed=false;
}

QVariant MyListItem::data(int role) const
{
    if(role==INDEX_ROLE)return index;
    else return QListWidgetItem::data(role);
}

void MyListItem::setData(int role, const QVariant &value)
{
    if(role==INDEX_ROLE)index=value.toInt();
    else QListWidgetItem::setData(role,value);
}


