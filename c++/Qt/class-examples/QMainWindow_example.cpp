#include <QtGui>
class Notepad : public QMainWindow{
Q_OBJECT
public:
	Notepad();

private slots:
	void open();
	void save();
	void quit();

private:
	QTextEdit *textEdit;

	QAction *openAction;
	QAction *saveAction;
	QAction *exitAction;

	QMenu *fileMenu;
};

Notepad::Notepad(){
	openAction = new QAction(tr("&Open"), this);
	saveAction = new QAction(tr("&Save"), this);
	exitAction = new QAction(tr("E&xit"), this);

	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	textEdit = new QTextEdit;
	setCentralWidget(textEdit);

	setWindowTitle(tr("Notepad"));
}