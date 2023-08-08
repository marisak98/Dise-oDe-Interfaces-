#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <esp_now.h>
#include <WiFi.h>

Adafruit_MPU6050 mpu;

///////////////////////////////// Variables Red Neuronal /////////////////////////////////
double a0[3];
double W1[2][3] = {{-2.971,-5.946,-1.607},{4.525,-3.015,-0.155}};
double a1[2];
double W2[5][2] = {{-5.312,-6.41},{1.409,-0.107},{-4.546,2.021},{2.876,-4.4},{-2.636,0.134}};
double a2[5]; 
double b1[2]= {0.938,2.747};
double b2[5]= {6.836,-4.389,-7.077,-1.423,2.411};
double aux = 0.0;
//////////////////////////////////////////////////////////////////



///////////////////////////////// Preprocesamiento Red Neuronal /////////////////////////////////
double mean[3]={-0.236,0.067,6.707};
double dstd[3]={5.21,4.348,2.914};
///////////////////////////////////////////////////////////////////////////////////////////////////////


// REEMPLAZA CON LA Dirección MAC de tu receptor
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0x6B, 0xCA, 0x64};

// Variable para almacenar si el envío de datos fue exitoso
String success;

//Ejemplo de estructura para enviar datos
//Debe coincidir con la estructura del receptor
typedef struct struct_message {
    int control;
} struct_message;

struct_message comandos;

esp_now_peer_info_t peerInfo;

// Devolución de llamada cuando se envían datos
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  status == ESP_NOW_SEND_SUCCESS ? "Entrega exitosa" : "Error de entrega";
  if (status ==0){
    success = "Entrega exitosa :)";
  }
  else{
    success = "Error de entrega :(";
  }
}
 
void setup() {

  if (!mpu.begin()) {
    while (1) {
      delay(10);
    }
  }


  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  
  initEmisor();
  
  delay(100);
}
 
void loop() {
  
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  a0[0]=dataNormalized(a.acceleration.x,mean[0],dstd[0]);
  a0[1]=dataNormalized(a.acceleration.y,mean[1],dstd[1]);
  a0[2]=dataNormalized(a.acceleration.z,mean[2],dstd[2]);
  
  ///////////////////////////////// Estructura Red Neuronal /////////////////////////////////
  for(int i = 0 ; i<2; i++ ) {aux=0.0;for(int j = 0 ; j <3 ; j++ ) { aux=aux+W1[i][j]*a0[j];} a1[i]=relu(aux+b1[i]);}
  double aux1 = 0;
  for(int i = 0 ; i<5; i++ ) {aux=0.0;for(int j = 0 ; j <2 ; j++ ){ aux=aux+W2[i][j]*a1[j];} a2[i]=(aux+b2[i]);aux1=aux1+exp(a2[i]);}
  double minimo = 0.0;
  int classes = 0;
  for(int i = 0;  i<5; i++){a2[i] = exp(a2[i])/aux1;if(a2[i]>minimo){minimo=a2[i];classes=i;}}
  //////////////////////////////////////////////////////////////////////////////////////////
  
  comandos.control = classes+1; 

  // Enviar mensaje a través de ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &comandos, sizeof(comandos));
   

  delay(200);
}

double dataNormalized(double inputData, double mean, double desvStandar)
{
  return (inputData-mean)/desvStandar; 
}

double relu(double n)
{
  if(n>=0) return n; else if (n<0) return 0;
}

void initEmisor(){
   
   // Establecer el dispositivo como estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Iniciar ESP-AHORA
  if (esp_now_init() != ESP_OK) {
    return;
  }

  // Una vez que ESPNow se inicie con éxito, nos registraremos en 
  // Enviar CB para obtener el estado del paquete transmitido
  esp_now_register_send_cb(OnDataSent);
  
  // Registrar par
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Agregar par      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }
}
