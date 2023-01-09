//Incluyendo librerias
#include <DHTesp.h>
#include <HTTPClient.h>               
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
                     

// Configuracion del sensor DHT11
DHTesp dht;                                                // Asigna un nombre a sensor DHT
int dhtPin = 23;                                           // Pin de entrada sensor DHT

// Configuracion del LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Configuracion del mensaje en scroll del LCD
String messageToScroll = "Monitoreando en linea ESP32 y sensor DHT";
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

// Informacion del WIFI
const char* ssid = "";                      // Nombre de la red WIFI
const char* password =  "";           // Contraseña de la red WIFI


// Variables usadas en el codigo
String datos_sensor = "";                 // Texto que se enviara al servidor (Temperatura y Humedad)
// String datos_sensor_h = "";                 // Texto que se enviara al servidor (Humedad)
unsigned int Actual_Millis, Previous_Millis;
int refresh_time = 3000;                   // Tasa de refresco de la conexion al sitio web (recomendado mas de 1s) 1000 = 1seg


void setup() {
  // PANTALLA LCD //
  lcd.init();
  lcd.backlight();
  
  dht.setup(dhtPin, DHTesp::DHT11);       // Inicia el sensor DHT11
  
  delay(10);
  Serial.begin(115200);                   // Inicia el monitor

  WiFi.begin(ssid, password);             // inicia la conexion WIFI
  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) { // Verifica la conexion
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado con IP: ");
  Serial.println(WiFi.localIP());
  Actual_Millis = millis();               // Guarda el tiempo para el bucle de refresco
  Previous_Millis = Actual_Millis; 
}


void loop() {  
  scrollText(1, messageToScroll, 250, lcdColumns);

  float temp = dht.getTemperature();                                              // Crea la variable temp y le asigna como valor la temperatura actual
  float hum = dht.getHumidity();                                                    // Crea la variable hum y le asigna como valor la humedad actual
  
  // Hacemos el bucle de refresco usando millis() no usamos delay();
  Actual_Millis = millis();
  if(Actual_Millis - Previous_Millis > refresh_time){
    Previous_Millis = Actual_Millis;  
    if(WiFi.status()== WL_CONNECTED){                                             // Verifica el estado de la conexion WIFI  
      HTTPClient http;                                                            // Crea un nuevo cliente
      
      datos_sensor = "temperatura="+String(temp)+"&humedad="+String(hum) ;        // Crea variable con los datos de temperatura y humedad (datos del POST)

      
      // Iniciando la conexion al sitio web       
      http.begin("https://mpl-esp32db.000webhostapp.com/esp32_dht_update.php");       // Indica la pagina web de destino 
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");        // Prepara la cabecera
      
      int response_code = http.POST(datos_sensor);                              //Envia el POST con datos de temperatura. Esto nos dara un codigo de respuesta
      
      // Si el codigo es mayor a 0, significa que recibimos una respuesta
      if(response_code > 0){
        Serial.println("Codigo HTTP " + String(response_code));                   // Imprime el codigo de respuesta
  
        if(response_code == 200){                                                 // Si el codigo es 200, recibimos una respuesta positiva y leemos los datos del ECHO
          String response_body = http.getString();                                // Leemos los datos procedentes del sitio web
          Serial.print("Respuesta del servidor: ");                               // Imprime los datos al monitor para depurar
          Serial.println(response_body);
          Serial.println(datos_sensor);
          lcd.setCursor(0, 0);
          lcd.print("T:");
          lcd.print(temp);
          lcd.print("C H:");
          lcd.print(hum);
          lcd.print("%");
          scrollText(1, messageToScroll, 250, lcdColumns);
          
        }                                                                         // Fin del response_code = 200
      }                                                                           // Fin del response_code > 0
      
      else{
       Serial.print("Error al enviar el POST, codigo: ");
       Serial.println(response_code);
      }
      http.end();                                                                 // Fin de la conexion
    }                                                                             // Fin de la conexion WIFI
    else{
      Serial.println("Error en la conexion WIFI");
    }
  }
}
