#ifndef COUNTERLABEL
#define COUNTERLABEL

#include <QLabel>

class CounterLabel : public QLabel {
Q_OBJECT // макрос Qt, обязателен для всех виджетов

public:
	CounterLabel() : counter(0) {
		setText("Counter has not been increased yet"); // метод QLabel
	}

public slots:
	// действие, которое будет вызвано в ответ на нажатие
	void increaseCounter() {
		setText(QString("Counter value: %1").arg(QString::number(++counter)));
	}

private:
	int counter;
};

#endif // COUNTERLABEL
//==============================================================
// main.cpp
// Почти тоже самое, что и в предыдущем примере

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QString>
#include "counterlabel.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QDialog dialogWindow;
	dialogWindow.show();

	QVBoxLayout layout;
	dialogWindow.setLayout(&layout);

	CounterLabel counterLabel;
	layout.addWidget(&counterLabel);

	QPushButton button("Push me once more");
	layout.addWidget(&button);
	QObject::connect(&button, &QPushButton::pressed,
	&counterLabel, &CounterLabel::increaseCounter);

	return app.exec();
}