#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QDialog dialogWindow;
	dialogWindow.show();

	// добавляем вертикальное расположение
	QVBoxLayout layout;
	dialogWindow.setLayout(&layout);

	QLabel textLabel("Thanks for pressing that button");
	layout.addWidget(&textLabel);
	textLabel.hide();

	QPushButton button("Press me");
	layout.addWidget(&button);

	// отображаем скрытую надпись после нажатия на кнопку
	QObject::connect(&button, &QPushButton::pressed,
	&textLabel, &QLabel::show);

	return app.exec();
}