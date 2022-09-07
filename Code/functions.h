// Emil 

bool gpio_export(char* export, const char* gpio_number_str ){
    int fd = 0;
    fd = open(export, O_WRONLY);
    if (fd == -1) {
        //perror("Unable to open /sys/class/gpio/export");
        return false;
    }
    if (write(fd, gpio_number_str, 3) != 3) {
        //perror("Error writing to /sys/class/gpio/export");
        return false;
    }
    close(fd);
    return true;
}

bool gpio_unexport(char* unexport, char* gpio_number_str ){
    int fd = 0;
    fd = open(unexport, O_WRONLY);
    if (fd == -1) {
        //perror("Unable to open /sys/class/gpio/unexport");
        return false;
    }

    if (write(fd, gpio_number_str, 3) != 3) {
        //perror("Error writing to /sys/class/gpio/unexport");
        return false;
    }
    close(fd);
    return true;
}

int gpio_set_direction(char* path_direction, char* direction){
    int fd = 0;
    fd = open(path_direction, O_WRONLY);
    if (fd == -1) {
        //perror("Unable to open /sys/class/gpio/gpio<n>/direction");
        return false;
    }
    if (write(fd, direction, 3) != 3) {
        //perror("Error writing to /sys/class/gpio/gpio<n>/direction");
        return false;
    }
    close(fd);
    return true; 
}

char *gpio_read_direction(char* path_direction){
    int fd = 0;
    static char gpio_direction_holder[10];
    char* in_out = malloc(5);

    fd = open(path_direction, O_RDONLY);
    if (fd == -1) {
        //perror("Unable to open /sys/class/gpio/gpio<n>/direction");
        return false;
    }
    else{
        read(fd, in_out, 5);
        snprintf(gpio_direction_holder, sizeof(gpio_direction_holder), "%s", in_out);
        //perror("Error writing to /sys/class/gpio/gpio<n>/direction");
    }
    close(fd);
    return gpio_direction_holder; 
}


void gpio_write(char* path_value, char* read_write){
    int fd = 0;
    fd = open(path_value, O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/gpio<n>/value");
        exit(1);
    }
    if (write(fd, read_write, 1) != 1) {
            perror("Error writing to /sys/class/gpio/gpio<n>/value");
            exit(1);
    }
    close(fd);
}

int gpio_read(char* path_value){
    int fd = 0;
    char value_char[2];
    int value_int;

    fd = open(path_value, O_RDONLY);
    if (fd == -1) {
        //perror("Unable to open /sys/class/gpio/gpio<n>/value");
        return false;
    }
    else {
        read(fd, value_char, 2);
        value_int = atoi(value_char); 
    }
    
    close(fd);
    return value_int;
}

void gpio_set_active_low(char* path_value, char* read_write){
    int fd = 0;
    fd = open(path_value, O_WRONLY);
    if (fd == -1) {
        perror("Unable to write to /sys/class/gpio/gpio<n>/active_low");
        exit(1);
    }
    if (write(fd, read_write, 1) != 1) {
            perror("Error writing to /sys/class/gpio/gpio<n>/active_low");
            exit(1);
    }
    close(fd);
}



int gpio_ngpio(char* path_ngpio) {
    int fd = 0;   
    char number_of_gpio_char[8];
    int  number_of_gpio_int;

    fd = open(path_ngpio, O_RDONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/gpio<n>/ngpio");
        exit(1);
    }
    else{
        read(fd, number_of_gpio_char,8);
        number_of_gpio_int = atoi(number_of_gpio_char); 
    }
    close(fd);
    return number_of_gpio_int;
}

void gpio_avaliable_gpio(int number_of_gpio_int,int BASENUMBER){
    printf("GPIO ports avaliable:\n");
    for (int i = 0; i < number_of_gpio_int; i++ ){
        printf("(%d) gpio%d \n",i,BASENUMBER+i);
        }
}

