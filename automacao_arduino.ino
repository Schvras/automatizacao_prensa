#include <AFMotor.h>
#include <Servo.h>

#define motor_esteira       4
#define motor_braco         3
#define enc_esteira_a       A3
#define enc_esteira_b       A4
#define enc_braco_a         10
#define enc_braco_b         9
#define fim_curso_braco     A2
#define pin_vacuo           13
#define pin_ini_esteira     A0
#define pin_fim_esteira     A1
#define pin_servo           9
#define aciona_tag          A5
#define rotacao_esteira     BACKWARD
#define verdadeiro_barreira LOW
#define falso_barreira      HIGH

bool  emergencia                = false;
int   passo_magazine            = 0;
int   passo_esteira             = 1;
int   pos_ini_servo             = 20;
int   pos_fim_servo             = 65;
int   meio_sensor_fim           = 700;
long  contador_encoder_esteira  = 0;
long  numero_passos             = 100;
unsigned long tempo_servo       = 0;
unsigned long t_esteira         = 0;
unsigned long diferenca_tempo   = 0;

AF_DCMotor esteira(motor_esteira);
AF_DCMotor braco(motor_braco);

void setup() {

  Serial.begin(9600);
  Serial.println("Iniciando");

  // Definir pino dos sensores
  pinMode(enc_esteira_a,    INPUT);
  pinMode(enc_esteira_b,    INPUT);
  pinMode(enc_braco_a,      INPUT);
  pinMode(enc_braco_b,      INPUT);
  pinMode(pin_ini_esteira,  INPUT);
  pinMode(pin_fim_esteira,  INPUT);
  pinMode(aciona_tag,       INPUT);
  pinMode(pin_vacuo,        OUTPUT);
  
  // Velocidade esteira
  esteira.setSpeed(255);
  braco.setSpeed(255);

  // Se possuir tag no final da esteira e nenhuma no inicio, solta a mesma
  if (analogRead(pin_fim_esteira) < meio_sensor_fim or digitalRead(pin_ini_esteira) == verdadeiro_barreira){

    t_esteira = millis();
    while(analogRead(pin_fim_esteira) < meio_sensor_fim){

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

    esteira.run(FORWARD);
  
  // Não em emergência
  }else{

  /* Servo alimentador */

    diferenca_tempo = millis() - tempo_servo;

    // Aciona braço com servo
    if((passo_magazine == 0) or (passo_magazine == 1 and diferenca_tempo > 2000)){
      passo_magazine  = 2;
      tempo_servo     = millis();

      //servo1.write(pos_fim_servo);
      digitalWrite(pin_vacuo, HIGH);

      Serial.println("Vacuo ligado");
    }

    // Desaciona braço com servo
    if(passo_magazine == 2 and diferenca_tempo > 1000){
      passo_magazine  = 3;
      tempo_servo     = millis();

      //servo1.write(pos_ini_servo);
    }

    // Desliga vacuo
    if(passo_magazine == 3 and diferenca_tempo > 1000 and digitalRead(pin_ini_esteira) == falso_barreira and analogRead(pin_fim_esteira) > meio_sensor_fim){
      passo_magazine  = 1;
      tempo_servo     = millis();

      digitalWrite(pin_vacuo, LOW);
      
      Serial.println("Vacuo desligado");
    }

  /* Esteira */

    // Aciona esteira
    if(passo_esteira == 1 and digitalRead(pin_ini_esteira) == verdadeiro_barreira and analogRead(pin_fim_esteira) > meio_sensor_fim and digitalRead(aciona_tag) == HIGH){
      passo_esteira = 2;

      esteira.run(rotacao_esteira);

      Serial.println("Acionando esteira");
    }
    
    // Contar passos esteira
    if(passo_esteira == 2 and analogRead(pin_fim_esteira) < meio_sensor_fim){
      contador_encoder_esteira  = 0;
      passo_esteira             = 3;

      Serial.println("Contando passos");
    }

    // Conta enquanto o passo for 3
    if(passo_esteira == 3){
      // Incrementa ou decrementa o contador de acordo com a condição do sinal no canal A
      if (digitalRead(enc_braco_a) == LOW && digitalRead(enc_braco_b) == HIGH) {
        contador_encoder_esteira --;
      }
      else {
        contador_encoder_esteira ++;
      }
      Serial.println(contador_encoder_esteira);
    }

    // Parar esteira se atingir numero de passos
    if(passo_esteira == 3 and contador_encoder_esteira >= numero_passos){
      passo_esteira = 4;
      t_esteira = millis();

      esteira.run(RELEASE);

      Serial.println("Atingiu numero de passos");
    }

    // Soltar tag esteira
    if(passo_esteira == 4 and millis() - t_esteira > 2000){
      passo_esteira = 5;

      esteira.run(rotacao_esteira);
      
      Serial.println("Soltando tag");
    }

    // Acabou de soltar tag esteira
    if(passo_esteira == 5 and analogRead(pin_fim_esteira) > meio_sensor_fim){
      passo_esteira = 1;

      esteira.run(RELEASE);

      Serial.println("Soltou a tag");
    }
  }
}
