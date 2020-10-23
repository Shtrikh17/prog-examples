// Создадим виджет-индикатор
#include <QWidget>

class LedIndicator: public QWidget{
Q_OBJECT
Q_PROPERTY(QString text READ text WRITE setText)
Q_PROPERTY(bool turnedOn READ isTurnedOn WRITE setTurnedOn NOTIFY stateToggled)
public:
	// ...
	QSize minimumSizeHint() const;
	// ...
protected:
	void paintEvent(QPaintEvent∗);
	// ...
};

void LedIndicator::paintEvent(QPaintEvent* pEvent){
	//Создаём объект QPainter и указываем QPaintDevice текущий виджет
	QPainter lPainter(this);
	//Используем сглаживание при рисовании для лучшего вида
	lPainter.setRenderHint(QPainter::Antialiasing);
	//Центр окружности индикатора QPoint - класс для описания точки
	QPoint lLedCenter(cLedRadius+1 , height()/2);
	//Фигура, которую мы будем рисовать QPainterPath - класс для описания фигуры
	//состоящей из нескольких примитивов
	QPainterPath lPath;
	//Добавляем окружность
	lPath.addEllipse(lLedCenter, cLedRadius, cLedRadius) ;
	//Сохраняем настройки после всех изменений мы восстановим их для рисования подписи
	lPainter.save();
	//Создаём радиальный (окружностями) градиент указываем центр для градиента и радиус
	QRadialGradient lGradient(lLedCenter, cLedRadius);

	if(mIsTurnedOn){
		//Устанавливаем цвет границы и градиент для включённого и выключенного состояний
		//Задаём объект QPen - настройки рисования контуров
		//Используем константу для задания цвета контура в конструкторе QPen
		lPainter.setPen(QPen(Qt::darkGreen));
		//Задаём цвет в разных точках (0 - центр, 1 - край) цвет будет равномерно изменяться
		//Для задания цвета пользуемся текстовым шестнадцатеричным RGB обозначением
		//- неявное преобразование в QColor
		lGradient.setColorAt(0.2, "#70FF70");
		lGradient.setColorAt(1, "#00CC00");
	}
	else{
		//Здесь задаём чёрный цвет
		lPainter.setPen(QPen(QColor(0,0,0)));
		lGradient.setColorAt(0.2, Qt::gray);
		lGradient.setColorAt(1, Qt::darkGray);
	}

	//Заполняем фигуру индикатора градиентом
	lPainter.fillPath(lPath, QBrush(lGradient));
	//Рисуем границу индикатора
	lPainter.drawPath(lPath);
	//Восстанавливаем настройки перед последним сохранением
	lPainter.restore();
	//Устанавливаем шрифт для рисования текста используем QWidget::font(),
	//чтобы иметь возможность стилизовать надпись
	lPainter.setFont(font());
	//Квадрат, в котором будет рисоваться текст. QRect - класс для обозначения прямоугольной области
	QRect lTextRect(cLedRadius∗2+cLedSpacing, 0, width()−(cLedRadius∗2+cLedSpacing), height());
	//Рисуем текст в заданном прямоугольнике, выравнивание по левому краю и вертикально по центру
	lPainter.drawText(lTextRect, Qt::AlignVCenter | Qt::AlignLeft, mText);
}

//Переопределяем виртуальный метод minimumSizeHint() для передачи корректных минимальных размеров
QSize LedIndicator::minimumSizeHint() const{
	return QSize(cLedRadius * 2 //Диаметр индикатора
	+ fontMetrics().width(mText) //Ширина текста mText
	+ cLedSpacing, //Отступ
	cLedRadius*2);
}

//===============================================
#include <QHBoxLayout>
#include <QCheckBox>
#include "ledindicator.h"
MainWindow::MainWindow(QWidget∗ parent): QWidget(parent){
	//Главное компонование
	QHBoxLayout* lLayout = new QHBoxLayout;
	setLayout(lLayout);
	//Создаём наш индикатор и добавляем его к компоновщику
	LedIndicator* lLedIndicator = new LedIndicator;
	lLedIndicator->setText("LED Indicator");
	lLayout−>addWidget(lLedIndicator);
	//Создаём и добавляем флажок
	QCheckBox* lCheckBox = new QCheckBox("Led ON");
	lLayout−>addWidget(lCheckBox);
	//Соединяем флажок и индикатор
	connect(lCheckBox, SIGNAL(toggled(bool)), lLedIndicator, SLOT(setTurnedOn(bool)), Qt::UniqueConnection);
}