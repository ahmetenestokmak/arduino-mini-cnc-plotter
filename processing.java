import java.awt.event.KeyEvent;
import javax.swing.JOptionPane;
import processing.serial.*;
Serial port = null;
// select and modify the appropriate line for your operating system---
// leave as null to use interactive port (press 'p' in the program)
String portname = null;
//String portname = Serial.list()[0]; // Mac OS X
//String portname = "/dev/ttyUSB0"; // Linux
//String portname = "COM6"; // Windows 
boolean streaming = false;
float speed = 0.001;
String[] gcode;
int i = 0;
void openSerialPort() {
    if (portname == null) return;
    if (port != null) port.stop();
    port = new Serial(this, portname, 9600);
    port.bufferUntil('\n');
}
void selectSerialPort() {
    String result = (String) JOptionPane.showInputDialog(frame,
        "Select the serial port that corresponds to your Arduino board.",
        "serial port seç",
        JOptionPane.QUESTION_MESSAGE,
        null,
        Serial.list(),
        0);
    if (result != null) {
        portname = result;
        openSerialPort();
    }
}
void setup() {
    size(500, 250);
    openSerialPort();
}
void draw() {
    background(0);
    fill(255);
    int y = 24, dy = 12;
    text("Menuler", 12, y);
    y += dy;
    text("p: serial port seç", 12, y);
    y += dy;
    text("1: hız 0.001 inch", 12, y);
    y += dy;
    text("2: hız 0.010 inch", 12, y);
    y += dy;
    text("3: hız 0.100 inch", 12, y);
    y += dy;
    text("Yön Tuşları: x-y Düzlemi", 12, y);
    y += dy;
    text("Sayfa aşağı-yukarı: z eksen ", 12, y);
    y += dy;
    text("$: grbl ayarlar", 12, y);
    y += dy;
    text("h: Başlangıç Konumu", 12, y);
    y += dy;
    text("0: Güncel Konum", 12, y);
    y += dy;
    text("g: g-code dosya seç ", 12, y);
    y += dy;
    text("x: Çizimi Durdur", 12, y);
    y += dy;
    y = height - dy;
    text("güncel hız: " + speed + " adım", 12, y);
    y -= dy;
    text("güncel serial port: " + portname, 12, y);
    y -= dy;
}
void keyPressed() {
    if (key == '1') speed = 0.001;
    if (key == '2') speed = 0.01;
    if (key == '3') speed = 0.1;
    if (!streaming) {
        if (keyCode == LEFT) port.write("G91\nG20\nG00 X-" + speed + " Y0.000 Z0.000\n");
        if (keyCode == RIGHT) port.write("G91\nG20\nG00 X" + speed + " Y0.000 Z0.000\n");
        if (keyCode == UP) port.write("G91\nG20\nG00 X0.000 Y" + speed + " Z0.000\n");
        if (keyCode == DOWN) port.write("G91\nG20\nG00 X0.000 Y-" + speed + " Z0.000\n");
        if (keyCode == KeyEvent.VK_PAGE_UP) port.write("G91\nG20\nG00 X0.000 Y0.000 Z" + speed + "\n");
        if (keyCode == KeyEvent.VK_PAGE_DOWN) port.write("G91\nG20\nG00 X0.000 Y0.000 Z-" + speed + "\n");
        if (key == 'h') port.write("G90\nG20\nG00 X0.000 Y0.000 Z0.000\n");
        if (key == 'v') port.write("$0=75\n$1=74\n$2=75\n");
        //if (key == 'v') port.write("$0=100\n$1=74\n$2=75\n");
        if (key == 's') port.write("$3=10\n");
        if (key == 'e') port.write("$16=1\n");
        if (key == 'd') port.write("$16=0\n");
        if (key == '0') openSerialPort();
        if (key == 'p') selectSerialPort();
        if (key == '$') port.write("$$\n");
    }
    if (!streaming && key == 'g') {
        gcode = null;
        i = 0;
        File file = null;
        println("Loading file...");
        selectInput("Select a file to process:", "fileSelected", file);
    }
    if (key == 'x') streaming = false;
}
void fileSelected(File selection) {
    if (selection == null) {
        println("Window was closed or the user hit cancel.");
    } else {
        println("User selected " + selection.getAbsolutePath());
        gcode = loadStrings(selection.getAbsolutePath());
        if (gcode == null) return;
        streaming = true;
        stream();
    }
}
void stream() {
    if (!streaming) return;
    while (true) {
        if (i == gcode.length) {
            streaming = false;
            return;
        }
        if (gcode[i].trim().length() == 0) i++;
        else break;
    }
    println(gcode[i]);
    port.write(gcode[i] + '\n');
    i++;
}
void serialEvent(Serial p) {
    String s = p.readStringUntil('\n');
    println(s.trim());
    if (s.trim().startsWith("ok")) stream();
    if (s.trim().startsWith("error")) stream(); // XXX: really?
}
