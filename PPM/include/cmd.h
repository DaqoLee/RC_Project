struct lookupVals
{
#ifdef AVR
    const char * PROGMEM name_pp;
#else
    const char * name_pp;
#endif
    int val;
    __FlashStringHelper * getNamePtr(void) const
    {
        auto nameInProgmem = &(name_pp);
        __FlashStringHelper * fshName
            = (__FlashStringHelper *)pgm_read_ptr(nameInProgmem);
        return fshName;
    }
    String getName(void) const
    {
        auto fshName = getNamePtr();
        return String(fshName);
    };
    int getValue(void) const
    {
        auto valInProgmem = &(val);
        return pgm_read_word(valInProgmem);
    }
};

////////////////////////////////////////////////////////////////////////////////
int lookup(const char * aName, const lookupVals entries[])
{
    int i = 0;
    String name(aName);
    //Serial << F("looking up \"") << name << F("\"") << endl;
    for (; entries[i].getNamePtr(); i++)
    {
        //Serial << F(" i = ") << i
        //       << F(" comparing against ") << entries[i].getName() << endl;
        if (name.equalsIgnoreCase(entries[i].getName())) {
            auto aVal = entries[i].getValue();
            //Serial << F("found entry: ") << aVal << endl;
            return aVal;
            break;
        }
    }
    auto v = entries[0].getValue();
    //Serial << "giving up; using" << v << endl;
    return v;
}

static const char on_str[] PROGMEM = "on";
static const char off_str[] PROGMEM = "off";
static const char one_str[] PROGMEM = "1";
static const char zero_str[] PROGMEM = "0";
static const lookupVals switchStates[] PROGMEM =
{
    {on_str, 1},
    {off_str, 0},
    {one_str, 1},
    {zero_str, 0}
};


int showID(int /*argc*/ = 0, char** /*argv*/ = NULL)
{
    shell.println(F( "Running " __FILE__ ",\nBuilt " __DATE__));
    return 0;
};

int handWrite(int argc, char **argv){
  if (argc != 2){
    shell.println("bad argument count");
    return -1;
  }

  int microSeconds = atoi(argv[1]);

  if(microSeconds < 1000 || microSeconds > 2000){
    shell.println("bad microSeconds");
    return -1;
  }

  Hand.writeMicroseconds(microSeconds);
  shell.print("Setting Hand to ");
  shell.print(microSeconds);
  shell.println("us");

  return EXIT_SUCCESS;
}

int getClothes(int argc, char **argv){
  if (argc != 1){
    shell.println("bad argument count");
    return -1;
  }

  bool clothes = digitalRead(CLOTHES_PIN);
  shell.print("Clothes is ");
  shell.println(clothes);

  return EXIT_SUCCESS;
}

int armMove(int argc, char **argv){
  if (argc != 2){
    shell.println("bad argument count");
    return -1;
  }

  int speed = atoi(argv[1]);
  shell.print("Moving arm at ");
  shell.print(speed);
  shell.println("step/s");

  driver.VACTUAL(speed);

  return EXIT_SUCCESS;
}

int stallGuardSwitch(int argc, char **argv){
  if (argc != 2){
    shell.println("bad argument count");
    return -1;
  }

  int on = lookup(argv[1], switchStates);
  shell.print("StallGuard is ");
  shell.println(on);

  if (on){
    attachInterrupt(DIAG_PIN, stallGuard, RISING);
  }else{
    detachInterrupt(DIAG_PIN);
  }

  return EXIT_SUCCESS;
}

int setSGTHRS(int argc, char **argv){
  if (argc != 2){
    shell.println("bad argument count");
    return -1;
  }

  int threshold = atoi(argv[1]);
  if(threshold < 0 || threshold > 255){
    shell.println("bad threshold");
    return -1;
  }

  shell.print("Setting StallGuard threshold to ");
  shell.println(threshold);

  driver.SGTHRS(threshold);

  return EXIT_SUCCESS;
}

int run(int argc, char **argv){
  if (argc != 1){
    shell.println("bad argument count");
    return -1;
  }

  detachInterrupt(DIAG_PIN);
  driver.VACTUAL(5000);

  long start = millis();

  while(digitalRead(CLOTHES_PIN) == HIGH && millis() - start < 13000){
    delay(100);
  }

  long end = millis();
  if (abs(end - start) > 13000){
    driver.VACTUAL(0);
    shell.println("Clothes not detected");
    return -1;
  }
  
  driver.VACTUAL(0);
  shell.println("Clothes detected");
  Hand.writeMicroseconds(1000);
  delay(500);

  driver.VACTUAL(-5000);
  delay(2000);
  driver.VACTUAL(0);

  return EXIT_SUCCESS;
}

int release(int argc, char **argv){
  if (argc != 1){
    shell.println("bad argument count");
    return -1;
  }

  Hand.writeMicroseconds(2000);
  delay(500);
  shell.println("Hand released");

  attachInterrupt(DIAG_PIN, stallGuard, RISING);
  driver.VACTUAL(-5000);
  shell.println("Arm back");

  return EXIT_SUCCESS;
}

volatile bool findzeroFlag = false;

int findZeroCMD(int argc, char **argv){
  if (argc != 1){
    shell.println("bad argument count");
    return -1;
  }

  findzeroFlag = true;
  shell.println("Zero finding");

  return EXIT_SUCCESS;
}