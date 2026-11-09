#include "cableselect.h"
#include "dirmngr.h"

extern DirMngr dirMngr;

static bool isLink(const QString &s)
{
    return !s.isEmpty() && s.startsWith("http");
}

// ──────────────────────────────────────────────
// FilterDialog
// ──────────────────────────────────────────────

FilterDialog::FilterDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
}


// ──────────────────────────────────────────────
// CableSelectDialog
// ──────────────────────────────────────────────

CableSelectDialog::CableSelectDialog(QWidget *parent)
    : QDialog(parent), m_filterDialog(this)
{
    setupUi(this);
    dataSheetButton->setEnabled(false);
    shopButton->setEnabled(false);
    ReadCables();
}


void CableSelectDialog::AppendCableWithFilter(Cable cable)
{
    bool doAppend = true;
    if (m_filterDialog.filterEnabled()) {
        switch (cable.diameter) {
            case 1: if (!m_filterDialog.isSemiRigid()) doAppend = false; break;
            case 2: if (!m_filterDialog.is3mm())       doAppend = false; break;
            case 3: if (!m_filterDialog.is5mm())       doAppend = false; break;
            case 4: if (!m_filterDialog.is7mm())       doAppend = false; break;
            case 5: if (!m_filterDialog.is10mm())      doAppend = false; break;
            case 6: if (!m_filterDialog.is15mm())      doAppend = false; break;
        }
        switch (cable.impedance) {
            case 100: if (!m_filterDialog.isOtherImp()) doAppend = false; break;
            case 1:   if (!m_filterDialog.is50Ohm())    doAppend = false; break;
            case 2:   if (!m_filterDialog.is60Ohm())    doAppend = false; break;
            case 3:   if (!m_filterDialog.is75Ohm())    doAppend = false; break;
        }
    }
    if (doAppend) cables.append(cable);
}

void CableSelectDialog::ReadCables()
{
    // Aktuelle Auswahl merken (anhand Dateiname), um sie nach dem Reload wiederherzustellen
    QString currentFile;
    MyListItem *cur = static_cast<MyListItem*>(cableView->currentItem());
    if (cur) {
        int idx = cur->data(INDEX_ROLE).toInt();
        if (idx >= 0 && idx < cables.count())
            currentFile = cables[idx].filename;
    }

    cables.clear();

    auto loadDir = [&](int dirId) {
        QDir dir(dirMngr.GetDir(dirId));
        const QStringList files = dir.entryList(QStringList("*.cbl"), QDir::Files);
        for (const QString &fn : files) {
            Cable cable;
            cable.Read(dirMngr.CreatePath(fn, dirId));
            AppendCableWithFilter(cable);
        }
    };
    loadDir(DIR_DATA1);
    loadDir(DIR_DATA2);

    cableView->clear();
    for (int i = 0; i < cables.count(); i++) {
        const Cable &cable = cables[i];
        QFileInfo fi(cable.filename);
        MyListItem *item = new MyListItem;
        item->setText(QString("%1 (%2)").arg(cable.name).arg(fi.fileName()));
        item->setData(INDEX_ROLE, i);
        cableView->addItem(item);
        if (!currentFile.isEmpty() && cable.filename == currentFile)
            cableView->setCurrentItem(item);
    }
}

Cable CableSelectDialog::selectedCable() const
{
    MyListItem *item = static_cast<MyListItem*>(cableView->currentItem());
    if (!item) return Cable();
    int idx = item->data(INDEX_ROLE).toInt();
    if (idx < 0 || idx >= cables.count()) return Cable();
    return cables[idx];
}

void CableSelectDialog::setCurrentCable(const Cable &cable)
{
    for (int i = 0; i < cableView->count(); i++) {
        MyListItem *item = static_cast<MyListItem*>(cableView->item(i));
        if (!item) continue;
        int idx = item->data(INDEX_ROLE).toInt();
        if (idx >= 0 && idx < cables.count() && cables[idx].filename == cable.filename) {
            cableView->setCurrentItem(item);
            return;
        }
    }
}

void CableSelectDialog::on_cableView_currentRowChanged(int)
{
    MyListItem *item = static_cast<MyListItem*>(cableView->currentItem());
    int kIdx = -1;
    if (item) kIdx = item->data(INDEX_ROLE).toInt();
    bool valid = (kIdx >= 0 && kIdx < cables.count());
    dataSheetButton->setEnabled(valid && !cables[kIdx].dataSheet.isEmpty());
    shopButton->setEnabled(valid && isLink(cables[kIdx].dealer));
}

void CableSelectDialog::on_newButton_clicked()
{
    CableDialog win(this);
#ifdef Q_OS_ANDROID
    win.showMaximized();
#endif
    if (win.exec() == QDialog::Accepted) {
        if (win.tableModel.changed) win.Calc();
        Cable cable = win.GetCable();
        cable.Write();
        ReadCables();
    }
}

void CableSelectDialog::on_changeButton_clicked()
{
    MyListItem *item = static_cast<MyListItem*>(cableView->currentItem());
    int kIdx = -1;
    if (item) kIdx = item->data(INDEX_ROLE).toInt();
    CableDialog win(this);
    if (kIdx >= 0) win.SetCable(cables[kIdx]);
#ifdef Q_OS_ANDROID
    win.showMaximized();
#endif
    if (win.exec() == QDialog::Accepted) {
        Cable cable = win.GetCable();
        cable.Write();
        ReadCables();
    }
}

void CableSelectDialog::on_updateCablesButton_clicked()
{
    downloadCables();
}

void CableSelectDialog::downloadCables()
{
    updateCablesButton->setEnabled(false);
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    const QString baseUrl = "http://dl4mw.net/cable/";

    QNetworkReply *indexReply = mgr->get(QNetworkRequest(QUrl(baseUrl)));
    connect(indexReply, &QNetworkReply::finished, this, [this, mgr, indexReply, baseUrl]() {
        indexReply->deleteLater();
        if (indexReply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, tr("Fehler"),
                tr("Verbindung fehlgeschlagen: ") + indexReply->errorString());
            updateCablesButton->setEnabled(true);
            mgr->deleteLater();
            return;
        }

        const QString html = QString::fromUtf8(indexReply->readAll());
        QRegularExpression re(R"(href="([^"]+\.cbl))",
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = re.globalMatch(html);
        QStringList files;
        while (it.hasNext())
            files << it.next().captured(1);

        if (files.isEmpty()) {
            QMessageBox::information(this, tr("Update"),
                tr("Keine Kabeldateien auf dem Server gefunden."));
            updateCablesButton->setEnabled(true);
            mgr->deleteLater();
            return;
        }

        int *remaining = new int(files.count());
        for (const QString &href : files) {
            QUrl fileUrl = QUrl(baseUrl).resolved(QUrl(href));
            QNetworkReply *fileReply = mgr->get(QNetworkRequest(fileUrl));
            connect(fileReply, &QNetworkReply::finished, this,
                    [this, mgr, fileReply, href, remaining]() {
                fileReply->deleteLater();
                if (fileReply->error() == QNetworkReply::NoError) {
                    QString destPath = dirMngr.CreatePath(
                        QFileInfo(href).fileName(), DIR_DATA1);
                    QFile f(destPath);
                    if (f.open(QIODevice::WriteOnly))
                        f.write(fileReply->readAll());
                }
                if (--(*remaining) == 0) {
                    delete remaining;
                    mgr->deleteLater();
                    updateCablesButton->setEnabled(true);
                    ReadCables();
                    QMessageBox::information(this, tr("Update"),
                        tr("Kabelbibliothek wurde aktualisiert."));
                }
            });
        }
    });
}

void CableSelectDialog::on_dataSheetButton_clicked()
{
    MyListItem *item = static_cast<MyListItem*>(cableView->currentItem());
    if (!item) return;
    int kIdx = item->data(INDEX_ROLE).toInt();
    if (kIdx < 0 || kIdx >= cables.count()) return;
    const QString link = cables[kIdx].dataSheet;
    if (link.isEmpty()) return;
    if (isLink(link)) {
        QDesktopServices::openUrl(QUrl(link));
    } else {
        QString fn = dirMngr.CreatePath(link, DIR_DATA1);
        if (!QFile::exists(fn))
            fn = dirMngr.CreatePath(link, DIR_DATA2);
        if (QFile::exists(fn))
            QDesktopServices::openUrl(QUrl::fromLocalFile(fn));
    }
}

void CableSelectDialog::on_shopButton_clicked()
{
    MyListItem *item = static_cast<MyListItem*>(cableView->currentItem());
    if (!item) return;
    int kIdx = item->data(INDEX_ROLE).toInt();
    if (kIdx < 0 || kIdx >= cables.count()) return;
    const QString link = cables[kIdx].dealer;
    if (isLink(link))
        QDesktopServices::openUrl(QUrl(link));
}

void CableSelectDialog::on_filterButton_clicked()
{
#ifdef Q_OS_ANDROID
    m_filterDialog.showMaximized();
#endif
    if (m_filterDialog.exec() == QDialog::Accepted)
        ReadCables();
}
