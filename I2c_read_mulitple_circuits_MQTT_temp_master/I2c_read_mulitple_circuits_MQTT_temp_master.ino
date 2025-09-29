//************************************************************************************************************
//                                            Include declerations
//************************************************************************************************************

// These declerations are for the Ethernet Library Using PubSubClient 

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h> //include PubSubClient for mqtt communication https://github.com/knolleary/pubsubclient

// These declerations are for the EZO sensors 

#include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
#include <Wire.h>    //include arduinos i2c library
#include <sequencer2.h> //imports a 2 function sequencer 
#include <Ezo_i2c_util.h> //brings in common print statements

//------------------------------------------------------------------------------------------------------------

//************************************************************************************************************
//                                            Internet IP's
//************************************************************************************************************

// This specifies the IP of the Arduino, Raspberry Pi, and the MAC addr for the PLC Eth Controller

byte mac[]    = {0x60, 0x52, 0xD0, 0x06, 0xF0, 0x3D}; // MAC addr for the Eth PLC Controller
IPAddress ip(192, 168, 1, 100);                        // IP addr of the Arduino
IPAddress server(192, 168, 1, 200);                    // IP addr of the Raspberry Pi

//------------------------------------------------------------------------------------------------------------

//************************************************************************************************************
//                     Defining the address for each board for their respective tank
//        The I2C address of the board can be changed using the datasheet for the atlas sensors on page 39
//           For example on the pH EZO sensor https://files.atlas-scientific.com/pH_EZO_Datasheet.pdf
//************************************************************************************************************

// I2C address can be from 1-127 (BIN number NOT HEX) 
// pH --> PH  |  Electrical Conductivity --> EC  | Dissolved Oxygen --> DO  | Temperature --> T
// Naming Convention (Variable_Tank) Ex: pH_1 --> Potential for Hydrogen in Tank 1

// TANK 1

Ezo_board PH_1 = Ezo_board(99, "PH_1");       //create a PH circuit object, who's address is 99 and name is "PH_1"
Ezo_board EC_1 = Ezo_board(100, "EC_1");      //create an EC circuit object who's address is 100 and name is "EC_1"
Ezo_board DO_1 = Ezo_board(101, "DO_1");      //create a PH circuit object, who's address is 101 and name is "DO_1"
Ezo_board T_1 = Ezo_board(102, "T_1");        //create an EC circuit object who's address is 102 and name is "T_1"

// TANK 2

Ezo_board PH_2 = Ezo_board(103, "PH_2");      //create a PH circuit object, who's address is 103 and name is "PH_2"
Ezo_board EC_2 = Ezo_board(104, "EC_2");      //create an EC circuit object who's address is 104 and name is "EC_2"
Ezo_board DO_2 = Ezo_board(105, "DO_2");      //create a PH circuit object, who's address is 105 and name is "DO_2"
Ezo_board T_2 = Ezo_board(106, "T_2");        //create an EC circuit object who's address is 106 and name is "T_2"

// TANK 3

Ezo_board PH_3 = Ezo_board(107, "PH_3");      //create a PH circuit object, who's address is 107 and name is "PH_3"
Ezo_board EC_3 = Ezo_board(108, "EC_3");      //create an EC circuit object who's address is 108 and name is "EC_3"
Ezo_board DO_3 = Ezo_board(109, "DO_3");      //create a PH circuit object, who's address is 109 and name is "DO_3"
Ezo_board T_3 = Ezo_board(110, "T_3");        //create an EC circuit object who's address is 110 and name is "T_3"


//------------------------------------------------------------------------------------------------------------

//************************************************************************************************************
//                         Declaring Functions and passing them to the Sequencer 
// More information on the sequencer can be found at https://github.com/Atlas-Scientific/Ezo_I2c_lib
//************************************************************************************************************

void step1();  //forward declarations of functions to use them in the sequencer before defining them
void step2();

Sequencer2 Seq(&step1, 1000, &step2, 0);  //calls the steps in sequence with time in between them

//------------------------------------------------------------------------------------------------------------


//************************************************************************************************************
//                                               The Setup 
//************************************************************************************************************

EthernetClient ethClient;                 //make an Eth Client
PubSubClient client(ethClient);           //pass the Eth Cleint to be the MQTT client

void setup() {
  Wire.begin();                           //start I2C
  Serial.begin(9600);                     //start Serial Comm. 9600 Should be fine but 115200 could be used?

  client.setServer(server, 1883);         //set the Raspberry Pi Server
  client.setCallback(callback);           //set the Callback fucntion

  Ethernet.begin(mac, ip);                // Allow the hardware to sort itself out
  delay(1500);
  
  Seq.reset();                            //initialize the sequencer
}

//------------------------------------------------------------------------------------------------------------


//************************************************************************************************************
//                                               The Loop
//************************************************************************************************************

void loop() {
  
  if (!client.connected()) {
  reconnect();
  }
  
  Seq.run();                              //run the sequncer 

  client.publish("PH_1", String(PH_1.get_last_received_reading(), 2).c_str());
  client.publish("EC_1", String(EC_1.get_last_received_reading(), 2).c_str());
  client.publish("DO_1", String(DO_1.get_last_received_reading(), 2).c_str());
  client.publish("T_1", String(T_1.get_last_received_reading(), 2).c_str());

  client.publish("PH_2", String(PH_2.get_last_received_reading(), 2).c_str());
  client.publish("EC_2", String(EC_2.get_last_received_reading(), 2).c_str());
  client.publish("DO_2", String(DO_2.get_last_received_reading(), 2).c_str());
  client.publish("T_2", String(T_2.get_last_received_reading(), 2).c_str());

  client.publish("PH_3", String(PH_3.get_last_received_reading(), 2).c_str());
  client.publish("EC_3", String(EC_3.get_last_received_reading(), 2).c_str());
  client.publish("DO_3", String(DO_3.get_last_received_reading(), 2).c_str());
  client.publish("T_3", String(T_3.get_last_received_reading(), 2).c_str());
  
  delay(1000);
  client.loop();
}                                         //1000ms interval from command to 0ms to another command
                                          //Sequencer2 Seq(&step1, 1000, &step2, 0)

//------------------------------------------------------------------------------------------------------------
                                          

//************************************************************************************************************
//                                            Defining Step1
//************************************************************************************************************

void step1(){
  
  //send a read command. we use this command instead of PH.send_cmd("R"); 
  //to let the library know to parse the reading

  // TANK 1 
  
  T_1.send_read_cmd();
  PH_1.send_read_with_temp_comp(T_1.get_last_received_reading());                      
  EC_1.send_read_with_temp_comp(T_1.get_last_received_reading());
  DO_1.send_read_with_temp_comp(T_1.get_last_received_reading());                      

    
  // TANK 2 
  
  T_2.send_read_cmd();
  PH_2.send_read_with_temp_comp(T_2.get_last_received_reading());                      
  EC_2.send_read_with_temp_comp(T_2.get_last_received_reading());
  DO_2.send_read_with_temp_comp(T_2.get_last_received_reading());                      
  

  // TANK 3 
  
  T_3.send_read_cmd();
  PH_3.send_read_with_temp_comp(T_3.get_last_received_reading());                      
  EC_3.send_read_with_temp_comp(T_3.get_last_received_reading());
  DO_3.send_read_with_temp_comp(T_3.get_last_received_reading());                      
  
}

//------------------------------------------------------------------------------------------------------------


//************************************************************************************************************
//                                            Defining Step2
//************************************************************************************************************

void step2(){

  // Recieving the Data
  // This data will later be formatted into a specific format to pass through a python script to collect into a csv
  // Code is not finished because format is unclear. 

  // Possible Formats --> Time stamp is PARAMOUNT 
  
  // Tank 1 | Tank 2 | Tank 3    OR    Tank 1  Ph EC DO T   OR    Tank 1       Tank 2         Tank 3
  //   pH       pH      pH       |     Tank 2  Ph EC DO T   |  pH EC DO T    pH EC DO T      pH EC DO T
  //   EC       EC      EC       |     Tank 3  Ph EC DO T   |  pH EC DO T    pH EC DO T      pH EC DO T
  //   DO       DO      DO       |                          |  pH EC DO T    pH EC DO T      pH EC DO T
  //    T        T       T       |                          |  pH EC DO T    pH EC DO T      pH EC DO T

  // I really like option 3 as its easy to manipulate if we populate headers for each coulumn 
  // This will make it convinient for plotting libraries to generate graphs. <-- Agustin :)

  // TANK 1 
  receive_and_print_reading(PH_1);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(EC_1);             //get the reading from the EC circuit
  Serial.print("  ");
  receive_and_print_reading(DO_1);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(T_1);             //get the reading from the EC circuit
  Serial.println();

   // TANK 2 
  receive_and_print_reading(PH_2);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(EC_2);             //get the reading from the EC circuit
  Serial.print("  ");
  receive_and_print_reading(DO_2);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(T_2);             //get the reading from the EC circuit
  Serial.println();

  // TANK 3
  receive_and_print_reading(PH_3);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(EC_3);             //get the reading from the EC circuit
  Serial.print("  ");
  receive_and_print_reading(DO_3);             //get the reading from the PH circuit
  Serial.print("  ");
  receive_and_print_reading(T_3);             //get the reading from the EC circuit
  Serial.println();
}

//------------------------------------------------------------------------------------------------------------

//************************************************************************************************************
//                                        MQTT Fucntions for Transmission
//************************************************************************************************************

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------------------------------------------------------------------------------------------------------
