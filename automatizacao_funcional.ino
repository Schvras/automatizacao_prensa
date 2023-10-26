#include <AFMotor.h>
#include <Servo.h>

#define s_fim_esteira A1
#define s_ini_esteira A0
#define enable_tag    A2
#define discard_tag   A3
#define sentido_esteira FORWARD

AF_DCMotor esteira(4);
Servo servo1;

bool  emergencia = false;
int   passo_magazine = 0;
int   passo_esteira = 1;
int   pos_ini_servo = 20;
int   pos_fim_servo = 90;
long  contador_encoder_esteira = 0;
unsigned long t_servo = 0;
unsigned long t_esteira = 0;

void setup() {

  Serial.begin(9600);
  Serial.println("Iniciando");

  // Definir pino dos sensores
  pinMode(s_fim_esteira,INPUT_PULLUP);
  pinMode(s_ini_esteira,INPUT_PULLUP);
  pinMode(enable_tag,INPUT_PULLUP);
  pinMode(discard_tag,INPUT_PULLUP);
  servo1.attach(9);                   // Pino de acionamento do servo
  esteira.setSpeed(200);              // Velocidade

  // Se possuir tag no final da esteira e também no inicio, entra em emergência
  if (digitalRead(s_fim_esteira) == LOW and digitalRead(s_ini_esteira) == LOW and emergencia == false){
    emergencia = true;
    Serial.println("Entrou em emergência");
  }

  // Se possuir tag no final da esteira e nenhuma no inicio, solta a mesma
  if (digitalRead(s_fim_esteira) == LOW and digitalRead(s_ini_esteira) == HIGH and emergencia == false){

    t_esteira = millis();
    while(digitalRead(s_fim_esteira) == LOW){
      esteira.run(sentido_esteira);       // Aciona para soltar a tag no final

      if(millis() - t_esteira > 3000){
        emergencia = true;
        break;
      }
    }

    esteira.run(RELEASE);               // Aciona para voltar qualquer tag alimentada
    delay(1000);
  }
  esteira.run(RELEASE);                 // Para o acionamento

  if(emergencia == false){
    // Posição inicial do servo
    servo1.write(pos_ini_servo);        // Posição inicial

    delay(1000);                        // Tempo para estabilização
  }
}

void loop() {

  if(Serial.available()){
    contador_encoder_esteira = Serial.readString().toInt();
  }

  // Emergência
  if(emergencia == true){

    if(servo1.read() == 20){
      servo1.write(30);
      delay(100);
    }else{
      servo1.write(20);
      delay(100);
    }
  }else{

    // Aciona braço com servo
    if((passo_magazine == 0) or (passo_magazine == 1 and servo1.read() == pos_ini_servo and millis() - t_servo > 1000)){
      servo1.write(pos_fim_servo);
      t_servo = millis();
      passo_magazine = 2;
      Serial.println("Vacuo ligado");
    }

    // Desaciona braço com servo
    if(passo_magazine == 2 and servo1.read() == pos_fim_servo and millis() - t_servo > 1000){
      servo1.write(pos_ini_servo);
      t_servo = millis();
      passo_magazine = 3;
    }

    // Desliga vacuo
    if(passo_magazine == 3 and servo1.read() == pos_ini_servo and millis() - t_servo > 1000 and digitalRead(s_ini_esteira) == HIGH and digitalRead(s_fim_esteira) == LOW){
      // Desliga vacuo
      t_servo = millis();
      passo_magazine = 1;
      Serial.println("Vacuo desligado");
    }

    /* ESTEIRA */

    // Aciona esteira
    if(passo_esteira == 1 and digitalRead(s_ini_esteira) == LOW and digitalRead(s_fim_esteira) == HIGH and digitalRead(enable_tag) == HIGH){
      esteira.run(sentido_esteira);
      passo_esteira = 2;
      Serial.println("Acionando esteira");
    }

    // Contar passos esteira
    if(passo_esteira == 2 and digitalRead(s_fim_esteira) == LOW){
      contador_encoder_esteira = 0;
      passo_esteira = 3;
      Serial.println("Contando passos");
    }

    // Parar esteira
    if(passo_esteira == 3 and contador_encoder_esteira >= 200){
      esteira.run(RELEASE);
      passo_esteira = 4;
      Serial.println("Atingiu numero de passos");
    }

    // Soltar tag esteira
    if(passo_esteira == 4 and digitalRead(discard_tag) == LOW){
      esteira.run(sentido_esteira);
      passo_esteira = 5;
      Serial.println("Soltando tag");
    }

    // Acabou de soltar tag esteira
    if(passo_esteira == 5 and digitalRead(s_fim_esteira) == HIGH){
      esteira.run(RELEASE);
      passo_esteira = 1;
      Serial.println("Soltou a tag");
    }
  }
}
