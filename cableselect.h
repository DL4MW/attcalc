#ifndef CABLESELECT_H
#define CABLESELECT_H

#include "widget.h"
#include "ui_cableselect.h"
#include "ui_filterdialog.h"

// FilterDialog: Speichert den Filter-Zustand persistent als Member von
// CableSelectDialog (wird nicht bei jedem Öffnen neu angelegt).
class FilterDialog : public QDialog, private Ui::FilterDialog
{
    Q_OBJECT
public:
    explicit FilterDialog(QWidget *parent = nullptr);

    bool filterEnabled() const { return groupBoxFilter->isChecked(); }
    bool isSemiRigid()   const { return checkBoxSemiRigid->isChecked(); }
    bool is3mm()         const { return checkBox3mm->isChecked(); }
    bool is5mm()         const { return checkBox5mm->isChecked(); }
    bool is7mm()         const { return checkBox7mm->isChecked(); }
    bool is10mm()        const { return checkBox10mm->isChecked(); }
    bool is15mm()        const { return checkBox15mm->isChecked(); }
    bool is50Ohm()       const { return checkBox50->isChecked(); }
    bool is60Ohm()       const { return checkBox60->isChecked(); }
    bool is75Ohm()       const { return checkBox75->isChecked(); }
    bool isOtherImp()    const { return checkBoxImp->isChecked(); }
};

// CableSelectDialog: Kabelliste durchsuchen, filtern, verwalten und auswählen.
// Wird als persistentes Member in Widget gehalten, damit der Filter-Zustand
// über mehrere Öffnungen erhalten bleibt.
class CableSelectDialog : public QDialog, private Ui::CableSelectDialog
{
    Q_OBJECT

public:
    explicit CableSelectDialog(QWidget *parent = nullptr);

    // Gibt das aktuell in der Liste markierte Kabel zurück.
    Cable selectedCable() const;

    // Markiert in der Liste das Kabel, das zuletzt ausgewählt war.
    void setCurrentCable(const Cable &cable);

    // Startet den Download der Kabeldateien vom Server.
    // Kann auch aufgerufen werden, wenn der Dialog noch nicht sichtbar ist.
    void downloadCables();

protected:
    QList<Cable> cables;
    FilterDialog m_filterDialog;

    void ReadCables();
    void AppendCableWithFilter(Cable cable);

public slots:
    void on_newButton_clicked();
    void on_changeButton_clicked();
    void on_updateCablesButton_clicked();
    void on_dataSheetButton_clicked();
    void on_shopButton_clicked();
    void on_filterButton_clicked();
    void on_cableView_currentRowChanged(int row);
};

#endif // CABLESELECT_H
