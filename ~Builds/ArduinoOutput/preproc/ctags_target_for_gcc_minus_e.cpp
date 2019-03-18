# 1 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
# 1 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
# 2 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 2

void setup()
{
    Serial.begin(19200);
    md.init();

    //Initialise Motor Encoder Pins, digitalWrite high to enable PullUp Resistors
    pinMode(3 /*Microcontroller pin 5, PORTD, PCINT2_vect, PCINT19*/, 0x0);
    pinMode(5 /*Microcontroller pin 11, PORTD,PCINT2_vect, PCINT21*/, 0x0);
    pinMode(13 /*Microcontroller pin 17, PORTB, PCINT0_vect, PCINT3*/, 0x0);
    pinMode(11 /*Microcontroller pin 19, PORTB, PCINT0_vect, PCINT5*/, 0x0);

    //Innitializes the Motor Encoders for Interrupts
    pciSetup(3 /*Microcontroller pin 5, PORTD, PCINT2_vect, PCINT19*/);
    pciSetup(5 /*Microcontroller pin 11, PORTD,PCINT2_vect, PCINT21*/);
    pciSetup(13 /*Microcontroller pin 17, PORTB, PCINT0_vect, PCINT3*/);
    pciSetup(11 /*Microcontroller pin 19, PORTB, PCINT0_vect, PCINT5*/);

    memset(&msgRCVD, 0, sizeof(RCVDMessage));
    memset(&msgSEND, 0, sizeof(SENDMessage));

    delay(500);

    /*     while (1)
    {
        usbReceiveMSG(&msgRCVD);
        if (msgRCVD.type == 'A')
        {
            msgSEND.type = 'O';
            usbSendMSG(&msgSEND);
            break;
        }
        msgSEND.type = 'I';
        usbSendMSG(&msgSEND);
    } */

    ;
}

void loop()
{
    if (commands[0] != '0')
    {
        stringCommands(commands, sizeof(commands) / sizeof(char));
    }

    else
    {
        //Wait for start event
        usbReceiveMSG(&msgRCVD); //Process incoming message packet
        char c = char(msgRCVD.type); //Then assign to a variable to control
        ;
        rpiCommunication(c);
        //memset(&msgRCVD, 0, sizeof(RCVDMessage)); //Clear received message
        delay(RPI_DELAY);
    }
}

//------------Functions for robot movements------------//
void mvmtFORWARD()
{
    long lastTime = micros();
    resetMCounters();
    lastError = 0;
    totalErrors = 0;
    lastTicks[0] = 0;
    lastTicks[1] = 0;

    if (setSpdL == 0 && setSpdR == 0)
    {
        setSpdL = MAXSPEED_L;
        setSpdR = MAXSPEED_R;
    }

    md.setSpeeds(setSpdR, setSpdL);
    lastTime = millis();

    //while (mCounter[0] < distance && mCounter[1] < distance)
    //{
    if (millis() - lastTime > 80)
    {
        PIDControl(&setSpdR, &setSpdL, 350, 200, 180, 0); //By block 40, 0, 80, 0
        lastTime = millis();
        md.setSpeeds(setSpdR, setSpdL);
    }
    //}
}

void mvmtRIGHTStatic(int angle)
{
    int setSpdRi = -MAXSPEED_R; //Right motor
    int setSpdLi = MAXSPEED_L; //Left motor
    long lastTime = millis();
    lastError = 0;
    totalErrors = 0;
    lastTicks[0] = 0;
    lastTicks[1] = 0;
    resetMCounters();

    md.setSpeeds(setSpdRi, setSpdLi);
    delay(50);

    while (mCounter[0] < angle && mCounter[1] < angle)
    {
        if (millis() - lastTime > 100)
        {
            PIDControl(&setSpdRi, &setSpdLi, 150, 6, 15, 1);
            lastTime = millis();
            md.setSpeeds(setSpdRi, setSpdLi);
        }
    }

    md.setBrakes(400, 400);
}

void mvmtLEFTStatic(int angle)
{
    int setSpdRi = MAXSPEED_R; //Right motor
    int setSpdLi = -MAXSPEED_L; //Left motor
    long lastTime = millis();
    lastError = 0;
    totalErrors = 0;
    lastTicks[0] = 0;
    lastTicks[1] = 0;
    resetMCounters();

    md.setSpeeds(setSpdRi, setSpdLi);
    delay(50);

    while (mCounter[0] < angle && mCounter[1] < angle)
    {
        if (millis() - lastTime > 100)
        {
            PIDControl(&setSpdRi, &setSpdLi, 150, 6, 15, -1);
            lastTime = millis();
            md.setSpeeds(setSpdRi, setSpdLi);
        }
    }

    md.setBrakes(400, 400);
}

void mvmtSTOP()
{
    setSpdL = 0;
    setSpdR = 0;
    resetMCounters();
    md.setBrakes(400, 400);
    //md.setM2Brake(400);
    delay(30);
    //md.setM1Brake(400);
    //delay(2000);
}

void mvmtRIGHT(int angle)
{
    md.setM1Brake(400);
    resetMCounters();
    while (mCounter[1] < (3012 + turnOffset))
    {
        md.setM2Speed(setSpdL);
    }
}

void mvmtLEFT(int angle)
{
    md.setM2Brake(400);
    resetMCounters();
    while (mCounter[0] < (3012 + turnOffset))
    {
        md.setM1Speed(setSpdR);
    }
}

//Direction(dr): -1 = left, 0 = straight, 1 = right
void PIDControl(int *setSpdR, int *setSpdL, int kP, int kI, int kD, int dr)
{
    int adjustment;
    int error = (mCounter[1] - lastTicks[1]) - (mCounter[0] - lastTicks[0]); //0 = right motor, 1 = left motor, lesser tick time mean faster
    int errorRate = error - lastError;
    lastError = error;
    lastTicks[0] = mCounter[0];
    lastTicks[1] = mCounter[1];
    //totalErrors += 2;
    //totalErrors = 0;
    //   totalErrors += error             ;                                           //Add up total number of errors (for Ki)
    //  if (error != 0) {                                                           //if error exists
    adjustment = ((kP * error) - (kI * totalErrors) + (kD * errorRate)) / 100;

    if (dr == 1 || dr == -1)
    {
        *setSpdR += -adjustment * dr;
        *setSpdL -= adjustment * dr;
    }
    else
    {
        *setSpdR += adjustment;
        *setSpdL -= adjustment;

        if (*setSpdR > 400)
        {
            *setSpdR = 400;
        }
        if (*setSpdL > 400)
        {
            *setSpdL = 400;
        }
    }
}

int angleToTicks(long angle)
{
    if (angle == 90)
        return (16800 * angle / 1000) + turnOffsetStatic;
    else
        return (17280 * angle / 1000);
}

int blockToTicks(int blocks)
{
    return 1192 * blocks; //1192 * blocks;
}

void spdAdjustment()
{
    int speedRate = msgRCVD.motorspeed;
    setSpdL = ((MAXSPEED_L * speedRate) / 100);
    setSpdR = ((MAXSPEED_R * speedRate) / 100);
}

//------------Functions for IR Sensors------------//
int scanFORWARD()
{
    int val = mfwdIrVal.distance(); // Middle
    delay(2);
    ;
    return val;
}

//------------Functions for Motors------------//
void resetMCounters()
{
    mCounter[0] = 0;
    mCounter[1] = 0;
}

//ISR for Motor 1 (Right) Encoders

# 249 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
extern "C" void __vector_5 /* Pin Change Interrupt Request 1 */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_5 /* Pin Change Interrupt Request 1 */ (void)

# 250 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
{
    mCounter[0]++;
}

//ISR for Motor 2 (Left) Encoders

# 255 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
extern "C" void __vector_3 /* Pin Change Interrupt Request 0 */ (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_3 /* Pin Change Interrupt Request 0 */ (void)

# 256 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
{
    mCounter[1]++;
}

//Standard function to enable interrupts on any pins
void pciSetup(byte pin)
{
    *(((pin) <= 7) ? (&
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
    (*(volatile uint8_t *)(0x6D))
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
    ) : (((pin) <= 13) ? (&
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
    (*(volatile uint8_t *)(0x6B))
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
    ) : (((pin) <= 21) ? (&
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
    (*(volatile uint8_t *)(0x6C))
# 263 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
    ) : ((uint8_t *)0)))) |= (1UL << ((((pin) <= 7) ? (pin) : (((pin) <= 13) ? ((pin) - 8) : ((pin) - 14))))); // enable pin
    
# 264 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
   (*(volatile uint8_t *)((0x1B) + 0x20)) 
# 264 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
         |= (1UL << ((((pin) <= 7) ? 2 : (((pin) <= 13) ? 0 : 1)))); // clear any outstanding interrupt
    
# 265 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino" 3
   (*(volatile uint8_t *)(0x68)) 
# 265 "c:\\Users\\Renzey\\Workspaces\\-NTU_FYP_Project_Files\\~Builds\\fyp_ard_fw_v04\\fyp_ard_fw_v04.ino"
         |= (1UL << ((((pin) <= 7) ? 2 : (((pin) <= 13) ? 0 : 1)))); // enable interrupt for the group
}

//------------Functions for communications------------//

void usbReceiveMSG(RCVDMessage *MSG_Buffer)
{
    static uint8_t tempBuffer[MAX_BYTE_DATA];
    static uint8_t tempByte = 0;
    static int index = 0;
    static boolean recieving = false;

    while (Serial.available() > 0) //Total index + STOP byte
    {
        tempByte = Serial.read();
        ;

        if (recieving == true)
        {
            if (tempByte != STOP)
            {
                tempBuffer[index] = tempByte;
                index++;
            }

            else
            {
                recieving = false;
                index = 0;
                ;
                MSG_Buffer->type = tempBuffer[0];
                MSG_Buffer->id = tempBuffer[1];
                MSG_Buffer->distance = ((uint16_t)tempBuffer[2] << 7) | tempBuffer[3];
                MSG_Buffer->motorspeed = ((uint16_t)tempBuffer[4] << 7) | tempBuffer[5];
                MSG_Buffer->motorangle = ((uint16_t)tempBuffer[6] << 7) | tempBuffer[7];
                tempByte = 0;
                memset(tempBuffer, 0, sizeof(tempBuffer));
            }
        }

        else if (tempByte == START)
        {
            recieving = true;
        }

        delay(5);
    }
}

void usbSendMSG(SENDMessage *MSG_Buffer)
{
    byte writebuff[] = {START, MSG_Buffer->type, MSG_Buffer->id, MSG_Buffer->state, ((uint8_t)((MSG_Buffer->frontDistance) >> 7)), ((uint8_t)((MSG_Buffer->frontDistance)&0x7f)), ((uint8_t)((MSG_Buffer->bearings) >> 7)), ((uint8_t)((MSG_Buffer->bearings)&0x7f)), STOP};
    Serial.write(writebuff, 9);
    //Serial << char(START) << char(MSG_Buffer->type) << char(MSG_Buffer->id) << char(MSG_Buffer->state) << char(STOP);
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
    ;
}

void rpiCommunication(char control)
{
    int irfront = scanFORWARD();
    ;

    switch (control)
    {
    case 'F':
        if (irfront > 90)
        {
            mvmtFORWARD();
            ;
            if (msgSEND.type != 'F')
            {
                msgSEND.type = 'F';
                msgSEND.state = 'W';
                msgSEND.frontDistance = irfront;
                msgSEND.id++;
                usbSendMSG(&msgSEND);
            }
        }
        else
        {
            if (msgSEND.type != 'E')
            {
                mvmtSTOP();
                msgSEND.type = 'E';
                msgSEND.state = 'S';
                msgSEND.frontDistance = irfront;
                msgSEND.id++;
                usbSendMSG(&msgSEND);
            }
        }

        break;

    case 'L':
        if (msgSEND.type != 'L')
        {
            msgSEND.type = 'L';
            msgSEND.state = 'T';
            msgSEND.id++;
            usbSendMSG(&msgSEND);
        }
        if (setSpdL == 0 && setSpdR == 0)
        {
            mvmtLEFTStatic(angleToTicks(90));
        }
        else
        {
            mvmtLEFT(angleToTicks(90));
        }
        if (msgSEND.type != 'F')
        {
            msgSEND.type = 'F';
            msgSEND.state = 'F';
            msgSEND.id++;
            usbSendMSG(&msgSEND);
        }
        break;

    case 'R':
        if (msgSEND.type != 'R')
        {
            msgSEND.type = 'R';
            msgSEND.state = 'T';
            msgSEND.id++;
            usbSendMSG(&msgSEND);
        }
        if (setSpdL == 0 && setSpdR == 0)
        {
            mvmtRIGHTStatic(angleToTicks(90));
        }
        else
        {
            mvmtRIGHT(angleToTicks(90));
        }
        if (msgSEND.type != 'F')
        {
            msgSEND.type = 'F';
            msgSEND.state = 'F';
            msgSEND.id++;
            usbSendMSG(&msgSEND);
        }
        break;

    case 'E':

    case 'S':
        mvmtSTOP();
        ;
        if (msgSEND.type != 'S')
        {
            msgSEND.type = 'S';
            msgSEND.state = 'S';
            msgSEND.id++;
            usbSendMSG(&msgSEND);
        }
        break;
    }
}

void stringCommands(char commands[], int len)
{
    static int calCounter = 0;
    static int x;

    switch (commands[x])
    {
    case 'f':
        if (setSpdL == 0 && setSpdR == 0)
        {
            setSpdL = MAXSPEED_L;
            setSpdR = MAXSPEED_R;
        }
        mvmtFORWARD();
        break;

    case 'l':
        if (setSpdL == 0 && setSpdR == 0)
        {
            mvmtLEFTStatic(angleToTicks(90));
        }
        else
        {
            mvmtLEFT(angleToTicks(90));
        }
        break;

    case 'r':
        if (setSpdL == 0 && setSpdR == 0)
        {
            mvmtRIGHTStatic(angleToTicks(90));
        }
        else
        {
            mvmtRIGHT(angleToTicks(90));
        }
        break;

    case 's':
        mvmtSTOP();
        break;

    case 'i':
        while (1)
        {
            scanFORWARD();
        }
        break;

    default:
        mvmtSTOP();
        break;
    }

    delay(ARD_DELAY);

    if (x <= len)
    {
        x++;
    }
}
