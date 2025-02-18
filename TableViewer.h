#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QTableView>
#include <QAbstractTableModel>
#include <QPushButton>
#include <QHeaderView>

class TableViewer : public QDialog {
    Q_OBJECT

public:
    explicit TableViewer(QWidget* parent = nullptr)
        : QDialog(parent) {

        setWindowTitle("Table Viewer");
        resize(800, 600);  // Default size

        // Enable resizable and maximizable features
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::Window);

        // Create main layout
        QVBoxLayout* layout = new QVBoxLayout(this);

        // Create QTableView
        tableView = new QTableView(this);
        tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Close Button
        QPushButton* closeButton = new QPushButton("Close", this);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

        // Add widgets to layout
        layout->addWidget(tableView);
        layout->addWidget(closeButton);

        setLayout(layout);
    }

    // Function to set the QAbstractTableModel
    void setModel(QAbstractTableModel* model) {
        if (!model) return;
        tableView->setModel(model);
    }

private:
    QTableView* tableView;
};
