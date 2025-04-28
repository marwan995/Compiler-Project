import re
import sys
import subprocess
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QPushButton, QTabWidget, QTextEdit,
    QPlainTextEdit, QFrame, QFileDialog, QLabel, QHBoxLayout
)
from PyQt6.QtGui import QFont, QPainter, QColor, QTextFormat
from PyQt6.QtCore import Qt, QRect, QSize
from PyQt6.QtWidgets import QLabel
from PyQt6.QtGui import QTextCursor, QTextCharFormat
from PyQt6.QtWidgets import QTableWidget, QTableWidgetItem
from PyQt6.QtGui import QSyntaxHighlighter, QTextCharFormat


COMPILER_PATH = "parser.exe"
WARNING_PATH = "warnings.txt"
ERROR_PATH ="syntax_errors.txt"
ASSIMBLE_PATH ="assembly.txt"
QUADRUPLES_PATH ="quadruples.txt"

class LineNumberArea(QFrame):
    def __init__(self, editor):
        super().__init__(editor)
        self.code_editor = editor

    def sizeHint(self):
        return QSize(self.code_editor.line_number_area_width(), 0)

    def paintEvent(self, event):
        self.code_editor.line_number_area_paint_event(event)


class CodeEditor(QPlainTextEdit):
    def __init__(self):
        super().__init__()
        self.setFont(QFont("Consolas", 12))
        self.setPlaceholderText("Type your code here...")
        self.line_number_area = LineNumberArea(self)   	
        self.blockCountChanged.connect(self.update_line_number_area_width)
        self.updateRequest.connect(self.update_line_number_area)
        self.cursorPositionChanged.connect(self.highlight_current_line)
        self.highlighter = SyntaxHighlighter(self.document())
        font_metrics = self.fontMetrics()
        tab_width_pixels = font_metrics.horizontalAdvance(' ') * 4
        self.setTabStopDistance(tab_width_pixels)
        self.update_line_number_area_width(0)
        self.highlight_current_line()

    def line_number_area_width(self):
        digits = len(str(max(1, self.blockCount())))
        space = 3 + self.fontMetrics().horizontalAdvance('9') * digits
        return space

    def update_line_number_area_width(self, _):
        self.setViewportMargins(self.line_number_area_width(), 0, 0, 0)

    def update_line_number_area(self, rect, dy):
        if dy:
            self.line_number_area.scroll(0, dy)
        else:
            self.line_number_area.update(0, rect.y(), self.line_number_area.width(), rect.height())

        if rect.contains(self.viewport().rect()):
            self.update_line_number_area_width(0)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        cr = self.contentsRect()
        self.line_number_area.setGeometry(QRect(cr.left(), cr.top(), self.line_number_area_width(), cr.height()))

    def line_number_area_paint_event(self, event):
        painter = QPainter(self.line_number_area)
        painter.fillRect(event.rect(), QColor("#2b2b2b"))

        block = self.firstVisibleBlock()
        block_number = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(block_number + 1)
                painter.setPen(Qt.GlobalColor.lightGray)
                painter.drawText(0, top, self.line_number_area.width() - 5, self.fontMetrics().height(),
                                 Qt.AlignmentFlag.AlignRight, number)

            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            block_number += 1

    def highlight_current_line(self):
        extraSelections = []
        if not self.isReadOnly():
            selection = QTextEdit.ExtraSelection()
            selection.format.setBackground(QColor("#3c3f41"))
            selection.format.setProperty(QTextFormat.Property.FullWidthSelection, True)
            selection.cursor = self.textCursor()
            selection.cursor.clearSelection()
            extraSelections.append(selection)
        self.setExtraSelections(extraSelections)


class CompilerApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DragMania")
        self.setGeometry(200, 100, 900, 700)

        layout = QVBoxLayout()
        self.code_editor = CodeEditor()
        layout.addWidget(self.code_editor)

        # Button Row
        button_row = QHBoxLayout()

        self.upload_button = QPushButton("📂 Upload Code")
        self.upload_button.setStyleSheet("padding: 8px; font-size: 14px;")
        self.upload_button.clicked.connect(self.upload_code)
        button_row.addWidget(self.upload_button)
        self.save_button = QPushButton("💾 Save Code")
        self.save_button.setStyleSheet("padding: 8px; font-size: 14px;")
        self.save_button.clicked.connect(self.save_code)
        button_row.addWidget(self.save_button)

        self.run_button = QPushButton("▶ Run Code")
        self.run_button.setStyleSheet("padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; ")
        self.run_button.clicked.connect(self.run_compiler)
        button_row.addWidget(self.run_button)

        layout.addLayout(button_row)

        # Tabs for output
        self.tabs = QTabWidget()
        self.assembly_tab = QTextEdit()
        self.assembly_tab.setReadOnly(True)
        self.quadruples_tab = QTextEdit()
        self.quadruples_tab.setReadOnly(True)

        self.error_tab = QTextEdit()
        self.error_tab.setReadOnly(True)

        self.output_tab = QTextEdit()
        self.output_tab.setReadOnly(True)

        self.warning_tab = QTextEdit()
        self.warning_tab.setReadOnly(True)
        self.symbol_table_tab = QTableWidget()
        self.symbol_table_tab.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers) 
        self.tabs.addTab(self.assembly_tab, "Assembly")
        self.tabs.addTab(self.quadruples_tab,"Quadruples")
        self.tabs.addTab(self.error_tab, "Errors")
        self.tabs.addTab(self.warning_tab, "Warnings")
        self.tabs.addTab(self.output_tab, "Output")
        self.tabs.addTab(self.symbol_table_tab, "Symbol Table")


        layout.addWidget(self.tabs)

        # Status label
        self.status_label = QLabel("Ready.")
        self.status_label.setStyleSheet("padding: 6px; color: #888;")
        layout.addWidget(self.status_label)

        self.setLayout(layout)

    def upload_code(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open Code File", "", "Code Files (*.txt *.c *.cpp *.dm);;All Files (*)")
        if path:
            try:
                with open(path, "r") as file:
                    self.code_editor.setPlainText(file.read())
                self.status_label.setText(f"Loaded file: {path}")
            except Exception as e:
                self.status_label.setText(f"Error loading file: {str(e)}")

    def run_compiler(self):
        code = self.code_editor.toPlainText()
        error_msg = ""
        with open("temp_input.txt", "w") as f:
            f.write(code)

        try:
            result = subprocess.run([COMPILER_PATH, "temp_input.txt"],capture_output=True, 
            text=True, check=True)
            self.output_tab.setText(result.stdout)

        except subprocess.CalledProcessError as e:
            error_msg = f"Compiler execution failed\n"

        self.assembly_tab.setText(self.read_file(ASSIMBLE_PATH))
        self.quadruples_tab.setText(self.read_file(QUADRUPLES_PATH))
        
        self.error_tab.setText(self.read_file(ERROR_PATH,True))
        if error_msg:
            self.error_tab.append(error_msg)
            self.status_label.setText("❌ Compilation failed.")
        else:
            self.status_label.setText("✅ Compilation complete.")
        self.load_symbol_table()



        self.warning_tab.setText(self.read_file(WARNING_PATH,True))
        self.highlight_lines(WARNING_PATH,error_type="warning")
        self.highlight_lines(ERROR_PATH)


    @staticmethod
    def read_file(filename, unique=False):
        try:
            with open(filename, "r") as f:
                # Read lines
                lines = f.readlines()
                # Process lines based on unique flag
                if unique:
                    # Remove duplicates while preserving order
                    processed_lines = list(dict.fromkeys(line.rstrip('\n') for line in lines))
                else:
                    # Keep all lines, just remove trailing newlines
                    processed_lines = [line.rstrip('\n') for line in lines]
                return '\n'.join(processed_lines)
        except FileNotFoundError:
            return f"{filename} not found."
    def highlight_lines(self, error_file, error_type="error"):
        try:
            with open(error_file, "r") as f:
                error_lines = f.readlines()
        except Exception:
            return

        selections = []

        # Red background format
        fmt = QTextCharFormat()
        if error_type == "error":
            fmt.setBackground(QColor("#ff4c4c"))
            fmt.setForeground(QColor("#ffffff"))  # white text for errors
        else:
            fmt.setBackground(QColor("#ffcc00"))
            fmt.setForeground(QColor("#000000"))

        for line in error_lines:
            match = re.search(r'line\s+(\d+)', line, re.IGNORECASE)
            if match:
                line_num = int(match.group(1)) - 1
                block = self.code_editor.document().findBlockByNumber(line_num)
                if block.isValid():
                    cursor = QTextCursor(block)
                    cursor.movePosition(QTextCursor.MoveOperation.EndOfBlock, QTextCursor.MoveMode.KeepAnchor)

                    selection = QTextEdit.ExtraSelection()
                    selection.cursor = cursor
                    selection.format = fmt
                    selections.append(selection)

        # Merge with current line highlight
        current_line_selection = self.code_editor.extraSelections()
        self.code_editor.setExtraSelections(current_line_selection + selections)
    def load_symbol_table(self, filepath="symbol_table.txt"):
        try:
            with open(filepath, "r") as f:
                lines = f.readlines()
        except FileNotFoundError:
            self.symbol_table_tab.setRowCount(0)
            self.symbol_table_tab.setColumnCount(1)
            self.symbol_table_tab.setHorizontalHeaderLabels(["Error"])
            self.symbol_table_tab.setItem(0, 0, QTableWidgetItem("symbol_table.txt not found."))
            return

        headers = lines[0].strip().split('\t')
        self.symbol_table_tab.setColumnCount(len(headers))
        self.symbol_table_tab.setHorizontalHeaderLabels(headers)

        self.symbol_table_tab.setRowCount(len(lines) - 1)
        for row, line in enumerate(lines[1:]):
            columns = line.strip().split('\t')
            for col, value in enumerate(columns):
                self.symbol_table_tab.setItem(row, col, QTableWidgetItem(value))
    def save_code(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save Code File", "", "Code Files (*.txt *.c *.cpp *.dm);;All Files (*)")
        if path:
            try:
                with open(path, "w") as file:
                    file.write(self.code_editor.toPlainText())
                self.status_label.setText(f"Saved file: {path}")
            except Exception as e:
                self.status_label.setText(f"Error saving file: {str(e)}")

class SyntaxHighlighter(QSyntaxHighlighter):
    def __init__(self, document):
        super().__init__(document)
        self.highlighting_rules = []

        # Format styles
        keyword_format = QTextCharFormat()
        keyword_format.setForeground(QColor("#cc7832"))  # orange
        keyword_format.setFontWeight(QFont.Weight.Bold)

        operator_format = QTextCharFormat()
        operator_format.setForeground(QColor("#a9b7c6"))  # light gray

        datatype_format = QTextCharFormat()
        datatype_format.setForeground(QColor("#ffc66d"))  # yellow-orange
        datatype_format.setFontWeight(QFont.Weight.Bold)

        string_format = QTextCharFormat()
        string_format.setForeground(QColor("#6a8759"))  # green

        comment_format = QTextCharFormat()
        comment_format.setForeground(QColor("#808080"))  # gray
        comment_format.setFontItalic(True)

        number_format = QTextCharFormat()
        number_format.setForeground(QColor("#6897bb"))  # blue
                # Special 'func' keyword format
        func_format = QTextCharFormat()
        func_format.setForeground(QColor("#9876aa"))  # purple or any color you like
        func_format.setFontWeight(QFont.Weight.Bold)
        self.highlighting_rules.append((re.compile(r"\bfunc\b"), func_format))


        # Keywords
        keywords = [
            r"\bif\b", r"\belse\b", r"\bfor\b", r"\bwhile\b", r"\bdo\b", r"\bbreak\b", r"\bcontinue\b",
            r"\breturn\b", r"\bswitch\b", r"\bcase\b", r"\bdefault\b"
        ]
        for kw in keywords:
            self.highlighting_rules.append((re.compile(kw), keyword_format))

        # Data types
        datatypes = [r"\bint\b", r"\bfloat\b", r"\bchar\b", r"\bstring\b", r"\bbool\b", r"\bvoid\b", r"\bconst\b"]
        for dt in datatypes:
            self.highlighting_rules.append((re.compile(dt), datatype_format))

        # Operators
        operators = [r"\+", r"-", r"\*", r"/", r"=", r"==", r"!=", r"<", r">", r"<=", r">=", r"&&", r"\|\|", r"!", r"\+\+", r"--"]
        for op in operators:
            self.highlighting_rules.append((re.compile(op), operator_format))

        # Strings
        self.highlighting_rules.append((re.compile(r'"[^"]*"'), string_format))
        self.highlighting_rules.append((re.compile(r"'[^']*'"), string_format))

        # Numbers
        self.highlighting_rules.append((re.compile(r"\b\d+(\.\d+)?\b"), number_format))

        # Comments
        self.highlighting_rules.append((re.compile(r"#.*"), comment_format))

    def highlightBlock(self, text):
        for pattern, fmt in self.highlighting_rules:
            for match in pattern.finditer(text):
                start, end = match.start(), match.end()
                self.setFormat(start, end - start, fmt)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = CompilerApp()
    window.show()
    sys.exit(app.exec())
