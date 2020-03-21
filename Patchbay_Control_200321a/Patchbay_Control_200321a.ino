
#include <SPI.h>

const int w0 = B00000000;   //Memory address for wiper 0
const int w1 = B00010000;   //Memory address for wiper 1
const int tcon = B01000000; //Memory address for tcon register control
const int tcon01 = B11110000; //Shuts down respective pins, effecitively opening the switch
const int tcon10 = B00001111;
const int tcon00 = B00000000;
const int tcon11 = B11111111;


const int outBtn = 11;
const int inBtn = 15;
const int outLed = 21;
const int inLed = 25;

const int statusLed = 20;

int routing[4][4] = {{1, 0, 0, 0},
                     {0, 1, 0, 0},
                     {0, 0, 1, 0},
                     {0, 0, 0, 1}};

int oldRouting[4][4] = {{2, 2, 2, 2},
                        {2, 2, 2, 2},
                        {2, 2, 2, 2},
                        {2, 2, 2, 2}};

int digi50kPins[4][2] = {{31, 33},
                         {35, 37},
                         {39, 41},
                         {43, 45}};

int digi5kPins[4][2] = {{32, 34},
                         {36, 38},
                         {40, 42},
                         {44, 46}};

int gainValues50k[4][4] = {{255, 255, 255, 255},
                           {255, 255, 255, 255},
                           {255, 255, 255, 255},
                           {255, 255, 255, 255}};

int gainValues5k[4][4] = {{152, 152, 152, 152},
                          {152, 152, 152, 152},
                          {152, 152, 152, 152},
                          {152, 152, 152, 152}};

int masterGains[4] = {128, 128, 128, 128};

int timer = 0;

int maxMsg = 0;

int temp = 0;
int maxArray[3] = {0, 0, 0};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();

  //Initialises chip select pins for digi pots
  for(int i = 31; i < 49; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }

  for(int i = 31; i < 47; i += 2)
  {
    digitalPotWrite(i, w0, 255);
    digitalPotWrite(i, w1, 255);
    digitalPotWrite(i, tcon, tcon00);
  }

  for(int i = 32; i < 48; i += 2)
  {
    digitalPotWrite(i, w0, 152);
    digitalPotWrite(i, w1, 152);    
  }

  for(int i = 47; i < 49; i++)
  {
    digitalPotWrite(i, w0, 128);
    digitalPotWrite(i, w1, 128);    
  }

  //Initialises button input pins
  for(int i = outBtn; i < outBtn + 8; i++)
  {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }

  //Initialises LED output pins
  for(int i = outLed; i < outLed + 8; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
}

void loop() {

  for(int i = outLed; i < outLed + 8; i++) //Resets all LEDs to off
    digitalWrite(i, LOW);



  for(int btn = outBtn; btn < outBtn + 4; btn++)  //Scans through each output button to see if it has been pushed
  {
    if(digitalRead(btn))  //If button is pushed
    {
      digitalWrite(btn + 10, HIGH); //Sets the button's LED to light
      for(int j = 0; j < 4; j++)  //Lights the LEDs for any inputs that are linked
        digitalWrite(j + inLed, routing[btn - outBtn][j]);

      timer = 0;  //Resets the timer
      while(digitalRead(btn))  //While the output button is pressed a timer is started
      {
        timer++;
        delay(1);
        if(timer > 1000)  //Once the timer expires the routing for that channel is reset
        {
          digitalWrite(statusLed, HIGH);
          
          for(int resetArray = 0; resetArray < 4; resetArray++) //Resets the routing for this row of the array
              routing[btn - outBtn][resetArray] = 0;
              
          while(digitalRead(btn))  //Any button input button pushed is linked to the output channel
          {
            for(int j = inBtn; j < inBtn + 4; j++)
            {
              if(digitalRead(j))
                routing[btn - outBtn][j - inBtn] = digitalRead(j); //Updates the routing array
                digitalWrite(j + 10, routing[btn - outBtn][j-inBtn]); //Lights the corresponding LED to show the link
            }
          }
        }
      }
      digitalWrite(statusLed, LOW);
    }
  }
  
  for(int btn = inBtn; btn < inBtn + 4; btn++)  //Scans through each output button to see if it has been pushed
  {
    if(digitalRead(btn))  //If button is pushed
    {
      digitalWrite(btn + 10, HIGH); //Sets the button's LED to light
      
      for(int j = 0; j < 4; j++)  //Lights the LEDs for any inputs that are linked
        digitalWrite(j + outLed, routing[j][btn - inBtn]);

      timer = 0;  //Resets the timer
      while(digitalRead(btn))  //While the output button is pressed a timer is started
      {
        timer++;
        delay(1);
        if(timer > 1000)  //Once the timer expires the routing for that channel is reset
        {
          digitalWrite(statusLed, HIGH);
          
          for(int resetArray = 0; resetArray < 4; resetArray++) //Resets the routing for this column of the array
              routing[resetArray][btn - inBtn] = 0;
              
          while(digitalRead(btn))  //Any button output button pushed is linked to the input channel
          {
            for(int j = outBtn; j < outBtn + 4; j++)
            {
              if(digitalRead(j))
                routing[j - outBtn][btn - inBtn] = digitalRead(j); //Updates the routing array
                digitalWrite(j + 10, routing[j - outBtn][btn-inBtn]); //Lights the corresponding LED to show the link
            }
          }
        }
      }
      digitalWrite(statusLed, LOW);
    }
    
  }

//    for(int i = 0; i < 4; i++)
//  {
//    Serial.println();
//    for (int j = 0; j < 4; j++)
//    {
//      Serial.print(routing[i][j]);
//    }
//  }
// digitalPotWrite(31, tcon, tcon11);
//  delay(100);
  
  //This section will read data from the array and set the correct routings by enabling the digipots
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j ++)
    {
      if(oldRouting[i][j] != routing[i][j])
      {
        digitalWrite(statusLed, HIGH);
        
        int chipSelect = 0;
        int k = 0;
        if((j == 0) || (j== 1))
          k = 0;
        else if((j == 2) ||  (j == 3))
          k = 2;

        if(j > 1)
          chipSelect = 1;
        
        if(!routing[i][k] && !routing [i][k+1])
        {
          for(int l = 255; l > 0; l--)
          {
            digitalPotWrite(digi50kPins[i][chipSelect], w0, l);
            digitalPotWrite(digi50kPins[i][chipSelect], w1, l);
            delay(1);
          }
          digitalPotWrite(digi50kPins[i][chipSelect], tcon, tcon00);            

          //Serial.println("00");
        }
        else if(routing[i][k] && routing [i][k+1])
        {
          digitalPotWrite(digi50kPins[i][chipSelect], tcon, tcon11);
          for(int l = 0; l < 256; l++)
          {  
            digitalPotWrite(digi50kPins[i][chipSelect], w0, l);
            digitalPotWrite(digi50kPins[i][chipSelect], w1, l);
            delay(1);
          }
          //Serial.println("11");
        }
        else if(!routing[i][k] && routing [i][k+1])
        {
          for(int l = 255; l > 0; l--)
          {
            digitalPotWrite(digi50kPins[i][chipSelect], w0, l);
            delay(1);
          }          
          digitalPotWrite(digi50kPins[i][chipSelect], tcon, tcon01);
          for(int l = 0; l < 256; l++)
          {
            digitalPotWrite(digi50kPins[i][chipSelect], w1, l);
            delay(1);
          }            
          //Serial.println("01");
        }
        else if(routing[i][k] && !routing [i][k+1])
        {
          for(int l = 255; l > 0; l--)
          {
            digitalPotWrite(digi50kPins[i][chipSelect], w1, l);
            delay(1);
          }          
          digitalPotWrite(digi50kPins[i][chipSelect], tcon, tcon10);
          for(int l = 0; l < 256; l++)
          {
            digitalPotWrite(digi50kPins[i][chipSelect], w0, l);
            delay(1);
          }          
          //Serial.println("10");
        }

        Serial.print(i);
        Serial.print(" ");
        Serial.print(j);
        Serial.print(" ");
        Serial.print(routing[i][j]);
        Serial.println();
        
        oldRouting[i][j] = routing[i][j];

        digitalWrite(statusLed, LOW);
      }
    }
  }
  if(Serial.available())
  {
    maxMsg = Serial.read();


    if(maxMsg == 99)
    {
      routing[maxArray[0]][maxArray[1]] = maxArray[2];
      temp = 0;
    }
    else if(maxMsg == 98)
    {
      gainValues50k[maxArray[0]][maxArray[1]] = maxArray[2];
      for(int i = 0; i < 4; i ++)
      {
        digitalPotWrite(digi50kPins[i][0], w0, gainValues50k[i][0]);
        digitalPotWrite(digi50kPins[i][0], w1, gainValues50k[i][1]);
        digitalPotWrite(digi50kPins[i][1], w0, gainValues50k[i][2]);
        digitalPotWrite(digi50kPins[i][1], w1, gainValues50k[i][3]);
      }
      temp = 0;
    }
    
    else if(maxMsg == 97)
    {
      gainValues5k[maxArray[0]][maxArray[1]] = maxArray[2];
      for(int i = 0; i < 4; i ++)
      {
        digitalPotWrite(digi5kPins[i][0], w0, gainValues5k[i][0]);
        digitalPotWrite(digi5kPins[i][0], w1, gainValues5k[i][1]);
        digitalPotWrite(digi5kPins[i][1], w0, gainValues5k[i][2]);
        digitalPotWrite(digi5kPins[i][1], w1, gainValues5k[i][3]);
      }
      temp = 0;
    }
    else if(maxMsg == 96)
    {
      masterGains[maxArray[0]] = maxArray[1];
      
      digitalPotWrite(47, w0, masterGains[0]);
      digitalPotWrite(47, w1, masterGains[1]);
      digitalPotWrite(48, w0, masterGains[2]);
      digitalPotWrite(48, w1, masterGains[3]);
      temp = 0;
    }
    else
    {
      maxArray[temp] = maxMsg;
      temp++;
    }
    
  }
}

// This function takes care of sending SPI data to the pot.
void digitalPotWrite(int slaveSelectPin, int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);
}
