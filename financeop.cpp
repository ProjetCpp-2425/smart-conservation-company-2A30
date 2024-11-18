#include "financeop.h"
#include <QSqlRecord>

#include <QTableView>
#include <QStandardItemModel>
financeop::financeop() {
    // Empty constructor initializes the database connection object but doesn't set it up
    // You can add any default initialization logic here if needed
    db = QSqlDatabase::addDatabase("QODBC");
   db.setDatabaseName("test_bd");//inserer le nom de la source de données
   db.setUserName("projetcpp2a30");//inserer nom de l'utilisateur
   db.setPassword("esprit24");//inserer mot de passe de cet utilisateur

}
financeop::financeop(const QString& dbPath) {
     db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("test_bd");//inserer le nom de la source de données
    db.setUserName("projetcpp2a30");//inserer nom de l'utilisateur
    db.setPassword("esprit24");//inserer mot de passe de cet utilisateur

}

bool financeop::open() {
    if (!db.open()) {
        qDebug() << "Error: Unable to open the database:" << db.lastError().text();
        return false;
    }
    return true;
}

void financeop::close() {
    db.close();
}

bool financeop::insertTransaction( const QString& modePaiement, const QString& type,
                                   const QString&categorieTransaction, const QDate& date, float montant) {
    QSqlQuery query;
    query.prepare( "INSERT INTO \"transaction\" (\"mode_payement\", \"type\", \"categorie-transaction\", \"date\", \"montant\") "
                   "VALUES (:modePaiement, :type, :categorieTransaction, TO_DATE(:date, 'DD/MM/YYYY'), :montant)");
   // query.bindValue(":idTransaction", idTransaction);
    query.bindValue(":modePaiement", modePaiement);
    query.bindValue(":type", type);
    query.bindValue(":categorieTransaction", categorieTransaction);
    query.bindValue(":date", date.toString("dd/MM/yyyy"));
    query.bindValue(":montant", montant);
    qDebug()<<query.lastQuery();
    if (!query.exec()) {
        qDebug() << "Error: Unable to insert transaction:" << query.lastError();
        return false;
    }
    if(!depenserevenue())
    sendWindowsNotification("URGENT","Le depense> Le revenue",5000);
    QString LogQuery="L'utilisateur a ajouté une transaction avec Mode paiement:"+modePaiement+" Type:"+type+" Categorie:"+categorieTransaction+" montant:"+QString::number(montant)+"\n";
    updateLog("C:\\Users\\KNUser\\Desktop\\oracle inst\\Atelier_Connexion (1)\\Atelier_Connexion\\historique.txt","Ajout",LogQuery);
    qDebug() << "Transaction updated successfully!";
    return true;
}
void financeop::displayTransactions(QTableView *tableView) {
    // Create a model to hold the data


    // Prepare the SQL query to select all records from the transaction table
    QString sql("SELECT \"id-transaction\" AS \"ID\", \"mode_payement\" AS \"Mode de payement\", \"type\" AS \"Type\", "
                "\"categorie-transaction\" AS \"Categorie de Transaction\", TO_CHAR(\"date\", 'DD/MM/YYYY') AS \"Date de transaction\", "
                "\"montant\" AS \"Montant\" FROM \"transaction\"");
    if(montant>=0.1)
        sql+=" WHERE \"montant\"="+QString::number(montant)+" ";
    if (sort == 1) {
            sql += " ORDER BY \"montant\" ASC"; // Sort by montant in ascending order
        } else if (sort == 2) {
            sql += " ORDER BY \"date\" ASC"; // Sort by date in ascending order
        }
    else if (sort==3)
        sql+=" WHERE  \"type\" = 'Revenue'";
    else if(sort==4)
        sql+=" WHERE  \"type\" = 'Depense'";

    QSqlQuery query(db);
    // Execute the query
    if (!query.exec(sql)) {
        qDebug() << "Error: Unable to execute query:" << query.lastError().text();
        return; // Exit if the query fails
    }
    QList<QList<QVariant>> data; // List of rows, each row is a list of columns

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < query.record().count(); ++i) {
            row.append(query.value(i));
        }
        data.append(row);
    }


    // Set the model's query
    QStandardItemModel *model = new QStandardItemModel(data.size(), query.record().count());

    for (int row = 0; row < data.size(); ++row) {
        for (int col = 0; col < data[row].size(); ++col) {
            QStandardItem *item = new QStandardItem(data[row][col].toString());
            model->setItem(row, col, item);
        }
    }

    // Set headers if necessary
    QStringList headers;
    for (int i = 0; i < query.record().count(); ++i) {
        headers << query.record().fieldName(i);
    }
    model->setHorizontalHeaderLabels(headers);

    // Create your QTableView and set the model
    tableView->setModel(model);
    tableView->resizeColumnsToContents(); // Optional: resize columns to fit content
    tableView->show();
}
bool financeop::deleteTransaction(int idTransaction) {
    QSqlQuery query(db);

    // Prepare the delete statement
    QString txt=("DELETE FROM \"transaction\" WHERE \"id-transaction\" = "+QString::number(idTransaction));


    // Execute the query
    if (!query.exec(txt)) {
        qDebug() << "Error deleting transaction:" << query.lastError().text();
        return false; // Return false if there was an error
    }
    QString LogQuery="L'utilisateur a supprimé la transaction n°"+QString::number(idTransaction)+"\n";
    updateLog("C:\\Users\\KNUser\\Desktop\\oracle inst\\Atelier_Connexion (1)\\Atelier_Connexion\\historique.txt","Suppression",LogQuery);
    if(!depenserevenue())
    sendWindowsNotification("URGENT","Le depense> Le revenue",5000);
    return true; // Return true if deletion was successful

}



// Function to update a transaction by ID
bool financeop::updateTransaction(int idTransaction, const QString &modePaiement, const QString &type,
                                  const QString &categorieTransaction, const QDate &date, float montant) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query;
    QString updateQuery = QString("UPDATE \"transaction\" "
                                  "SET \"mode_payement\" = '%1', "
                                  "\"type\" = '%2', "
                                  "\"categorie-transaction\" = '%3', "
                                  "\"date\" = TO_DATE('%4', 'DD/MM/YYYY'), "
                                  "\"montant\" = %5 "
                                  "WHERE \"id-transaction\" = '%6'")
                           .arg(modePaiement)
                           .arg(type)
                           .arg(categorieTransaction)
                           .arg(date.toString("dd/MM/yyyy"))
                           .arg(montant)
                           .arg(idTransaction);

    // Now execute the query

    if (!query.exec(updateQuery)) {
        qDebug() << "Update failed:" << query.lastError();
        return false;
    }
    if(!depenserevenue())
    sendWindowsNotification("URGENT","Le depense> Le revenue",5000);
    // Execute the query
    QString LogQuery="L'utilisateur a modifié la transaction n°"+QString::number(idTransaction)+" pour devenir Mode paiement:"+modePaiement+" Type:"+type+" Categorie:"+categorieTransaction+" montant:"+QString::number(montant)+"\n";
    updateLog("C:\\Users\\KNUser\\Desktop\\oracle inst\\Atelier_Connexion (1)\\Atelier_Connexion\\historique.txt","Modification",LogQuery);
    qDebug() << "Transaction updated successfully!";
    return true;
}
void financeop::exportToCSV(QTableView *tableView) {
    // Check if the table view or model is valid
    if (!tableView || !tableView->model()) {
        QMessageBox::warning(nullptr, "Export Error", "The table view or its model is not set up correctly.");
        return;
    }

    // Prompt for file save location
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save CSV File", "", "*.csv");
    if (fileName.isEmpty()) return;

    // Ensure the file has a .csv extension
    if (!fileName.endsWith(".csv")) {
        fileName += ".csv";
    }

    // Open file for writing
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Export Error", "Cannot write to file!");
        return;
    }

    QTextStream stream(&file);
    QAbstractItemModel *model = tableView->model();

    // Write column headers
    for (int col = 0; col < model->columnCount(); ++col) {
        stream << model->headerData(col, Qt::Horizontal).toString();
        if (col < model->columnCount() - 1) {
            stream << ",";
        }
    }
    stream << "\n";

    // Write each row of data
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QVariant data = model->data(model->index(row, col));

            // Check if the data is a date and format it
            if (data.type() == QVariant::Date || data.type() == QVariant::DateTime) {
                stream << data.toDateTime().toString("yyyy-MM-dd");  // Format as needed
            } else {
                stream << data.toString();
            }

            if (col < model->columnCount() - 1) {
                stream << ",";
            }
        }
        stream << "\n";
    }

    file.close();
    QMessageBox::information(nullptr, "Export", "Data exported to CSV successfully!");
}
int financeop::countTransactions() {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM \"transaction\"");

    if (!query.exec()) {
        qDebug() << "Error counting rows in transaction table:" << query.lastError().text();
        return -1;  // Return -1 if there was an error
    }

    if (query.next()) {
        return query.value(0).toInt();  // Return the count from the first column
    } else {
        return 0;  // Return 0 if the query has no results
    }
}
double financeop::totalMontant() {
    double totalRevenue = 0.0;
       double totalExpense = 0.0;

       // Calculate total revenue
       QSqlQuery queryRevenue;
       queryRevenue.prepare("SELECT SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Revenue'");
       if (queryRevenue.exec()) {
           if (queryRevenue.next()) {
               totalRevenue = queryRevenue.value(0).toDouble();
           }
       } else {
           qDebug() << "Error calculating revenue:" << queryRevenue.lastError().text();
       }

       // Calculate total expense
       QSqlQuery queryExpense;
       queryExpense.prepare("SELECT SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Depense'");
       if (queryExpense.exec()) {
           if (queryExpense.next()) {
               totalExpense = queryExpense.value(0).toDouble();
           }
       } else {
           qDebug() << "Error calculating expense:" << queryExpense.lastError().text();
       }

       // Calculate net revenue
       double netRevenue = totalRevenue - totalExpense;
       return netRevenue;
}
double financeop::totaldepense()
{
    double totalRevenue = 0.0;

        // Calculate total revenue
        QSqlQuery query;
        query.prepare("SELECT SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Depense'");
        if (query.exec()) {
            if (query.next()) {
                totalRevenue = query.value(0).toDouble();
            }
        } else {
            qDebug() << "Error calculating total revenue:" << query.lastError().text();
        }

        return totalRevenue;
}
double financeop::totalrevenue()
{
    double totalRevenue = 0.0;
        // Calculate total revenue
        QSqlQuery query;
        query.prepare("SELECT SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Revenue'");
        if (query.exec()) {
            if (query.next()) {
                totalRevenue = query.value(0).toDouble();
            }
        } else {
            qDebug() << "Error calculating total revenue:" << query.lastError().text();
        }
        return totalRevenue;
}
void financeop::updateLog(const QString &filePath, const QString &operationType, const QString &newLogEntry) {
    QFile logFile(filePath);
    QString logContent;

    // Read the existing log file
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&logFile);
        logContent = in.readAll();
        logFile.close();
    } else {
        qWarning() << "Unable to open log file for reading!";
        return;
    }

    // Ensure the section exists in the log or initialize it
    QString sectionHeader = "Historique de " + operationType + ":";
    if (!logContent.contains(sectionHeader)) {
        logContent += sectionHeader + "\n";
    }

    // Prepare the timestamped log entry
    QString timestamp = QDateTime::currentDateTime().toString("ddd. MMM dd hh:mm:ss");
    QString logEntry = "[" + timestamp + "] - " + newLogEntry + "\n";

    // Update the specified section
    QStringList lines = logContent.split("\n");
    QString updatedContent;
    bool sectionFound = false;

    for (const QString &line : lines) {
        updatedContent += line + "\n";

        if (line.startsWith(sectionHeader)) {
            sectionFound = true;
            qDebug()<<"here";
        } else if (sectionFound && (line.startsWith("Historique de") || line.isEmpty())) {
            updatedContent += logEntry; // Insert the new log entry
            sectionFound = false;
        }
    }

    if (sectionFound) {
        updatedContent += logEntry; // Add to the end of the section if it's the last one
    }

    // Write back the updated log to the file
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << updatedContent;
        logFile.close();
        qDebug() << "Log updated successfully!";
    } else {
        qWarning() << "Unable to open log file for writing!";
    }
}
void financeop::showhistorique(QTextBrowser *tt)
{
    QString filePath = R"(C:\Users\KNUser\Desktop\oracle inst\Atelier_Connexion (1)\Atelier_Connexion\historique.txt)";

        // Load the file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, "Error", "Could not open the file: " + filePath);
            return ;
        }

        // Read the file contents
        QTextStream stream(&file);
        QString fileContent = stream.readAll();
        file.close();

        // Create QTextBrowser and display the content as HTML

        tt->setText(fileContent); // Treat the content as HTML
}

void financeop::sendWindowsNotification(const QString &title, const QString &message, int duration = 3000) {
    static QSystemTrayIcon trayIcon; // Static to keep it alive for the app's lifetime

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning("System tray is not available on this system.");
        return;
    }

    if (!trayIcon.isVisible()) {
        // Set an icon (required for the notification to show)
        trayIcon.setIcon(QIcon(":/img/logo1.png")); // Replace with your app's icon
        trayIcon.setVisible(true);
    }

    // Show the notification
    trayIcon.showMessage(
        title,               // Notification title
        message,             // Notification message
        QSystemTrayIcon::Information, // Notification type (e.g., Information, Warning, Critical)
        duration             // Duration in milliseconds
    );
}
bool financeop::depenserevenue()
{
    double revenue=totalrevenue();
    double depense=totaldepense();
    if(depense>revenue)
        return false;
    else return true;
    //QString sql("select ");
   // qry.prepare("")
}
 QList<QPair<QString, qreal>>  financeop::fetchAndSumTransactions(int n) {
    // Create a SQL query to sum the 'montant' by 'categorie' where 'type' = 'Revenue'
    QSqlQuery query;
    if(n==0)
    query.prepare("SELECT \"categorie-transaction\", SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Revenue' GROUP BY \"categorie-transaction\"");
    else
        query.prepare("SELECT \"categorie-transaction\", SUM(\"montant\") FROM \"transaction\" WHERE \"type\" = 'Depense' GROUP BY \"categorie-transaction\"");

    QList<QPair<QString, qreal>>  data;
    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return data;
    }

    // Iterate over the results and store them in the data list
    while (query.next()) {
        QString categorie = query.value(0).toString(); // Get the category
        qreal montant = query.value(1).toReal();      // Get the summed montant
        data.append(QPair<QString, qreal>(categorie, montant)); // Store in the list
    }
    return data;
}




/*void financeop::sortTransactionsByType() {
    QTableView *tableView = ui->tableView;  // Assurez-vous que le nom de votre tableView est correct
    QAbstractItemModel *model = tableView->model();

    if (!model) {
        qWarning() << "Le modèle du tableau n'est pas défini.";
        return;
    }

    // Créez un proxy pour trier
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    // Trier par type de transaction (exemple: colonne "type" contenant "Revenu" et "Dépense")
    proxyModel->setSortRole(Qt::DisplayRole);  // Trier selon les données affichées
    proxyModel->sort(0, Qt::AscendingOrder); // Tri de la première colonne (index 0) qui contient les types ("Revenu" et "Dépense")

    // Appliquer le proxyModel au QTableView
    tableView->setModel(proxyModel);
}*/


