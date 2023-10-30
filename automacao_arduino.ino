#include <AFMotor.h>
#include <Servo.h>
#include <Encoder.h>

#define motor_number        4
#define pin_a_encoder       5
#define pin_b_encoder       2
#define pin_servo           9
#define pin_vacuo           10
#define pin_ini_esteira     A0
#define pin_fim_esteira     A1
#define aciona_tag          A2
#define descarta_tag        A3
#define rotacao_esteira     FORWARD
#define verdadeiro_barreira HIGH
#define falso_barreira      LOW

bool  emergencia                = false;
int   passo_magazine            = 0;
int   passo_esteira             = 1;
int   pos_ini_servo             = 20;
int   pos_fim_servo             = 90;
long  contador_encoder_esteira  = 0;
long  numero_passos             = 200;
unsigned long tempo_servo           = 0;
unsigned long t_esteira         = 0;
unsigned long diferenca_tempo   = 0;

Encoder encoder_esteira(pin_a_encoder, pin_b_encoder);
AF_DCMotor esteira(motor_number);
Servo servo1;

void setup() {

  Serial.begin(9600);
  Serial.println("Iniciando");

  // Definir pino dos sensores
  pinMode(pin_ini_esteira,  INPUT_PULLUP);
  pinMode(pin_fim_esteira,  INPUT_PULLUP);
  pinMode(aciona_tag,       INPUT_PULLUP);
  pinMode(descarta_tag,     INPUT_PULLUP);
  pinMode(pin_vacuo,        OUTPUT);

  // Pino de acionamento do servo
  servo1.attach(pin_servo);

  // Velocidade esteira
  esteira.setSpeed(200);

  // Posição inicial do servo
  servo1.write(pos_ini_servo);
  delay(1000);

  // Se possuir tag no final da esteira e nenhuma no inicio, solta a mesma
  if (digitalRead(pin_fim_esteira) == verdadeiro_barreira or digitalRead(pin_ini_esteira) == verdadeiro_barreira){

    t_esteira = millis();
    while(digitalRead(pin_fim_esteira) == LOW){

      // Aciona para soltar a tag no final
      esteira.run(rotacao_esteira);

      // Caso demore mais de 5 segundos entra em emergência
      if(millis() - t_esteira > 5000){
        emergencia = true;
        break;
      }
    }
  }
  esteira.run(RELEASE);                 // Para o acionamento
}

void loop() {

  if(Serial.available()){
    numero_passos = Serial.readString().toInt();
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
  
  // Não em emergência
  }else{

  /* Servo alimentador */

    diferenca_tempo = millis() - tempo_servo;

    // Aciona braço com servo
    if((passo_magazine == 0) or (passo_magazine == 1 and diferenca_tempo > 2000)){
      passo_magazine  = 2;
      tempo_servo     = millis();

      servo1.write(pos_fim_servo);
      digitalWrite(pin_vacuo, HIGH);

      Serial.println("Vacuo ligado");
    }

    // Desaciona braço com servo
    if(passo_magazine == 2 and diferenca_tempo > 1000){
      passo_magazine  = 3;
      tempo_servo     = millis();

      servo1.write(pos_ini_servo);
    }

    // Desliga vacuo
    if(passo_magazine == 3 and diferenca_tempo > 1000 and digitalRead(pin_ini_esteira) == falso_barreira and digitalRead(pin_fim_esteira) == falso_barreira){
      passo_magazine  = 1;
      tempo_servo     = millis();

      digitalWrite(pin_vacuo, LOW);
      
      Serial.println("Vacuo desligado");
    }

  /* Esteira */

    // Aciona esteira
    if(passo_esteira == 1 and digitalRead(pin_ini_esteira) == verdadeiro_barreira and digitalRead(pin_fim_esteira) == falso_barreira and digitalRead(aciona_tag) == HIGH){
      passo_esteira = 2;

      esteira.run(rotacao_esteira);

      Serial.println("Acionando esteira");
    }

    // Contar passos esteira
    if(passo_esteira == 2 and digitalRead(pin_fim_esteira) == verdadeiro_barreira){
      contador_encoder_esteira  = 0;
      passo_esteira             = 3;

      Serial.println("Contando passos");
    }

    // Conta enquanto o passo for 3
    if(passo_esteira = 3){
      contador_encoder_esteira = encoder_esteira.read();
    }

    // Parar esteira se atingir numero de passos
    if(passo_esteira == 3 and contador_encoder_esteira >= numero_passos){
      passo_esteira = 4;

      esteira.run(RELEASE);

      Serial.println("Atingiu numero de passos");
    }

    // Soltar tag esteira
    if(passo_esteira == 4 and digitalRead(descarta_tag) == LOW){
      passo_esteira = 5;

      esteira.run(rotacao_esteira);
      
      Serial.println("Soltando tag");
    }

    // Acabou de soltar tag esteira
    if(passo_esteira == 5 and digitalRead(pin_fim_esteira) == falso_barreira){
      passo_esteira = 1;

      esteira.run(RELEASE);

      Serial.println("Soltou a tag");
    }
  }
}
