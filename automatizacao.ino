#include <AFMotor.h>

#define enable              13
#define ativa               12
#define fimCursoVacuo       11
#define barreiraPrimaria    3
#define barreiraSecundaria  2
#define encoderA            5
#define encoderB            6
#define motorEsteira        7
#define motorVacuo          8

AF_DCMotor motorDC(4);

int passo = 0 ;
unsigned long tempoAnterior = 0;

// passo 0, inicialização
// passo 1, pegar tag
// passo 2, soltar tag
// passo 3, alimetar cabeçote

void inicializarMagazine();
void pegarTag();
void soltarTag();
void alimetarCabecote();

void setup() { 
  // Inicia a comunicação serial com uma taxa de transmissão de 9600 boud rate
  Serial.begin(9600);
}
 
void loop(){
  if (digitalRead(enable)){

    switch (passo) {
      case 0:
        inicializarMagazine();
        break;
      case 1:
        pegarTag();
        break;
      case 2:
        soltarTag();
        break;
      case 3:
        alimetarCabecote();
        break;
      default:
        // comando(s)
        break;
    }
    Serial.println("Enabled");
  }else{
    
    passo = 0;

    motorDC.setSpeed(255);
    motorDC.run(FORWARD);
  }
  delay(5);
}

void inicializarMagazine(){

  if(digitalRead(fimCursoVacuo)){
    digitalWrite(motorVacuo, LOW);
    digitalWrite(motorEsteira, HIGH);
    tempoAnterior = millis();
    Serial.println("Acionado fim de curso");
  }else{

    digitalWrite(motorVacuo, HIGH);
    
    if(millis() - tempoAnterior >= 2000){
      tempoAnterior = millis();

      motorDC.setSpeed(255);
      motorDC.run(FORWARD);
      passo = 1;
    }else{
      motorDC.setSpeed(255);
      motorDC.run(RELEASE);
      Serial.println("Acionado motor esteira");
    }
  }
}

void pegarTag(){
  if(!digitalRead(fimCursoVacuo)){
    tempoAnterior = millis();
    digitalWrite(motorVacuo, LOW);
  }else{
    if(millis() - tempoAnterior >= 1000){
      tempoAnterior = millis();
      digitalWrite(motorVacuo, HIGH);
  
      passo = 2;
    }else{
      digitalWrite(motorVacuo, LOW);
    }
  }
}

void soltarTag(){
  if(digitalRead(fimCursoVacuo)){
    digitalWrite(motorVacuo, LOW);
    tempoAnterior = millis();
  }else{
    if(millis() - tempoAnterior >= 1000){
      tempoAnterior = millis();
      passo = 3;
      digitalWrite(motorVacuo, HIGH);
    }
  }
}

void alimetarCabecote(){
  if(millis() - tempoAnterior >= 1000){
    tempoAnterior = millis();
    motorDC.setSpeed(0);
    motorDC.run(RELEASE);

    passo = 1;
  }else{
    motorDC.setSpeed(255);
    motorDC.run(FORWARD);
  }
}
