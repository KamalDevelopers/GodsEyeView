#ifndef ITOA_HPP
#define ITOA_HPP

static int digit_count(int num)
{
    int count = 0;
    if(num == 0)
        return 1;

    while(num > 0){
        count++;
        num = num/10;
    }
    return count;
}

static void itoa(int num, char *number)
{
    int dgcount = digit_count(num);
    int index = dgcount - 1;
    char x;
    if(num == 0 && dgcount == 1){
        number[0] = '0';
        number[1] = '\0';
    }else{
        while(num != 0){
            x = num % 10;
            number[index] = x + '0';
            index--;
            num = num / 10;
        }
        number[dgcount] = '\0';
    }
}
#endif