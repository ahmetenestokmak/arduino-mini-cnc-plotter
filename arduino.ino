#include <Servo.h>
#include <Stepper.h>
#define LINE_BUFFER_LENGTH 512
const int kalem_kaldir = 140;      
const int kalem_indir = 110;          
const int kalem_servo_pin = 12;   
const int adim_sayisi = 20; 
const int hareket_hizi = 250; 
Servo kalem_servo;         
Stepper y_ekseni(adim_sayisi, 5,4,3,2);           
Stepper x_ekseni(adim_sayisi, 6,7,8,9); 
float x_adim_mm = 6.0;
float y_adim_mm = 6.0;
struct point { 
  float x; 
  float y; 
  float z; 
};
struct point actuatorPos;
float StepInc = 1;
int StepDelay = 0;
int LineDelay = 50;
int penDelay = 50;
float Xmin = 0;
float Xmax = 40;
float Ymin = 0;
float Ymax = 40;
float Zmin = 0;
float Zmax = 1;
float Xpos = Xmin;
float Ypos = Ymin;
float Zpos = Zmax; 
boolean verbose = false;

void setup() {
  //  Setup
  Serial.begin( 9600 );
  kalem_servo.attach(kalem_servo_pin);
  kalem_servo.write(kalem_kaldir);
  delay(200);
  x_ekseni.setSpeed(hareket_hizi);
  y_ekseni.setSpeed(hareket_hizi);  
  Serial.println("Çizim Yapan Mini CNC!");
  Serial.print("X min "); 
  Serial.print(Xmin); 
  Serial.print(" den "); 
  Serial.print(Xmax); 
  Serial.println(" mm."); 
  Serial.print("Y min "); 
  Serial.print(Ymin); 
  Serial.print(" den "); 
  Serial.print(Ymax); 
  Serial.println(" mm."); 
}

void loop() 
{
  delay(200);
  char line[ LINE_BUFFER_LENGTH ];
  char c;
  int lineIndex;
  bool lineIsComment, lineSemiColon;
  lineIndex = 0;
  lineSemiColon = false;
  lineIsComment = false;
  while (1) {
    while ( Serial.available()>0 ) {
      c = Serial.read();
      if (( c == '\n') || (c == '\r') ) {         
        if ( lineIndex > 0 ) {                 
          line[ lineIndex ] = '\0';       
          if (verbose) { 
            Serial.print( "Alıcı : "); 
            Serial.println( line ); 
          }
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } 
        lineIsComment = false;
        lineSemiColon = false;
        Serial.println("ok");    
      } 
      else {
        if ( (lineIsComment) || (lineSemiColon) ) { 
          if ( c == ')' )  lineIsComment = false; } 
        else {
          if ( c <= ' ' ) { } 
          else if ( c == '/' ) {} 
          else if ( c == '(' ) {lineIsComment = true; } 
          else if ( c == ';' ) {lineSemiColon = true;} 
          else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "Yazma Hatası" );
            lineIsComment = false;
            lineSemiColon = false;
          } 
          else if ( c >= 'a' && c <= 'z' ) {
            line[ lineIndex++ ] = c-'a'+'A';
          } 
          else {line[ lineIndex++ ] = c;}
        }
      }
    }
  }
}
void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  char buffer[ 64 ];                  
  struct point newPos;
  newPos.x = 0.0;
  newPos.y = 0.0;
  while( currentIndex < charNB ) {
    switch ( line[ currentIndex++ ] ) {           
    case 'U':penUp(); break;
    case 'D':penDown(); break;
    case 'G':buffer[0] = line[ currentIndex++ ];          
      	      buffer[1] = '\0'; switch ( atoi( buffer ) ){  
      case 1:char* indexX = strchr( line+currentIndex, 'X' );  
        char* indexY = strchr( line+currentIndex, 'Y' );
        if ( indexY <= 0 ) {
          newPos.x = atof( indexX + 1); 
          newPos.y = actuatorPos.y;
        } 
        else if ( indexX <= 0 ) {
          newPos.y = atof( indexY + 1);
          newPos.x = actuatorPos.x;
        } 
        else {
          newPos.y = atof( indexY + 1);
          indexY = '\0';
          newPos.x = atof( indexX + 1);
        }
        drawLine(newPos.x, newPos.y );
        //        Serial.println("ok");
        actuatorPos.x = newPos.x;
        actuatorPos.y = newPos.y;
        break;
      }
      break;
    case 'M':
      buffer[0] = line[ currentIndex++ ];       
      buffer[1] = line[ currentIndex++ ];
      buffer[2] = line[ currentIndex++ ];
      buffer[3] = '\0';
      switch ( atoi( buffer ) ){
      case 300:
        {
          char* indexS = strchr( line+currentIndex, 'S' );
          float Spos = atof( indexS + 1);
          if (Spos == 30) {  penDown();    }
          if (Spos == 50) {      penUp();}  break;
        }
      case 114:                    
        Serial.print( "Tam Pozisyon : X = " );
        Serial.print( actuatorPos.x );
        Serial.print( "  -  Y = " );
        Serial.println( actuatorPos.y );
        break;
      default:
        Serial.print( "Komut Tanınmadı : M");
        Serial.println( buffer );
      }
    }
  }
}
void drawLine(float x1, float y1) {
  if (verbose)
  {
    Serial.print("fx1, fy1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }  
  if (x1 >= Xmax) { x1 = Xmax;  }
  if (x1 <= Xmin) { x1 = Xmin; }
  if (y1 >= Ymax) { y1 = Ymax; }
  if (y1 <= Ymin) { y1 = Ymin; }
  if (verbose)
  {
    Serial.print("Xpos, Ypos: ");
    Serial.print(Xpos);
    Serial.print(",");
    Serial.print(Ypos);
    Serial.println("");
  }
  if (verbose)
  {
    Serial.print("x1, y1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }
  x1 = (int)(x1*x_adim_mm);
  y1 = (int)(y1*y_adim_mm);
  float x0 = Xpos;
  float y0 = Ypos;
  long dx = abs(x1-x0);
  long dy = abs(y1-y0);
  int sx = x0<x1 ? StepInc : -StepInc;
  int sy = y0<y1 ? StepInc : -StepInc;
  long i;
  long over = 0;
  if (dx > dy) {
    for (i=0; i<dx; ++i) {
      x_ekseni.step(sx);
      over+=dy;
      if (over>=dx) {
        over-=dx;
        y_ekseni.step(sy);
      }
      delay(StepDelay);
    }
  }
  else {
    for (i=0; i<dy; ++i) {
      y_ekseni.step(sy);
      over+=dx;
      if (over>=dy) {
        over-=dy;
        x_ekseni.step(sx);
      }
      delay(StepDelay);
    }    
  }
  if (verbose)
  {
    Serial.print("dx, dy:");
    Serial.print(dx);
    Serial.print(",");
    Serial.print(dy);
    Serial.println("");
  }
  if (verbose)
  {
    Serial.print("Going to (");
    Serial.print(x0);
    Serial.print(",");
    Serial.print(y0);
    Serial.println(")");
  }
  delay(LineDelay);

  Xpos = x1;
  Ypos = y1;
}


void penUp() { 
  kalem_servo.write(kalem_kaldir); 
  delay(LineDelay); 
  Zpos=Zmax; 
  if (verbose) { 
    Serial.println("Pen up!"); 
  } 
}
void penDown() { 
  kalem_servo.write(kalem_indir); 
  delay(LineDelay); 
  Zpos=Zmin; 
  if (verbose) { 
    Serial.println("Pen down."); 
  } 
}
