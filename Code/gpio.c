// Emil 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "functions.h"

// The highst pin number in the test. assumes following pin to increment with one. 
#define STARTING_PIN 40

int fd, number_of_gpio_lanes;

int j = 1;
int e = 0;
int test_counter = 1;

char gpio_number_str[99];
char gpio_number_str_chip[99];
char path_direction[256];
char path_value[256];
char path_ngpio[99];
char path_base[256];
char test[8];

char *export = "/sys/class/gpio/export";
char *unexport = "/sys/class/gpio/unexport";

char str[] = "/sys/class/gpio/gpio";
char str_chip[] = "/sys/class/gpio/gpiochip";
char dir[] = "/direction";
char val[] = "/value";
char ngpio[] = "/ngpio";

// Describing gpio configuartion in struct
struct Gpio
{
    int pin;
    int value;
    int succes;
    int test_val;
    char gpio_name[20];
    char io[12];
    char test[10];
};


int main(int argc, char **argv)
{   

    // user must pass gpiochip address as an argument
    if (argc != 2)
    {
        printf("Usage: %s <gpiochip[n]/base>\n", argv[0]);
        return -1;
    }

    char *arg = argv[1]; 

    // converting char to int
    int basenumber = atoi(arg);
    
    // merge strings together to the specific GPIO
    // outputs /sys/class/gpio/gpiochip<n>/ngpio
    snprintf(path_ngpio, sizeof(path_ngpio), "%s%s%s", str_chip, arg, ngpio);

    // returns the value of GPIOs lanes
    int avaliable_gpio = gpio_ngpio(path_ngpio);

    // Defines variables of structure 
    struct Gpio gpio[avaliable_gpio];

    for (int i = 0; i < avaliable_gpio; i++)
    {
        char gpio_name_holder[8];
        char gpio_direction_holder[4];

        // updates strings for every array, and makes paths
        int gpio_number = basenumber + i;
        sprintf(gpio_number_str, "%d", gpio_number);
        snprintf(path_direction, sizeof(path_direction), "%s%s%s", str, gpio_number_str, dir);
        snprintf(path_value, sizeof(path_value), "%s%s%s", str, gpio_number_str, val);
       
        // holds name for current gpio
        snprintf(gpio_name_holder, sizeof(gpio_name_holder), "gpio%s", gpio_number_str);

        // sets gpio[i] structure members
        strcpy(gpio[i].gpio_name, gpio_name_holder);
        gpio[i].pin = STARTING_PIN - i;

        // checks if exported, and if not, export it.
        fd = open(path_direction, O_WRONLY);
        if (fd == -1) {
            gpio_export( export,gpio_number_str);
        }
        else {
            close(fd);
        }

        // This is for splitting the gpio's into two equal parts.
        if ( 0 == i % 2)
        {   
            // For even numbers
            gpio_set_direction(path_direction, "out");
            snprintf(gpio_direction_holder, sizeof(gpio_direction_holder), "%s", gpio_read_direction(path_direction));
            gpio_direction_holder[strcspn(gpio_direction_holder, "\n")] = 0; // removes new line
            strcpy(gpio[i].io, gpio_direction_holder);
            gpio_write(path_value,"0");
        }
        else
        {   
            // For odd numbers
            gpio_set_direction(path_direction, "in");
            snprintf(gpio_direction_holder, sizeof(gpio_direction_holder), "%s", gpio_read_direction(path_direction));
            gpio_direction_holder[strcspn(gpio_direction_holder, "\n")] = 0; // removes new line
            strcpy(gpio[i].io, gpio_direction_holder);
        }
    gpio[i].value = gpio_read( path_value);
    }

    printf("\n\n GPIO test\n");
    printf("***************************************************************************\n");
    printf(" Test:\t   Pins:\tOutput:\t Input:\t\tStatus:\n");
    printf("***************************************************************************\n");
    for (int j = 1; avaliable_gpio > j; j+=2)
    {   
        if (gpio[j-1].value == gpio[j].value ) {
            strcpy(gpio[j].test, "Success");
            gpio[j].succes = 1;
        }
        else {
            strcpy(gpio[j].test, "Failed");
            gpio[j].succes = 0;
        }

        printf(" Test%d: (%d --> %d) \t  %d\t   %d\t\t%s\n", test_counter, gpio[j-1].pin, gpio[j].pin,
         gpio[j-1].value, gpio[j].value, gpio[j].test);
        test_counter++;
    }
    printf("***************************************************************************\n");
    printf(" Switching input and output\n");
    printf("***************************************************************************\n");

 /*
 / Switch GPIO in/out and high and low
*/
    for (int i = 0; i < avaliable_gpio; i++)
    {   
        
        char gpio_name_holder[8];
        char gpio_direction_holder[4];

        // Holds name for current gpio
        snprintf(gpio_name_holder, sizeof(gpio_name_holder), "gpio%s", gpio_number_str);

        // Save name to gpio[i] struct
        strcpy(gpio[i].gpio_name, gpio_name_holder);

        // Sets GPIO pin
        gpio[i].pin = STARTING_PIN - i;

        if (1 == i % 2) // take 'pairs' of two
        {   
            // First set output
            // updates output path
            int gpio_number = basenumber + i;
            sprintf(gpio_number_str, "%d", gpio_number);
            snprintf(path_direction, sizeof(path_direction), "%s%s%s", str, gpio_number_str, dir);
            snprintf(path_value, sizeof(path_value), "%s%s%s", str, gpio_number_str, val);

            //set as output
            gpio_set_direction(path_direction, "out");
            snprintf(gpio_direction_holder, sizeof(gpio_direction_holder), "%s", gpio_read_direction( path_direction));            
            gpio_direction_holder[strcspn(gpio_direction_holder, "\n")] = 0; // removes new line
            strcpy(gpio[i].io, gpio_direction_holder);

            gpio_write(path_value,"1");
            gpio[i].value = gpio_read( path_value);

            // Second set input
            // updates input
            gpio_number = basenumber + i-1;
            sprintf(gpio_number_str, "%d", gpio_number);
            snprintf(path_direction, sizeof(path_direction), "%s%s%s", str, gpio_number_str, dir);
            snprintf(path_value, sizeof(path_value), "%s%s%s", str, gpio_number_str, val);

            // set as input
            gpio_set_direction(path_direction, "in");
            snprintf(gpio_direction_holder, sizeof(gpio_direction_holder), "%s", gpio_read_direction( path_direction));
            
            gpio_direction_holder[strcspn(gpio_direction_holder, "\n")] = 0; // removes new line
            strcpy(gpio[i-1].io, gpio_direction_holder);
            gpio[i-1].value = gpio_read(path_value);
        }
    }

    for (int j = 1; avaliable_gpio > j; j+=2)
    {   
        if (gpio[j-1].value == gpio[j].value) {
            strcpy(gpio[j].test, "Success");
            gpio[j-1].succes = 1;
        }
        else {
            strcpy(gpio[j].test, "Failed");
            gpio[j-1].succes = 0;
        }
        
        printf(" Test%d: (%d --> %d) \t  %d\t   %d\t\t%s\n", test_counter, gpio[j].pin, gpio[j-1].pin,
        gpio[j].value, gpio[j-1].value, gpio[j].test);
        test_counter++;
    }
    printf("***************************************************************************\n");
    printf(" Final Test Result \n");
    printf("***************************************************************************\n");
    for (int j = 1; avaliable_gpio > j; j+=2)
    {
        if ((gpio[j-1].succes && gpio[j].succes) == 1) {
            strcpy(test, "Success");
        }
        else {
            strcpy(test, "Failed");
            e++;
        }
        printf(" Test between (%d <--> %d) \t\t\t%s\n", gpio[j-1].pin, gpio[j].pin, test);
    }
    printf("***************************************************************************\n");
    if (e > 0){
    printf("Number of errors are: %d\n\n", e);
    } else {
    printf("No errors detected\n\n");
    }

    

    //Reverses the effect of exporting to userspace.
    for (int i = 0; i < avaliable_gpio; i++){
        int gpio_number = basenumber + i;
        sprintf(gpio_number_str, "%d", gpio_number);
        gpio_unexport(  unexport, gpio_number_str);
    } 
    return 0;

 }

