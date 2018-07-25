
/*
  MB-8RO_Modbus.ino
  25/07/2018 - v1.00
  - Versión inicial
 
  Sketch para usar el módulo MB-8RO como esclavo modbus RTU bajo RS485
  Copyright (c) 2018 Raimundo Alfonso
  Ray Ingeniería Electrónica, S.L.
  
  Este sketch está basado en software libre. Tu puedes redistribuir
  y/o modificarlo bajo los terminos de licencia GNU.

  Esta biblioteca se distribuye con la esperanza de que sea útil,
  pero SIN NINGUNA GARANTÍA, incluso sin la garantía implícita de
  COMERCIALIZACIÓN O PARA UN PROPÓSITO PARTICULAR.
  Consulte los terminos de licencia GNU para más detalles.
  
  * CARACTERISTICAS
  - Escritura de reles
  - Parametros de comunicación RTU, 9600 baudios,n,8,1
  
  * MAPA MODBUS
  
	MODO R: FUNCION 1 - READ BLOCK COILS REGISTERS (dirección coil = dirección * 16) 
    MODO R: FUNCION 3 - READ BLOCK HOLDING REGISTERS
    MODO W: FUNCION 6 - WRITE SINGLE HOLDING REGISTER
    
  DIRECCION   TIPO    MODO  FORMATO    MAXIMO      MINIMO    UNIDADES    DESCRIPCION
  ---------------------------------------------------------------------------------------------------------
  0x0005      int     R/W   00000      00015       00000     ---         Salidas reles en formato binario
  0x0006      int     R     00000      00063       00000     ---         Dirección modbus o estado del dipswitch     
*/

#include <ModbusSlave.h>
#include <avr/wdt.h> 

// Pines E/S asignados:
#define RELE1      7
#define RELE2      8
#define RELE3      9
#define RELE4      10
#define RELE5      14
#define RELE6      15
#define RELE7      16
#define RELE8      17
#define DIPSW1     4   
#define DIPSW2     5    
#define DIPSW3     6    
#define DIPSW4     11    
#define DIPSW5     AN6    
#define DIPSW6     AN7    

#define MAX_BUFFER_RX  15

// Mapa de registros modbus
enum {        
        MB_1,            // reservado
        MB_2,            // reservado
        MB_3,            // reservado
        MB_4,            // reservado
        MB_5,            // reservado
        MB_RELAY,        // Salidas de rele
        MB_DIPSW,        // Estado del dipswitch 
        MB_REGS	 	       // Numero total de registros
};
int regs[MB_REGS];	

// Crea la clase para el modbus...
ModbusSlave modbus;

void setup()  { 
  wdt_disable();
  
  // Configura entradas y salidas...
  pinMode(RELE1, OUTPUT);  
  pinMode(RELE2, OUTPUT);  
  pinMode(RELE3, OUTPUT);  
  pinMode(RELE4, OUTPUT);  
  pinMode(RELE5, OUTPUT);
  pinMode(RELE6, OUTPUT);
  pinMode(RELE7, OUTPUT);
  pinMode(RELE8, OUTPUT);
  pinMode(DIPSW1, INPUT_PULLUP);
  pinMode(DIPSW2, INPUT_PULLUP);
  pinMode(DIPSW3, INPUT_PULLUP);
  pinMode(DIPSW4, INPUT_PULLUP);
   
  // configura modbus...
  modbus.config(9600,'n');
  modbus.direccion = leeDIPSW();
  
  // Activa WDT cada 4 segundos...   
  wdt_enable(WDTO_4S); 
} 



void loop()  { 
  unsigned long curMillis = millis();          // Get current time

  // Lee dipswitch...
  regs[MB_DIPSW] = leeDIPSW();
      
  // Espera lectura de tramas modbus
  delay_modbus(100);
  
  // Actualiza reles...
  actualizaReles();
  
  // Reset WDT
  wdt_reset();
}

// Rutina de espera que atiende la tarea modbus...
void delay_modbus(int t){
  int n,tt;
  tt = t/10;
  
  for(n=0;n<=tt;n++){
    modbus.actualiza(regs,MB_REGS);
    delay(10);
  }  
}

void actualizaReles(void){
  if((regs[MB_RELAY] & 0x01) == 0x01) digitalWrite(RELE1,HIGH); else digitalWrite(RELE1,LOW);
  if((regs[MB_RELAY] & 0x02) == 0x02) digitalWrite(RELE2,HIGH); else digitalWrite(RELE2,LOW);
  if((regs[MB_RELAY] & 0x04) == 0x04) digitalWrite(RELE3,HIGH); else digitalWrite(RELE3,LOW);
  if((regs[MB_RELAY] & 0x08) == 0x08) digitalWrite(RELE4,HIGH); else digitalWrite(RELE4,LOW); 
  if((regs[MB_RELAY] & 0x10) == 0x10) digitalWrite(RELE5,HIGH); else digitalWrite(RELE5,LOW); 
  if((regs[MB_RELAY] & 0x20) == 0x20) digitalWrite(RELE6,HIGH); else digitalWrite(RELE6,LOW); 
  if((regs[MB_RELAY] & 0x40) == 0x40) digitalWrite(RELE7,HIGH); else digitalWrite(RELE7,LOW); 
  if((regs[MB_RELAY] & 0x80) == 0x80) digitalWrite(RELE8,HIGH); else digitalWrite(RELE8,LOW); 
}

// Rutina para leer el dipswitch
byte leeDIPSW(void){
  byte a0,a1,a2,a3,a4,a5;
  
  // Lee dipswitch...
  a0 = !digitalRead(DIPSW1);  
  a1 = !digitalRead(DIPSW2);
  a2 = !digitalRead(DIPSW3);
  a3 = !digitalRead(DIPSW4);
  if(analogRead(A6) < 512)
    a4 = 1;  
  else
    a4 = 0;
  if(analogRead(A7) < 512)
    a5 = 1;  
  else
    a5 = 0;
  // Calcula dirección...
  return(a0 + a1*2 + a2*4 + a3*8 + a4*16 + a5*32);
}


