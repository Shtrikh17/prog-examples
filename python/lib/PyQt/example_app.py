from PyQt5 import QtCore, QtGui
import sys

# Объект приложения - как в С++
app = QtGui.QApplication(sys.argv)
# Объект окна
window = QtGui.QWidget()
window.setWindowTitle("MyFirstProgram")
window.resize(300, 70)

label = QtGui.QLabel("Hello, world!")
btnQuit = QtGui.QPushButton("&Close")

vbox = QtGui.QVBoxLayout()
vbox.addWidget(label)
vbox.addWidget(btnQuit)
window.setLayout(vbox)

QtCore.QObject.connect(btnQuit, QtCore.SIGNAL("clicked()"), QtGui.qApp, QtCore.SLOT("quit()"))

window.show()
sys.exit(app.exec_())
