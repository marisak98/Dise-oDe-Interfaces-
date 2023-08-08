#include <esp_now.h>
#include <WiFi.h>



// REEMPLAZA CON LA Dirección MAC de tu emisor
uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xA1, 0x87, 0x60};

// Definir variables para almacenar lecturas entrantes
int comando;

//Ejemplo de estructura para enviar datos
//Debe coincidir con la estructura del receptor
typedef struct struct_message {
    int control;
} struct_message;

struct_message comandos;

esp_now_peer_info_t peerInfo;

// Devolución de llamada cuando se reciben datos
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&comandos, incomingData, sizeof(comandos));
  comando = comandos.control;
}

///////////////////PARAMETROS////////////////
int velocidadMax = 220;

//////////////////////MOTOR DERECHO///////////////////////////////
int velocidadDerecha = velocidadMax; // Ciclo de trabajo 0 - 255
int canal1MotorD = 0;
int canal2MotorD = 1;

//// Puente H L298N ////
const int    IN1 = 26;                 
const int    IN2 = 25;         

//////////////////////MOTOR IZQUIERDO///////////////////////////////
int velocidadIzquierda = velocidadMax; // Ciclo de trabajo 0 - 255
int canal1MotorI = 2;
int canal2MotorI = 3;

//// Puente H L298N ////
const int    IN3 = 33;                  
const int    IN4 = 32;                  

// Configuracion de las salidas pwm
const int frecuencia = 10000;
const int resolucion= 8;


void setup()
{
  
  initReceptor();
 
  ledcSetup(canal1MotorD, frecuencia, resolucion);
  ledcAttachPin(IN1,canal1MotorD);
  
  ledcSetup(canal2MotorD, frecuencia, resolucion);
  ledcAttachPin(IN2,canal2MotorD); 
  
  ledcSetup(canal1MotorI, frecuencia, resolucion);
  ledcAttachPin(IN3,canal1MotorI);

  ledcSetup(canal2MotorI, frecuencia, resolucion);
  ledcAttachPin(IN4,canal2MotorI);

  pararMotor(canal1MotorD,canal2MotorD);
  pararMotor(canal1MotorI,canal2MotorI);
     
}
void loop() 
{

    
    if(comando == 1) // Adelante
    {
      adelante();
    }

    else if(comando == 2) // Atras
    {
      atras();
    }
    
    else if(comando == 3) // Derecha
    {
      derecha();
    }
    
    else if(comando == 4) // Izquierda
    {
      izquierda();
    }
    else // Parar
    {
      detener();
    }
    


  delay(100);
}


void adelante()
{
  giroHorario(canal1MotorD,canal2MotorD,velocidadDerecha);
  giroAntihorario(canal1MotorI,canal2MotorI,velocidadIzquierda);
}

void atras()
{
  giroAntihorario(canal1MotorD,canal2MotorD,velocidadDerecha);
  giroHorario(canal1MotorI,canal2MotorI,velocidadIzquierda);
}

void derecha()
{
  giroAntihorario(canal1MotorD,canal2MotorD,velocidadDerecha);
  giroAntihorario(canal1MotorI,canal2MotorI,velocidadIzquierda);
}

void izquierda()
{
  giroHorario(canal1MotorD,canal2MotorD,velocidadDerecha);
  giroHorario(canal1MotorI,canal2MotorI,velocidadIzquierda);
}

void detener()
{
  pararMotor(canal1MotorD,canal2MotorD);
  pararMotor(canal1MotorI,canal2MotorI);
}


void giroHorario(int canal1, int canal2, int cv)
{     
  ledcWrite(canal1,cv);
  ledcWrite(canal2,0);
}

void giroAntihorario(int canal1, int canal2, int cv)
{     
  ledcWrite(canal1,0);
  ledcWrite(canal2,cv);
}

void pararMotor(int canal1, int canal2)
{     
  ledcWrite(canal1,0);
  ledcWrite(canal2,0);
}

void initReceptor()
{
    // Establecer el dispositivo como estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Iniciar ESP-AHORA
  if (esp_now_init() != ESP_OK) {
    return;
  }
 // Registrar par
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Agregar par      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }
  
  // Regístrese para una función de devolución de llamada que se llamará cuando se reciban los datos
  esp_now_register_recv_cb(OnDataRecv);
}
