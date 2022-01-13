// flanc montant
class PositivEdge {
  private :
    boolean memPrevState;
    boolean out;
  public :
    PositivEdge(boolean condition);                      //constructor
    boolean eval(boolean condition);
    boolean get_out();
};
PositivEdge::PositivEdge(boolean condition) {
  this->memPrevState = condition;
}
boolean PositivEdge::eval(boolean condition) { //update positiv edge state must be done ONLY ONCE by loop cycle
  out = condition && !memPrevState;
  memPrevState = condition;
  return out;
}
boolean PositivEdge::get_out() {  //use get_out() to know positiv edge state (use more than once by cycle is possible)
  return this < - out;
} // fin flanc montant
//timer
class OnDelayTimer {

  private :
    unsigned long presetTime = 1000;
    unsigned long memStartTimer = 0;            //memory top timer activation
    unsigned long elpasedTime = 0;             //elapsed time from start timer activation
    boolean memStartActivation;                //for positive edge detection of the activation condition
    boolean outTimer;                          //timer's out : like normally open timer switch
  public :
    OnDelayTimer(unsigned long _presetTime);   //constructor
    boolean updtTimer(boolean activation);      //return tempo done must be executed on each program scan
    unsigned long get_elapsedTime();           //return
    set_presetTime(unsigned long _presetTime); //change defaut preset assigned when instance created
    boolean get_outTimer();

};//end class OnDelayTimer
//constructor
OnDelayTimer::OnDelayTimer(unsigned long presetTime) {
  this -> presetTime = presetTime;
}
boolean OnDelayTimer::updtTimer(boolean activation) {
  if (!activation) {
    elpasedTime = 0;
    memStartActivation = false;
    outTimer = false;
    return false;
  } else {

    if (!memStartActivation) {
      memStartTimer = millis();
      memStartActivation = true;
    }
    elpasedTime = millis() - memStartTimer;
    outTimer = elpasedTime >= this->presetTime; //update timer 's "switch"
    return  outTimer;

  }
}//end endTimer()
//constructor
boolean OnDelayTimer::get_outTimer() {

  return this->outTimer;
}

// pin de sortie 53->35

const int iPIN_H1 = 51; // marche
const int iPIN_H3 = 52; // warning
const int iPIN_H2 = 53; // stop
const int iPIN_outP = 50;
const int iPIN_outYV1 = 49;
const int iPIN_outYV2 = 48;

//pin entrée NO 15->26
const int iPIN_inSelAutoMan = 15;
const int iPIN_inBpm = 16;
const int iPIN_inReset = 17;
const int iPIN_inFlow = 18;
const int iPIN_inHum = 19;

// pin entrée NF 27->34
const int iPIN_inBpa = 27;
const int iPIN_inNivB = 28;

boolean H1, H2, H3, outP, outYV1, outYV2 = 0; //boolean sortie
boolean inSelAutoMan, inBpm, inReset, inFlow, inHum, inBpa, inNivB = 0; //boolean entrée
//variable additionel


// nombre étapes et transistions
const unsigned int nbStepPr = 5;
const unsigned int nbTransition = 5;
boolean stepPr[nbStepPr];
boolean transition[nbTransition];
boolean stepStop[2], transitionStop[2];
boolean stepNivBas[3], transitionNivBas[3];
boolean stepFuite[4], transitionFuite[5];
boolean stepNoDebit[2], transitionNoDebit[2];

//déclaration débug
String strDebugLine;
int stp, stpStop, stpNivBas, stpFuite, stpNoDebit;// étape numéro x dans le debug

// déclaration des flanc montant à getter: PositivEdge nomDeVariable(nomDeVariable à évaluer)
PositivEdge posEdge_inBpm(inBpm);
PositivEdge posEdge_inReset(inReset);

// déclaration timer : OnDelayTimer nomDeVariable(temps en milliseconde);
OnDelayTimer timerPr1(2000);
OnDelayTimer timerPr2(5000);
OnDelayTimer timerPr3(7000);
OnDelayTimer timerPr4(3000);

OnDelayTimer timerNivBas1(3000);
OnDelayTimer timerNivBas2(1000);

OnDelayTimer timerFuite(5000);
OnDelayTimer timerNoDebit(5000);

void setup() {
  Serial.begin(9600); // déclaration moniteur série
  // sortie
  pinMode(iPIN_H1, OUTPUT);
  pinMode(iPIN_H2, OUTPUT);
  pinMode(iPIN_H3, OUTPUT);
  pinMode(iPIN_outP, OUTPUT);
  pinMode(iPIN_outYV1, OUTPUT);
  pinMode(iPIN_outYV2, OUTPUT);

  //entrée
  pinMode(iPIN_inSelAutoMan, INPUT);
  pinMode(iPIN_inBpm, INPUT);
  pinMode(iPIN_inReset, INPUT);
  pinMode(iPIN_inFlow, INPUT);
  pinMode(iPIN_inHum, INPUT);

  pinMode(iPIN_inBpa, INPUT);
  pinMode(iPIN_inNivB, INPUT);

  stepPr[0] = true; // début stepPr
  stepStop[0] = true;
  stepNivBas[0] = true;
  stepFuite[0] = true;
  stepNoDebit[0] = true;
}

void loop() {
  //lecture entrée
  inSelAutoMan = digitalRead (iPIN_inSelAutoMan);
  inBpm = digitalRead (iPIN_inBpm);
  inReset = digitalRead (iPIN_inReset);
  inFlow = digitalRead (iPIN_inFlow);
  inHum = digitalRead (iPIN_inHum);

  inBpa = digitalRead (iPIN_inBpa);
  inNivB = digitalRead (iPIN_inNivB);

  //evaluation flanc montant: posEdge_nomDeVariable.eval(nomDeVariable)
  posEdge_inBpm.eval(inBpm);
  posEdge_inReset.eval(inReset);
  // G7 principal
  // déclaration des transitions
  transition[0] = stepPr[0] && ((inSelAutoMan && posEdge_inBpm.get_out()) || (!inSelAutoMan && inHum)) && inBpa && inNivB && stepFuite[0] && stepNoDebit[0];
  transition[1] = stepPr[1] && timerPr1.get_outTimer();
  transition[2] = stepPr[2] && timerPr2.get_outTimer();
  transition[3] = stepPr[3] && timerPr3.get_outTimer();
  transition[4] = stepPr[4] && timerPr4.get_outTimer();

  // step
  if (transition[0]) {
    stepPr[0] = false;
    stepPr[1] = true;
  }
  if (transition[1]) {
    stepPr[1] = false;
    stepPr[2] = true;
  }
  if (transition[2]) {
    stepPr[2] = false;
    stepPr[3] = true;
  }
  if (transition[3]) {
    stepPr[3] = false;
    stepPr[4] = true;
  }
  if (transition[4]) {
    stepPr[4] = false;
    stepPr[0] = true;
  }

  // g7 stop
  transitionStop[0] = stepStop[0] && !inBpa ;
  transitionStop[1] = stepStop[1] && inBpa;
  // step
  if (transitionStop[0]) {
    stepStop[0] = false;
    stepStop[1] = true;
  }
  if (transitionStop[1]) {
    stepStop[1] = false;
    stepStop[0] = true;
  }

  // g7 niv BAS
  transitionNivBas[0] = stepNivBas[0] && !inNivB && !stepPr[0];
  transitionNivBas[1] = stepNivBas[1] && timerNivBas1.get_outTimer();
  transitionNivBas[2] = stepNivBas[2] && timerNivBas2.get_outTimer();
  // step
  if (transitionNivBas[0]) {
    stepNivBas[0] = false;
    stepNivBas[1] = true;
  }
  if (transitionNivBas[1]) {
    stepNivBas[1] = false;
    stepNivBas[2] = true;
  }
  if (transitionNivBas[2]) {
    stepNivBas[2] = false;
    stepNivBas[0] = true;
  }

  // G7 fuite
  transitionFuite[0] = stepFuite[0] && inFlow && !outYV1 && !outYV2 && timerFuite.get_outTimer();
  transitionFuite[1] = stepFuite[1] && posEdge_inReset.get_out();

  if (transitionFuite[0]) {
    stepFuite[0] = false;
    stepFuite[1] = true;
  }
  if (transitionFuite[1]) {
    stepFuite[1] = false;
    stepFuite[0] = true;
  }

  // G7 NoDebit
  transitionNoDebit[0] = stepNoDebit[0] && !inFlow && (outYV1 || outYV2) && timerNoDebit.get_outTimer() && !stepPr[0];
  transitionNoDebit[1] = stepNoDebit[1] && posEdge_inReset.get_out();

  if (transitionNoDebit[0]) {
    stepNoDebit[0] = false;
    stepNoDebit[1] = true;
  }
  if (transitionNoDebit[1]) {
    stepNoDebit[1] = false;
    stepNoDebit[0] = true;
  }

  //déclaration code

  if (stepStop[1] || stepNivBas[2] || stepNoDebit[1]) {
    stepPr[0] = true;
    for (int i = 1; i < nbStepPr; i++) {
      stepPr[i] = false;
    }
  }


  //sortie activée par Step (sortie = stepPr[x])
  H1 = !stepPr[0];
  H2 = stepPr[0];
  H3 = stepFuite[1] || stepNoDebit[1];
  outP = stepPr[2] || stepPr[3];
  outYV1 = stepPr[1] || stepPr[2];
  outYV2 = stepPr[3] || stepPr[4];

  // timer update (s'active à l'étape x)
  timerPr1.updtTimer(stepPr[1]);
  timerPr2.updtTimer(stepPr[2]);
  timerPr3.updtTimer(stepPr[3]);
  timerPr4.updtTimer(stepPr[4]);

  timerNivBas1.updtTimer(stepNivBas[1]);
  timerNivBas2.updtTimer(stepNivBas[2]);

  timerFuite.updtTimer(inFlow && stepPr[0]);
  timerNoDebit.updtTimer(outYV1 || outYV2);
  //association Sortie-Pin
  digitalWrite (iPIN_H1, H1);
  digitalWrite (iPIN_H2, H2);
  digitalWrite (iPIN_H3, H3);
  digitalWrite (iPIN_outP, outP);
  digitalWrite (iPIN_outYV1, outYV1);
  digitalWrite (iPIN_outYV2, outYV2);

  //debug: étapes active
  for (int i = 0; i < nbStepPr; i++) {
    if (stepPr[i]) {
      stp = i;
      break;
    }
  }
  for (int i = 0; i < 2; i++) {
    if (stepStop[i]) {
      stpStop = i;
      break;
    }
  }
  for (int i = 0; i < 3; i++) {
    if (stepNivBas[i]) {
      stpNivBas = i;
      break;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (stepFuite[i]) {
      stpFuite = i;
      break;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (stepNoDebit[i]) {
      stpNoDebit = i;
      break;
    }
  }

  // sortie debug
  strDebugLine = "stepPr:" + String(stp, DEC) + " stepStop:" + String(stpStop, DEC) + " stepNivBas:" + String(stpNivBas, DEC) + " stepFuite:" + String(stpFuite, DEC) +  " stepNoDebit:" + String(stpNoDebit, DEC) +
                 " inSelAutoMan:" + String(inSelAutoMan, DEC) + " inBpm:" + String(inBpm, DEC) + " inReset:" + String(inReset, DEC) + " inFlow:" + String(inFlow, DEC) + " inHum:" + String(inHum, DEC) +
                 " inBpa:" + String(inBpa, DEC) + " inNivB:" + String(inNivB, DEC) +
                 " H1:" + String(H1, DEC) + " H2:" + String(H2, DEC) + " H3:" + String(H3, DEC) + " outP:" + String(outP, DEC) + " outYV1:" + String(outYV1, DEC) +
                 " outYV2:" + String(outYV2, DEC);
  Serial.println(strDebugLine);
}
