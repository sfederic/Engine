module;
#include <qdockwidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>
export module ConsoleDock;

//Console for printing debug information in-editor
export class ConsoleDock : public QDockWidget
{
public:
	QLineEdit* searchBar;
	QTextEdit* consoleMessageBox;
	QPushButton* clearButton;

	ConsoleDock() : QDockWidget(QString("Console"))
	{
		auto vbLayout = new QVBoxLayout();

		auto hbLayout = new QHBoxLayout();

		searchBar = new QLineEdit();
		hbLayout->addWidget(searchBar);
		connect(searchBar, &QLineEdit::returnPressed, this, &ConsoleDock::Search);

		clearButton = new QPushButton("Clear");
		hbLayout->addWidget(clearButton);
		connect(clearButton, &QPushButton::pressed, this, &ConsoleDock::ClearConsole);

		consoleMessageBox = new QTextEdit();
		consoleMessageBox->setReadOnly(true);

		vbLayout->addLayout(hbLayout);
		vbLayout->addWidget(consoleMessageBox);

		auto consoleWidget = new QWidget();
		consoleWidget->setLayout(vbLayout);

		setWidget(consoleWidget);
	}

	//Highlights specified text in search bar
	void Search()
	{
		consoleMessageBox->find(searchBar->text());
	}

	void ClearConsole()
	{
		consoleMessageBox->clear();
	}
};
