from PyQt5 import QtCore, QtGui
import sys

class MyWindow(QtGui.QWidget):
def __init__(self, parent=None): # parent - родительский элемент GUI, не в смысле ООП
QtGui.QWidget.__init__(self, parent)
self.label = QtGui.QLabel("Hello, world!")
self.label.setAlignment(QtCore.Qt.AlignCenter)
self.btnQuit = QtGui.QPushButton("&Close")
self.vbox = QtGui.QVBoxLayout()
self.vbox.addWidget(label)
self.vbox.addWidget(btnQuit)
self.setLayout(vbox)
self.connect(self.btnQuit, QtCore.SIGNAL("clicked()"), QtGui.qApp.quit)

if __name__ == "__main__":
# Объект приложения - как в С++
app = QtGui.QApplication(sys.argv)
# Объект окна
window = MyWindow()
window.setWindowTitle("MyFirstProgram")
window.resize(300, 70)
window.show()
sys.exit(app.exec_())
