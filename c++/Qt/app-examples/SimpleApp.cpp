#include <QApplication>
#include <QLineEdit>

int main(int argc, char *argv[]) {
	// создаем объект для управления данными приложения
	QApplication app(argc, argv);

	// создаем редактируемую строку и отобразим её на экране
	QLineEdit lineEdit("Hello world!");
	lineEdit.show();

	// запускаем цикл для обработки событий (event loop)
	return app.exec();
}