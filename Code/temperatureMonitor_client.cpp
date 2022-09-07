// has been solve with the usages of threading.
//
// For usage please be aware that the  port number and IP must match the server.
//
//
// Reference:
// https://riptutorial.com/cplusplus/example/24000/hello-tcp-client
////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <iostream>
#include <string>

#include <fstream>

#include <sstream>

/* used for socket */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* used for threads */
#include <thread>
#include <pthread.h>

/* PS main sensor - Measures the LPD near the APU measured by the PS SYSMON unit. */
#define PATH_PS "/sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_raw"

/* PL sensor - Measures the PL near the PL SYSMON unit measured by the PL SYSMON unit. */
#define PATH_PL "/sys/bus/iio/devices/iio:device0/in_temp2_pl_temp_raw"

/* Remote remote sensor - Measures the FPD near the RPU measured by the PS SYSMON unit. */
#define PATH_REMOTE "/sys/bus/iio/devices/iio:device0/in_temp1_remote_temp_raw"

float temperature(const char* path){
                std::ostringstream adc_val;
                std::ifstream myFile_Handler;
                std::string myLine;

                myFile_Handler.open(path);
                if (myFile_Handler.is_open())
                {
                        while (getline(myFile_Handler, myLine))
                        {
                                adc_val << myLine;
                        }
                        myFile_Handler.close();
                }
                else
                {
                        std::cout << "Unable to open the file!";
                }
                float temp = std::stof(adc_val.str());
                return ((temp * 509.3140064)/65536.0)-280.23;
 }
/* Function for sending message */
void send_msg(int sockFD)
{
        while (1)
        {  
                std::ostringstream sensorPL, sensorPS, sensorREMOTE;
                float temperaturePL, temperaturePS, temperatureREMOTE; // temp_ps, temp_remote;

                using namespace std::chrono_literals;

                temperaturePL = temperature(PATH_PL);
                temperaturePS = temperature(PATH_PS);
                temperatureREMOTE = temperature(PATH_REMOTE);

                sensorPL << temperaturePL << " ";
                sensorPS << temperaturePS << " ";
                sensorREMOTE << temperatureREMOTE;

                std::string send_msg = sensorPL.str() + sensorPS.str() + sensorREMOTE.str();
                send(sockFD, send_msg.data(), send_msg.length(), 0);
                sleep(1);
        }
}

int main(int argc, char *argv[])
{

        /* Now we're taking an ipaddress and a port number as arguments to our program */
        if (argc != 1)
        {
                std::cerr << "Run program with no arguments\n";
                return -1;
        }

        /* IP and Port - must match server! */
        auto &ipAddress = "128.141.94.197";
        auto &portNum = "65431";
        std::cout << "Trying to connect to server.\nIP: " << ipAddress << "\nPORT: " << portNum << std::endl;
        std::cout << "Waiting for connection to be etablished.. " << std::endl;
        addrinfo hints, *p;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int gAddRes = getaddrinfo(ipAddress, portNum, &hints, &p);
        if (gAddRes != 0)
        {
                std::cerr << gai_strerror(gAddRes) << "\n";
                return -2;
        }

        if (p == NULL)
        {
                std::cerr << "No addresses found\n";
                return -3;
        }

        /* socket() call creates a new socket and returns it's descriptor */
        int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockFD == -1)
        {
                std::cerr << "Error while creating socket\n";
                return -4;
        }

        /* connect() call tries to establish a TCP connection to the specified server */
        int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
        if (connectR == -1)
        {
                close(sockFD);
                std::cerr << "Error while connecting socket\n";
                return -5;
        }
        else
        {

                std::cout << "Connected\n";
        }

        /* Thread created and started */
        std::thread t1(send_msg, sockFD);

        while (1)
        {
                std::string message;

                /* while loop for recieving data */
                while (1)
                {       
                        
                        std::string reply(15, ' ');
                        auto bytes_recv = recv(sockFD, &reply.front(), reply.size(), 0);

                        /* join 'chuncks' of data in message */
                        message = message.append(reply);

                        /* checks for an error - could happen when connection is abrupt */
                        if (bytes_recv == -1)
                        {
                                std::cerr << "Error while receiving bytes\n";
                                return -6;
                        }

                        if (bytes_recv == 0)
                        {
                                std::cerr << "Error no bytes recieved.\n";
                                return -7;
                        }

                        /* no more data -  exit while loop 
                        if (bytes_recv < 15)*/
                        {       
                                std::cerr << "no bytes recieved.\n";
                                break;
                        } 
                        
                }

                /* When while loop finish - prints message to user */
                std::cout << "Server: " << message << std::endl;
        }

        /* Close and free socket */
        close(sockFD);
        freeaddrinfo(p);
        return 0;
}
